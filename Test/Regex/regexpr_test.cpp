#include<iostream>
#include<fstream>
#include<sstream>
#include<iomanip>
#include<string>
#include<vector>
#include<set>
#include<unordered_map>
#include<queue>
#include<algorithm>
#include<memory>
#include<cctype>
#include<limits>
#include "Regex/regexpr.hpp"
#include "Error/error.hpp"
#include "gtest/gtest.h"
#include "regexpr_test.hpp"

#define INIT_COUNTER size_t counter{ 0 }
#define COUNT ++counter
#define PRINT_COUNTER RegexTest::PrintNumberOfTests(std::cout, counter)

namespace RegexTest
{
	class NFAnodeTest : public ::testing::Test {
	protected:
		RE::NFAnode n1{ RE::NFAnode::Type::ACCEPT };
		RE::NFAnode n2{ RE::NFAnode::Type::EPSILON };
		RE::NFAnode n3{ RE::NFAnode::Type::LITERAL, RE::Character{'a'} };
	};
	
	TEST_F(NFAnodeTest, NFAnodeMembers) {
		ASSERT_EQ(n1.succ1, nullptr);
		ASSERT_EQ(n1.succ2, nullptr);
		ASSERT_EQ(n1.ty, RE::NFAnode::Type::ACCEPT);
		ASSERT_EQ(n1.ch, RE::CHARFL_NOTCHAR);
		ASSERT_EQ(n1.mark, false);
	
		ASSERT_EQ(n2.succ1, nullptr);
		ASSERT_EQ(n2.succ2, nullptr);
		ASSERT_EQ(n2.ty, RE::NFAnode::Type::EPSILON);
		ASSERT_EQ(n2.ch, RE::CHARFL_NOTCHAR);
		ASSERT_EQ(n2.mark, false);
	
		ASSERT_EQ(n3.succ1, nullptr);
		ASSERT_EQ(n3.succ2, nullptr);
		ASSERT_EQ(n3.ty, RE::NFAnode::Type::LITERAL);
		ASSERT_EQ(n3.ch, 'a');
		ASSERT_EQ(n3.mark, false);
	}
	
	class NFATest : public ::testing::Test {
	protected:
		RE::NFA nfa1{ RE::Character{'b'} };
		RE::NFA nfa2{ RE::Character{'='} };
	};

	TEST_F(NFATest, NFAMembers) {
		ASSERT_EQ(nfa1.Size(), 2);
		RE::NFAnode* first1 = nfa1.GetFirstNode();
		RE::NFAnode* last1 = nfa1.GetLastNode();
		ASSERT_EQ(first1->succ1, last1);
		ASSERT_EQ(first1->succ2, nullptr);
		ASSERT_EQ(first1->ty, RE::NFAnode::Type::LITERAL);
		ASSERT_EQ(first1->ch, 'b');
		ASSERT_EQ(first1->mark, false);
		ASSERT_EQ(last1->succ1, nullptr);
		ASSERT_EQ(last1->succ2, nullptr);
		ASSERT_EQ(last1->ty, RE::NFAnode::Type::ACCEPT);
		ASSERT_EQ(last1->ch, RE::CHARFL_NOTCHAR);
		ASSERT_EQ(last1->mark, false);

		ASSERT_EQ(nfa2.Size(), 2);
		RE::NFAnode* first2 = nfa2.GetFirstNode();
		RE::NFAnode* last2 = nfa2.GetLastNode();
		ASSERT_EQ(first2->succ1, last2);
		ASSERT_EQ(first2->succ2, nullptr);
		ASSERT_EQ(first2->ty, RE::NFAnode::Type::LITERAL);
		ASSERT_EQ(first2->ch, '=');
		ASSERT_EQ(first2->mark, false);
		ASSERT_EQ(last2->succ1, nullptr);
		ASSERT_EQ(last2->succ2, nullptr);
		ASSERT_EQ(last2->ty, RE::NFAnode::Type::ACCEPT);
		ASSERT_EQ(last2->ch, RE::CHARFL_NOTCHAR);
		ASSERT_EQ(last2->mark, false);
	}

	TEST(RegexpTest, ValidRegexes) {
		INIT_COUNTER;
		const std::string fileName{ "ValidRegexes.txt" };
		std::ifstream ifs{ fileName };
		ASSERT_TRUE(ifs);
		RegexVector valid;
		ifs >> valid;
		ASSERT_TRUE(ifs.eof());
		for (RE::REstring& s : valid.vec) {
			ASSERT_NO_THROW(RE::Regexp{ s }) << "RE: " << s; COUNT;
		}
		PRINT_COUNTER;
	}

	TEST(RegexpTest, InvalidRegexes) {
		INIT_COUNTER;
		const std::string fileName{ "InvalidRegexes.txt" };
		std::ifstream ifs{ fileName };
		ASSERT_TRUE(ifs);
		RegexVector invalid;
		ifs >> invalid;
		ASSERT_TRUE(ifs.eof());
		for (RE::REstring& s : invalid.vec) {
			ASSERT_THROW(RE::Regexp{ s }, Error::InvalidRegex) << "RE: " << s; COUNT;
		}
		PRINT_COUNTER;
	}

	TEST(RegexpTest, RegexMatch001) {
		INIT_COUNTER;
		const std::string fileName{ "RegexMatch_001.txt" };
		std::ifstream ifs{ fileName };
		ASSERT_TRUE(ifs);
		RegexMatch rmatch;
		ifs >> rmatch;
		ASSERT_TRUE(ifs.eof());
		for (const RegexMatchCase& rmcase : rmatch.vec) {
			try {
				RE::Regexp re{ rmcase.re }; COUNT;
				for (const RE::REstring rs : rmcase.valid) {
					EXPECT_TRUE(re.Match(rs)) << "rs: " << rs << std::endl << rmcase; COUNT;
				}
				for (const RE::REstring rs : rmcase.invalid) {
					EXPECT_FALSE(re.Match(rs)) << "rs: " << rs << std::endl << rmcase; COUNT;
				}
			}
			catch (const Error::InvalidRegex& e) {
				std::cout << rmcase;
				FAIL() << "Ctor threw 'InvalidRegex' exception";
			}
			catch (...) {
				std::cout << rmcase;
				FAIL() << "Someone threw an exception";
			}
		}
		PRINT_COUNTER;
	}

	///----------------------------------------------------------------------------------------------------

	std::istream& operator>>(std::istream& is, RegexVector& rvector)
	{
		RE::REstring line;
		while (is) {
			getline(is, line);
			if (is) {
				rvector.vec.push_back(line);
			}
		}
		return is;
	}

	std::istream& operator>>(std::istream& is, RegexMatch& rmatch)
	{
		is.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // first line is comment, skip it
		RE::REstring secondLine;			// line of symbols
		getline(is, secondLine);
		if (!is) return is;
		RE::REstring prefixRE;
		RE::REstring prefixMatchValid;
		RE::REstring prefixMatchInvalid;
		std::istringstream iss{ secondLine };
		iss >> prefixRE >> prefixMatchValid >> prefixMatchInvalid;
		if (!iss) {
			is.setstate(std::ios_base::failbit);
			return is;
		}
		RegexMatch rm;
		while (is) {
			RegexMatchCase rmcase;
			RE::REstring prefix;
			is >> prefix;
			if (!is) return is;
			if (prefix != prefixRE) {
				is.setstate(std::ios_base::failbit);
				return is;
			}
			is.get();
			getline(is, rmcase.re);
			if (!is) {
				is.setstate(std::ios_base::failbit);
				return is;
			}
			while (true) {
				is >> prefix;
				if (!is) {
					if (is.eof()) break;
					else return is;
				}
				if (prefix != prefixMatchValid && prefix != prefixMatchInvalid) {
					if (prefix == prefixRE) {
						for (int i = 0; i < prefixRE.size(); ++i) {
							is.unget();
						}
						break;
					}
					is.setstate(std::ios_base::failbit);
					return is;
				}
				is.get();
				RE::REstring s;
				getline(is, s);
				if (!is) {
					if (is.eof()) break;
					else return is;
				}
				if (prefix == prefixMatchValid) {
					rmcase.valid.push_back(s);
				}
				else if (prefix == prefixMatchInvalid) {
					rmcase.invalid.push_back(s);
				}
				else {
					is.setstate(std::ios_base::failbit);
					return is;
				}
			}
			rm.vec.push_back(rmcase);
		}
		rmatch = rm;
		return is;
	}

	std::ostream& operator<<(std::ostream& os, const RegexMatch& rmatch)
	{
		std::string sp{ "  " };
		os << sp << "RegexMatch.vec.size(): " << rmatch.vec.size() << std::endl;
		for (int i = 0; i < rmatch.vec.size(); ++i) {
			os << sp + sp << "RegexMatch.vec[" << i << "].re: " << rmatch.vec[i].re << std::endl;
			os << sp + sp << "RegexMatch.vec[" << i << "].valid.size(): "
				<< rmatch.vec[i].valid.size() << std::endl;
			for (int j = 0; j < rmatch.vec[i].valid.size(); ++j) {
				os << sp + sp + sp << "RegexMatch.vec[" << i << "].valid[" << j << "]: "
					<< rmatch.vec[i].valid[j] << std::endl;
			}
			os << sp + sp << "RegexMatch.vec[" << i << "].invalid.size(): "
				<< rmatch.vec[i].invalid.size() << std::endl;
			for (int j = 0; j < rmatch.vec[i].invalid.size(); ++j) {
				os << sp + sp + sp << "RegexMatch.vec[" << i << "].invalid[" << j << "]: "
					<< rmatch.vec[i].invalid[j] << std::endl;
			}
		}
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const RegexMatchCase& rmcase)
	{
		std::string sp{ "  " };
		os << sp << "RegexMatchCase.re: " << rmcase.re << std::endl;
		os << sp << "RegexMatchCase.valid.size(): " << rmcase.valid.size() << std::endl;
		for (int j = 0; j < rmcase.valid.size(); ++j) {
			os << sp + sp << "RegexMatchCase.valid[" << j << "]: " << rmcase.valid[j] << std::endl;
		}
		os << sp << "RegexMatchCase.invalid.size(): " << rmcase.invalid.size() << std::endl;
		for (int j = 0; j < rmcase.invalid.size(); ++j) {
			os << sp + sp << "RegexMatchCase.invalid[" << j << "]: " << rmcase.invalid[j] << std::endl;
		}
		return os;
	}

	inline void PrintNumberOfTests(std::ostream& os, const size_t n)
	{
		os << "               Number of subtests in this test: " << n << std::endl;
	}
} // namespace RegexTest