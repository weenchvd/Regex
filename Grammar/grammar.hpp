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
	using First = std::vector<LexemeID>;						// FIRST set
	using Follow = std::vector<LexemeID>;						// FOLLOW set
	using FirstPlus = std::vector<LexemeID>;					// FIRST+ set

	constexpr int derivedShift = 10;

	enum class LexemeType {
		EMPTYSTRING, ENDOFFILE, T, NT
	};

	class BNFGrammar;

	struct Production {
		LexemeID nt;											// LexemeID of NT
		std::vector<Derived> derSet;							// list of T and/or NT derived from the 'nt'
		static BNFGrammar* gram;

		Production() : nt{ 0 } {}
		Production(LexemeID nonterminal)
			: nt{ nonterminal } {}
	};

	// Backus-Naur form
	class BNFGrammar {
		std::map<std::string, LexemeID> id;						// map[lexeme] -> LexemeID
		std::vector<std::string> lex;							// vector[LexemeID] -> lexeme
		std::vector<LexemeType> typeSet;						// vector[LexemeID] -> LexemeType, lexeme type
		std::vector<First> firstSet;							// vector[LexemeID] -> First, FIRST sets
		std::vector<Follow> followSet;							// vector[LexemeID] -> Follow, FOLLOW sets
		std::vector<std::vector<FirstPlus>> firstPlusSet;		// vector[productionIndex][derivedIndex] -> FirstPlus, FIRST+ sets
		std::vector<Production> prodSet;						// productions
		LexemeID next;											// next free LexemeID, first ID == 0, last ID == next - 1
		LexemeID idES;											// LexemeID of the empty string lexeme
		LexemeID idEOF;											// LexemeID of the END OF FILE lexeme
		// static
		static std::string prefNT;								// NT prefix
		static std::string prefDer;								// prefix of derived
		static std::string suffNT;								// NT suffix
		static std::string lexES;								// lexeme of an empty string
		static std::string lexEOF;								// lexeme of END OF FILE
		static std::string comment;

	private:
		// const members
		void Print(std::ostream& os, const std::vector<LexemeID>& set) const;
		void PrintFirstSet(std::ostream& os, LexemeID id) const;
		void PrintFollowSet(std::ostream& os, LexemeID id) const;
		void PrintFirstPlusSet(std::ostream& os, size_t productionIndex, size_t derivedIndex) const;
		bool OriginalGrammarIsValid(std::ostream& error) const;
		bool OriginalGrammarContainsInfiniteCycle(std::ostream& error) const;
		bool OriginalGrammarContainsEpsilonProduction(std::ostream& error) const;
		bool TransformedGrammarIsBacktrackFree(std::ostream& error) const;
		std::pair<int, bool> GrammarContainsProductionsForNonterminal(LexemeID nonterminal) const;
		bool SetContainsOnlyTerminalsAndNonterminals(const Derived& derived) const;
		bool SetContains(const std::vector<LexemeID>& set, LexemeID target) const;
		bool SetsContainIdenticalElements(const std::vector<LexemeID>& a, const std::vector<LexemeID>& b) const;
		First GetFirstSetForProduction(size_t productionIndex, size_t derivedIndex) const;
		int Merge(std::vector<LexemeID>& receiver, const std::vector<LexemeID>& source) const;
		int MergeExcept(std::vector<LexemeID>& receiver, const std::vector<LexemeID>& source, LexemeID exception) const;

		// nonconst members
		LexemeID GetLexemeID(const std::string& lexeme);
		void RemoveAllLeftRecursion();
		void FindAllNonterminals();
		void FindAllFirstSets();
		void FindAllFollowSets();
		void FindAllFirstPlusSets();

	public:
		BNFGrammar()
			: next{ 0 }, idES{ 0 }, idEOF{ 0 }
		{
			Production::gram = this;
		}

		// nonconst members
		void TransformGrammar(std::ostream& error);

		// friends
		friend struct Production;
		friend std::istream& operator>>(std::istream& is, Production& prod);
		friend std::istream& operator>>(std::istream& is, BNFGrammar& gram);
		friend std::ostream& operator<<(std::ostream& os, const Production& prod);
		friend std::ostream& operator<<(std::ostream& os, const BNFGrammar& gram);
	};
}

#endif // GRAMMAR_HPP