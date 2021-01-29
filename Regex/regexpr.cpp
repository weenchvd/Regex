#include<iostream>
#include<fstream>
#include<sstream>
#include<iomanip>
#include<string>
#include<vector>
#include<memory>
#include"../Error/error.hpp"
#include"regexpr.hpp"

int main()
{
	try
	{
		const std::string regexList{ "Regexes.txt" };
		//const std::string regexList{ R"(D:\_Code\Project_A\Build\Regexes.txt)" };
		std::ifstream ifs{ regexList };
		if (!ifs) {
			Error::ErrPrint(std::cerr, Error::Level::ERROR, Error::Type::INFILE, regexList);
			return 1;
		}
		const std::string output{ "DFA.txt" };
		//const std::string output{ R"(D:\_Code\Project_A\Build\DFA.txt)" };
		std::ofstream ofs{ output };
		if (!ofs) {
			Error::ErrPrint(std::cerr, Error::Level::ERROR, Error::Type::OUTFILE, output);
			return 1;
		}
		RE::REstring rs;
		ifs >> rs;
		//if (ifs || !ifs && !ifs.eof()) {
		//	Error::ErrPrint(std::cerr, Error::Level::ERROR, "File data read error");
		//	return 1;
		//}
		RE::Regexp regex{ rs };
		regex.PrintNFA(std::cout);

		std::cout << std::endl << "Success!" << std::endl;
		return 0;
	}
	catch (const Error::RuntimeError& e)
	{
		Error::ErrPrint(std::cerr, Error::Level::EXCEPTION, Error::Type::RUNTIME, e.what());
		return -1;
	}
	catch (const std::exception& e)
	{
		Error::ErrPrint(std::cerr, Error::Level::EXCEPTION, Error::Type::STD, e.what());
		return -2;
	}
}

namespace RE
{
	NFAnode* NFAnode::ptr = nullptr;
	std::allocator<NFAnode> NFA::alloc;

	void NFA::AddNodeToSet(std::vector<NFAnode*>& set, NFAnode* node) const
	{
		if (node == nullptr || node->mark == true) {
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

	NFAnode* NFA::CreateNFANode(NFAnode::Type type)
	{
		NFAnode* p = alloc.allocate(1, NFAnode::ptr);
		if (p == nullptr) {
			throw Error::RuntimeError{ "No memory allocated for NFA node" };
		}
		NFAnode::ptr = p;
		alloc.construct(p, type);
		return p;
	}

	NFAnode* NFA::CreateNFANode(NFAnode::Type type, Character character)
	{
		NFAnode* p = alloc.allocate(1, NFAnode::ptr);
		if (p == nullptr) {
			throw Error::RuntimeError{ "No memory allocated for NFA node" };
		}
		alloc.construct(p, type, character);
		NFAnode::ptr = p;
		return p;
	}

	NFA::NFA(Character character)
		: sz{ 2 }
	{
		first = CreateNFANode(NFAnode::Type::LETTER, character);
		last = CreateNFANode(NFAnode::Type::ACCEPT);
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

	// the function concatenates two NFAs via an Epsilon-transition
	void NFA::Concatenate(NFA& other)
	{
		last->succ1 = other.first;
		last->ty = NFAnode::Type::EPSILON;
		last = other.last;
		sz += other.sz;
		other.first = other.last = nullptr;
		other.sz = 0;
	}

	void NFA::Alternate(NFA& other)
	{
		NFAnode* newFirst = CreateNFANode(NFAnode::Type::EPSILON);
		NFAnode* newLast = CreateNFANode(NFAnode::Type::ACCEPT);
		newFirst->succ1 = first;
		newFirst->succ2 = other.first;
		last->succ1 = newLast;
		other.last->succ1 = newLast;
		last->ty = NFAnode::Type::EPSILON;
		other.last->ty = NFAnode::Type::EPSILON;
		first = newFirst;
		last = newLast;
		sz += other.sz + 2;
		other.first = other.last = nullptr;
		other.sz = 0;
	}

	// the function creates a Kleene (0 or more times) closure for NFA
	void NFA::ClosureKleene()
	{
		NFAnode* newFirst = CreateNFANode(NFAnode::Type::EPSILON);
		NFAnode* newLast = CreateNFANode(NFAnode::Type::ACCEPT);
		newFirst->succ1 = first;
		newFirst->succ2 = newLast;
		last->succ1 = first;
		last->succ2 = newLast;
		last->ty = NFAnode::Type::EPSILON;
		sz += 2;
	}

	std::pair<Character, Regexp::TokenStream::TokenType> Regexp::TokenStream::GetToken() const
	{
		if (pos >= s.size()) {
			return { 0, TokenType::EOS };
		}
		Character ch = s[pos];
		switch (ch) {
		case '(':
		case ')':
		case '|':
		case '*':
		case '+':
		case '?':
			return { ch, TokenType::SPECIAL };
		default:
			return { ch, TokenType::LITERAL };
		}
	}

	void Regexp::MakeDFA()
	{
		REtoNFA();
		NFAtoDFA();
		MinimizeDFA();
	}

	void Regexp::REtoNFA()
	{
		nfa = PGoal();
	}

	void Regexp::NFAtoDFA()
	{

	}

	void Regexp::MinimizeDFA()
	{

	}

	// parse Goal
	NFA Regexp::PGoal()
	{
		token = ts.GetToken();
		return PAlternation();
	}

	// parse Alternation
	NFA Regexp::PAlternation()
	{
		NFA a = PConcatenation();
		PAlternationPrime(a);
		return a;
	}

	// parse Alternation'
	void Regexp::PAlternationPrime(NFA& a)
	{
		switch (token.second) {
		case Regexp::TokenStream::TokenType::EOS:
			return;
		case Regexp::TokenStream::TokenType::SPECIAL:
			switch (token.first) {
			case ')':
				return;
			case '|':
				token = ts.AdvanceAndGetToken();
				a.Alternate(PConcatenation());
				PAlternationPrime(a);
				return;
			default:
				break;
			}
		case Regexp::TokenStream::TokenType::LITERAL:
			break;
		default:
			break;
		}
		ThrowInvalidRegex(ts.GetPosition());
	}

	// parse Concatenation
	NFA Regexp::PConcatenation()
	{
		NFA a = PSymbol();
		PConcatenationPrime(a);
		return a;
	}

	// parse Concatenation'
	void Regexp::PConcatenationPrime(NFA& a)
	{
		switch (token.second) {
		case Regexp::TokenStream::TokenType::EOS:
			return;
		case Regexp::TokenStream::TokenType::SPECIAL:
			switch (token.first) {
			case '(': {
				a.Concatenate(PSymbol());
				PConcatenationPrime(a);
				return;
			}
			case ')':
			case '|':
				return;
			default:
				break;
			}
		case Regexp::TokenStream::TokenType::LITERAL:
			a.Concatenate(PSymbol());
			PConcatenationPrime(a);
			return;
		default:
			break;
		}
		ThrowInvalidRegex(ts.GetPosition());
	}

	// parse Symbol
	NFA Regexp::PSymbol()
	{
		NFA a = PBlock();
		PClosure(a);
		return a;
	}

	// parse Block
	NFA Regexp::PBlock()
	{
		switch (token.second) {
		case Regexp::TokenStream::TokenType::EOS:
			break;
		case Regexp::TokenStream::TokenType::SPECIAL:
			switch (token.first) {
			case '(': {
				token = ts.AdvanceAndGetToken();
				NFA a = PAlternation();
				if (token.first == ')') {
					token = ts.AdvanceAndGetToken();
					return a;
				}
				break;
			}
			default:
				break;
			}
			break;
		case Regexp::TokenStream::TokenType::LITERAL: {
			Character ch = token.first;
			token = ts.AdvanceAndGetToken();
			return NFA{ ch };
		}
		default:
			break;
		}
		ThrowInvalidRegex(ts.GetPosition());
	}

	// parse Closure
	void Regexp::PClosure(NFA& a)
	{
		switch (token.second) {
		case Regexp::TokenStream::TokenType::EOS:
			return;
		case Regexp::TokenStream::TokenType::SPECIAL:
			switch (token.first) {
			case '*':
				a.ClosureKleene();
				token = ts.AdvanceAndGetToken();
				return;
			//case '+':
			//	
			//	token = ts.AdvanceAndGetToken();
			//	return;
			//case '?':
			//	
			//	token = ts.AdvanceAndGetToken();
			//	return;
			case '(':
			case ')':
			case '|':
				return;
			default:
				break;
			}
			break;
		case Regexp::TokenStream::TokenType::LITERAL:
			return;
		default:
			break;
		}
		ThrowInvalidRegex(ts.GetPosition());
	}

	void Regexp::ThrowInvalidRegex(size_t position) const
	{
		std::string message{ "Invalid character '" };
		message += std::string{ source, position, 1 } + "' was encountered after substring '";
		message += std::string{ source, 0, position } + "'. Regular expression: ";
		message += source;
		throw Error::InvalidRegex{ message };
	}

	Regexp::Regexp(const REstring& string)
		: ts{ string }
	{
		PutRE(string);
	}

	void Regexp::PrintNFA(std::ostream& os) const
	{
		if (nfa.sz == 0) {
			os << "The number of NFA nodes is 0" << std::endl;
			return;
		}
		const std::string dl{ "--------------------------------------------------------------------------------" }; // dash line
		const std::string cs{ "| " };		// column start
		const std::string ce{ " |" };		// column end
		constexpr int cw1{ 6 };				// column width 1
		const size_t cw2{ dl.size() - cw1 - cs.size() * 2 - ce.size() };

		os << std::endl << dl << std::endl;
		os << cs << std::setw(cw1) << std::left << "#"
			<< cs << std::setw(cw2) << "NFA" << ce << std::endl << dl << std::endl;
		std::vector<NFAnode*> nodes;
		nodes.reserve(nfa.sz);
		nfa.AddNodeToSet(nodes, nfa.first);
		for (int i = 0; i < nodes.size(); ++i) {
			NFAnode* p = nodes[i];
			p->mark = false;
			os << cs << std::setw(cw1) << i + 1 << cs;
			switch (p->ty) {
			case RE::NFAnode::Type::LETTER:
				os << std::setw(cw2) << char(p->ch) << ce << std::endl << dl << std::endl;
				break;
			case RE::NFAnode::Type::EPSILON:
				os << std::setw(cw2) << char(p->ch) << ce << std::endl << dl << std::endl;
				break;
			case RE::NFAnode::Type::ACCEPT:
				os << std::setw(cw2) << char(p->ch) << ce << std::endl << dl << std::endl;
				break;
			default:
				os << "Unknown RE::NFAnode::Type" << std::endl;
				break;
			}
		}
		// TODO
	}

	void Regexp::PutRE(const REstring& string)
	{
		if (string.size() > 0) {
			source = string;
			MakeDFA();
		}
	}

	std::istream& operator>>(std::istream& is, REstring& string)
	{
		std::string line;
		while (is) {
			getline(is, line);
			if (is) {
				string += line;
			}
		}
		return is;
	}
}
