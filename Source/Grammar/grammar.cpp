#include<iostream>
#include<fstream>
#include<sstream>
#include<iomanip>
#include<string>
#include<vector>
#include<map>
#include<queue>
#include"grammar.hpp"
#include"../Error/error.hpp"

int main()
{
	try
	{
		const std::string sourceGrammar{ "OriginalGrammar.txt" };
		std::ifstream ifs{ sourceGrammar };
		if (!ifs) {
			Error::ErrPrint(std::cerr, Error::Level::ERROR, Error::Type::INFILE, sourceGrammar);
			return 1;
		}
		const std::string destinationGrammar{ "BacktrackFreeGrammar.txt" };
		std::ofstream ofs{ destinationGrammar };
		if (!ofs) {
			Error::ErrPrint(std::cerr, Error::Level::ERROR, Error::Type::OUTFILE, destinationGrammar);
			return 1;
		}

		CFG::BNFGrammar grammar;
		ifs >> grammar;
		if (ifs || !ifs && !ifs.eof()) {
			Error::ErrPrint(std::cerr, Error::Level::ERROR, "File data read error");
			return 1;
		}
		grammar.TransformGrammar(std::cerr);
		ofs << grammar;

		std::cout << std::endl << "Success!" << std::endl;
		return 0;
	}
	catch (const Error::RuntimeError& e)
	{
		Error::ErrPrint(std::cerr, Error::Level::EXCEPTION, Error::Type::RUNTIME, e.what());
		return -1;
	}
	catch (const std::exception& e)
	{
		Error::ErrPrint(std::cerr, Error::Level::EXCEPTION, Error::Type::STD, e.what());
		return -2;
	}
}

namespace CFG
{
	BNFGrammar* Production::gram = nullptr;
	std::string BNFGrammar::prefNT;
	std::string BNFGrammar::prefDer;
	std::string BNFGrammar::suffNT;
	std::string BNFGrammar::lexES;
	std::string BNFGrammar::lexEOF;
	std::string BNFGrammar::comment;

	std::string BNFGrammar::GetLexemeString(const std::vector<LexemeID>& set) const
	{
		std::ostringstream oss;
		for (int i = 0; i < set.size(); ++i) {
			if (i > 0) {
				oss << ' ';
			}
			oss << lex[set[i]];
		}
		return oss.str();
	}

	void BNFGrammar::Print(std::ostream& os, const std::vector<LexemeID>& set) const
	{
		for (int i = 0; i < set.size(); ++i) {
			if (i > 0) {
				os << ' ';
			}
			os << lex[set[i]];
		}
	}

	void BNFGrammar::PrintFirstSet(std::ostream& os, LexemeID id) const
	{
		os << "FIRST(" << lex[id] << "): ";
		Print(os, firstSet[id]);
		os << std::endl;
	}

	void BNFGrammar::PrintFollowSet(std::ostream& os, LexemeID id) const
	{
		os << "FOLLOW(" << lex[id] << "): ";
		Print(os, followSet[id]);
		os << std::endl;
	}

	void BNFGrammar::PrintFirstPlusSet(std::ostream& os, size_t productionIndex, size_t derivedIndex) const
	{
		os << "FIRST+(" << lex[prodSet[productionIndex].nt] << " -> ";
		Print(os, prodSet[productionIndex].derSet[derivedIndex]);
		os << "): ";
		Print(os, firstPlusSet[productionIndex][derivedIndex]);
		os << std::endl;
	}

	bool BNFGrammar::OriginalGrammarIsValid(std::ostream& error) const
	{
		bool originalGrammarIsValid = true;
		if (OriginalGrammarContainsInfiniteCycle(error)) {
			originalGrammarIsValid = false;
		}
		else {
			if (OriginalGrammarContainsInvalidEpsilonProduction(error)) {
				originalGrammarIsValid = false;
			}
		}
		if (ProductionsContainCommonPrefix(error)) {
			originalGrammarIsValid = false;
		}
		return originalGrammarIsValid;
	}

	// the original grammar has cycles (A ->+ A)
	// CHAPTER 3 Parsers, 3.3 TOP-DOWN PARSING, Part "Eliminating Left Recursion", page 102, the first paragraph
	bool BNFGrammar::OriginalGrammarContainsInfiniteCycle(std::ostream& error) const
	{
		bool grammarContainsCycle = false;
		for (const Production& p : prodSet) {
			for (const Derived& d : p.derSet) {
				if (d.size() == 1 && d[0] == p.nt) {
					Error::ErrPrint(error, Error::Level::ERROR,
						"The original grammar has a direct cycle: " + lex[p.nt] + " -> " + lex[p.nt]);
					grammarContainsCycle = true;
				}
			}
		}
		if (CycleIsPresent(error)) {
			grammarContainsCycle = true;
		}
		return grammarContainsCycle;
	}

	// the original grammar has a cycle (A ->+ A), i.e. A -> Y1, Y1 -> Y2, ..., Yn -> A
	bool BNFGrammar::CycleIsPresent(std::ostream& error) const
	{
		LexemeID initialProd = FindInitialNonterminal();
		std::vector<LexemeID> encountered;
		encountered.push_back(initialProd);
		std::queue<LexemeID> q;
		q.push(initialProd);
		bool cycleIsPresent = false;
		while (q.size() > 0) {
			LexemeID id = q.front();
			q.pop();
			for (const Production& p : prodSet) {
				if (p.nt != id) {
					continue;
				}
				for (const Derived& d : p.derSet) {
					if (d.size() == 1) {
						if (SetContains(encountered, d[0])) {
							Error::ErrPrint(error, Error::Level::ERROR,
								"The original grammar has a cycle: " + lex[d[0]] + " ->+ " + lex[d[0]]);
							cycleIsPresent = true;
						}
						else {
							encountered.push_back(d[0]);
							q.push(d[0]);
						}
					}
				}
			}
		}
		return cycleIsPresent;
	}

	// the original grammar has Epsilon-productions (A ->+ E), A - initial production, E - Epsilon-production
	// CHAPTER 3 Parsers, 3.3 TOP-DOWN PARSING, Part "Eliminating Left Recursion", page 102, the first paragraph
	bool BNFGrammar::OriginalGrammarContainsInvalidEpsilonProduction(std::ostream& error) const
	{
		if (!EpsilonProductionsContainOnlyEpsilon(error)) {
			return true;
		}
		LexemeID initialProd = FindInitialNonterminal();
		bool invalidProduction = false;
		for (int i = 0; i < prodSet.size(); ++i) {
			const Production& p = prodSet[i];
			for (const Derived& d : p.derSet) {
				if (d.size() == 1 && d[0] == idES) {
					if (OriginalGrammarContainsChainOfProduction(initialProd, idES)) {
						Error::ErrPrint(error, Error::Level::ERROR,
							"The original grammar has an invalid Epsilon-production: "
							+ lex[p.nt] + " -> " + lexES + ". This leads to "
							+ lex[initialProd] + " ->+ " + lexES);
						invalidProduction = true;
					}
				}
			}
		}
		return invalidProduction;
	}

	// Epsilon-productions contain only Epsilon on the right-hand side
	bool BNFGrammar::EpsilonProductionsContainOnlyEpsilon(std::ostream& error) const
	{
		bool onlyEpsilon = true;
		for (const Production& p : prodSet) {
			for (const Derived& d : p.derSet) {
				if (d.size() == 1) {
					continue;
				}
				for (LexemeID id : d) {
					if (id == idES) {
						Error::ErrPrint(error, Error::Level::ERROR,
							"The original grammar has an invalid Epsilon-production: "
							+ lex[p.nt] + " -> " + GetLexemeString(d));
						onlyEpsilon = false;
					}
				}
			}
		}
		return onlyEpsilon;
	}

	LexemeID BNFGrammar::FindInitialNonterminal() const
	{
		for (const Production& p : prodSet) {
			LexemeID nonterminal = p.nt;
			bool isInitial = true;
			for (const Production& pp : prodSet) {
				for (const Derived& dd : pp.derSet) {
					for (LexemeID id : dd) {
						if (id == nonterminal) {
							isInitial = false;
						}
					}
				}
			}
			if (isInitial) {
				return nonterminal;
			}
		}
		throw Error::RuntimeError{ "Initial nonterminal not found" };
	}

	// the original grammar has a chain of productions (A ->+ B), i.e. A -> Y1, Y1 -> Y2, ..., Yn -> B
	bool BNFGrammar::OriginalGrammarContainsChainOfProduction(LexemeID from, LexemeID to) const
	{
		std::queue<LexemeID> q;
		q.push(from);
		while (q.size() > 0) {
			LexemeID id = q.front();
			q.pop();
			for (const Production& p : prodSet) {
				if (p.nt != id) {
					continue;
				}
				for (const Derived& d : p.derSet) {
					if (d.size() == 1) {
						if (d[0] == to) {
							return true;
						}
						else {
							q.push(d[0]);
						}
					}
				}
			}
		}
		return false;
	}

	// productions for A contain a common prefix on the right-hand side
	// CHAPTER 3 Parsers, 3.3 TOP-DOWN PARSING, Part "Left-Factoring to Eliminate Backtracking"
	bool BNFGrammar::ProductionsContainCommonPrefix(std::ostream& error) const
	{
		bool commonPrefixIsPresent = false;
		for (const Production& p : prodSet) {
			std::vector<LexemeID> prefixes;
			std::vector<LexemeID> commonPrefixes;
			for (const Derived& d : p.derSet) {
				if (p.nt == d[0]) {
					continue;
				}
				if (SetContains(prefixes, d[0])) {
					if (!SetContains(commonPrefixes, d[0])) {
						commonPrefixes.push_back(d[0]);
					}
					commonPrefixIsPresent = true;
				}
				else {
					prefixes.push_back(d[0]);
				}
			}
			if (commonPrefixes.size() > 0) {
				for (LexemeID id : commonPrefixes) {
					Error::ErrPrint(error, Error::Level::ERROR,
						"The original grammar has productions for \"" + lex[p.nt]
						+ "\" with a common prefix \"" + lex[id] + "\". You need to do Left-Factoring");
				}
			}
		}
		return commonPrefixIsPresent;
	}

	// CHAPTER 3 Parsers, 3.3 TOP-DOWN PARSING, Part "Backtrack-Free Parsing", page 107
	bool BNFGrammar::TransformedGrammarIsBacktrackFree(std::ostream& error) const
	{
		bool grammarIsBacktrackFree = true;
		for (int i = 0; i < prodSet.size(); ++i) {
			const Production& p = prodSet[i];
			for (int j = 0; j < p.derSet.size(); ++j) {
				for (int k = j + 1; k < p.derSet.size(); ++k) {
					if (j != k && SetsContainIdenticalElements(firstPlusSet[i][j], firstPlusSet[i][k])) {
						Error::ErrPrint(error, Error::Level::ERROR,
							"The transformed grammar is not free from backtracking. "
							"Identical elements are present in the sets:");
						PrintFirstPlusSet(error, i, j);
						PrintFirstPlusSet(error, i, k);
						grammarIsBacktrackFree = false;
					}
				}
			}
		}
		return grammarIsBacktrackFree;
	}

	std::pair<int, bool> BNFGrammar::GrammarContainsProductionsForNonterminal(LexemeID nonterminal) const
	{
		for (int i = 0; i < prodSet.size(); ++i) {
			if (prodSet[i].nt == nonterminal) {
				return { i, true };
			}
		}
		return { 0, false };
	}

	bool BNFGrammar::SetContainsOnlyTerminalsAndNonterminals(const Derived& derived) const
	{
		for (LexemeID id : derived) {
			switch (typeSet[id]) {
			case LexemeType::T:
			case LexemeType::NT:
				break;
			default:
				return false;
			}
		}
		return true;
	}

	bool BNFGrammar::SetContains(const std::vector<LexemeID>& set, LexemeID target) const
	{
		for (LexemeID id : set) {
			if (id == target) return true;
		}
		return false;
	}

	// both sets contain at least 1 identical element
	bool BNFGrammar::SetsContainIdenticalElements(const std::vector<LexemeID>& a, const std::vector<LexemeID>& b) const
	{
		for (LexemeID idA : a) {
			for (LexemeID idB : b) {
				if (idA == idB) return true;
			}
		}
		return false;
	}

	First BNFGrammar::GetFirstSetForProduction(size_t productionIndex, size_t derivedIndex) const
	{
		First set;
		for (LexemeID id : prodSet[productionIndex].derSet[derivedIndex]) {
			Merge(set, firstSet[id]);
			if (!SetContains(firstSet[id], idES)) {
				break;
			}
		}
		return set;
	}

	// add SOURCE content to RECEIVER (only adds unique elements),
	// returns the number of elements added
	int BNFGrammar::Merge(std::vector<LexemeID>& receiver, const std::vector<LexemeID>& source) const
	{
		int num = 0;
		for (LexemeID id : source) {
			if (!SetContains(receiver, id)) {
				receiver.push_back(id);
				++num;
			}
		}
		return num;
	}

	// add SOURCE content to RECEIVER (only adds unique elements, but other than EXCEPTION),
	// returns the number of elements added
	int BNFGrammar::MergeExcept(std::vector<LexemeID>& receiver, const std::vector<LexemeID>& source,
		LexemeID exception) const
	{
		int num = 0;
		for (LexemeID id : source) {
			if (id != exception && !SetContains(receiver, id)) {
				receiver.push_back(id);
				++num;
			}
		}
		return num;
	}

	LexemeID BNFGrammar::GetLexemeID(const std::string& lexeme)
	{
		auto result = id.emplace(lexeme, next);
		if (result.second == true) {
			lex.push_back(lexeme);
			++next;
		}
		return result.first->second;
	}

	void BNFGrammar::TransformGrammar(std::ostream& error)
	{
		if (!OriginalGrammarIsValid(error)) {
			throw Error::RuntimeError{ "The original grammar is invalid" };
		}
		RemoveAllLeftRecursion();
		FindAllNonterminals();
		FindAllFirstSets();
		FindAllFollowSets();
		FindAllFirstPlusSets();
		if (!TransformedGrammarIsBacktrackFree(error)) {
			error << std::endl << std::endl << *this << std::endl;
			throw Error::RuntimeError{ "The transformed grammar is not free from backtracking" };
		}
	}

	// CHAPTER 3 Parsers, 3.3 TOP-DOWN PARSING, Part "Eliminating Left Recursion",
	// FIGURE 3.6 Removal of Indirect Left Recursion
	void BNFGrammar::RemoveAllLeftRecursion()
	{
		std::vector<Production> newProdList;
		for (int i = 0; i < prodSet.size(); ++i) {
			Production& curProd = prodSet[i];
			// convert each indirect left recursion into a direct left recursion
			for (int j = 0; j < i; ++j) {
				std::vector<Derived> newDerList;
				for (const Derived& d : curProd.derSet) {
					if (prodSet[j].nt == d[0]) {
						int index = -1;
						for (int k = 0; k < newProdList.size(); ++k) {
							if (newProdList[k].nt == d[0]) {
								index = k;
								break;
							}
						}
						for (const Derived& dd : newProdList[index].derSet) {
							newDerList.push_back(dd);
							const auto it = newDerList[newDerList.size() - 1].end();
							newDerList[newDerList.size() - 1].insert(it, ++d.begin(), d.end());
						}
					}
					else {
						newDerList.push_back(d);
					}
				}
				curProd.derSet = newDerList;
			}
			// remove any direct left recursion
			bool leftRecursion = false;
			for (const Derived& d : curProd.derSet) {
				if (curProd.nt == d[0]) {
					leftRecursion = true;
					break;
				}
			}
			if (leftRecursion) {
				Production replacedProd{ curProd.nt };
				Production newProd{ GetLexemeID(lex[curProd.nt] + suffNT) };
				for (const Derived& d : curProd.derSet) {
					if (curProd.nt == d[0]) {
						Derived newDer{ ++d.begin(), d.end() };
						newDer.push_back(newProd.nt);
						newProd.derSet.push_back(newDer);
					}
					else {
						replacedProd.derSet.push_back(d);
						replacedProd.derSet[replacedProd.derSet.size() - 1].push_back(newProd.nt);
					}
				}
				newProd.derSet.push_back(Derived{ {idES} });
				newProdList.push_back(replacedProd);
				newProdList.push_back(newProd);
			}
			else {
				newProdList.push_back(curProd);
			}
		}
		prodSet = newProdList;
	}

	void BNFGrammar::FindAllNonterminals()
	{
		typeSet.resize(next);
		for (LexemeType& lex : typeSet) {
			lex = LexemeType::T;
		}
		for (const Production& p : prodSet) {
			typeSet[p.nt] = LexemeType::NT;
		}
		typeSet[idES] = LexemeType::EMPTYSTRING;
		typeSet[idEOF] = LexemeType::ENDOFFILE;
	}

	// CHAPTER 3 Parsers, 3.3 TOP-DOWN PARSING, Part "Backtrack-Free Parsing",
	// FIGURE 3.7 Computing FIRST Sets for Symbols in a Grammar
	void BNFGrammar::FindAllFirstSets()
	{
		firstSet.resize(next);
		for (LexemeID id = 0; id < next; ++id) {
			switch (typeSet[id]) {
			case LexemeType::EMPTYSTRING:
			case LexemeType::ENDOFFILE:
			case LexemeType::T:
				firstSet[id].push_back(id);
				break;
			}
		}
		bool setsAreChanging = true;
		while (setsAreChanging) {
			setsAreChanging = false;
			for (const Production& p : prodSet) {
				for (const Derived& d : p.derSet) {
					std::vector<LexemeID> rhs;
					int i = 0;
					int k = d.size() - 1;
					if (SetContainsOnlyTerminalsAndNonterminals(d)) {
						MergeExcept(rhs, firstSet[d[i]], idES);
						while (SetContains(firstSet[d[i]], idES) && i < k) {
							MergeExcept(rhs, firstSet[d[++i]], idES);
						}
					}
					if (i == k && SetContains(firstSet[d[k]], idES)) {
						rhs.push_back(idES);
					}
					if (Merge(firstSet[p.nt], rhs) > 0) {
						setsAreChanging = true;
					}
				}
			}
		}
	}

	// CHAPTER 3 Parsers, 3.3 TOP-DOWN PARSING, Part "Backtrack-Free Parsing",
	// FIGURE 3.8 Computing FOLLOW Sets for Non-Terminal Symbols
	void BNFGrammar::FindAllFollowSets()
	{
		followSet.resize(next);
		bool setsAreChanging = true;
		while (setsAreChanging) {
			setsAreChanging = false;
			for (const Production& p : prodSet) {
				for (const Derived& d : p.derSet) {
					std::vector<LexemeID> trailer = followSet[p.nt];
					for (int i = d.size() - 1; i >= 0; --i) {
						LexemeID id = d[i];
						if (typeSet[id] == LexemeType::NT) {
							if (Merge(followSet[id], trailer) > 0) {
								setsAreChanging = true;
							}
							if (SetContains(firstSet[id], idES)) {
								MergeExcept(trailer, firstSet[id], idES);
							}
							else {
								trailer = firstSet[id];
							}
						}
						else {
							trailer = firstSet[id];
						}
					}
				}
			}
		}
		for (const Production& p : prodSet) {
			followSet[p.nt].push_back(idEOF);
		}
	}

	// CHAPTER 3 Parsers, 3.3 TOP-DOWN PARSING, Part "Backtrack-Free Parsing", page 107
	void BNFGrammar::FindAllFirstPlusSets()
	{
		firstPlusSet.resize(prodSet.size());
		for (int i = 0; i < prodSet.size(); ++i) {
			firstPlusSet[i].resize(prodSet[i].derSet.size());
		}
		for (int i = 0; i < prodSet.size(); ++i) {
			for (int j = 0; j < prodSet[i].derSet.size(); ++j) {
				First set = GetFirstSetForProduction(i, j);
				if (SetContains(set, idES)) {
					Merge(set, followSet[prodSet[i].nt]);
				}
				firstPlusSet[i][j] = set;
			}
		}
	}

	std::istream& operator>>(std::istream& is, Production& prod)
	{
		std::string prefix;
		is >> prefix;
		if (!is) return is;
		if (prefix != BNFGrammar::prefNT) {
			is.setstate(std::ios_base::failbit);
			return is;
		}
		std::string lexeme;
		is >> lexeme;
		if (!is) {
			is.setstate(std::ios_base::failbit);
			return is;
		}
		Production temp{ prod.gram->GetLexemeID(lexeme) };
		while (true) {
			is >> prefix;
			if (!is) {
				if (is.eof()) break;
				else return is;
			}
			if (prefix != BNFGrammar::prefDer) {
				if (prefix == BNFGrammar::prefNT) {
					for (int i = 0; i < BNFGrammar::prefNT.size(); ++i) {
						is.unget();
					}
					break;
				}
				is.setstate(std::ios_base::failbit);
				return is;
			}
			std::string s;
			getline(is, s);
			if (!is) {
				if (is.eof()) break;
				else return is;
			}
			Derived derived;
			std::istringstream iss{ s };
			while (iss >> lexeme) {
				derived.push_back(prod.gram->GetLexemeID(lexeme));
			}
			if (!iss.eof()) {
				is.setstate(std::ios_base::failbit);
				return is;
			}
			temp.derSet.push_back(derived);
		}
		prod = temp;
		return is;
	}

	std::istream& operator>>(std::istream& is, BNFGrammar& gram)
	{
		getline(is, gram.comment);			// comment
		std::string secondLine;				// line of symbols
		getline(is, secondLine);
		if (!is) return is;
		std::istringstream iss{ secondLine };
		iss >> gram.prefNT >> gram.prefDer >> gram.suffNT >> gram.lexES >> gram.lexEOF;
		if (!iss) return is;
		gram.idES = gram.GetLexemeID(gram.lexES);
		gram.idEOF = gram.GetLexemeID(gram.lexEOF);
		Production prod;
		while (is) {
			is >> prod;
			if (!is && is.eof() == false) break;
			auto p = gram.GrammarContainsProductionsForNonterminal(prod.nt);
			if (p.second) {
				for (const Derived& d : prod.derSet) {
					gram.prodSet[p.first].derSet.push_back(d);
				}
			}
			else {
				gram.prodSet.push_back(prod);
			}
		}
		return is;
	}

	std::ostream& operator<<(std::ostream& os, const Production& prod)
	{
		constexpr int spaceCount = 2;
		// output left part (NT) of production
		os << BNFGrammar::prefNT << ' ' << prod.gram->lex[prod.nt] << ' ';
		for (size_t j = 0; j < prod.derSet.size(); ++j) {
			// output right part of production
			if (j == 0) {
				os << BNFGrammar::prefDer;
			}
			else {
				int indent = BNFGrammar::prefNT.size() + prod.gram->lex[prod.nt].size() + spaceCount
					+ BNFGrammar::prefDer.size();
				os << std::setw(indent) << std::right << BNFGrammar::prefDer;
			}
			const Derived& derived = prod.derSet[j];
			for (size_t k = 0; k < derived.size(); ++k) {
				os << ' ' << prod.gram->lex[derived[k]];
			}
			os << std::endl;
		}
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const BNFGrammar& gram)
	{
		constexpr int spaceCount = 2;
		int lexMaxLength = 0;
		for (const std::string& s : gram.lex) {
			if (s.size() > lexMaxLength) {
				lexMaxLength = s.size();
			}
		}
		int indent1 = derivedShift + gram.prefDer.size();
		int indent2 = gram.prefNT.size() + lexMaxLength + spaceCount + derivedShift
			+ gram.prefDer.size();
		// output comment
		os << gram.comment << std::endl;
		// output prefixes
		os << gram.prefNT << ' ' << gram.prefDer << ' '
			<< gram.suffNT << ' ' << gram.lexES << ' ' << gram.lexEOF << std::endl;
		// output all productions
		for (size_t i = 0; i < gram.prodSet.size(); ++i) {
			const Production& prod = gram.prodSet[i];
			// output left part (NT) of production
			os << gram.prefNT << ' ' << std::setw(lexMaxLength) << std::left << gram.lex[prod.nt] << ' ';
			for (size_t j = 0; j < prod.derSet.size(); ++j) {
				// output right part of production
				if (j == 0) {
					os << std::setw(indent1) << std::right << gram.prefDer;
				}
				else {
					os << std::setw(indent2) << std::right << gram.prefDer;
				}
				const Derived& derived = prod.derSet[j];
				for (size_t k = 0; k < derived.size(); ++k) {
					os << ' ' << gram.lex[derived[k]];
				}
				os << std::endl;
			}
		}
		// output FIRST sets
		os << std::endl << std::endl;
		for (int i = 0; i < gram.firstSet.size(); ++i) {
			gram.PrintFirstSet(os, i);
		}
		// output FOLLOW sets
		os << std::endl << std::endl;
		for (int i = 0; i < gram.followSet.size(); ++i) {
			if (gram.followSet[i].size() > 0) {
				gram.PrintFollowSet(os, i);
			}
		}
		// output FIRST+ sets
		os << std::endl << std::endl;
		for (int i = 0; i < gram.prodSet.size(); ++i) {
			for (int j = 0; j < gram.prodSet[i].derSet.size(); ++j) {
				gram.PrintFirstPlusSet(os, i, j);
			}
		}
		return os;
	}
}
