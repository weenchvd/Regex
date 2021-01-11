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
		type.resize(next);
		for (LexemeType& lex : type) {
			lex = LexemeType::T;
		}
		for (const Production& p : prod) {
			type[p.nt] = LexemeType::NT;
		}
	}

	void BNFGrammar::RemoveRecursion(std::ostream& error)
	{
		std::vector<Production> newSet;
		for (int i = 0; i < prod.size(); ++i) {
			Production& curProd = prod[i];
			// convert each indirect left recursion into a direct left recursion
			for (int j = 0; j < i; ++j) {
				// TODO
			}
			// remove any direct left recursion
			bool leftRecursion = false;
			for (const Derived& d : curProd.der) {
				if (curProd.nt == d[0]) {
					leftRecursion = true;
					break;
				}
			}
			if (leftRecursion) {
				Production replacedProd{ curProd.nt };
				Production newProd{ GetLexemeID(lex[curProd.nt] + suffixNT) };
				for (const Derived& d : curProd.der) {
					if (curProd.nt == d[0]) {
						Derived newDer{ ++d.begin(), d.end() };
						newDer.push_back(newProd.nt);
						newProd.der.push_back(newDer);
					}
					else {
						replacedProd.der.push_back(d);
						replacedProd.der[replacedProd.der.size() - 1].push_back(newProd.nt);
					}
				}
				newProd.der.push_back(Derived{ {GetLexemeID(lexemeES)} });
				newSet.push_back(replacedProd);
				newSet.push_back(newProd);
			}
			else {
				newSet.push_back(curProd);
			}
		}
		prod = newSet;
	}

	bool BNFGrammar::CheckGrammar(std::ostream& error) const
	{
		for (const Production& p : prod) {
			for (const Derived& d : p.der) {
				if (d.size() == 1 && d[0] == p.nt) {
					std::string lexeme;
					for (const auto& pair : id) {
						if (pair.second == p.nt) {
							lexeme = pair.first;
							break;
						}
					}
					error << "\tError. Grammar has cycle '" << lexeme << " -> " << lexeme << "'" << std::endl;
					return false;
				}
			}
		}
		return true;
	}

	//std::vector<std::string> BNFGrammar::LexemeIDToLexeme() const
	//{
	//	std::vector<std::string> lexeme(next);					// vector[lexeme ID] == lexeme
	//	for (auto it = id.begin(); it != id.end(); ++it) {
	//		lexeme[it->second] = it->first;
	//	}
	//	return lexeme;
	//}

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
		Production temp{ Production::gram->GetLexemeID(lexeme) };
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
				derived.push_back(Production::gram->GetLexemeID(lexeme));
			}
			if (!iss.eof()) {
				is.setstate(std::ios_base::failbit);
				return is;
			}
			temp.der.push_back(derived);
		}
		prod = temp;
		return is;
	}

	std::istream& operator>>(std::istream& is, BNFGrammar& gram)
	{
		std::string firstLine;				// prefixes
		getline(is, firstLine);
		if (!is) return is;
		std::istringstream iss{ firstLine };
		iss >> gram.prefNT >> gram.prefDer;
		if (!iss) return is;
		Production prod;
		while (is) {
			is >> prod;
			if (!is && is.eof() == false) break;
			gram.AddProduction(prod);
		}
		return is;
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
		int indent1 = derivedShift + BNFGrammar::prefDer.size();
		int indent2 = BNFGrammar::prefNT.size() + lexMaxLength + spaceCount + derivedShift
			+ BNFGrammar::prefDer.size();
		// output prefixes
		os << BNFGrammar::prefNT << ' ' << BNFGrammar::prefDer << std::endl;
		// output all productions
		for (size_t i = 0; i < gram.prod.size(); ++i) {
			const Production& prod = gram.prod[i];
			// output left part (NT) of production
			os << BNFGrammar::prefNT << ' ' << std::setw(lexMaxLength) << std::left << gram.lex[prod.nt] << ' ';
			for (size_t j = 0; j < prod.der.size(); ++j) {
				// output right part of production
				if (j == 0) {
					os << std::setw(indent1) << std::right << BNFGrammar::prefDer;
				}
				else {
					os << std::setw(indent2) << std::right << BNFGrammar::prefDer;
				}
				const auto& derived = prod.der[j];
				for (size_t k = 0; k < derived.size(); ++k) {
					os << ' ' << gram.lex[derived[k]];
				}
				os << std::endl;
			}
		}
		return os;
	}
}
