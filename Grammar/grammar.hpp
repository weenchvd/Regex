#ifndef GRAMMAR_HPP
#define GRAMMAR_HPP

namespace CFG								// context-free grammar
{
	/*
		T - terminal symbol
		NT - nonterminal symbol
	*/

	using LexemeID = size_t;									// lexeme ID
	using Derived = std::vector<LexemeID>;

	constexpr int derivedShift = 10;
	const std::string suffixNT{ "'" };
	const std::string lexemeES{ "EMPTYSTRING" };

	enum class LexemeType {
		T, NT
	};

	enum class TType {											// T type
		EMPTYSTRING, END
	};

	class BNFGrammar;

	struct Production {
		LexemeID nt;											// NT
		std::vector<Derived> der;								// T and/or NT derived from the 'nt'
		static BNFGrammar* gram;

		Production() : nt{ 0 } {}
		Production(LexemeID nonterminal)
			: nt{ nonterminal } {}
	};

	// Backus-Naur form
	class BNFGrammar {
		std::map<std::string, LexemeID> id;						// map[lexeme] == LexemeID
		std::vector<std::string> lex;							// vector[LexemeID] == lexeme
		std::vector<LexemeType> type;							// lexeme type
		std::vector<Production> prod;							// production set
		LexemeID next;											// next free ID
		static std::string prefNT;								// NT prefix
		static std::string prefDer;								// prefix of derived
	public:
		BNFGrammar()
			: next{ 0 }
		{
			Production::gram = this;
		}

		// nonconst members
		void SetPrefixes(const std::string& prefixNT, const std::string& prefixDerived)
		{
			prefNT = prefixNT;
			prefDer = prefixDerived;
		}
		void AddProduction(const Production& production) { prod.push_back(production); }
		LexemeID GetLexemeID(const std::string& lexeme);
		void RemoveDirectAndIndirectLeftRecursion(std::ostream& error);
		void FindAllNonterminals(std::ostream& error);
		void RemoveRecursion(std::ostream& error);

		// const members
		const std::string& GetPrefixNT() const { return prefNT; }
		const std::string& GetPrefixDerived() const { return prefDer; }
		size_t GetProductionSize() const { return prod.size(); }
		const Production& GetProduction(size_t index) const { return prod[index]; }
		bool CheckGrammar(std::ostream& error) const;
		//std::vector<std::string> LexemeIDToLexeme() const;

		// friends
		friend struct Production;
		friend std::istream& operator>>(std::istream& is, Production& prod);
		friend std::istream& operator>>(std::istream& is, BNFGrammar& gram);
		friend std::ostream& operator<<(std::ostream& os, const BNFGrammar& gram);
	};

	BNFGrammar* Production::gram = nullptr;
	std::string BNFGrammar::prefNT;
	std::string BNFGrammar::prefDer;
}

#endif