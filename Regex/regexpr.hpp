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
			LETTER,							// letter or symbol
			ACCEPT,							// Accept state
			EPSILON							// Epsilon transition
		};
	public:
		NFAnode* succ1;
		NFAnode* succ2;
		Character ch;
		Type ty;
		bool mark;
		// static
		static NFAnode* ptr;
	public:
		NFAnode(Type type, Character character = 0)
			: succ1{ nullptr }, succ2{ nullptr }, ch{ character }, ty{ type }, mark{ false } {}
	};

	class NFA {
		NFAnode* first;
		NFAnode* last;
		size_t sz;
		// static
		static std::allocator<NFAnode> alloc;
	private:
		// const members
		void AddNodeToSet(std::vector<NFAnode*>& set, NFAnode* node) const;

		// nonconst members
		void ReleaseResources();
		NFAnode* CreateNFANode(NFAnode::Type type);
		NFAnode* CreateNFANode(NFAnode::Type type, Character character);
		
		friend class Regexp;
	public:
		NFA(Character character);
		~NFA();

		NFA(const NFA& other) = delete;
		NFA& operator=(const NFA& other) = delete;
		NFA(NFA&& other);
		NFA& operator=(NFA&& other);

		// const members
		size_t Size() const { return sz; }

		// nonconst members
		void Concatenate(NFA& other);
		void Alternate(NFA& other);
		void ClosureKleene();
	};

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
		public:
			TokenStream(const REstring& string)
				: s{ string }, pos{ 0 } {}
			// const members
			size_t GetPosition() const { return pos; }
			std::pair<Character, TokenType> GetToken() const;

			// nonconst members
			void Advance() { ++pos; }
			std::pair<Character, TokenType> AdvanceAndGetToken() { Advance(); return GetToken(); }
		};
	private:
		REstring source;
		TokenStream ts;
		std::pair<Character, Regexp::TokenStream::TokenType> token;
		NFA nfa{ 0 };
	private:
		// const members

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
		void PrintNFA(std::ostream& os) const;

		// nonconst members
		void PutRE(const REstring& string);

		// friends
	};

	std::istream& operator>>(std::istream& is, REstring& string);
}

#endif // REGEXPR_HPP