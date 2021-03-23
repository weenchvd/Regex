#ifndef REGEXPR_TEST_HPP
#define REGEXPR_TEST_HPP

namespace RegexTest
{
	struct RegexVector {
		std::vector<RE::REstring> vec;
	};

	struct RegexMatchCase {
		RE::REstring re;
		std::vector<RE::REstring> valid;
		std::vector<RE::REstring> invalid;
	};

	struct RegexMatch {
		std::vector<RegexMatchCase> vec;
	};

	struct InputBuffer {
		RE::REstring buf;
		bool full;

		InputBuffer()
			: full{ false } {}
		
		operator bool() { return full; }
	};

	std::basic_istream<char32_t>& operator>>(std::basic_istream<char32_t>& is, RegexVector& rvector);
	std::basic_istream<char32_t>& operator>>(std::basic_istream<char32_t>& is, RegexMatch& rmatch);
	std::ostream& operator<<(std::ostream& os, const RegexMatch& rmatch);
	std::ostream& operator<<(std::ostream& os, const RegexMatchCase& rmcase);

	inline void PrintNumberOfTests(std::ostream& os, const size_t n);
	void RegexMatchTest(const std::string fileName);
}

#endif // REGEXPR_TEST_HPP