#include<iostream>
#include<memory>
#include<vector>
#include"regexpr.hpp"

int main()
{
	
	return 0;
}

namespace Regex
{
	void NFA::AddNodeToSet(std::vector<NFAnode*>& set, NFAnode* node) const
	{
		if (node == nullptr) {
			return;
		}
		set.push_back(node);
		node->mark = true;
		AddNodeToSet(set, node->succ1);
		AddNodeToSet(set, node->succ2);
	}

	void NFA::ReleaseResources()
	{
		std::vector<NFAnode*> nodes;
		nodes.reserve(sz);
		AddNodeToSet(nodes, first);
		for (NFAnode* p : nodes) {
			alloc.destroy(p);
			alloc.deallocate(p, 1);
		}
	}

	NFA::NFA(Character character)
		: sz{ 2 }
	{
		first = alloc.allocate(1);
		last = alloc.allocate(1);
		alloc.construct(first, Type::LETTER, character);
		alloc.construct(last, Type::ACCEPT);
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

	void NFA::Concatenate(NFA& other)
	{
		last->succ1 = other.first;
		last->ty = Type::EPSILON;
		last = other.last;
		sz += other.sz;
		other.first = other.last = nullptr;
		other.sz = 0;
	}

	void NFA::Alternate(NFA& other)
	{
		NFAnode* newFirst = alloc.allocate(1);
		alloc.construct(newFirst, Type::EPSILON);
		NFAnode* newLast = alloc.allocate(1);
		alloc.construct(newLast, Type::ACCEPT);
		newFirst->succ1 = first;
		newFirst->succ2 = other.first;
		last->succ1 = newLast;
		other.last->succ1 = newLast;
		last->ty = Type::EPSILON;
		other.last->ty = Type::EPSILON;
		first = newFirst;
		last = newLast;
		sz += other.sz + 2;
		other.first = other.last = nullptr;
		other.sz = 0;
	}

	void NFA::Closure()
	{
		NFAnode* newFirst = alloc.allocate(1);
		alloc.construct(newFirst, Type::EPSILON);
		NFAnode* newLast = alloc.allocate(1);
		alloc.construct(newLast, Type::ACCEPT);
		newFirst->succ1 = first;
		newFirst->succ2 = newLast;
		last->succ1 = first;
		last->succ2 = newLast;
		last->ty = Type::EPSILON;
		sz += 2;
	}
}
