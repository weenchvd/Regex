#ifndef REGEXPR_TEST_HPP
#define REGEXPR_TEST_HPP

namespace RegexTest
{
	struct RegexVector {
		std::vector<RE::REstring> vec;
	};

	std::istream& operator>>(std::istream& is, RegexVector& rvector);
}

#endif // REGEXPR_TEST_HPP