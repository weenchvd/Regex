#include<iostream>
#include<fstream>
#include<sstream>
#include<iomanip>
#include<string>
#include<vector>
#include<map>
#include"grammar.hpp"

int main()
{
	const std::string sourceGrammar{ "Regex Grammar.txt" };
	std::ifstream ifs{ sourceGrammar };
	if (!ifs) {
		std::cerr << "Cannot open input file" << std::endl;
		return -1;
	}
	const std::string resultGrammar{ "Result Grammar.txt" };
	std::ofstream ofs{ resultGrammar };
	if (!ofs) {
		std::cerr << "Cannot open output file" << std::endl;
		return -1;
	}

	CFG::BNFGrammar grammar;
	ifs >> grammar;
	if (!ifs) {
		if (ifs.eof()) std::cout << "ifs.eof() is true" << std::endl;
		else if (ifs.fail()) std::cout << "ifs.fail() is true" << std::endl;
		else std::cout << "ifs.bad() is true" << std::endl;
	}
	grammar.TransformGrammar(std::cerr);
	ofs << grammar;

	return 0;
}

namespace CFG
{
	void BNFGrammar::Print(std::ostream& os, const std::vector<LexemeID>& set) const
	{
		for (LexemeID id : set) {
			os << lex[id] << ' ';
		}
	}

	bool BNFGrammar::CheckOriginalGrammar(std::ostream& error) const
	{
		std::vector<int> counter(next);
		for (int& i : counter) {
			i = 0;
		}
		bool lexESisPresent = true;
		auto iter = id.find(lexES);
		if (iter == id.end()) {
			lexESisPresent = false;
		}
		for (int i = 0; i < prodSet.size(); ++i) {
			const Production& p = prodSet[i];
			++counter[p.nt];
			for (const Derived& d : p.derSet) {
				if (d.size() == 1 && d[0] == p.nt) {
					error << "\tError. Grammar has cycle '" << lex[p.nt] << " -> " << lex[p.nt] << "'" << std::endl;
					return false;
				}
				if (lexESisPresent && d.size() == 1 && d[0] == iter->second) {
					error << "\tError. Grammar has " << lexES << std::endl;
					return false;
				}
				for (int j = 0; j < i; ++j) {
					if (d.size() == 1 && d[0] == prodSet[j].nt) {
						error << "\tWarning. Probably the grammar has indirect cycle '" << lex[prodSet[j].nt]
							<< " -> " << lex[prodSet[i].nt] << " -> " << lex[prodSet[j].nt] << "'" << std::endl;
					}
				}
			}
		}
		for (int i = 0; i < counter.size(); ++i) {
			if (counter[i] > 1) {
				error << "\tError. Grammar has more than 1 production for '" << lex[i] << "'" << std::endl;
				return false;
			}
		}
		return true;
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
		RemoveDirectAndIndirectLeftRecursion(error);
		FindAllNonterminals(error);
		FindAllFirstSets(error);
		FindAllFollowSets(error);
	}

	void BNFGrammar::RemoveDirectAndIndirectLeftRecursion(std::ostream& error)
	{
		if (!CheckOriginalGrammar(error)) return;
		RemoveRecursion(error);
	}

	// CHAPTER 3 Parsers, 3.3 TOP-DOWN PARSING, Part "Eliminating Left Recursion",
	// FIGURE 3.6 Removal of Indirect Left Recursion
	void BNFGrammar::RemoveRecursion(std::ostream& error)
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
//error << "--AFTER INDIRECT i = " << i << ", j = " << j << ":" << std::endl;
//error << "-----PRODLIST:" << std::endl;
//for (const Production& p : prodSet) error << p;
//error << "-----NEWPRODLIST:" << std::endl;
//for (const Production& p : newProdList) error << p;
//error << "----------------------------------------" << std::endl;
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
//error << "``AFTER DIRECT i = " << i << ":" << std::endl;
//error << "`````PRODLIST:" << std::endl;
//for (const Production& p : prodSet) error << p;
//error << "`````NEWPRODLIST:" << std::endl;
//for (const Production& p : newProdList) error << p;
//error << "````````````````````````````````````````" << std::endl;
		}
		prodSet = newProdList;
	}

	void BNFGrammar::FindAllNonterminals(std::ostream& error)
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
	void BNFGrammar::FindAllFirstSets(std::ostream& error)
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
	void BNFGrammar::FindAllFollowSets(std::ostream& error)
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
			gram.AddProduction(prod);
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
			os << "FIRST(" << gram.lex[i] << "): ";
			gram.Print(os, gram.firstSet[i]);
			os << std::endl;
		}
		// output FOLLOW sets
		os << std::endl << std::endl;
		for (int i = 0; i < gram.followSet.size(); ++i) {
			if (gram.followSet[i].size() > 0) {
				os << "FOLLOW(" << gram.lex[i] << "): ";
				gram.Print(os, gram.followSet[i]);
				os << std::endl;
			}
		}
		return os;
	}
}
