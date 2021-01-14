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
	grammar.RemoveDirectAndIndirectLeftRecursion(std::cerr);
	ofs << grammar;

	return 0;
}

namespace CFG
{
	LexemeID BNFGrammar::GetLexemeID(const std::string& lexeme)
	{
		auto result = id.emplace(lexeme, next);
		if (result.second == true) {
			lex.push_back(lexeme);
			++next;
		}
		return result.first->second;
	}

	void BNFGrammar::RemoveDirectAndIndirectLeftRecursion(std::ostream& error)
	{
		if (!CheckGrammar(error)) return;
		FindAllNonterminals(error);
		RemoveRecursion(error);
	}

	void BNFGrammar::FindAllNonterminals(std::ostream& error)
	{
		typeList.resize(next);
		for (LexemeType& lex : typeList) {
			lex = LexemeType::T;
		}
		for (const Production& p : prodList) {
			typeList[p.nt] = LexemeType::NT;
		}
	}

	// CHAPTER 3 Parsers, 3.3 TOP-DOWN PARSING, Part "Eliminating Left Recursion"
	void BNFGrammar::RemoveRecursion(std::ostream& error)
	{
		std::vector<Production> newProdList;
		for (int i = 0; i < prodList.size(); ++i) {
			Production& curProd = prodList[i];
			// convert each indirect left recursion into a direct left recursion
			for (int j = 0; j < i; ++j) {
				std::vector<Derived> newDerList;
				for (const Derived& d : curProd.derList) {
					if (prodList[j].nt == d[0]) {
						int index = -1;
						for (int k = 0; k < newProdList.size(); ++k) {
							if (newProdList[k].nt == d[0]) {
								index = k;
								break;
							}
						}
						for (const Derived& dd : newProdList[index].derList) {
							newDerList.push_back(dd);
							const auto it = newDerList[newDerList.size() - 1].end();
							newDerList[newDerList.size() - 1].insert(it, ++d.begin(), d.end());
						}
					}
					else {
						newDerList.push_back(d);
					}
				}
				curProd.derList = newDerList;
//error << "--AFTER INDIRECT i = " << i << ", j = " << j << ":" << std::endl;
//error << "-----PRODLIST:" << std::endl;
//for (const Production& p : prodList) error << p;
//error << "-----NEWPRODLIST:" << std::endl;
//for (const Production& p : newProdList) error << p;
//error << "----------------------------------------" << std::endl;
			}
			// remove any direct left recursion
			bool leftRecursion = false;
			for (const Derived& d : curProd.derList) {
				if (curProd.nt == d[0]) {
					leftRecursion = true;
					break;
				}
			}
			if (leftRecursion) {
				Production replacedProd{ curProd.nt };
				Production newProd{ GetLexemeID(lex[curProd.nt] + suffNT) };
				for (const Derived& d : curProd.derList) {
					if (curProd.nt == d[0]) {
						Derived newDer{ ++d.begin(), d.end() };
						newDer.push_back(newProd.nt);
						newProd.derList.push_back(newDer);
					}
					else {
						replacedProd.derList.push_back(d);
						replacedProd.derList[replacedProd.derList.size() - 1].push_back(newProd.nt);
					}
				}
				newProd.derList.push_back(Derived{ {GetLexemeID(lexES)} });
				newProdList.push_back(replacedProd);
				newProdList.push_back(newProd);
			}
			else {
				newProdList.push_back(curProd);
			}
//error << "``AFTER DIRECT i = " << i << ":" << std::endl;
//error << "`````PRODLIST:" << std::endl;
//for (const Production& p : prodList) error << p;
//error << "`````NEWPRODLIST:" << std::endl;
//for (const Production& p : newProdList) error << p;
//error << "````````````````````````````````````````" << std::endl;
		}
		prodList = newProdList;
	}

	bool BNFGrammar::CheckGrammar(std::ostream& error) const
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
		for (int i = 0; i < prodList.size(); ++i) {
			const Production& p = prodList[i];
			++counter[p.nt];
			for (const Derived& d : p.derList) {
				if (d.size() == 1 && d[0] == p.nt) {
					error << "\tError. Grammar has cycle '" << lex[p.nt] << " -> " << lex[p.nt] << "'" << std::endl;
					return false;
				}
				if (lexESisPresent && d.size() == 1 && d[0] == iter->second) {
					error << "\tError. Grammar has " << lexES << std::endl;
					return false;
				}
				for (int j = 0; j < i; ++j) {
					if (d.size() == 1 && d[0] == prodList[j].nt) {
						error << "\tWarning. Probably the grammar has indirect cycle '" << lex[prodList[j].nt]
							<< " -> " << lex[prodList[i].nt] << " -> " << lex[prodList[j].nt] << "'" << std::endl;
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
			temp.derList.push_back(derived);
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
		iss >> gram.prefNT >> gram.prefDer >> gram.suffNT >> gram.lexES;
		if (!iss) return is;
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
		for (size_t j = 0; j < prod.derList.size(); ++j) {
			// output right part of production
			if (j == 0) {
				os << BNFGrammar::prefDer;
			}
			else {
				int indent = BNFGrammar::prefNT.size() + prod.gram->lex[prod.nt].size() + spaceCount
					+ BNFGrammar::prefDer.size();
				os << std::setw(indent) << std::right << BNFGrammar::prefDer;
			}
			const Derived& derived = prod.derList[j];
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
			<< gram.suffNT << ' ' << gram.lexES << std::endl;
		// output all productions
		for (size_t i = 0; i < gram.prodList.size(); ++i) {
			const Production& prod = gram.prodList[i];
			// output left part (NT) of production
			os << gram.prefNT << ' ' << std::setw(lexMaxLength) << std::left << gram.lex[prod.nt] << ' ';
			for (size_t j = 0; j < prod.derList.size(); ++j) {
				// output right part of production
				if (j == 0) {
					os << std::setw(indent1) << std::right << gram.prefDer;
				}
				else {
					os << std::setw(indent2) << std::right << gram.prefDer;
				}
				const Derived& derived = prod.derList[j];
				for (size_t k = 0; k < derived.size(); ++k) {
					os << ' ' << gram.lex[derived[k]];
				}
				os << std::endl;
			}
		}
		return os;
	}
}
