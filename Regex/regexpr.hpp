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
		static NFAnode* ptr;
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
		void AddNodeToSet(std::vector<NFAnode*>& set, NFAnode* node) const;
		NFAnode* CreateNFANode(NFAnode::Type type) const;
		NFAnode* CreateNFANode(NFAnode::Type type, Character character) const;

		// nonconst members
		void ReleaseResources();

		// friends
#if PRINTFA
		friend void PrintNFA(std::ostream& os, const RE::Regexp& re);
#endif // PRINTFA
	public:
		NFA(Character character);
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

	struct DFAnode {
	public:
		std::vector<std::pair<Character, DFAnode*>> trans;		// transitions
		bool acc;												// accept
		bool mark;
		static DFAnode* ptr;
	public:
		DFAnode(bool accept)
			: acc{ accept }, mark{ false } {}
	};

	bool operator<(const std::pair<Character, DFAnode*>& a, const std::pair<Character, DFAnode*>& b);

	class DFA {
		DFAnode* first;
		size_t sz;							// size
		static std::allocator<DFAnode> alloc;
	private:
		// const members
		void AddNodeToSet(std::vector<DFAnode*>& set, DFAnode* node) const;
		DFAnode* CreateDFANode(bool accept) const;

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

	using NFAset = std::set<NFAnode*>;
	using TableIndex = long long int;
	constexpr TableIndex noTransition = -1;

	struct TableEntry {
		NFAset state;						// set of NFAnode*
		std::vector<TableIndex> trans;	// transitions
	};

	using Table = std::vector<TableEntry>;

	using REstring = std::string;

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
			std::pair<Character, TokenType> GetToken();
			void Advance() { ++pos; }
			std::pair<Character, TokenType> AdvanceAndGetToken() { Advance(); return GetToken(); }
			void EraseAlphabet() { alphabet.clear(); }
		};
	private:
		REstring source;
		TokenStream ts;
		std::pair<Character, Regexp::TokenStream::TokenType> token;
		std::vector<Character> alphabet;
		NFA nfa{ 0 };
		DFA dfa;
	private:
		// const members
		NFAset Delta(const NFAset& set, Character ch) const;
		NFAset EpsilonClosure(NFAset set) const;
		void AddNodesReachableViaEpsilonTransition(NFAset& set, const NFAnode* node) const;
		bool Equal(const NFAset& a, const NFAset& b) const;

		// nonconst members
		void MakeDFA();
		void REtoNFA();
		void NFAtoDFA();
		void MinimizeDFA();
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
		void ThrowInvalidRegex(size_t position) const;
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