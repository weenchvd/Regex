#ifndef REGEXPR_HPP
#define REGEXPR_HPP

namespace Regex
{
	using Character = unsigned int;

	enum class Type : unsigned char {
		LETTER,								// letter or symbol
		ACCEPT,								// Accept state
		EPSILON								// Epsilon transition
	};

	struct NFAnode {
		NFAnode* succ1;
		NFAnode* succ2;
		Character ch;
		Type ty;
		bool mark;

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
		void Closure();
	};









}

#endif // REGEXPR_HPP