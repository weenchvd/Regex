#include<iostream>
#include<sstream>
#include<iomanip>
#include<string>
#include<vector>
#include<set>
#include<unordered_map>
#include"printFA.hpp"
#include"../Error/error.hpp"
#include"regexpr.hpp"


#if PRINTFA

namespace RE
{
	std::string GetSymbol(const char ch)
	{
		switch (ch) {
		case CTRL_NULL:
			return std::string{ R"(\0)" };
		case CTRL_HTAB:
			return std::string{ R"(\t)" };
		case CTRL_NEWLINE:
			return std::string{ R"(\n)" };
		case CTRL_VTAB:
			return std::string{ R"(\v)" };
		case CTRL_FORMFEED:
			return std::string{ R"(\f)" };
		case CTRL_CRETURN:
			return std::string{ R"(\r)" };
		default:
			return std::string{ ch };
		}
	}

	void PrintNFA(std::ostream& os, const RE::Regexp& re)
	{
		if (re.nfa.sz == 0) {
			os << "The number of NFA nodes is 0" << std::endl;
			return;
		}
		const std::vector<NFAnode*> nodes = re.nfa.GetAllNodes();
		std::unordered_map<const NFAnode*, const Number> numbers;
		for (size_t i = 0; i < nodes.size(); ++i) {
			numbers.emplace(nodes[i], i + 1);
		}
		if (nodes.size() != numbers.size()) {
			throw Error::RuntimeError{ "PrintNFA(): nodes.size() != numbers.size()" };
		}
		size_t nDigits{ 0 };									// number of digits
		size_t n{ nodes.size() };
		while (n > 0) {
			++nDigits;
			n /= 10;
		}

		std::string dl;											// dash line
		const std::string sep{ "|" };							// separtor
		const std::string sp{ " " };							// space
		const std::string accept{ "ACCEPT" };
		const std::string start{ "START" };
		const std::string to{ "->" };							// transition mark
		const std::string ns{ "#" };							// number sign
		const std::string eps{ "Eps" };							// Epsilon mark
		const size_t nLetters{ eps.size() };					// number of letters
		const size_t cw1{ sp.size() + ((accept.size() > start.size()) ? accept.size() : start.size()) + sp.size() }; // column width 1
		const size_t cw2{ sizeof(NFAnode*) * 2 }; // column width 2
		const size_t cw3{ sp.size() + ns.size() + nDigits + sp.size() }; // column width 3
		const size_t cw4{ sp.size() + nLetters + sp.size() + to.size() + sp.size() + ns.size() + nDigits + sp.size() }; // column width 4
		const size_t cw5{ cw2 }; // column width 5

		const size_t nDashes{ sep.size() * 6 + cw1 + cw2 + cw3 + cw4 + cw5 }; // number of dashes
		for (size_t i = 0; i < nDashes; ++i) {
			dl += "-";
		}

		os << std::endl << dl << std::endl;
		os << sep << std::setw(cw1 + sep.size() + 1) << sp
			<< std::setw(dl.size() - sep.size() - cw1 - sep.size() - 1 - sep.size()) << std::left << "NFA" << sep
			<< std::endl << dl << std::endl;

		for (size_t i = 0; i < nodes.size(); ++i) {
			const NFAnode* p = nodes[i];
			auto it = numbers.find(p);
			if (p == re.nfa.first) {
				os << sep << sp << std::setw(cw1 - sp.size()) << start << sep;
			}
			else if (p == re.nfa.last) {
				os << sep << sp << std::setw(cw1 - sp.size()) << accept << sep;
			}
			else {
				os << sep << sp << std::setw(cw1 - sp.size()) << sp << sep;
			}
			switch (p->ty) {
			case RE::NFAnode::Type::LITERAL: {
				os << std::setw(cw2) << p << sep
					<< sp << ns << std::setw(cw3 - sp.size() - ns.size()) << it->second << sep
					<< std::setw(cw4 + sep.size() + cw5) << sp << sep << std::endl;
				std::ostringstream oss;
				oss << sp << std::setw(nLetters) << GetSymbol(p->ch) << sp
					<< to << sp << ns << numbers.find(p->succ1)->second;
				os << sep << std::setw(cw1 + sep.size() + cw2 + sep.size() + cw3) << sp
					<< sep << std::setw(cw4) << std::left << oss.str()
					<< sep << p->succ1 << sep << std::endl << dl << std::endl;
				break;
			}
			case RE::NFAnode::Type::EPSILON: {
				os << std::setw(cw2) << p << sep
					<< sp << ns << std::setw(cw3 - sp.size() - ns.size()) << it->second << sep
					<< std::setw(cw4 + sep.size() + cw5) << sp << sep << std::endl;
				std::ostringstream oss;
				oss << sp << std::setw(nLetters) << eps << sp
					<< to << sp << ns << numbers.find(p->succ1)->second;
				os << sep << std::setw(cw1 + sep.size() + cw2 + sep.size() + cw3) << sp
					<< sep << std::setw(cw4) << std::left << oss.str()
					<< sep << p->succ1 << sep << std::endl;
				if (p->succ2 != nullptr) {
					std::ostringstream oss;
					oss << sp << std::setw(nLetters) << eps << sp
						<< to << sp << ns << numbers.find(p->succ2)->second;
					os << sep << std::setw(cw1 + sep.size() + cw2 + sep.size() + cw3) << sp
						<< sep << std::setw(cw4) << std::left << oss.str()
						<< sep << p->succ2 << sep << std::endl;
				}
				os << dl << std::endl;
				break;
			}
			case RE::NFAnode::Type::ACCEPT:
				os << std::setw(cw2) << p << sep
					<< sp << ns << std::setw(cw3 - sp.size() - ns.size()) << it->second << sep
					<< std::setw(cw4 + sep.size() + cw5) << sp << sep << std::endl << dl << std::endl;
				break;
			default:
				os << "Unknown RE::NFAnode::Type" << std::endl;
				break;
			}
		}
	}

	void PrintDFA(std::ostream& os, const RE::Regexp& re)
	{
		if (re.dfa.sz == 0) {
			os << "The number of DFA nodes is 0" << std::endl;
			return;
		}
		const std::vector<DFAnode*> nodes = re.dfa.GetAllNodes();
		std::unordered_map<const DFAnode*, const Number> numbers;
		for (size_t i = 0; i < nodes.size(); ++i) {
			numbers.emplace(nodes[i], i + 1);
		}
		if (nodes.size() != numbers.size()) {
			throw Error::RuntimeError{ "PrintDFA(): nodes.size() != numbers.size()" };
		}
		size_t nDigits{ 0 };									// number of digits
		size_t n{ nodes.size() };
		while (n > 0) {
			++nDigits;
			n /= 10;
		}

		std::string dl;											// dash line
		const std::string sep{ "|" };							// separtor
		const std::string sp{ " " };							// space
		const std::string accept{ "ACCEPT" };
		const std::string start{ "START" };
		const std::string to{ "->" };							// transition mark
		const std::string ns{ "#" };							// number sign
		const size_t nLetters{ 3 };								// number of letters
		const size_t cw1{ sp.size() + ((accept.size() > start.size()) ? accept.size() : start.size()) + sp.size() }; // column width 1
		const size_t cw2{ sizeof(DFAnode*) * 2 }; // column width 2
		const size_t cw3{ sp.size() + ns.size() + nDigits + sp.size() }; // column width 3
		const size_t cw4{ sp.size() + nLetters + sp.size() + to.size() + sp.size() + ns.size() + nDigits + sp.size() }; // column width 4
		const size_t cw5{ cw2 }; // column width 5

		const size_t nDashes{ sep.size() * 6 + cw1 + cw2 + cw3 + cw4 + cw5 }; // number of dashes
		for (size_t i = 0; i < nDashes; ++i) {
			dl += "-";
		}

		os << std::endl << dl << std::endl;
		os << sep << std::setw(cw1 + sep.size() + 1) << sp
			<< std::setw(dl.size() - sep.size() - cw1 - sep.size() - 1 - sep.size()) << std::left << "DFA" << sep
			<< std::endl << dl << std::endl;

		for (size_t i = 0; i < nodes.size(); ++i) {
			const DFAnode* p = nodes[i];
			auto it = numbers.find(p);
			if (p == re.dfa.first) {
				os << sep << sp << std::setw(cw1 - sp.size()) << start << sep;
			}
			else if (p->acc == true) {
				os << sep << sp << std::setw(cw1 - sp.size()) << accept << sep;
			}
			else {
				os << sep << sp << std::setw(cw1 - sp.size()) << sp << sep;
			}
			os << std::setw(cw2) << p << sep
				<< sp << ns << std::setw(cw3 - sp.size() - ns.size()) << it->second << sep
				<< std::setw(cw4 + sep.size() + cw5) << sp << sep << std::endl;
			for (const Transition& t : p->trans) {
				std::ostringstream oss;
				oss << sp << std::setw(nLetters) << GetSymbol(t.first) << sp
					<< to << sp << ns << numbers.find(t.second)->second;
				os << sep << std::setw(cw1 + sep.size() + cw2 + sep.size() + cw3) << sp
					<< sep << std::setw(cw4) << std::left << oss.str()
					<< sep << t.second << sep << std::endl;
			}
			os << dl << std::endl;
		}
	}
}

#endif // PRINTFA