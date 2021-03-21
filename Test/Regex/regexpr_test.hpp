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

	std::istream& operator>>(std::istream& is, RegexVector& rvector);
	std::istream& operator>>(std::istream& is, RegexMatch& rmatch);
	std::ostream& operator<<(std::ostream& os, const RegexMatch& rmatch);
	std::ostream& operator<<(std::ostream& os, const RegexMatchCase& rmcase);

	inline void PrintNumberOfTests(std::ostream& os, const size_t n);
	void RegexMatchTest(const std::string fileName);
}

#endif // REGEXPR_TEST_HPP