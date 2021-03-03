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
#include<cctype>
#include"printFA.hpp"
#include"../Error/error.hpp"
#include"regexpr.hpp"

namespace RE
{
	namespace Strings
	{
		const char* eof{ "EOF" };
		const char* del{ "DEL" };
		const char* asciiCC[] = {
			"NULL",		// NULL
			"SOH",		// Start Of Header
			"STX",		// Start Of Text
			"ETX",		// End Of Text
			"EOT",		// End Of Transmission
			"ENQ",		// ENQuiry
			"ACK",		// ACKnowledge
			"BEL",		// BELl
			"BS",		// BackSpace
			"HT",		// Horizontal Tabulation
			"LF",		// Line Feed
			"VT",		// Vertical Tabulation
			"FF",		// Form Feed
			"CR",		// Carriage Return
			"SO",		// Shift Out
			"SI",		// Shift In
			"DLE",		// Data Link Escape
			"DC1",		// Device Control 1
			"DC2",		// Device Control 2
			"DC3",		// Device Control 3
			"DC4",		// Device Control 4
			"NAK",		// Negative AcKnowledge
			"SYN",		// SYNchronous Idle
			"ETB",		// End of Transmission Block
			"CAN",		// CANcel
			"EM",		// End of Medium
			"SUB",		// SUBstitute
			"ESC",		// ESCape
			"FS",		// File Separator
			"GS",		// Group Separator
			"RS",		// Record Separator
			"US"		// Unit Separator
		};
	}
}

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

	void NFA::ReleaseResources() const
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

	NFA NFA::CreateCopy() const
	{
		const std::vector<NFAnode*> nodes = GetAllNodes();
		std::unordered_map<const NFAnode*, const Index> indexes;
		std::vector<NFAnode*> nodeCopies;
		NFA nfaCopy{};
		for (Index i = 0; i < nodes.size(); ++i) {
			const NFAnode* p = nodes[i];
			indexes.emplace(p, i);
			nodeCopies.push_back(CreateNFANode(p->ty, p->ch));
			if (p->ty == NFAnode::Type::ACCEPT) {
				nfaCopy.last = nodeCopies[i];
			}
		}
		if (nodes.size() != indexes.size()) {
			throw Error::RuntimeError{ "CreateCopy(): nodes.size() != indexes.size()" };
		}
		for (Index i = 0; i < nodes.size(); ++i) {
			const NFAnode* p = nodes[i];
			if (p->succ1 != nullptr) {
				nodeCopies[i]->succ1 = nodeCopies[indexes.find(p->succ1)->second];
			}
			if (p->succ2 != nullptr) {
				nodeCopies[i]->succ2 = nodeCopies[indexes.find(p->succ2)->second];
			}
		}
		nfaCopy.first = nodeCopies[0];
		nfaCopy.sz = nodeCopies.size();
		return nfaCopy;
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

	// the function creates a positive (1 or more times) closure for NFA
	void NFA::ClosurePositive()
	{
		NFAnode* newFirst = CreateNFANode(NFAnode::Type::EPSILON);
		NFAnode* newLast = CreateNFANode(NFAnode::Type::ACCEPT);
		newFirst->succ1 = first;
		last->succ1 = first;
		last->succ2 = newLast;
		last->ty = NFAnode::Type::EPSILON;
		first = newFirst;
		last = newLast;
		sz += 2;
	}

	// the function creates a "binary" (0 or 1 time) closure for NFA
	void NFA::ClosureBinary()
	{
		NFAnode* newFirst = CreateNFANode(NFAnode::Type::EPSILON);
		newFirst->succ1 = first;
		newFirst->succ2 = last;
		first = newFirst;
		sz += 1;
	}

	// the function creates a custom ({INT}, {INT,}, {MIN,MAX}) closure for NFA
	// for {INT} INT >= 1; for {INT,} INT >= 0; for {MIN,MAX} MIN >= 0, MIN < MAX
	void NFA::ClosureCustom(const int min, const int max, const Constants::ClosureType ty)
	{
		switch (ty) {
		case Constants::ClosureType::FINITE: {
			if (min < 1) {
				throw Error::InvalidRegex{ "For {INT}, INT must be greater than or equal to 1" };
			}
			std::vector<NFA> copies;
			for (int i = 1; i < min; ++i) {
				copies.push_back(CreateCopy());
			}
			for (NFA& nfa : copies) {
				this->Concatenate(nfa);
			}
			break;
		}
		case Constants::ClosureType::INFITITE: {
			if (min < 0) {
				throw Error::InvalidRegex{ "For {INT,}, INT must be greater than or equal to 0" };
			}
			else if (min == 0) {
				this->ClosureKleene();
			}
			else if (min == 1) {
				this->ClosurePositive();
			}
			else {
				std::vector<NFA> copies;
				for (int i = 1; i < min; ++i) {
					copies.push_back(CreateCopy());
				}
				copies[copies.size() - 1].ClosurePositive();
				for (NFA& nfa : copies) {
					this->Concatenate(nfa);
				}
			}
			break;
		}
		case Constants::ClosureType::RANGE: {
			if (min < 0 || min >= max) {
				throw Error::InvalidRegex{ "For {MIN,MAX}, MIN must be greater than or equal to 0, "
										   "MIN must be less than MAX" };
			}
			std::vector<NFA> copies;
			for (int i = 1; i < max; ++i) {
				copies.push_back(CreateCopy());
			}
			if (min == 0) {
				this->ClosureBinary();
				for (size_t i = 0; i < copies.size(); ++i) {
					copies[i].ClosureBinary();
				}
			}
			else {
				for (size_t i = min - 1; i < copies.size(); ++i) {
					copies[i].ClosureBinary();
				}
			}
			for (NFA& nfa : copies) {
				this->Concatenate(nfa);
			}
			break;
		}
		default:
			throw Error::RuntimeError{ "ClosureCustom(): Unknown ClosureType" };
		}
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

	void DFA::ReleaseResources() const
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

	Regexp::TokenStream& Regexp::TokenStream::operator=(TokenStream&& other)
	{
		if (this == &other) {
			return *this;
		}
		s = other.s;
		pos = other.pos;
		return *this;
	}

	Regexp::TokenStream::TokenType Regexp::TokenStream::GetTokenType(const Character ch) const
	{
		switch (ch) {
		case SPEC_LPAR:
		case SPEC_RPAR:
		case SPEC_STAR:
		case SPEC_PLUS:
		case SPEC_QUESTION:
		case SPEC_LBRACKET:
		case SPEC_BSLASH:
		case SPEC_RBRACKET:
		case SPEC_LBRACE:
		case SPEC_BAR:
		case SPEC_RBRACE:
			return TokenType::SPECIAL;
		default:
			return TokenType::LITERAL;
		}
	}

	//std::pair<Character, Regexp::TokenStream::TokenType> Regexp::TokenStream::GetPreviousToken() const
	//{
	//	if (pos == 0) {
	//		throw Error::RuntimeError{ "No previous token" };
	//	}
	//	const Character ch = s[pos - 1];
	//	return { ch, GetTokenType(ch) };
	//}

	std::pair<Character, Regexp::TokenStream::TokenType> Regexp::TokenStream::GetToken() const
	{
		if (pos >= s.size()) {
			return { CHARFL_NOTCHAR, TokenType::EOS };
		}
		const Character ch = s[pos];
		return { ch, GetTokenType(ch) };
	}

	std::set<const NFAnode*> Regexp::Delta(const std::set<const NFAnode*>& set, const Character ch) const
	{
		std::set<const NFAnode*> newSet;
		for (const NFAnode* p : set) {
			if (p->ty == NFAnode::Type::LITERAL && p->ch == ch) {
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
		const std::unordered_map<const DFAnode*, Index>& indexes) const
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
			for (Index i = 0; i < set.size(); ++i) {
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
			const Index targetNum = indexes.find(trans[0])->second;
			for (Index i = 0; i < set.size(); ++i) {
				if (targetNum == indexes.find(trans[i])->second) {
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

	void Regexp::AssignNumber(const std::vector<DFAnode*>& set, const Index num,
		std::unordered_map<const DFAnode*, Index>& indexes) const
	{
		for (const DFAnode* p : set) {
			indexes[p] = num;
		}
	}

	DFA Regexp::CreateMinimalDFA(const SetPartition& sp,
		const std::unordered_map<const DFAnode*, Index>& indexes) const
	{
		DFAnode::hint = nullptr;
		std::vector<DFAnode*> nodes;
		nodes.reserve(sp.size());
		for (const std::vector<DFAnode*>& set : sp) {
			nodes.push_back(dfa.CreateDFANode(set[0]->acc));
		}
		for (Index i = 0; i < sp.size(); ++i) {
			for (const Transition& t : sp[i][0]->trans) {
				nodes[i]->trans.push_back(Transition{ t.first, nodes[indexes.find(t.second)->second] });
			}
		}
		DFA newDFA;
		newDFA.first = nodes[indexes.find(dfa.first)->second];
		newDFA.sz = nodes.size();
		return newDFA;
	}

	void RE::Regexp::CheckNFA() const
	{
		std::set<const NFAnode*> set;
		AddNodesReachableViaEpsilonTransition(set, nfa.GetFirstNode());
		if (set.find(nfa.GetLastNode()) != set.end()) {
			ThrowInvalidRegex("This regular expression is invalid. It matches any string");
		}
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

	// Thompson’s Construction
	// CHAPTER 2 Scanners, 2.4 FROM REGULAR EXPRESSION TO SCANNER, 2.4.2 Regular Expression to NFA: Thompson’s Construction
	void Regexp::REtoNFA()
	{
		nfa = PGoal();
		if (token.second != Regexp::TokenStream::TokenType::EOS) {
			ThrowInvalidRegex(ts.GetPosition());
		}
		CheckNFA();
		alphabet.insert(alphabet.end(), alphabetTemp.begin(), alphabetTemp.end());
		std::sort(alphabet.begin(), alphabet.end());
		alphabetTemp.clear();
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
		nfa = NFA{ CHARFL_NOTCHAR };
		dfa.first = nodes[0];
		dfa.sz = table.size();
		return nodes;
	}

	// Hopcroft’s Algorithm
	// CHAPTER 2 Scanners, 2.4 FROM REGULAR EXPRESSION TO SCANNER, 2.4.4 DFA to Minimal DFA: Hopcroft’s Algorithm
	// FIGURE 2.9 DFA Minimization Algorithm
	void Regexp::MinimizeDFA(const std::vector<DFAnode*> nodes)
	{
		constexpr size_t qty = 2;								// initial quantity of SP
		constexpr Index indexA = 0;								// index of the std::vector<DFAnode*> with accepting states
		constexpr Index indexNA = 1;							// index of the std::vector<DFAnode*> with nonaccepting states
		Index index{ indexNA };									// index of the last std::vector<DFAnode*>
		std::unordered_map<const DFAnode*, Index> indexes;		// indexes of std::vector<DFAnode*>
		SetPartition sp;										// set partition
		SetPartition temp;										// set partition
		sp.reserve(nodes.size());
		temp.reserve(nodes.size());
		temp.resize(qty);
		for (DFAnode* ptr : nodes) {
			if (ptr->acc == true) {
				indexes.emplace(ptr, indexA);
				temp[indexA].push_back(ptr);
			}
			else {
				indexes.emplace(ptr, indexNA);
				temp[indexNA].push_back(ptr);
			}
		}
		while (sp.size() != temp.size()) {
			sp = temp;
			temp.clear();
			temp.resize(sp.size());
			for (Index i = 0; i < sp.size(); ++i) {
				const std::vector<DFAnode*>& set = sp[i];
				if (set.size() == 1) {
					temp[i] = set;
					continue;
				}
				std::pair<std::vector<DFAnode*>, std::vector<DFAnode*>> pair = Split(set, indexes);
				temp[i] = pair.first;
				if (pair.second.size() != 0) {
					AssignNumber(pair.second, ++index, indexes);
					temp.push_back(pair.second);
				}
			}
		}
		if (nodes.size() != sp.size()) {
			dfa = CreateMinimalDFA(sp, indexes);
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
			case SPEC_RPAR:
				return;
			case SPEC_BAR:
				NextToken();
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
		NFA a = PTerm();
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
			case SPEC_LPAR:
			case SPEC_LBRACKET:
			case SPEC_BSLASH:
				a.Concatenate(PTerm());
				PConcatenationPrime(a);
				return;
			case SPEC_RPAR:
			case SPEC_BAR:
				return;
			default:
				break;
			}
		case Regexp::TokenStream::TokenType::LITERAL:
			a.Concatenate(PTerm());
			PConcatenationPrime(a);
			return;
		default:
			break;
		}
		ThrowInvalidRegex(ts.GetPosition());
	}

	// parse Term
	NFA Regexp::PTerm()
	{
		NFA a = PBlock();
		PClosure(a);
		return a;
	}

	// parse Block
	NFA Regexp::PBlock()
	{
		switch (token.second) {
		case Regexp::TokenStream::TokenType::SPECIAL:
			switch (token.first) {
			case SPEC_LPAR: {
				NextToken();
				NFA a = PAlternation();
				if (token.first == SPEC_RPAR) {
					NextToken();
					return a;
				}
				break;
			}
			case SPEC_LBRACKET: {
				NextToken();
				NFA a = PCharacterClass();
				if (token.first == SPEC_RBRACKET) {
					NextToken();
					return a;
				}
				break;
			}
			default:
				return PAtom();
			}
			break;
		default:
			return PAtom();
		}
		ThrowInvalidRegex(ts.GetPosition());
	}

	// parse CharacterClass
	NFA Regexp::PCharacterClass()
	{
		const CharacterFlag flag = PNegation() ? CHARFL_NEGATED : CHARFL_NOFLAGS;
		return PCharacterClassRange(flag, Constants::AtomType::CHARCLASS);
	}

	// parse Negation
	bool Regexp::PNegation()
	{
		if (token.first == LIT_CARET) {
			NextToken();
			return true;
		}
		return false;
	}

	// parse CharacterClassRange
	NFA Regexp::PCharacterClassRange(const CharacterFlag flags, const Constants::AtomType type)
	{
		NFA a = PAtom(flags, type);
		PCharacterClassRangePrime(flags, type, a);
		return a;
	}

	// parse CharacterClassRange'
	void Regexp::PCharacterClassRangePrime(const CharacterFlag flags, const Constants::AtomType type, NFA& a)
	{
		switch (token.second) {
		case Regexp::TokenStream::TokenType::EOS:
			return;
		case Regexp::TokenStream::TokenType::SPECIAL:
			switch (token.first) {
			case SPEC_RBRACKET:
				return;
			default:
				a.Alternate(PAtom(flags, type));
				PCharacterClassRangePrime(flags, type, a);
				return;
			}
			break;
		case Regexp::TokenStream::TokenType::LITERAL:
			if (token.first == LIT_HYPHEN) {
				const Character firstC = last;
				NextToken();
				NFA b = PAtom(flags, type);
				const Character lastC = last;
				CheckRange(firstC, lastC);
				Character ch = firstC;
				while (++ch < lastC) {
					AddToAlphabet(ch);
					a.Alternate(NFA{ ch });
				}
				a.Alternate(b);
				PCharacterClassRangePrime(flags, type, a);
				return;
			}
			a.Alternate(PAtom(flags, type));
			PCharacterClassRangePrime(flags, type, a);
			return;
		default:
			break;
		}
		ThrowInvalidRegex(ts.GetPosition());
	}

	void RE::Regexp::CheckRange(Character firstC, Character lastC)
	{
		firstC = ERASEALLCHARACTERFLAGS(firstC);
		lastC = ERASEALLCHARACTERFLAGS(lastC);
		if (firstC >= lastC) {
			std::string message{ "Range '" };
			message += GetGlyph(firstC, Constants::GlyphType::ASINPUT);
			message += "-";
			message += GetGlyph(lastC, Constants::GlyphType::ASINPUT);
			message += "' is invalid";
			ThrowInvalidRegex(message);
		}
	}

	// parse Atom
	NFA RE::Regexp::PAtom(const CharacterFlag flags, const Constants::AtomType type)
	{
		switch (token.second) {
		case Regexp::TokenStream::TokenType::EOS:
			break;
		case Regexp::TokenStream::TokenType::SPECIAL:
			switch (token.first) {
			case SPEC_BSLASH:
				NextToken();
				return PEscape(flags, type);
			case SPEC_LBRACKET:
			case SPEC_RBRACKET:
				break;
			default:
				if (type == Constants::AtomType::CHARCLASS) {
					last = token.first;
					last |= flags;
					NextToken();
					AddToAlphabet(last);
					return NFA{ last };
				}
				break;
			}
			break;
		case Regexp::TokenStream::TokenType::LITERAL: {
			last = token.first;
			last |= flags;
			NextToken();
			AddToAlphabet(last);
			return NFA{ last };
		}
		default:
			break;
		}
		ThrowInvalidRegex(ts.GetPosition());
	}

	// parse Escape
	NFA RE::Regexp::PEscape(const CharacterFlag flags, const Constants::AtomType type)
	{
		switch (token.second) {
		case Regexp::TokenStream::TokenType::EOS:
			break;
		case Regexp::TokenStream::TokenType::SPECIAL: {
			last = token.first;
			last |= flags;
			NextToken();
			AddToAlphabet(last);
			return NFA{ last };
		}
		case Regexp::TokenStream::TokenType::LITERAL: {
			last = CHARFL_NOTCHAR;
			if (!PIsEscape(last, type)) {
				last = token.first;
			}
			last |= flags;
			NextToken();
			AddToAlphabet(last);
			return NFA{ last };
		}
		default:
			break;
		}
		ThrowInvalidRegex(ts.GetPosition());
	}

	// parse ESCAPE
	bool RE::Regexp::PIsEscape(Character& ch, const Constants::AtomType type)
	{
		switch (token.first) {
		case ESC_NULL:
			ch = CTRL_NULL;
			return true;
		case ESC_B:
			if (type == Constants::AtomType::CHARCLASS) {
				ch = CTRL_BACKSPACE;
				return true;
			}
			return false;
		case ESC_HTAB:
			ch = CTRL_HTAB;
			return true;
		case ESC_NEWLINE:
			ch = CTRL_NEWLINE;
			return true;
		case ESC_VTAB:
			ch = CTRL_VTAB;
			return true;
		case ESC_FORMFEED:
			ch = CTRL_FORMFEED;
			return true;
		case ESC_CRETURN:
			ch = CTRL_CRETURN;
			return true;
		default:
			return false;
		}
	}

	// parse Closure
	void Regexp::PClosure(NFA& a)
	{
		switch (token.second) {
		case Regexp::TokenStream::TokenType::EOS:
			return;
		case Regexp::TokenStream::TokenType::SPECIAL:
			switch (token.first) {
			case SPEC_STAR:
				a.ClosureKleene();
				NextToken();
				return;
			case SPEC_PLUS:
				a.ClosurePositive();
				NextToken();
				return;
			case SPEC_QUESTION:
				a.ClosureBinary();
				NextToken();
				return;
			case SPEC_LBRACE: {
				NextToken();
				int min{ -1 };
				int max{ -1 };
				Constants::ClosureType type{ Constants::ClosureType::NOTYPE };
				PCount(min, max, type);
				if (token.first != SPEC_RBRACE) {
					break;
				}
				try {
					a.ClosureCustom(min, max, type);
				}
				catch (const Error::InvalidRegex& e) {
					ThrowInvalidRegex(e.what());
				}
				NextToken();
				return;
			}
			case SPEC_LPAR:
			case SPEC_RPAR:
			case SPEC_LBRACKET:
			case SPEC_BSLASH:
			case SPEC_BAR:
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

	// parse Count
	void Regexp::PCount(int& min, int& max, Constants::ClosureType& ty)
	{
		switch (token.second) {
		case Regexp::TokenStream::TokenType::EOS:
			break;
		case Regexp::TokenStream::TokenType::SPECIAL:
			break;
		case Regexp::TokenStream::TokenType::LITERAL:
			if (isdigit(token.first)) {
				min = PGetInteger();
				ty = Constants::ClosureType::FINITE;
				PCountMore(max, ty);
				return;
			}
			break;
		default:
			break;
		}
		ThrowInvalidRegex(ts.GetPosition());
	}

	// parse CountMore
	void Regexp::PCountMore(int& max, Constants::ClosureType& ty)
	{
		switch (token.second) {
		case Regexp::TokenStream::TokenType::EOS:
			return;
		case Regexp::TokenStream::TokenType::SPECIAL:
			switch (token.first) {
			case SPEC_RBRACE:
				return;
			default:
				break;
			}
			break;
		case Regexp::TokenStream::TokenType::LITERAL:
			switch (token.first) {
			case LIT_COMMA:
				ty = Constants::ClosureType::INFITITE;
				NextToken();
				PMax(max, ty);
				return;
			default:
				break;
			}
			break;
		default:
			break;
		}
		ThrowInvalidRegex(ts.GetPosition());
	}

	// parse Max
	void Regexp::PMax(int& max, Constants::ClosureType& ty)
	{
		switch (token.second) {
		case Regexp::TokenStream::TokenType::EOS:
			return;
		case Regexp::TokenStream::TokenType::SPECIAL:
			switch (token.first) {
			case SPEC_RBRACE:
				return;
			default:
				break;
			}
			break;
		case Regexp::TokenStream::TokenType::LITERAL:
			if (isdigit(token.first)) {
				max = PGetInteger();
				ty = Constants::ClosureType::RANGE;
				return;
			}
			break;
		default:
			break;
		}
		ThrowInvalidRegex(ts.GetPosition());
	}

	// parse INTEGER
	int Regexp::PGetInteger()
	{
		std::string s;
		while (isdigit(token.first)) {
			s += token.first;
			NextToken();
		}
		return atoi(s.c_str());
	}

	void Regexp::ThrowInvalidRegex(const size_t position) const
	{
		std::string message{ "Invalid character '" };
		if (position < source.size()) {
			message += std::string{ source, position, 1 };
		}
		else {
			message += Strings::eof;
		}
		message += "' was encountered after substring '";
		message += std::string{ source, 0, position } + "'. Regular expression: ";
		message += source;
		throw Error::InvalidRegex{ message };
	}

	void Regexp::ThrowInvalidRegex(const std::string& message) const
	{
		std::string fullMessage{ message };
		fullMessage += ". Regular expression: ";
		fullMessage += source;
		throw Error::InvalidRegex{ fullMessage };
	}

	Regexp::Regexp(const REstring& string)
		: source{ string }, ts{ source }, nfa{ CHARFL_NOTCHAR }, last{ CHARFL_NOTCHAR }
	{
		if (string.size() == 0) {
			throw Error::InvalidRegex{ "Empty regular expression " };
		}
		MakeDFA();
	}

	void RE::Regexp::PutRE(const REstring& string)
	{
		if (string.size() == 0) {
			throw Error::InvalidRegex{ "Empty regular expression " };
		}
		source = string;
		ts = TokenStream{ source };
		alphabetTemp = std::set<Character>{};
		alphabet = std::vector<Character>{};
		nfa = NFA{ CHARFL_NOTCHAR };
		dfa = DFA{};
		last = CHARFL_NOTCHAR;
		MakeDFA();
	}

	std::istream& operator>>(std::istream& is, REstring& string)
	{
		REstring line;
		while (is) {
			getline(is, line);
			if (is) {
				string += line;
			}
		}
		return is;
	}

	std::string GetGlyph(const Character ch, const Constants::GlyphType type)
	{
		const Character c = ERASEALLCHARACTERFLAGS(ch);
		switch (type) {
		case Constants::GlyphType::ASINPUT: {
			switch (c) {
			case CTRL_NULL:
				return std::string{ '\\' } + char(ESC_NULL);
			case CTRL_HTAB:
				return std::string{ '\\' } + char(ESC_HTAB);
			case CTRL_NEWLINE:
				return std::string{ '\\' } + char(ESC_NEWLINE);
			case CTRL_VTAB:
				return std::string{ '\\' } + char(ESC_VTAB);
			case CTRL_FORMFEED:
				return std::string{ '\\' } + char(ESC_FORMFEED);
			case CTRL_CRETURN:
				return std::string{ '\\' } + char(ESC_CRETURN);
			default:
				return std::string{ char(c) };
			}
		}
		case Constants::GlyphType::ASOUTPUT: {
			std::string s;
			if (ch & CHARFL_NEGATED) {
				s += LIT_CARET;
			}
			if (c >= ASCIICC_FIRST && c <= ASCIICC_LAST) {
				return s + Strings::asciiCC[c];
			}
			else if (c == ASCIICC_DEL) {
				return s + Strings::del;
			}
			else {
				return s + '\'' + char(c) + '\'';
			}
		}
		default:
			return std::string{};
		}
	}
}
