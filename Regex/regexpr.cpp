#include<iostream>
#include<fstream>
#include<sstream>
#include<iomanip>
#include<string>
#include<vector>
#include<set>
#include<queue>
#include<algorithm>
#include<memory>
#include"printFA.hpp"
#include"../Error/error.hpp"
#include"regexpr.hpp"

int main()
{
	try
	{
		const std::string regexList{ "Regexes.txt" };
		//const std::string regexList{ R"(D:\_Code\Project_A\Build\Regexes.txt)" };
		std::ifstream ifs{ regexList };
		if (!ifs) {
			Error::ErrPrint(std::cerr, Error::Level::ERROR, Error::Type::INFILE, regexList);
			return 1;
		}
		const std::string output{ "DFA.txt" };
		//const std::string output{ R"(D:\_Code\Project_A\Build\DFA.txt)" };
		std::ofstream ofs{ output };
		if (!ofs) {
			Error::ErrPrint(std::cerr, Error::Level::ERROR, Error::Type::OUTFILE, output);
			return 1;
		}
		RE::REstring rs;
		ifs >> rs;
		//if (ifs || !ifs && !ifs.eof()) {
		//	Error::ErrPrint(std::cerr, Error::Level::ERROR, "File data read error");
		//	return 1;
		//}
		RE::Regexp regex{ rs };

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

namespace RE
{
	NFAnode* NFAnode::ptr = nullptr;
	std::allocator<NFAnode> NFA::alloc;

	void NFA::AddNodeToSet(std::vector<NFAnode*>& set, NFAnode* node) const
	{
		if (node == nullptr || node->mark == true) {
			return;
		}
		set.push_back(node);
		node->mark = true;
		AddNodeToSet(set, node->succ1);
		AddNodeToSet(set, node->succ2);
	}

	void NFA::ReleaseResources()
	{
		std::vector<NFAnode*> nodes;
		nodes.reserve(sz);
		AddNodeToSet(nodes, first);
		for (NFAnode* p : nodes) {
			alloc.destroy(p);
			alloc.deallocate(p, 1);
		}
	}

	NFAnode* NFA::CreateNFANode(NFAnode::Type type) const
	{
		NFAnode* p = alloc.allocate(1, NFAnode::ptr);
		if (p == nullptr) {
			throw Error::RuntimeError{ "No memory allocated for NFA node" };
		}
		NFAnode::ptr = p;
		alloc.construct(p, type);
		return p;
	}

	NFAnode* NFA::CreateNFANode(NFAnode::Type type, Character character) const
	{
		NFAnode* p = alloc.allocate(1, NFAnode::ptr);
		if (p == nullptr) {
			throw Error::RuntimeError{ "No memory allocated for NFA node" };
		}
		alloc.construct(p, type, character);
		NFAnode::ptr = p;
		return p;
	}

	NFA::NFA(Character character)
		: sz{ 2 }
	{
		first = CreateNFANode(NFAnode::Type::LITERAL, character);
		last = CreateNFANode(NFAnode::Type::ACCEPT);
		first->succ1 = last;
	}

	NFA::~NFA()
	{
		if (sz > 0) {
			ReleaseResources();
		}
	}

	NFA::NFA(NFA&& other)
	{
		first = other.first;
		last = other.last;
		sz = other.sz;
		other.first = other.last = nullptr;
		other.sz = 0;
	}

	NFA& NFA::operator=(NFA&& other)
	{
		if (this == &other) {
			return *this;
		}
		if (sz > 0) {
			ReleaseResources();
		}
		first = other.first;
		last = other.last;
		sz = other.sz;
		other.first = other.last = nullptr;
		other.sz = 0;
		return *this;
	}

	// the function concatenates two NFAs via an Epsilon-transition
	void NFA::Concatenate(NFA& other)
	{
		last->succ1 = other.first;
		last->ty = NFAnode::Type::EPSILON;
		last = other.last;
		sz += other.sz;
		other.first = other.last = nullptr;
		other.sz = 0;
	}

	void NFA::Alternate(NFA& other)
	{
		NFAnode* newFirst = CreateNFANode(NFAnode::Type::EPSILON);
		NFAnode* newLast = CreateNFANode(NFAnode::Type::ACCEPT);
		newFirst->succ1 = first;
		newFirst->succ2 = other.first;
		last->succ1 = newLast;
		other.last->succ1 = newLast;
		last->ty = NFAnode::Type::EPSILON;
		other.last->ty = NFAnode::Type::EPSILON;
		first = newFirst;
		last = newLast;
		sz += other.sz + 2;
		other.first = other.last = nullptr;
		other.sz = 0;
	}

	// the function creates a Kleene (0 or more times) closure for NFA
	void NFA::ClosureKleene()
	{
		NFAnode* newFirst = CreateNFANode(NFAnode::Type::EPSILON);
		NFAnode* newLast = CreateNFANode(NFAnode::Type::ACCEPT);
		newFirst->succ1 = first;
		newFirst->succ2 = newLast;
		last->succ1 = first;
		last->succ2 = newLast;
		last->ty = NFAnode::Type::EPSILON;
		first = newFirst;
		last = newLast;
		sz += 2;
	}

	//----------------------------------------------------------------------------------------------------

	bool operator<(const std::pair<Character, DFAnode*>& a, const std::pair<Character, DFAnode*>& b)
	{
		return (a.first < b.first) ? true : false;
	}

	DFAnode* DFAnode::ptr = nullptr;
	std::allocator<DFAnode> DFA::alloc;

	void DFA::AddNodeToSet(std::vector<DFAnode*>& set, DFAnode* node) const
	{
		if (node == nullptr || node->mark == true) {
			return;
		}
		set.push_back(node);
		node->mark = true;
		for (auto& p : node->trans) {
			AddNodeToSet(set, p.second);
		}
	}

	void DFA::ReleaseResources()
	{
		std::vector<DFAnode*> nodes;
		nodes.reserve(sz);
		AddNodeToSet(nodes, first);
		for (DFAnode* p : nodes) {
			alloc.destroy(p);
			alloc.deallocate(p, 1);
		}
	}

	DFAnode* DFA::CreateDFANode(bool accept) const
	{
		DFAnode* p = alloc.allocate(1, DFAnode::ptr);
		if (p == nullptr) {
			throw Error::RuntimeError{ "No memory allocated for DFA node" };
		}
		DFAnode::ptr = p;
		alloc.construct(p, accept);
		return p;
	}

	DFA::~DFA()
	{
		if (sz > 0) {
			ReleaseResources();
		}
	}

	DFA::DFA(DFA&& other)
	{
		first = other.first;
		sz = other.sz;
		other.first = nullptr;
		other.sz = 0;
	}

	DFA& DFA::operator=(DFA&& other)
	{
		if (this == &other) {
			return *this;
		}
		if (sz > 0) {
			ReleaseResources();
		}
		first = other.first;
		sz = other.sz;
		other.first = nullptr;
		other.sz = 0;
		return *this;
	}

	//----------------------------------------------------------------------------------------------------

	std::pair<Character, Regexp::TokenStream::TokenType> Regexp::TokenStream::GetToken()
	{
		if (pos >= s.size()) {
			return { 0, TokenType::EOS };
		}
		Character ch = s[pos];
		switch (ch) {
		case '(':
		case ')':
		case '|':
		case '*':
		case '+':
		case '?':
			return { ch, TokenType::SPECIAL };
		default:
			alphabet.emplace(ch);
			return { ch, TokenType::LITERAL };
		}
	}

	NFAset Regexp::Delta(const NFAset& set, Character ch) const
	{
		NFAset newSet;
		for (NFAnode* p : set) {
			if (p->ch == ch) {
				newSet.emplace(p->succ1);
			}
		}
		return newSet;
	}

	NFAset Regexp::EpsilonClosure(NFAset set) const
	{
		NFAset newSet{ set.begin(), set.end() };
		for (NFAnode* p : set) {
			AddNodesReachableViaEpsilonTransition(newSet, p);
		}
		return newSet;
	}

	void Regexp::AddNodesReachableViaEpsilonTransition(NFAset& set, const NFAnode* node) const
	{
		if (node->ty == NFAnode::Type::EPSILON) {
			set.emplace(node->succ1);
			AddNodesReachableViaEpsilonTransition(set, node->succ1);
			if (node->succ2 != nullptr) {
				set.emplace(node->succ2);
				AddNodesReachableViaEpsilonTransition(set, node->succ2);
			}
		}
	}

	bool Regexp::Equal(const NFAset& a, const NFAset& b) const
	{
		if (a.size() != b.size()) {
			return false;
		}
		NFAset::iterator itA = a.begin();
		NFAset::iterator itB = b.begin();
		while (itA != a.end()) {
			if (*itA != *itB) {
				return false;
			}
			++itA;
			++itB;
		}
		return true;
	}

#if PRINTFA
	void Regexp::MakeDFA()
	{
		std::cout << std::endl << "RE: " << this->source << std::endl;
		REtoNFA();
		PrintNFA(std::cout, *this);
		NFAtoDFA();
		std::cout << std::endl << "RE: " << this->source << std::endl;
		PrintDFA(std::cout, *this);
		MinimizeDFA();
		std::cout << std::endl << "RE: " << this->source << std::endl;
		PrintDFA(std::cout, *this);
	}
#else
	void Regexp::MakeDFA()
	{
		REtoNFA();
		NFAtoDFA();
		MinimizeDFA();
	}
#endif // PRINTFA

	// Thompson’s Construction
	// CHAPTER 2 Scanners, 2.4 FROM REGULAR EXPRESSION TO SCANNER, 2.4.2 Regular Expression to NFA: Thompson’s Construction
	void Regexp::REtoNFA()
	{
		nfa = PGoal();
		auto p = ts.GetAlphabet();
		alphabet.insert(alphabet.end(), p.first, p.second);
		std::sort(alphabet.begin(), alphabet.end());
		ts.EraseAlphabet();
	}

	// Subset Construction
	// CHAPTER 2 Scanners, 2.4 FROM REGULAR EXPRESSION TO SCANNER, 2.4.3 NFA to DFA: The Subset Construction
	// FIGURE 2.6 The Subset Construction
	void Regexp::NFAtoDFA()
	{
		TableEntry first;
		first.state = EpsilonClosure(NFAset{ {nfa.GetFirstNode()} });
		first.trans.resize(alphabet.size());
		Table table;
		table.push_back(first);
		std::queue<TableIndex> workList;
		workList.push(0);
		while (workList.size() > 0) {
			TableIndex i = workList.front();
			workList.pop();
			for (size_t k = 0; k < alphabet.size(); ++k) {
				NFAset newSet = EpsilonClosure(Delta(table[i].state, alphabet[k]));
				if (newSet.size() == 0) {
					table[i].trans[k] = noTransition;
					continue;
				}
				bool inTable = false;
				for (TableIndex j = 0; j < table.size(); ++j) {
					if (Equal(table[j].state, newSet)) {
						table[i].trans[k] = j;
						inTable = true;
					}
				}
				if (inTable == false) {
					TableEntry newEntry;
					newEntry.state = newSet;
					newEntry.trans.resize(alphabet.size());
					table.push_back(newEntry);
					TableIndex newIndex = table.size() - 1;
					workList.push(newIndex);
					table[i].trans[k] = newIndex;
				}
			}
		}
		std::vector<DFAnode*> nodes;
		nodes.reserve(table.size());
		NFAnode* accepted = nfa.GetLastNode();
		for (TableIndex i = 0; i < table.size(); ++i) {
			TableEntry& entry = table[i];
			NFAset::iterator it = entry.state.find(accepted);
			nodes.push_back(dfa.CreateDFANode((it == entry.state.end()) ? false : true));
		}
		for (TableIndex i = 0; i < table.size(); ++i) {
			TableEntry& entry = table[i];
			for (size_t j = 0; j < entry.trans.size(); ++j) {
				TableIndex index = entry.trans[j];
				if (index == noTransition) {
					continue;
				}
				nodes[i]->trans.push_back(std::pair<Character, DFAnode*>{ alphabet[j], nodes[index] });
			}
		}
		dfa.first = nodes[0];
		dfa.sz = table.size();
	}

	void Regexp::MinimizeDFA()
	{

	}

	// parse Goal
	NFA Regexp::PGoal()
	{
		token = ts.GetToken();
		return PAlternation();
	}

	// parse Alternation
	NFA Regexp::PAlternation()
	{
		NFA a = PConcatenation();
		PAlternationPrime(a);
		return a;
	}

	// parse Alternation'
	void Regexp::PAlternationPrime(NFA& a)
	{
		switch (token.second) {
		case Regexp::TokenStream::TokenType::EOS:
			return;
		case Regexp::TokenStream::TokenType::SPECIAL:
			switch (token.first) {
			case ')':
				return;
			case '|':
				token = ts.AdvanceAndGetToken();
				a.Alternate(PConcatenation());
				PAlternationPrime(a);
				return;
			default:
				break;
			}
		case Regexp::TokenStream::TokenType::LITERAL:
			break;
		default:
			break;
		}
		ThrowInvalidRegex(ts.GetPosition());
	}

	// parse Concatenation
	NFA Regexp::PConcatenation()
	{
		NFA a = PSymbol();
		PConcatenationPrime(a);
		return a;
	}

	// parse Concatenation'
	void Regexp::PConcatenationPrime(NFA& a)
	{
		switch (token.second) {
		case Regexp::TokenStream::TokenType::EOS:
			return;
		case Regexp::TokenStream::TokenType::SPECIAL:
			switch (token.first) {
			case '(': {
				a.Concatenate(PSymbol());
				PConcatenationPrime(a);
				return;
			}
			case ')':
			case '|':
				return;
			default:
				break;
			}
		case Regexp::TokenStream::TokenType::LITERAL:
			a.Concatenate(PSymbol());
			PConcatenationPrime(a);
			return;
		default:
			break;
		}
		ThrowInvalidRegex(ts.GetPosition());
	}

	// parse Symbol
	NFA Regexp::PSymbol()
	{
		NFA a = PBlock();
		PClosure(a);
		return a;
	}

	// parse Block
	NFA Regexp::PBlock()
	{
		switch (token.second) {
		case Regexp::TokenStream::TokenType::EOS:
			break;
		case Regexp::TokenStream::TokenType::SPECIAL:
			switch (token.first) {
			case '(': {
				token = ts.AdvanceAndGetToken();
				NFA a = PAlternation();
				if (token.first == ')') {
					token = ts.AdvanceAndGetToken();
					return a;
				}
				break;
			}
			default:
				break;
			}
			break;
		case Regexp::TokenStream::TokenType::LITERAL: {
			Character ch = token.first;
			token = ts.AdvanceAndGetToken();
			return NFA{ ch };
		}
		default:
			break;
		}
		ThrowInvalidRegex(ts.GetPosition());
	}

	// parse Closure
	void Regexp::PClosure(NFA& a)
	{
		switch (token.second) {
		case Regexp::TokenStream::TokenType::EOS:
			return;
		case Regexp::TokenStream::TokenType::SPECIAL:
			switch (token.first) {
			case '*':
				a.ClosureKleene();
				token = ts.AdvanceAndGetToken();
				return;
			//case '+':
			//	
			//	token = ts.AdvanceAndGetToken();
			//	return;
			//case '?':
			//	
			//	token = ts.AdvanceAndGetToken();
			//	return;
			case '(':
			case ')':
			case '|':
				return;
			default:
				break;
			}
			break;
		case Regexp::TokenStream::TokenType::LITERAL:
			return;
		default:
			break;
		}
		ThrowInvalidRegex(ts.GetPosition());
	}

	void Regexp::ThrowInvalidRegex(size_t position) const
	{
		std::string message{ "Invalid character '" };
		message += std::string{ source, position, 1 } + "' was encountered after substring '";
		message += std::string{ source, 0, position } + "'. Regular expression: ";
		message += source;
		throw Error::InvalidRegex{ message };
	}

	Regexp::Regexp(const REstring& string)
		: ts{ string }
	{
		PutRE(string);
	}

	void Regexp::PutRE(const REstring& string)
	{
		if (string.size() > 0) {
			source = string;
			MakeDFA();
		}
	}

	std::istream& operator>>(std::istream& is, REstring& string)
	{
		std::string line;
		while (is) {
			getline(is, line);
			if (is) {
				string += line;
			}
		}
		return is;
	}
}
