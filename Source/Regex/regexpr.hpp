
// Copyright (c) 2021 Vitaly Dikov
// 
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef REGEXPR_HPP
#define REGEXPR_HPP

#define FLAGS_SET(target, flags) target | flags
#define FLAGS_UNSET(target, flags) target & ~flags

/*
    RE - Regular Expression
    NFA - Nondeterministic Finite Automata
    DFA - Deterministic Finite Automata
*/

namespace RE
{
    using Character = unsigned int;

    enum ControlCharacter : Character {
        CTRL_NULL       = '\0',
        CTRL_BS         = '\b',
        CTRL_TAB        = '\t',
        CTRL_LF         = '\n',
        CTRL_VT         = '\v',
        CTRL_FF         = '\f',
        CTRL_CR         = '\r',
        CTRL_LS         = 0x2028,
        CTRL_PS         = 0x2029
    };

    enum EscapeCharacter : unsigned char {
        ESC_0           = '0',
        ESC_b           = 'b',
        ESC_t           = 't',
        ESC_n           = 'n',
        ESC_v           = 'v',
        ESC_f           = 'f',
        ESC_r           = 'r',
        ESC_c           = 'c',
        ESC_x           = 'x',
        ESC_u           = 'u',
        ESC_U           = 'U'
    };

    // SpecialCharacters must not match EscapeCharacters
    enum SpecialCharacter : unsigned char {
        SPEC_LPAR       = '(',
        SPEC_RPAR       = ')',
        SPEC_STAR       = '*',
        SPEC_PLUS       = '+',
        SPEC_DOT        = '.',
        SPEC_QUESTION   = '?',
        SPEC_LBRACKET   = '[',
        SPEC_BSLASH     = '\\',
        SPEC_RBRACKET   = ']',
        SPEC_LBRACE     = '{',
        SPEC_BAR        = '|',
        SPEC_RBRACE     = '}'
    };

    enum LiteralCharacter : unsigned char {
        LIT_COMMA       = ',',
        LIT_HYPHEN      = '-',
        LIT_CARET       = '^'
    };

    enum UnicodeCodePointRange : unsigned int {
        BASICPLANE_MIN = 0x0000,
        BASICPLANE_MAX = 0xFFFF,
        ALLSUPPLEMENTARYPLANES_MIN = 0x010000,
        ALLSUPPLEMENTARYPLANES_MAX = 0x10FFFF
    };

    enum ASCIIRange : unsigned char {
        ASCII_CTRL_MIN = 0x00,
        ASCII_CTRL_MAX = 0x1F,
        ASCII_CTRL_DEL = 0x7F
    };

    using Index = size_t;
    using Number = size_t;
    using CharacterFlags = Character;

    enum CharacterFlag : CharacterFlags {
        CHARFL_NOTCHAR  = 0x80000000,       // Character bit #32: not Character
        CHARFL_NEGATED  = 0x40000000,       // Character bit #31: NEGATED LITERAL (any other than this)
        CHARFL_NOFLAGS  = 0x00000000,       // NO FLAGS
        CHARFL_ALLFLAGS = CHARFL_NOTCHAR | CHARFL_NEGATED | CHARFL_NOFLAGS
    };

    using RegexpFlags = unsigned int;

    enum RegexpFlag : RegexpFlags {
        REGFL_NEGATED   = 0x80000000,       // uint bit #32: negated characters are present
        REGFL_NOFLAGS   = 0x00000000,       // NO FLAGS
        REGFL_ALLFLAGS  = REGFL_NEGATED | REGFL_NOFLAGS
    };

    namespace Constants
    {
        constexpr int unicodeDigits_4 = 4;  // number of Unicode code point digits after '\u'
        constexpr int unicodeDigits_6 = 6;  // number of Unicode code point digits after '\U'

        enum class ClosureType : unsigned char {
            NOTYPE,
            FINITE,
            INFITITE,
            RANGE
        };

        enum class AtomType : unsigned char {
            STANDART,
            CHARCLASS
        };
    }

    namespace Strings
    {
        extern const char* eof;
        extern const char* del;
        extern const char* asciiCC[ASCII_CTRL_MAX + 1];
    }

    ///----------------------------------------------------------------------------------------------------

    class Regexp;

    struct NFAnode {
    public:
        enum class Type : unsigned char {
            LITERAL,                        // character, letter, symbol
            ACCEPT,                         // Accept state
            EPSILON                         // Epsilon transition
        };
    public:
        NFAnode* succ1;                     // succsessor 1
        NFAnode* succ2;                     // succsessor 2
        Character ch;                       // character
        Type ty;                            // type
        bool mark;
        static NFAnode* hint;
    public:
        NFAnode(Type type, Character character = CHARFL_NOTCHAR)
            : succ1{ nullptr }, succ2{ nullptr }, ch{ character }, ty{ type }, mark{ false } {}
    };

    class NFA {
        NFAnode* first;
        NFAnode* last;
        size_t sz;                          // size
        static std::allocator<NFAnode> alloc;
    private:
        NFA()
            : first{ nullptr }, last{ nullptr }, sz{ 0 } {}

        // const members
        std::vector<NFAnode*> GetAllNodes() const;
        void AddNodeToSet(std::vector<NFAnode*>& set, NFAnode* node) const;
        void ReleaseResources() const;
        NFAnode* CreateNFANode(const NFAnode::Type type) const;
        NFAnode* CreateNFANode(const NFAnode::Type type, const Character character) const;
        NFA CreateCopy() const;

        // friends
#if REGEX_PRINT_FA_STATE
        friend void PrintNFA(std::ostream& os, const RE::Regexp& re);
#endif // REGEX_PRINT_FA_STATE
    public:
        NFA(const Character character);
        ~NFA();

        NFA(const NFA& other) = delete;
        NFA& operator=(const NFA& other) = delete;
        NFA(NFA&& other);
        NFA& operator=(NFA&& other);

        // const members
        size_t Size() const { return sz; }
        NFAnode* GetFirstNode() const { return first; }
        NFAnode* GetLastNode() const { return last; }

        // nonconst members
        void Concatenate(NFA& other);
        void Alternate(NFA& other);
        void ClosureKleene();
        void ClosurePositive();
        void ClosureBinary();
        void ClosureCustom(const int min, const int max, const Constants::ClosureType ty);
    };

    ///----------------------------------------------------------------------------------------------------

    struct DFAnode;

    using Transition = std::pair<Character, DFAnode*>;
    using TransitionTable = std::vector<Transition>;

    struct LessTransitionCharacter {
        bool operator()(const Transition& t, Character ch) { return t.first < ch; }
    };

    struct DFAnode {
    public:
        TransitionTable trans;              // table of transitions
        bool acc;                           // accept
        bool mark;
        static DFAnode* hint;
    public:
        DFAnode(bool accept)
            : acc{ accept }, mark{ false } {}
    };

    class DFA {
        DFAnode* first;
        size_t sz;                          // size
        static std::allocator<DFAnode> alloc;
    private:
        // const members
        std::vector<DFAnode*> GetAllNodes() const;
        void AddNodeToSet(std::vector<DFAnode*>& set, DFAnode* node) const;
        void ReleaseResources() const;
        DFAnode* CreateDFANode(const bool accept) const;

        // friends
        friend class Regexp;
        friend bool operator==(const DFA& left, const DFA& right);
        friend bool Equal(const DFAnode* left, const DFAnode* right, std::set<const DFAnode*>& visited);
#if REGEX_PRINT_FA_STATE
        friend void PrintDFA(std::ostream& os, const RE::Regexp& re);
#endif // REGEX_PRINT_FA_STATE
    public:
        DFA()
            : first{ nullptr }, sz{ 0 } {}
        ~DFA();

        DFA(const DFA& other) = delete;
        DFA& operator=(const DFA& other) = delete;
        DFA(DFA&& other);
        DFA& operator=(DFA&& other);

        // const members
        size_t Size() const { return sz; }
    };

    ///----------------------------------------------------------------------------------------------------
    
    using SubsetTableIndex = int;
    constexpr SubsetTableIndex noTransition = -1;

    struct SubsetTableEntry {
        const std::set<const NFAnode*> state;                   // set of NFAnode*
        std::vector<SubsetTableIndex> trans;                    // transitions to SubsetTableEntry
    };

    using SubsetTable = std::vector<SubsetTableEntry>;

    using UString = std::u32string;                             // Unicode string

    using SetPartition = std::vector<std::vector<DFAnode*>>;
    constexpr size_t ringBufferSize = 4;

    struct MatchResults {
        using LineNumber     = size_t;
        using PositionInLine = size_t;
        using Matched        = std::pair<std::u32string::const_iterator, std::u32string::const_iterator>;
        
        LineNumber ln;
        PositionInLine pos;
        Matched str;

        MatchResults(size_t lineNumber, size_t positionInLine,
            std::u32string::const_iterator begin, std::u32string::const_iterator end)
            : ln{ lineNumber }, pos{ positionInLine }, str{ begin, end } {}
    };

    class Regexp {
        class TokenStream {
        public:
            enum class TokenType : unsigned char {
                EOS,                        // EOS - End Of Stream
                SPECIAL,                    // special character
                LITERAL                     // character, letter, symbol
            };
        private:
            UString& s;
            size_t pos;                     // index
            std::vector<UString> tss;       // array of token substrings (ring buffer)
            size_t tpos;                    // index of the array of token substrings
        public:
            TokenStream(UString& string)
                : s{ string }, pos{ std::numeric_limits<size_t>::max() }, tpos{ 0 }
            {
                constexpr size_t capacity = Constants::unicodeDigits_6 + 2;
                tss.resize(ringBufferSize);
                for (UString& s : tss) {
                    s.reserve(capacity);
                }
            }

            TokenStream(const TokenStream& other) = delete;
            TokenStream& operator=(const TokenStream& other) = delete;
            TokenStream(TokenStream&& other) = delete;
            TokenStream& operator=(TokenStream&& other);

            // const members
            size_t GetPosition() const { return pos; }
            UString GetSubstring(const size_t qty) const;
            TokenType GetTokenType(const Character ch) const;
            std::pair<Character, TokenType> GetToken() const;

            // nonconst members
            void Advance(const bool beginSubstring)
            {
                ++pos;
                if (beginSubstring) {
                    tpos = (tpos + 1) % tss.size();
                    tss[tpos].clear();
                }
                if (pos < s.size()) {
                    tss[tpos] += s[pos];
                }
            }
        };
    private:
        UString source;
        TokenStream ts;
        std::pair<Character, TokenStream::TokenType> token;
        std::set<Character> alphabetTemp;
        std::vector<Character> alphabet;
        NFA nfa;
        DFA dfa;
        Character last;
        RegexpFlags fl;
    private:
        // const members
        std::set<const NFAnode*> Delta(
            const std::set<const NFAnode*>& set,
            const Character ch) const;

        std::set<const NFAnode*> EpsilonClosure(
            const std::set<const NFAnode*>& set) const;

        void AddNodesReachableViaEpsilonTransition(
            std::set<const NFAnode*>& set,
            const NFAnode* node) const;

        bool Equal(
            const std::set<const NFAnode*>& a,
            const std::set<const NFAnode*>& b) const;

        std::pair<std::vector<DFAnode*>, std::vector<DFAnode*>> Split(
            const std::vector<DFAnode*>& set,
            const std::unordered_map<const DFAnode*, Index>& indexes) const;

        const DFAnode* FindTransition(
            const DFAnode* node,
            const Character ch) const;

        bool TransitionExists(const DFAnode* node) const { return (node == nullptr) ? false : true; }

        void AssignNumber(
            const std::vector<DFAnode*>& set,
            const Index index,
            std::unordered_map<const DFAnode*, Index>& indexes) const;

        DFA CreateMinimalDFA(
            const SetPartition& sp,
            const std::unordered_map<const DFAnode*, Index>& indexes) const;

        void CheckNFA() const;

        DFAnode* TakeStepThroughDFA(
            const DFAnode* current,
            const Character ch) const;

        std::vector<DFAnode*> TakeStepThroughDFA(
            std::vector<DFAnode*>& currentState,
            const Character ch) const;

        bool IsNewLineSymbol(const Character ch) const;

        void AdjustPositions(
            std::u32string::const_iterator begin,
            std::u32string::const_iterator end,
            size_t& line,
            size_t& pos) const;

        // nonconst members
        void NextToken(const bool beginSubstring = true) { ts.Advance(beginSubstring); token = ts.GetToken(); }
        void AddToAlphabet(const Character ch) { alphabetTemp.emplace(ch); }
        void MakeDFA();
        void REtoNFA();
        std::vector<DFAnode*> NFAtoDFA();
        void MinimizeDFA(const std::vector<DFAnode*> nodes);
        void SetFlags();
    private:
        // Parsing
         NFA PGoal();
         NFA PAlternation();
        void PAlternationPrime(NFA& a);
         NFA PConcatenation();
        void PConcatenationPrime(NFA& a);
         NFA PTerm();
         NFA PBlock();
         NFA PCharacterClass();
        bool PNegation();
         NFA PCharacterClassRange(const CharacterFlags flags, const Constants::AtomType type);
        void PCharacterClassRangePrime(const CharacterFlags flags, const Constants::AtomType type, NFA& a);
        void CheckRange(Character first, Character last);
         NFA PAtom(
                const CharacterFlags flags = CHARFL_NOFLAGS,
                const Constants::AtomType type = Constants::AtomType::STANDART);
         NFA PNotNewline();
         NFA PEscape(const CharacterFlags flags, const Constants::AtomType type);
        Character PGetControlCode();
        Character PGetASCIICharacter();
        Character PGetUnicodeCharacter(const int nDigits);
        bool PIsEscape(Character& ch, const Constants::AtomType type);
        void PClosure(NFA& a);
        void PCount(int& min, int& max, Constants::ClosureType& ty);
        void PCountMore(int& max, Constants::ClosureType& ty);
        void PMax(int& max, Constants::ClosureType& ty);
         int PGetInteger();
        void ThrowInvalidRegexCharacter(const size_t position) const;
        void ThrowInvalidRegexRange(const size_t position, const UString& range) const;
        void ThrowInvalidRegexEscape(const size_t position, const UString& escapeSequence) const;
        void ThrowInvalidRegex(const std::string& message) const;
    public:
        Regexp(const UString& string);

        Regexp(const Regexp& other) = delete;
        Regexp& operator=(const Regexp& other) = delete;
        Regexp(Regexp&& other) = delete;
        Regexp& operator=(Regexp&& other) = delete;

        // const members
        bool Match(const UString& string);
        std::vector<MatchResults> Search(const UString& string);

        // nonconst members
        void PutRE(const UString& string);

        // friends
        friend bool operator==(const Regexp& left, const Regexp& right);
        friend bool operator!=(const Regexp& left, const Regexp& right);
#if REGEX_PRINT_FA_STATE
        friend void PrintNFA(std::ostream& os, const RE::Regexp& re);
        friend void PrintDFA(std::ostream& os, const RE::Regexp& re);
#endif // REGEX_PRINT_FA_STATE
    };

    std::string GetGlyph(const Character ch, bool withQuotes = false);
    std::string GetGlyph(const UString& string);
}

#endif // REGEXPR_HPP
