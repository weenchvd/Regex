#include<iostream>
#include<sstream>
#include<iomanip>
#include<string>
#include<vector>
#include<set>
#include<map>
#include"printFA.hpp"
#include"../Error/error.hpp"
#include"regexpr.hpp"


#if PRINTFA

namespace RE
{
	void PrintNFA(std::ostream& os, const RE::Regexp& re)
	{
		if (re.nfa.sz == 0) {
			os << "The number of NFA nodes is 0" << std::endl;
			return;
		}
		std::vector<NFAnode*> nodes;
		nodes.reserve(re.nfa.sz);
		re.nfa.AddNodeToSet(nodes, re.nfa.first);
		for (NFAnode* p : nodes) {
			p->mark = false;
		}
		std::map<NFAnode*, size_t> numbers;
		for (size_t i = 0; i < nodes.size(); ++i) {
			numbers.emplace(nodes[i], i + 1);
		}
		if (nodes.size() != numbers.size()) {
			throw Error::RuntimeError{ "PrintNFA: nodes.size() != numbers.size()" };
		}
		size_t nDigits{ 0 };				// number of digits
		size_t n{ nodes.size() };
		while (n > 0) {
			++nDigits;
			n /= 10;
		}

		std::string dl;						// dash line
		const std::string sep{ "|" };		// separtor
		const std::string sp{ " " };		// space
		const std::string accept{ "ACCEPT" };
		const std::string start{ "START" };
		const std::string to{ "->" };
		const std::string ns{ "#" };		// number sign
		const std::string eps{ "Eps" };
		const size_t nLetters{ eps.size() };
		const size_t cw1{ sp.size() + ((accept.size() > start.size()) ? accept.size() : start.size()) + sp.size() }; // column width 1
		const size_t cw2{ sizeof(NFAnode*) * 2 };
		const size_t cw3{ sp.size() + ns.size() + nDigits + sp.size() };
		const size_t cw4{ sp.size() + nLetters + sp.size() + to.size() + sp.size() + ns.size() + nDigits + sp.size() };
		const size_t cw5{ cw2 };

		size_t nDashes{ sep.size() * 6 + cw1 + cw2 + cw3 + cw4 + cw5 };
		for (int i = 0; i < nDashes; ++i) {
			dl += "-";
		}

		os << std::endl << dl << std::endl;
		os << sep << std::setw(cw1 + sep.size() + 1) << sp
			<< std::setw(dl.size() - sep.size() - cw1 - sep.size() - 1 - sep.size()) << std::left << "NFA" << sep
			<< std::endl << dl << std::endl;

		for (int i = 0; i < nodes.size(); ++i) {
			NFAnode* p = nodes[i];
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
				oss << sp << std::setw(nLetters) << char(p->ch) << sp
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
		std::vector<DFAnode*> nodes;
		nodes.reserve(re.dfa.sz);
		re.dfa.AddNodeToSet(nodes, re.dfa.first);
		for (DFAnode* p : nodes) {
			p->mark = false;
		}
		std::map<DFAnode*, size_t> numbers;
		for (size_t i = 0; i < nodes.size(); ++i) {
			numbers.emplace(nodes[i], i + 1);
		}
		if (nodes.size() != numbers.size()) {
			throw Error::RuntimeError{ "PrintDFA: nodes.size() != numbers.size()" };
		}
		size_t nDigits{ 0 };				// number of digits
		size_t n{ nodes.size() };
		while (n > 0) {
			++nDigits;
			n /= 10;
		}

		std::string dl;						// dash line
		const std::string sep{ "|" };		// separtor
		const std::string sp{ " " };		// space
		const std::string accept{ "ACCEPT" };
		const std::string start{ "START" };
		const std::string to{ "->" };
		const std::string ns{ "#" };		// number sign
		const size_t nLetters{ 1 };
		const size_t cw1{ sp.size() + ((accept.size() > start.size()) ? accept.size() : start.size()) + sp.size() }; // column width 1
		const size_t cw2{ sizeof(DFAnode*) * 2 }; // column width 2
		const size_t cw3{ sp.size() + ns.size() + nDigits + sp.size() }; // column width 3
		const size_t cw4{ sp.size() + nLetters + sp.size() + to.size() + sp.size() + ns.size() + nDigits + sp.size() }; // column width 4
		const size_t cw5{ cw2 }; // column width 5

		size_t nDashes{ sep.size() * 6 + cw1 + cw2 + cw3 + cw4 + cw5 };
		for (int i = 0; i < nDashes; ++i) {
			dl += "-";
		}

		os << std::endl << dl << std::endl;
		os << sep << std::setw(cw1 + sep.size() + 1) << sp
			<< std::setw(dl.size() - sep.size() - cw1 - sep.size() - 1 - sep.size()) << std::left << "DFA" << sep
			<< std::endl << dl << std::endl;

		for (int i = 0; i < nodes.size(); ++i) {
			DFAnode* p = nodes[i];
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
			for (auto& t : p->trans) {
				std::ostringstream oss;
				oss << sp << std::setw(nLetters) << char(t.first) << sp
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