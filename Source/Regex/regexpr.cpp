
// Copyright (c) 2021 Vitaly Dikov
// 
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

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
#include<limits>
#include"regexpr_config.hpp"
#include"../Error/error.hpp"
#include"regexpr.hpp"

namespace RE
{
    namespace Strings
    {
        const char* eof{ "EOF" };
        const char* del{ "DEL" };
        const char* asciiCC[] = {
            "NULL",     // NULL
            "SOH",      // Start Of Header
            "STX",      // Start Of Text
            "ETX",      // End Of Text
            "EOT",      // End Of Transmission
            "ENQ",      // ENQuiry
            "ACK",      // ACKnowledge
            "BEL",      // BELl
            "BS",       // BackSpace
            "HT",       // Horizontal Tabulation
            "LF",       // Line Feed
            "VT",       // Vertical Tabulation
            "FF",       // Form Feed
            "CR",       // Carriage Return
            "SO",       // Shift Out
            "SI",       // Shift In
            "DLE",      // Data Link Escape
            "DC1",      // Device Control 1
            "DC2",      // Device Control 2
            "DC3",      // Device Control 3
            "DC4",      // Device Control 4
            "NAK",      // Negative AcKnowledge
            "SYN",      // SYNchronous Idle
            "ETB",      // End of Transmission Block
            "CAN",      // CANcel
            "EM",       // End of Medium
            "SUB",      // SUBstitute
            "ESC",      // ESCape
            "FS",       // File Separator
            "GS",       // Group Separator
            "RS",       // Record Separator
            "US"        // Unit Separator
        };
    }


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

    bool operator==(const DFA& left, const DFA& right)
    {
        if (left.sz != right.sz) {
            return false;
        }
        if (left.first == nullptr && right.first == nullptr) {
            return true;
        }
        std::set<const DFAnode*> visited;
        if (left.first == nullptr || right.first == nullptr || !Equal(left.first, right.first, visited)) {
            return false;
        }
        return true;
    }

    bool Equal(const DFAnode* left, const DFAnode* right, std::set<const DFAnode*>& visited)
    {
        visited.emplace(left);
        if (left->acc != right->acc || left->mark != right->mark) {
            return false;
        }
        if (left->trans.size() != right->trans.size()) {
            return false;
        }
        for (size_t i = 0; i < left->trans.size(); ++i) {
            if (left->trans[i].first != right->trans[i].first) {
                return false;
            }
            if (visited.find(left->trans[i].second) == visited.end()) {
                if (!Equal(left->trans[i].second, right->trans[i].second, visited)) {
                    return false;
                }
            }
        }
        return true;
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

    // 1 substring == 1 sequence of tokens from which 1 atom is created
    // qty == the quantity of last atoms for which the total substring will be obtained 
    UString Regexp::TokenStream::GetSubstring(const size_t qty) const
    {
        UString s;
        if (tss.size() == 0 || qty == 0) {
            return s;
        }
        int i = tpos;
        int counter = qty;
        while (counter > 0) {
            --counter;
            i = (i <= 0) ? tss.size() - 1 : i - 1;
        }
        while (counter < qty) {
            ++counter;
            s += tss[i];
            i = (i == tss.size() - 1) ? 0 : i + 1;
        }
        return s;
    }

    inline Regexp::TokenStream::TokenType Regexp::TokenStream::GetTokenType(const Character ch) const
    {
        switch (ch) {
        case SPEC_LPAR:
        case SPEC_RPAR:
        case SPEC_STAR:
        case SPEC_PLUS:
        case SPEC_DOT:
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

    inline std::pair<Character, Regexp::TokenStream::TokenType> Regexp::TokenStream::GetToken() const
    {
        if (pos >= s.size()) {
            return { Constants::notCharacter, TokenType::EOS };
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
            std::vector<const DFAnode*> trans;                  // transition to node on current ch
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
    
    // returns a pointer to the 'DFAnode' if there is a transition for this character,
    // otherwise it returns 'nullptr'
    inline DFAnode* Regexp::TakeStepThroughDFA(const DFAnode* current, const Character ch) const
    {
        return const_cast<DFAnode*>(FindTransition(current, ch));
    }
    
    // returns an array of 'DFAnode' pointers if there is a transition for this character,
    // otherwise it returns an empty array
    std::vector<DFAnode*> Regexp::TakeStepThroughDFA(std::vector<DFAnode*>& currentState, const Character ch) const
    {
        constexpr size_t defaultSize = 8;
        std::vector<DFAnode*> transitions;
        transitions.reserve(defaultSize);
        for (const DFAnode* p : currentState) {
            DFAnode* node = const_cast<DFAnode*>(FindTransition(p, ch));
            if (node) {
                transitions.push_back(node);
            }
            TransitionTable::const_iterator itBeg =
                std::lower_bound(p->trans.begin(), p->trans.end(), Constants::firstNegated, LessTransitionCharacter{});
            if (itBeg != p->trans.end()) {
                auto it = itBeg;
                std::vector<DFAnode*> negated;
                negated.reserve(defaultSize);
                while (it != p->trans.end()) {
                    negated.push_back(it->second);
                    ++it;
                }
                std::sort(negated.begin(), negated.end());
                DFAnode* excluded = const_cast<DFAnode*>(FindTransition(p, FLAGS_SET(ch, CHARFL_NEGATED)));
                DFAnode* prev = nullptr;
                for (DFAnode* cur : negated) {
                    if (prev != cur) {
                        if (cur != excluded) {
                            transitions.push_back(cur);
                        }
                        prev = cur;
                    }
                }
            }
        }
        return transitions;
    }

    inline bool Regexp::IsNewLineSymbol(const Character ch) const
    {
        switch (ch) {
        case CTRL_LF:
        case CTRL_CR:
        case CTRL_LS:
        case CTRL_PS:
            return true;
        default:
            return false;
        }
    }

    inline void Regexp::AdjustPositions(
        std::u32string::const_iterator begin,
        std::u32string::const_iterator end,
        size_t& line,
        size_t& pos) const
    {
        while (begin != end) {
            if (IsNewLineSymbol(*begin)) {
                ++line;
                pos = 1;
            }
            else {
                ++pos;
            }
            ++begin;
        }
    }

#if REGEX_PRINT_FA_STATE
    void Regexp::MakeDFA()
    {
        std::cout << std::endl << "RE: " << GetGlyph(this->source) << std::endl;
        REtoNFA();
        PrintNFA(std::cout, *this);
        std::vector<DFAnode*> nodes = NFAtoDFA();
        std::cout << std::endl << "RE: " << GetGlyph(this->source) << std::endl;
        PrintDFA(std::cout, *this);
        MinimizeDFA(nodes);
        std::cout << std::endl << "RE: " << GetGlyph(this->source) << std::endl;
        PrintDFA(std::cout, *this);
        SetFlags();
    }
#else
    void Regexp::MakeDFA()
    {
        REtoNFA();
        MinimizeDFA(NFAtoDFA());
        SetFlags();
    }
#endif // REGEX_PRINT_FA_STATE

    // Thompson�s Construction
    // CHAPTER 2 Scanners, 2.4 FROM REGULAR EXPRESSION TO SCANNER, 2.4.2 Regular Expression to NFA: Thompson�s Construction
    void Regexp::REtoNFA()
    {
        nfa = PGoal();
        if (token.second != Regexp::TokenStream::TokenType::EOS) {
            ThrowInvalidRegexCharacter(ts.GetPosition());
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
        nfa = NFA{ Constants::notCharacter };
        dfa.first = nodes[0];
        dfa.sz = table.size();
        return nodes;
    }

    // Hopcroft�s Algorithm
    // CHAPTER 2 Scanners, 2.4 FROM REGULAR EXPRESSION TO SCANNER, 2.4.4 DFA to Minimal DFA: Hopcroft�s Algorithm
    // FIGURE 2.9 DFA Minimization Algorithm
    void Regexp::MinimizeDFA(const std::vector<DFAnode*> nodes)
    {
        constexpr size_t qty = 2;                               // initial quantity of SP
        constexpr Index indexA = 0;                             // index of the std::vector<DFAnode*> with accepting states
        constexpr Index indexNA = 1;                            // index of the std::vector<DFAnode*> with nonaccepting states
        Index index{ indexNA };                                 // index of the last std::vector<DFAnode*>
        std::unordered_map<const DFAnode*, Index> indexes;      // indexes of std::vector<DFAnode*>
        SetPartition sp;                                        // set partition
        SetPartition temp;                                      // set partition
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

    void Regexp::SetFlags()
    {
        for (const Character ch : alphabet) {
            if (FLAG_IS_SET(ch, CHARFL_NEGATED)) {
                fl = FLAGS_SET(fl, REGFL_NEGATED);
                break;
            }
        }
    }

    // parse Goal
    NFA Regexp::PGoal()
    {
        NextToken();
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
        ThrowInvalidRegexCharacter(ts.GetPosition());
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
        ThrowInvalidRegexCharacter(ts.GetPosition());
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
        ThrowInvalidRegexCharacter(ts.GetPosition());
    }

    // parse CharacterClass
    NFA Regexp::PCharacterClass()
    {
        const CharacterFlags flag = PNegation() ? CHARFL_NEGATED : CHARFL_NOFLAGS;
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
    NFA Regexp::PCharacterClassRange(const CharacterFlags flags, const Constants::AtomType type)
    {
        NFA a = PAtom(flags, type);
        PCharacterClassRangePrime(flags, type, a);
        return a;
    }

    // parse CharacterClassRange'
    void Regexp::PCharacterClassRangePrime(const CharacterFlags flags, const Constants::AtomType type, NFA& a)
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
                const Character firstChar = last;
                NextToken();
                NFA b = PAtom(flags, type);
                const Character lastChar = last;
                CheckRange(firstChar, lastChar);
                Character ch = firstChar;
                while (++ch < lastChar) {
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
        ThrowInvalidRegexCharacter(ts.GetPosition());
    }

    void RE::Regexp::CheckRange(Character first, Character last)
    {
        constexpr size_t qty = 3;
        first = FLAGS_UNSET(first, CHARFL_ALLFLAGS);
        last = FLAGS_UNSET(last, CHARFL_ALLFLAGS);
        if (first >= last) {
            ThrowInvalidRegexRange(ts.GetPosition(), ts.GetSubstring(qty));
        }
    }

    // parse Atom
    NFA RE::Regexp::PAtom(const CharacterFlags flags, const Constants::AtomType type)
    {
        switch (token.second) {
        case Regexp::TokenStream::TokenType::EOS:
            break;
        case Regexp::TokenStream::TokenType::SPECIAL:
            switch (token.first) {
            case SPEC_DOT:
                if (type == Constants::AtomType::CHARCLASS) {
                    last = FLAGS_SET(token.first, flags);
                    NextToken();
                    AddToAlphabet(last);
                    return NFA{ last };
                }
                else if (type == Constants::AtomType::STANDART) {
                    NextToken();
                    return PNotNewline();
                }
                break;
            case SPEC_BSLASH:
                NextToken(false);
                return PEscape(flags, type);
            case SPEC_LBRACKET:
            case SPEC_RBRACKET:
                break;
            default:
                if (type == Constants::AtomType::CHARCLASS) {
                    last = FLAGS_SET(token.first, flags);
                    NextToken();
                    AddToAlphabet(last);
                    return NFA{ last };
                }
                break;
            }
            break;
        case Regexp::TokenStream::TokenType::LITERAL: {
            last = FLAGS_SET(token.first, flags);
            NextToken();
            AddToAlphabet(last);
            return NFA{ last };
        }
        default:
            break;
        }
        ThrowInvalidRegexCharacter(ts.GetPosition());
    }

    // parse '.' (dot)
    NFA RE::Regexp::PNotNewline()
    {
        RE::Character lf = FLAGS_SET(CTRL_LF, CHARFL_NEGATED);
        RE::Character cr = FLAGS_SET(CTRL_CR, CHARFL_NEGATED);
        RE::Character ls = FLAGS_SET(CTRL_LS, CHARFL_NEGATED);
        RE::Character ps = FLAGS_SET(CTRL_PS, CHARFL_NEGATED);
        AddToAlphabet(lf);
        AddToAlphabet(cr);
        AddToAlphabet(ls);
        AddToAlphabet(ps);
        NFA a{ lf };
        a.Alternate(NFA{ cr });
        a.Alternate(NFA{ ls });
        a.Alternate(NFA{ ps });
        return a;
    }

    // parse Escape
    NFA RE::Regexp::PEscape(const CharacterFlags flags, const Constants::AtomType type)
    {
        switch (token.second) {
        case Regexp::TokenStream::TokenType::EOS:
            break;
        case Regexp::TokenStream::TokenType::SPECIAL: {
            last = FLAGS_SET(token.first, flags);
            NextToken();
            AddToAlphabet(last);
            return NFA{ last };
        }
        case Regexp::TokenStream::TokenType::LITERAL: {
            last = Constants::notCharacter;
            if (!PIsEscape(last, type)) {
                last = token.first;
            }
            last = FLAGS_SET(last, flags);
            NextToken();
            AddToAlphabet(last);
            return NFA{ last };
        }
        default:
            break;
        }
        ThrowInvalidRegexCharacter(ts.GetPosition());
    }

    // parse ESCAPE
    bool RE::Regexp::PIsEscape(Character& ch, const Constants::AtomType type)
    {
        switch (token.first) {
        case ESC_0:
            ch = CTRL_NULL;
            return true;
        case ESC_b:
            if (type == Constants::AtomType::CHARCLASS) {
                ch = CTRL_BS;
                return true;
            }
            return false;
        case ESC_t:
            ch = CTRL_TAB;
            return true;
        case ESC_n:
            ch = CTRL_LF;
            return true;
        case ESC_v:
            ch = CTRL_VT;
            return true;
        case ESC_f:
            ch = CTRL_FF;
            return true;
        case ESC_r:
            ch = CTRL_CR;
            return true;
        case ESC_c:
            NextToken(false);
            ch = PGetControlCode();
            return true;
        case ESC_x:
            NextToken(false);
            ch = PGetASCIICharacter();
            return true;
        case ESC_u: {
            NextToken(false);
            ch = PGetUnicodeCharacter(Constants::unicodeDigits_4);
            return true;
        }
        case ESC_U: {
            NextToken(false);
            ch = PGetUnicodeCharacter(Constants::unicodeDigits_6);
            return true;
        }
        default:
            return false;
        }
    }

    // parse ESCAPE '\c'
    Character RE::Regexp::PGetControlCode()
    {
        constexpr size_t qty = 1;
        const Character ch{ token.first };
        if (ch >= 'a' && ch <= 'z') {
            return static_cast<Character>(static_cast<unsigned int>(ch) - ('a' - 1));
        }
        else if (ch >= 'A' && ch <= 'Z') {
            return static_cast<Character>(static_cast<unsigned int>(ch) - ('A' - 1));
        }
        NextToken();
        ThrowInvalidRegexEscape(ts.GetPosition(), ts.GetSubstring(qty));
    }

    // parse ESCAPE '\x'
    Character RE::Regexp::PGetASCIICharacter()
    {
        constexpr size_t qty = 1;
        const Character ch1{ token.first };
        NextToken(false);
        const Character ch2{ token.first };
        if (isxdigit(ch1) && isxdigit(ch2)) {
            std::istringstream iss{ std::string{ static_cast<char>(ch1) } + static_cast<char>(ch2) };
            unsigned int value{ 0 };
            iss >> std::hex >> value;
            if (iss && value >= 0x00 && value <= 0x7F) {
                return static_cast<Character>(value);
            }
        }
        NextToken();
        ThrowInvalidRegexEscape(ts.GetPosition(), ts.GetSubstring(qty));
    }

    // parse ESCAPE '\u' and '\U'
    Character RE::Regexp::PGetUnicodeCharacter(const int nDigits)
    {
        constexpr size_t qty = 1;
        std::string s;
        bool valid{ true };
        for (size_t i = 0; i < nDigits; ++i) {
            if (isxdigit(token.first)) {
                s += static_cast<char>(token.first);
            }
            else {
                valid = false;
            }
            if (i < nDigits - 1) {
                NextToken(false);
            }
        }
        if (valid) {
            std::istringstream iss{ s };
            unsigned long long int value{ 0 };
            iss >> std::hex >> value;
            if (iss) {
                if (nDigits == Constants::unicodeDigits_4 && value >= BASICPLANE_MIN
                    && value <= BASICPLANE_MAX) {
                    return static_cast<Character>(value);
                }
                else if (nDigits == Constants::unicodeDigits_6 && value >= ALLSUPPLEMENTARYPLANES_MIN
                    && value <= ALLSUPPLEMENTARYPLANES_MAX) {
                    return static_cast<Character>(value);
                }
            }
        }
        NextToken();
        ThrowInvalidRegexEscape(ts.GetPosition(), ts.GetSubstring(qty));
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
            case SPEC_DOT:
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
        ThrowInvalidRegexCharacter(ts.GetPosition());
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
        ThrowInvalidRegexCharacter(ts.GetPosition());
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
        ThrowInvalidRegexCharacter(ts.GetPosition());
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
        ThrowInvalidRegexCharacter(ts.GetPosition());
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

    void Regexp::ThrowInvalidRegexCharacter(const size_t position) const
    {
        std::string message{ "Invalid character '" };
        if (position < source.size()) {
            message += GetGlyph(source[position]);
        }
        else {
            message += Strings::eof;
        }
        message += "' was encountered after substring '";
        message += GetGlyph(UString{ source, 0, position }) + "'";
        ThrowInvalidRegex(message);
    }

    void Regexp::ThrowInvalidRegexRange(const size_t position, const UString& range) const
    {
        std::string message{ "Invalid range '" };
        message += GetGlyph(range);
        message += "' was encountered after substring '";
        message += GetGlyph(UString{ source, 0, position - range.size() }) + "'";
        ThrowInvalidRegex(message);
    }

    void Regexp::ThrowInvalidRegexEscape(const size_t position, const UString& escapeSequence) const
    {
        std::string message{ "Invalid escape sequence '" };
        message += GetGlyph(escapeSequence);
        message += "' was encountered after substring '";
        if (position > source.size()) {
            message += GetGlyph(UString{ source, 0, source.size() - escapeSequence.size() }) + "'";
        }
        else {
            message += GetGlyph(UString{ source, 0, position - escapeSequence.size() }) + "'";
        }
        ThrowInvalidRegex(message);
    }

    void Regexp::ThrowInvalidRegex(const std::string& message) const
    {
        std::string fullMessage{ message };
        fullMessage += ". Regular expression: ";
        fullMessage += GetGlyph(source);
        throw Error::InvalidRegex{ fullMessage };
    }

    Regexp::Regexp(const UString& string)
        : source{ string }, ts{ source }, nfa{ Constants::notCharacter }, last{ Constants::notCharacter },
        fl{ REGFL_NOFLAGS }
    {
        if (string.size() == 0) {
            throw Error::InvalidRegex{ "Empty regular expression " };
        }
        MakeDFA();
    }

    bool Regexp::Match(const UString& string)
    {
        if (FLAG_IS_SET(fl, REGFL_NEGATED)) {
            std::vector<DFAnode*>cur{ {dfa.first} };
            size_t pos = 0;
            while (pos < string.size()) {
                cur = TakeStepThroughDFA(cur, string[pos]);
                if (cur.size() == 0) {
                    return false;
                }
                ++pos;
            }
            for (const DFAnode* p : cur) {
                if (p->acc) {
                    return true;
                }
            }
        }
        else {
            DFAnode* cur = dfa.first;
            size_t pos = 0;
            while (pos < string.size()) {
                cur = TakeStepThroughDFA(cur, string[pos]);
                if (cur == nullptr) {
                    return false;
                }
                ++pos;
            }
            if (cur->acc) {
                return true;
            }
        }
        return false;
    }

    std::vector<MatchResults> Regexp::Search(const UString& string)
    {
        std::vector<MatchResults> results;
        size_t line = 1;                    // line number
        size_t pos = 1;                     // position in line
        UString::const_iterator iter = string.cbegin();
        if (FLAG_IS_SET(fl, REGFL_NEGATED)) {
            while (iter != string.cend()) {
                Character ch = *iter;
                std::vector<DFAnode*> cur{ {dfa.first} };
                cur = TakeStepThroughDFA(cur, ch);
                if (cur.size() == 0) {
                    AdjustPositions(iter, iter + 1, line, pos);
                    ++iter;
                    continue;
                }
                MatchResults mr{ line, pos, iter, iter };
                while (cur.size() != 0) {
                    for (DFAnode* p : cur) {
                        if (p->acc) {
                            mr.str.second = iter + 1;
                            break;
                        }
                    }
                    if (++iter == string.cend()) {
                        break;
                    }
                    ch = *iter;
                    cur = TakeStepThroughDFA(cur, ch);
                }
                if (mr.str.second == mr.str.first) {
                    iter = mr.str.first + 1;
                }
                else {
                    iter = mr.str.second;
                    results.push_back(mr);
                }
                AdjustPositions(mr.str.first, iter, line, pos);
            }
        }
        else {
            while (iter != string.cend()) {
                Character ch = *iter;
                DFAnode* cur = TakeStepThroughDFA(dfa.first, ch);
                if (cur == nullptr) {
                    AdjustPositions(iter, iter + 1, line, pos);
                    ++iter;
                    continue;
                }
                MatchResults mr{ line, pos, iter, iter };
                while (cur != nullptr) {
                    if (cur->acc) {
                        mr.str.second = iter + 1;
                    }
                    if (++iter == string.cend()) {
                        break;
                    }
                    ch = *iter;
                    cur = TakeStepThroughDFA(cur, ch);
                }
                if (mr.str.second == mr.str.first) {
                    iter = mr.str.first + 1;
                }
                else {
                    iter = mr.str.second;
                    results.push_back(mr);
                }
                AdjustPositions(mr.str.first, iter, line, pos);
            }
        }
        return results;
    }

    void Regexp::PutRE(const UString& string)
    {
        if (string.size() == 0) {
            throw Error::InvalidRegex{ "Empty regular expression " };
        }
        source = string;
        ts.Reset();
        alphabetTemp = std::set<Character>{};
        alphabet = std::vector<Character>{};
        nfa = NFA{ Constants::notCharacter };
        dfa = DFA{};
        last = Constants::notCharacter;
        fl = REGFL_NOFLAGS;
        MakeDFA();
    }

    bool operator==(const Regexp& left, const Regexp& right)
    {
        return (left.source == right.source && left.dfa == right.dfa && left.fl == right.fl);
    }

    bool operator!=(const Regexp& left, const Regexp& right)
    {
        return !(left == right);
    }

    std::string GetGlyph(const Character ch, bool withQuotes)
    {
        const Character c = FLAGS_UNSET(ch, CHARFL_ALLFLAGS);
        std::string s;
        if (FLAG_IS_SET(ch, CHARFL_NEGATED)) {
            s += LIT_CARET;
        }
        if (c >= ASCII_CTRL_MIN && c <= ASCII_CTRL_MAX) {
            return s + Strings::asciiCC[static_cast<unsigned int>(c)];
        }
        else if (c == ASCII_CTRL_DEL) {
            return s + Strings::del;
        }
        else if (c > ASCII_CTRL_MAX && c < ASCII_CTRL_DEL) {
            if (withQuotes) {
                s += '\'';
            }
            s += static_cast<char>(c);
            if (withQuotes) {
                s += '\'';
            }
            return s;
        }
        else {
            std::ostringstream oss;
            oss << std::hex << std::uppercase << std::setfill('0');
            if (c >= BASICPLANE_MIN && c <= BASICPLANE_MAX) {
                oss << std::setw(Constants::unicodeDigits_4) << static_cast<unsigned int>(c);
                s += "\\u";
            }
            else if (c >= ALLSUPPLEMENTARYPLANES_MIN && c <= ALLSUPPLEMENTARYPLANES_MAX) {
                oss << std::setw(Constants::unicodeDigits_6) << static_cast<unsigned int>(c);
                s += "\\U";
            }
            s += oss.str();
            return s;
        }
    }

    std::string GetGlyph(const UString& string)
    {
        std::string s;
        for (RE::Character c : string) {
            s += RE::GetGlyph(c);
        }
        return s;
    }

#if REGEX_PRINT_FA_STATE
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
        size_t nDigits{ 0 };                                    // number of digits
        size_t n{ nodes.size() };
        while (n > 0) {
            ++nDigits;
            n /= 10;
        }

        std::string dl;                                         // dash line
        const std::string sep{ "|" };                           // separtor
        const std::string sp{ " " };                            // space
        const std::string accept{ "ACCEPT" };
        const std::string start{ "START" };
        const std::string to{ "->" };                           // transition mark
        const std::string ns{ "#" };                            // number sign
        const std::string eps{ "Epsilon" };                     // Epsilon mark
        const size_t nLetters{ Constants::unicodeDigits_6 + 3 }; // number of letters
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
                oss << sp << std::setw(nLetters) << GetGlyph(p->ch, true) << sp
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
        size_t nDigits{ 0 };                                    // number of digits
        size_t n{ nodes.size() };
        while (n > 0) {
            ++nDigits;
            n /= 10;
        }

        std::string dl;                                         // dash line
        const std::string sep{ "|" };                           // separtor
        const std::string sp{ " " };                            // space
        const std::string accept{ "ACCEPT" };
        const std::string start{ "START" };
        const std::string to{ "->" };                           // transition mark
        const std::string ns{ "#" };                            // number sign
        const size_t nLetters{ Constants::unicodeDigits_6 + 3 }; // number of letters
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
                oss << sp << std::setw(nLetters) << GetGlyph(t.first, true) << sp
                    << to << sp << ns << numbers.find(t.second)->second;
                os << sep << std::setw(cw1 + sep.size() + cw2 + sep.size() + cw3) << sp
                    << sep << std::setw(cw4) << std::left << oss.str()
                    << sep << t.second << sep << std::endl;
            }
            os << dl << std::endl;
        }
    }
#endif // REGEX_PRINT_FA_STATE

} // namespace RE
