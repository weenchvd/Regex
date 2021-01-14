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

	enum class LexemeType {
		T, NT
	};

	enum class TType {											// T type
		EMPTYSTRING, END
	};

	class BNFGrammar;

	struct Production {
		LexemeID nt;											// NT
		std::vector<Derived> derList;							// list of T and/or NT derived from the 'nt'
		static BNFGrammar* gram;

		Production() : nt{ 0 } {}
		Production(LexemeID nonterminal)
			: nt{ nonterminal } {}
	};

	// Backus-Naur form
	class BNFGrammar {
		std::map<std::string, LexemeID> id;						// map[lexeme] == LexemeID
		std::vector<std::string> lex;							// vector[LexemeID] == lexeme
		std::vector<LexemeType> typeList;						// lexeme type
		std::vector<Production> prodList;						// productions
		LexemeID next;											// next free ID
		static std::string prefNT;								// NT prefix
		static std::string prefDer;								// prefix of derived
		static std::string suffNT;								// NT suffix
		static std::string lexES;								// lexeme of an empty string
		static std::string comment;
	public:
		BNFGrammar()
			: next{ 0 }
		{
			Production::gram = this;
		}

		// const members
		const std::string& GetPrefixNT() const { return prefNT; }
		const std::string& GetPrefixDerived() const { return prefDer; }
		size_t GetProductionSize() const { return prodList.size(); }
		const Production& GetProduction(size_t index) const { return prodList[index]; }
		bool CheckGrammar(std::ostream& error) const;

		// nonconst members
		void SetPrefixes(const std::string& prefixNT, const std::string& prefixDerived)
		{
			prefNT = prefixNT;
			prefDer = prefixDerived;
		}
		void AddProduction(const Production& production) { prodList.push_back(production); }
		LexemeID GetLexemeID(const std::string& lexeme);
		void RemoveDirectAndIndirectLeftRecursion(std::ostream& error);
		void FindAllNonterminals(std::ostream& error);
		void RemoveRecursion(std::ostream& error);

		// friends
		friend struct Production;
		friend std::istream& operator>>(std::istream& is, Production& prod);
		friend std::istream& operator>>(std::istream& is, BNFGrammar& gram);
		friend std::ostream& operator<<(std::ostream& os, const Production& prod);
		friend std::ostream& operator<<(std::ostream& os, const BNFGrammar& gram);
	};

	BNFGrammar* Production::gram = nullptr;
	std::string BNFGrammar::prefNT;
	std::string BNFGrammar::prefDer;
	std::string BNFGrammar::suffNT;
	std::string BNFGrammar::lexES;
	std::string BNFGrammar::comment;
}

#endif