#include<iostream>
#include<fstream>
#include<sstream>
#include<iomanip>
#include<string>
#include<vector>
#include<set>
#include<unordered_map>
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
	NFAnode* NFAnode::hint = nullptr;
	std::allocator<NFAnode> NFA::alloc;

	std::vector<NFAnode*> NFA::GetAllNodes() const
	{
		std::vector<NFAnode*> all;
		all.reserve(sz);
		AddNodeToSet(all, first);
		for (NFAnode* p : all) {
			p->mark = false;
		}
		return all;
	}

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
		std::vector<NFAnode*> nodes = GetAllNodes();
		for (NFAnode* p : nodes) {
			alloc.destroy(p);
			alloc.deallocate(p, 1);
		}
	}

	NFAnode* NFA::CreateNFANode(const NFAnode::Type type) const
	{
		NFAnode* p = alloc.allocate(1, NFAnode::hint);
		if (p == nullptr) {
			throw Error::RuntimeError{ "No memory allocated for NFA node" };
		}
		NFAnode::hint = p;
		alloc.construct(p, type);
		return p;
	}

	NFAnode* NFA::CreateNFANode(const NFAnode::Type type, const Character character) const
	{
		NFAnode* p = alloc.allocate(1, NFAnode::hint);
		if (p == nullptr) {
			throw Error::RuntimeError{ "No memory allocated for NFA node" };
		}
		alloc.construct(p, type, character);
		NFAnode::hint = p;
		return p;
	}

	NFA::NFA(const Character character)
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

	///----------------------------------------------------------------------------------------------------

	DFAnode* DFAnode::hint = nullptr;
	std::allocator<DFAnode> DFA::alloc;

	std::vector<DFAnode*> DFA::GetAllNodes() const
	{
		std::vector<DFAnode*> all;
		all.reserve(sz);
		AddNodeToSet(all, first);
		for (DFAnode* p : all) {
			p->mark = false;
		}
		return all;
	}

	void DFA::AddNodeToSet(std::vector<DFAnode*>& set, DFAnode* node) const
	{
		if (node == nullptr || node->mark == true) {
			return;
		}
		set.push_back(node);
		node->mark = true;
		for (auto& t : node->trans) {
			AddNodeToSet(set, t.second);
		}
	}

	void DFA::ReleaseResources()
	{
		std::vector<DFAnode*> nodes = GetAllNodes();
		for (DFAnode* p : nodes) {
			alloc.destroy(p);
			alloc.deallocate(p, 1);
		}
	}

	DFAnode* DFA::CreateDFANode(const bool accept) const
	{
		DFAnode* p = alloc.allocate(1, DFAnode::hint);
		if (p == nullptr) {
			throw Error::RuntimeError{ "No memory allocated for DFA node" };
		}
		DFAnode::hint = p;
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

	///----------------------------------------------------------------------------------------------------

	std::pair<const Character, const Regexp::TokenStream::TokenType> Regexp::TokenStream::GetToken()
	{
		if (pos >= s.size()) {
			return { 0, TokenType::EOS };
		}
		const Character ch = s[pos];
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

	std::set<const NFAnode*> Regexp::Delta(const std::set<const NFAnode*>& set, const Character ch) const
	{
		std::set<const NFAnode*> newSet;
		for (const NFAnode* p : set) {
			if (p->ch == ch) {
				newSet.emplace(p->succ1);
			}
		}
		return newSet;
	}

	std::set<const NFAnode*> Regexp::EpsilonClosure(const std::set<const NFAnode*>& set) const
	{
		std::set<const NFAnode*> newSet{ set.begin(), set.end() };
		for (const NFAnode* p : set) {
			AddNodesReachableViaEpsilonTransition(newSet, p);
		}
		return newSet;
	}

	void Regexp::AddNodesReachableViaEpsilonTransition(std::set<const NFAnode*>& set, const NFAnode* node) const
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

	bool Regexp::Equal(const std::set<const NFAnode*>& a, const std::set<const NFAnode*>& b) const
	{
		if (a.size() != b.size()) {
			return false;
		}
		std::set<const NFAnode*>::const_iterator itA = a.begin();
		std::set<const NFAnode*>::const_iterator itB = b.begin();
		while (itA != a.end()) {
			if (*itA != *itB) {
				return false;
			}
			++itA;
			++itB;
		}
		return true;
	}

	std::pair<std::vector<DFAnode*>, std::vector<DFAnode*>> Regexp::Split(const std::vector<DFAnode*>& set,
		const std::unordered_map<const DFAnode*, Number> numbers) const
	{
		std::vector<DFAnode*> first;
		std::vector<DFAnode*> second;
		for (const Character ch : alphabet) {
			std::vector<const DFAnode*> trans;					// transition to node on current ch
			trans.reserve(set.size());
			for (const DFAnode* p : set) {
				trans.push_back(FindTransition(p, ch));
			}
			first.clear();
			const bool target = TransitionExists(trans[0]);
			for (size_t i = 0; i < set.size(); ++i) {
				if (target == TransitionExists(trans[i])) {
					first.push_back(set[i]);
				}
				else {
					second.push_back(set[i]);
				}
			}
			if (second.size() != 0) {
				return { first, second };
			}
			if (target == false) {
				continue;
			}
			first.clear();
			const Number targetNum = numbers.find(trans[0])->second;
			for (size_t i = 0; i < set.size(); ++i) {
				if (targetNum == numbers.find(trans[i])->second) {
					first.push_back(set[i]);
				}
				else {
					second.push_back(set[i]);
				}
			}
			if (second.size() != 0) {
				return { first, second };
			}
		}
		return { set, second };
	}

	const DFAnode* Regexp::FindTransition(const DFAnode* node, const Character ch) const
	{
		TransitionTable::const_iterator it =
			std::lower_bound(node->trans.begin(), node->trans.end(), ch, LessTransitionCharacter{});
		if (it == node->trans.end() || it->first != ch) {
			return nullptr;
		}
		else {
			return it->second;
		}
	}

	DFA Regexp::CreateMinimalDFA(const SetPartition& sp,
		const std::unordered_map<const DFAnode*, Number> numbers) const
	{
		DFAnode::hint = nullptr;
		std::vector<DFAnode*> nodes;
		nodes.reserve(sp.size());
		for (const std::vector<DFAnode*>& set : sp) {
			nodes.push_back(dfa.CreateDFANode(set[0]->acc));
		}
		for (size_t i = 0; i < sp.size(); ++i) {
			for (const Transition& t : sp[i][0]->trans) {
				nodes[i]->trans.push_back(Transition{ t.first, nodes[numbers.find(t.second)->second] });
			}
		}
		DFA newDFA;
		newDFA.first = nodes[numbers.find(dfa.first)->second];
		newDFA.sz = nodes.size();
		return newDFA;
	}

#if PRINTFA
	void Regexp::MakeDFA()
	{
		std::cout << std::endl << "RE: " << this->source << std::endl;
		REtoNFA();
		PrintNFA(std::cout, *this);
		std::vector<DFAnode*> nodes = NFAtoDFA();
		std::cout << std::endl << "RE: " << this->source << std::endl;
		PrintDFA(std::cout, *this);
		MinimizeDFA(nodes);
		std::cout << std::endl << "RE: " << this->source << std::endl;
		PrintDFA(std::cout, *this);
	}
#else
	void Regexp::MakeDFA()
	{
		REtoNFA();
		MinimizeDFA(NFAtoDFA());
	}
#endif // PRINTFA

	// Thompson�s Construction
	// CHAPTER 2 Scanners, 2.4 FROM REGULAR EXPRESSION TO SCANNER, 2.4.2 Regular Expression to NFA: Thompson�s Construction
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
	std::vector<DFAnode*> Regexp::NFAtoDFA()
	{
		SubsetTableEntry first{ EpsilonClosure(std::set<const NFAnode*>{ {nfa.GetFirstNode()} }),
			std::vector<SubsetTableIndex>(alphabet.size(), noTransition) };
		SubsetTable table;
		table.push_back(first);
		std::queue<SubsetTableIndex> workList;
		workList.push(0);
		while (workList.size() > 0) {
			SubsetTableIndex i = workList.front();
			workList.pop();
			for (size_t k = 0; k < alphabet.size(); ++k) {
				const std::set<const NFAnode*> newSet = EpsilonClosure(Delta(table[i].state, alphabet[k]));
				if (newSet.size() == 0) {
					continue;
				}
				bool inTable = false;
				for (SubsetTableIndex j = 0; j < table.size(); ++j) {
					if (Equal(table[j].state, newSet)) {
						table[i].trans[k] = j;
						inTable = true;
					}
				}
				if (inTable == false) {
					SubsetTableEntry newEntry{ newSet,
						std::vector<SubsetTableIndex>(alphabet.size(), noTransition) };
					table.push_back(newEntry);
					SubsetTableIndex newIndex = table.size() - 1;
					workList.push(newIndex);
					table[i].trans[k] = newIndex;
				}
			}
		}
		std::vector<DFAnode*> nodes;
		nodes.reserve(table.size());
		NFAnode* accepted = nfa.GetLastNode();
		for (SubsetTableEntry& entry : table) {
			std::set<const NFAnode*>::iterator it = entry.state.find(accepted);
			nodes.push_back(dfa.CreateDFANode((it == entry.state.end()) ? false : true));
		}
		for (SubsetTableIndex i = 0; i < table.size(); ++i) {
			SubsetTableEntry& entry = table[i];
			for (size_t j = 0; j < entry.trans.size(); ++j) {
				SubsetTableIndex index = entry.trans[j];
				if (index == noTransition) {
					continue;
				}
				nodes[i]->trans.push_back(Transition{ alphabet[j], nodes[index] });
			}
		}
		dfa.first = nodes[0];
		dfa.sz = table.size();
		return nodes;
	}

	// Hopcroft�s Algorithm
	// CHAPTER 2 Scanners, 2.4 FROM REGULAR EXPRESSION TO SCANNER, 2.4.4 DFA to Minimal DFA: Hopcroft�s Algorithm
	// FIGURE 2.9 DFA Minimization Algorithm
	void Regexp::MinimizeDFA(const std::vector<DFAnode*> nodes)
	{
		constexpr size_t qty = 2;								// initial quantity of SP
		constexpr Number numA = 0;								// number of the std::vector<DFAnode*> with accepting states
		constexpr Number numNA = 1;								// number of the std::vector<DFAnode*> with nonaccepting states
		Number num{ numNA };									// number of the last std::vector<DFAnode*>
		std::unordered_map<const DFAnode*, Number> numbers;		// numbers of std::vector<DFAnode*>
		SetPartition sp;										// set partition
		SetPartition temp;										// set partition
		sp.reserve(nodes.size());
		temp.reserve(nodes.size());
		temp.resize(qty);
		for (DFAnode* ptr : nodes) {
			if (ptr->acc == true) {
				numbers.emplace(ptr, numA);
				temp[numA].push_back(ptr);
			}
			else {
				numbers.emplace(ptr, numNA);
				temp[numNA].push_back(ptr);
			}
		}
		while (sp.size() != temp.size()) {
			sp = temp;
			temp.clear();
			for (const std::vector<DFAnode*>& set : sp) {
				if (set.size() == 1) {
					temp.push_back(set);
					continue;
				}
				std::pair<std::vector<DFAnode*>, std::vector<DFAnode*>> pair = Split(set, numbers);
				temp.push_back(pair.first);
				if (pair.second.size() != 0) {
					temp.push_back(pair.second);
					++num;
					for (DFAnode* ptr : pair.second) {
						numbers[ptr] = num;
					}
				}
			}
		}
		if (nodes.size() != sp.size()) {
			dfa = CreateMinimalDFA(sp, numbers);
		}
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

	void Regexp::ThrowInvalidRegex(const size_t position) const
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
