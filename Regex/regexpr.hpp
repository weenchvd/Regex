#ifndef REGEXPR_HPP
#define REGEXPR_HPP

/*
	RE - Regular Expression
	NFA - Nondeterministic Finite Automata
	DFA - Deterministic Finite Automata
*/

namespace RE
{
	enum ControlCharacter : unsigned char {
		CTRL_NULL		= '\0',
		CTRL_HTAB		= '\t',
		CTRL_NEWLINE	= '\n',
		CTRL_VTAB		= '\v',
		CTRL_FORMFEED	= '\f',
		CTRL_CRETURN	= '\r'
	};
	
	enum EscapeCharacter : unsigned char {
		ESC_NULL		= '0',
		ESC_HTAB		= 't',
		ESC_NEWLINE		= 'n',
		ESC_VTAB		= 'v',
		ESC_FORMFEED	= 'f',
		ESC_CRETURN		= 'r'
	};

	// SpecialCharacters must not match EscapeCharacters
	enum SpecialCharacter : unsigned char {
		SPEC_LPAR		= '(',
		SPEC_RPAR		= ')',
		SPEC_STAR		= '*',
		SPEC_PLUS		= '+',
		SPEC_QUESTION	= '?',
		SPEC_BSLASH		= '\\',
		SPEC_LBRACE		= '{',
		SPEC_BAR		= '|',
		SPEC_RBRACE		= '}'
	};

	enum LiteralCharacter : unsigned char {
		LIT_COMMA		= ','
	};

	namespace Constants
	{
		enum class ClosureType : unsigned char {
			NOTYPE,
			FINITE,
			INFITITE,
			RANGE
		};
	}

	using Character = int;
	using Index = size_t;
	using Number = size_t;

	constexpr Character notCharacter = -1;

	class Regexp;

	struct NFAnode {
	public:
		enum class Type : unsigned char {
			LITERAL,						// character, letter, symbol
			//NEGATION,						// any other than this
			ACCEPT,							// Accept state
			EPSILON							// Epsilon transition
		};
	public:
		NFAnode* succ1;						// succsessor 1
		NFAnode* succ2;						// succsessor 2
		Character ch;						// character
		Type ty;							// type
		bool mark;
		static NFAnode* hint;
	public:
		NFAnode(Type type, Character character = notCharacter)
			: succ1{ nullptr }, succ2{ nullptr }, ch{ character }, ty{ type }, mark{ false } {}
	};

	class NFA {
		NFAnode* first;
		NFAnode* last;
		size_t sz;							// size
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
#if PRINTFA
		friend void PrintNFA(std::ostream& os, const RE::Regexp& re);
#endif // PRINTFA
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
		TransitionTable trans;				// table of transitions
		bool acc;							// accept
		bool mark;
		static DFAnode* hint;
	public:
		DFAnode(bool accept)
			: acc{ accept }, mark{ false } {}
	};

	class DFA {
		DFAnode* first;
		size_t sz;							// size
		static std::allocator<DFAnode> alloc;
	private:
		// const members
		std::vector<DFAnode*> GetAllNodes() const;
		void AddNodeToSet(std::vector<DFAnode*>& set, DFAnode* node) const;
		void ReleaseResources() const;
		DFAnode* CreateDFANode(const bool accept) const;

		// friends
		friend class Regexp;
#if PRINTFA
		friend void PrintDFA(std::ostream& os, const RE::Regexp& re);
#endif // PRINTFA
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
	
	using SubsetTableIndex = long long int;
	constexpr SubsetTableIndex noTransition = -1;

	struct SubsetTableEntry {
		const std::set<const NFAnode*> state;					// set of NFAnode*
		std::vector<SubsetTableIndex> trans;					// transitions to SubsetTableEntry
	};

	using SubsetTable = std::vector<SubsetTableEntry>;

	class REstring : public std::string {};

	using SetPartition = std::vector<std::vector<DFAnode*>>;

	class Regexp {
		class TokenStream {
		public:
			enum class TokenType : unsigned char {
				EOS,						// EOS - End Of Stream
				SPECIAL,					// special character
				LITERAL						// character, letter, symbol
			};
		private:
			std::string& s;
			size_t pos;
		public:
			TokenStream(std::string& string)
				: s{ string }, pos{ 0 } {}

			TokenStream(const TokenStream& other) = delete;
			TokenStream& operator=(const TokenStream&& other) = delete;
			TokenStream(TokenStream& other) = delete;
			TokenStream& operator=(TokenStream&& other);

			// const members
			size_t GetPosition() const { return pos; }

			// nonconst members
			void Advance() { ++pos; }
			std::pair<Character, TokenType> GetToken();
			std::pair<Character, TokenType> GetNextToken() { Advance(); return GetToken(); }
		};
	private:
		std::string source;
		TokenStream ts;
		std::pair<Character, TokenStream::TokenType> token;
		std::set<Character> alphabetTemp;
		std::vector<Character> alphabet;
		NFA nfa;
		DFA dfa;
	private:
		// const members
		std::set<const NFAnode*> Delta(const std::set<const NFAnode*>& set, const Character ch) const;
		std::set<const NFAnode*> EpsilonClosure(const std::set<const NFAnode*>& set) const;
		void AddNodesReachableViaEpsilonTransition(std::set<const NFAnode*>& set, const NFAnode* node) const;
		bool Equal(const std::set<const NFAnode*>& a, const std::set<const NFAnode*>& b) const;
		std::pair<std::vector<DFAnode*>, std::vector<DFAnode*>> Split(const std::vector<DFAnode*>& set,
			const std::unordered_map<const DFAnode*, Index>& indexes) const;
		const DFAnode* FindTransition(const DFAnode* node, const Character ch) const;
		bool TransitionExists(const DFAnode* node) const { return (node == nullptr) ? false : true; }
		void AssignNumber(const std::vector<DFAnode*>& set, const Index index,
			std::unordered_map<const DFAnode*, Index>& indexes) const;
		DFA CreateMinimalDFA(const SetPartition& sp,
			const std::unordered_map<const DFAnode*, Index>& indexes) const;
		void CheckNFA() const;

		// nonconst members
		void MakeDFA();
		void REtoNFA();
		std::vector<DFAnode*> NFAtoDFA();
		void MinimizeDFA(const std::vector<DFAnode*> nodes);
	private:
		// Parsing
		 NFA PGoal();
		 NFA PAlternation();
		void PAlternationPrime(NFA& a);
		 NFA PConcatenation();
		void PConcatenationPrime(NFA& a);
		 NFA PSymbol();
		 NFA PBlock();
		 NFA PEscape();
		bool PIsEscape(Character& ch);
		void PClosure(NFA& a);
		void PCount(int& min, int& max, Constants::ClosureType& ty);
		void PCountMore(int& max, Constants::ClosureType& ty);
		void PMax(int& max, Constants::ClosureType& ty);
		 int PGetInteger();
		void ThrowInvalidRegex(const size_t position) const;
		void ThrowInvalidRegex(const std::string& message) const;
	public:
		Regexp(const REstring& string);

		Regexp(const Regexp& other) = delete;
		Regexp& operator=(const Regexp& other) = delete;
		Regexp(Regexp&& other) = delete;
		Regexp& operator=(Regexp&& other) = delete;

		// nonconst members
		void PutRE(const REstring& string);

		// friends
#if PRINTFA
		friend void PrintNFA(std::ostream& os, const RE::Regexp& re);
		friend void PrintDFA(std::ostream& os, const RE::Regexp& re);
#endif // PRINTFA
	};

	std::istream& operator>>(std::istream& is, REstring& string);
}

#endif // REGEXPR_HPP