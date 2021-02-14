#ifndef REGEXPR_HPP
#define REGEXPR_HPP

/*
	RE - Regular Expression
	NFA - Nondeterministic Finite Automata
	DFA - Deterministic Finite Automata
*/

namespace RE
{
	using Character = unsigned int;

	class Regexp;

	struct NFAnode {
	public:
		enum class Type : unsigned char {
			LITERAL,						// character, letter, symbol
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
		NFAnode(Type type, Character character = 0)
			: succ1{ nullptr }, succ2{ nullptr }, ch{ character }, ty{ type }, mark{ false } {}
	};

	class NFA {
		NFAnode* first;
		NFAnode* last;
		size_t sz;							// size
		static std::allocator<NFAnode> alloc;
	private:
		// const members
		std::vector<NFAnode*> GetAllNodes() const;
		void AddNodeToSet(std::vector<NFAnode*>& set, NFAnode* node) const;
		NFAnode* CreateNFANode(const NFAnode::Type type) const;
		NFAnode* CreateNFANode(const NFAnode::Type type, const Character character) const;

		// nonconst members
		void ReleaseResources();

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
		DFAnode* CreateDFANode(const bool accept) const;

		// nonconst members
		void ReleaseResources();

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

	using REstring = std::string;
	using Number = size_t;
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
			const REstring& s;
			size_t pos;
			std::set<Character> alphabet;
		public:
			TokenStream(const REstring& string)
				: s{ string }, pos{ 0 } {}
			// const members
			size_t GetPosition() const { return pos; }
			std::pair<std::set<Character>::iterator, std::set<Character>::iterator> GetAlphabet() const
			{
				return { alphabet.begin(), alphabet.end() };
			}

			// nonconst members
			std::pair<const Character, const TokenType> GetToken();
			void Advance() { ++pos; }
			std::pair<const Character, const TokenType> AdvanceAndGetToken() { Advance(); return GetToken(); }
			void EraseAlphabet() { alphabet.clear(); }
		};
	private:
		REstring source;
		TokenStream ts;
		std::pair<Character, TokenStream::TokenType> token;
		std::vector<Character> alphabet;
		NFA nfa{ 0 };
		DFA dfa;
	private:
		// const members
		std::set<const NFAnode*> Delta(const std::set<const NFAnode*>& set, const Character ch) const;
		std::set<const NFAnode*> EpsilonClosure(const std::set<const NFAnode*>& set) const;
		void AddNodesReachableViaEpsilonTransition(std::set<const NFAnode*>& set, const NFAnode* node) const;
		bool Equal(const std::set<const NFAnode*>& a, const std::set<const NFAnode*>& b) const;
		std::pair<std::vector<DFAnode*>, std::vector<DFAnode*>> Split(const std::vector<DFAnode*>& set,
			const std::unordered_map<const DFAnode*, Number> numbers) const;
		const DFAnode* FindTransition(const DFAnode* node, const Character ch) const;
		bool TransitionExists(const DFAnode* node) const { return (node == nullptr) ? false : true; }
		DFA CreateMinimalDFA(const SetPartition& sp,
			const std::unordered_map<const DFAnode*, Number> numbers) const;

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
		void PClosure(NFA& a);
		void ThrowInvalidRegex(const size_t position) const;
	public:
		Regexp(const REstring& string);
		// const members

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