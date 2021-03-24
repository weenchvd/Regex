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
#include<locale>
#include<codecvt>
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
		std::locale loc(std::locale(), new std::codecvt_utf8<char32_t>);
		const std::string fileName{ "RegexValid.txt" };
		std::basic_ifstream<char32_t> ifs{ fileName };
		ASSERT_TRUE(ifs);
		ifs.imbue(loc);
		RegexVector valid;
		ifs >> valid;
		ASSERT_TRUE(ifs.eof());
		for (RE::REstring& s : valid.vec) {
			ASSERT_NO_THROW(RE::Regexp{ s }) << "RE: " << RE::GetGlyph(s); COUNT;
		}
		PRINT_COUNTER;
	}

	TEST(RegexpTest, InvalidRegexes) {
		INIT_COUNTER;
		std::locale loc(std::locale(), new std::codecvt_utf8<char32_t>);
		const std::string fileName{ "RegexInvalid.txt" };
		std::basic_ifstream<char32_t> ifs{ fileName };
		ASSERT_TRUE(ifs);
		ifs.imbue(loc);
		RegexVector invalid;
		ifs >> invalid;
		ASSERT_TRUE(ifs.eof());
		for (RE::REstring& s : invalid.vec) {
			ASSERT_THROW(RE::Regexp{ s }, Error::InvalidRegex) << "RE: " << RE::GetGlyph(s); COUNT;
		}
		PRINT_COUNTER;
	}

	TEST(RegexpTest, RegexMatch001) {
		RegexMatchTest("RegexMatch_001.txt");
	}

	TEST(RegexpTest, RegexMatch002) {
		RegexMatchTest("RegexMatch_002.txt");
	}

	TEST(RegexpTest, RegexMatch003) {
		RegexMatchTest("RegexMatch_003.txt");
	}

	TEST(RegexpTest, RegexMatch004) {
		RegexMatchTest("RegexMatch_004.txt");
	}

	///----------------------------------------------------------------------------------------------------

	std::basic_istream<char32_t>& operator>>(std::basic_istream<char32_t>& is, RegexVector& rvector)
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

	std::basic_istream<char32_t>& operator>>(std::basic_istream<char32_t>& is, RegexMatch& rmatch)
	{
		is.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // first line is comment, skip it
		RE::REstring secondLine;			// line of symbols
		getline(is, secondLine);
		if (!is) return is;
		RE::REstring prefixRE;
		RE::REstring prefixMatchValid;
		RE::REstring prefixMatchInvalid;
		std::basic_istringstream<char32_t> iss{ secondLine };
		iss >> prefixRE >> prefixMatchValid >> prefixMatchInvalid;
		if (!iss) {
			is.setstate(std::ios_base::failbit);
			return is;
		}
		InputBuffer ibuff;
		RegexMatch rm;
		while (is) {
			RegexMatchCase rmcase;
			RE::REstring prefix;
			if (ibuff) {
				prefix = ibuff.buf;
				ibuff.full = false;
			}
			else {
				is >> prefix;
				if (!is) return is;
			}
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
						ibuff.buf = prefix;
						ibuff.full = true;
						break;
					}
					is.setstate(std::ios_base::failbit);
					return is;
				}
				is.get();
				RE::REstring s;
				getline(is, s, prefix[0]);
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
			os << sp + sp << "RegexMatch.vec[" << i << "].re: " << RE::GetGlyph(rmatch.vec[i].re) << std::endl;
			os << sp + sp << "RegexMatch.vec[" << i << "].valid.size(): "
				<< rmatch.vec[i].valid.size() << std::endl;
			for (int j = 0; j < rmatch.vec[i].valid.size(); ++j) {

				os << sp + sp + sp << "RegexMatch.vec[" << i << "].valid[" << j << "]: "
					<< RE::GetGlyph(rmatch.vec[i].valid[j]) << std::endl;
			}
			os << sp + sp << "RegexMatch.vec[" << i << "].invalid.size(): "
				<< rmatch.vec[i].invalid.size() << std::endl;
			for (int j = 0; j < rmatch.vec[i].invalid.size(); ++j) {
				os << sp + sp + sp << "RegexMatch.vec[" << i << "].invalid[" << j << "]: "
					<< RE::GetGlyph(rmatch.vec[i].invalid[j]) << std::endl;
			}
		}
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const RegexMatchCase& rmcase)
	{
		std::string sp{ "  " };
		os << sp << "RegexMatchCase.re: " << RE::GetGlyph(rmcase.re) << std::endl;
		os << sp << "RegexMatchCase.valid.size(): " << rmcase.valid.size() << std::endl;
		for (int j = 0; j < rmcase.valid.size(); ++j) {
			os << sp + sp << "RegexMatchCase.valid[" << j << "]: " << RE::GetGlyph(rmcase.valid[j]) << std::endl;
		}
		os << sp << "RegexMatchCase.invalid.size(): " << rmcase.invalid.size() << std::endl;
		for (int j = 0; j < rmcase.invalid.size(); ++j) {
			os << sp + sp << "RegexMatchCase.invalid[" << j << "]: " << RE::GetGlyph(rmcase.invalid[j]) << std::endl;
		}
		return os;
	}

	inline void PrintNumberOfTests(std::ostream& os, const size_t n)
	{
		os << "               Number of subtests in this test: " << n << std::endl;
	}

	void RegexMatchTest(const std::string fileName)
	{
		INIT_COUNTER;
		std::locale loc(std::locale(), new std::codecvt_utf8<char32_t>);
		std::basic_ifstream<char32_t> ifs{ fileName };
		ASSERT_TRUE(ifs) << "File: " << fileName;
		ifs.imbue(loc);
		RegexMatch rmatch;
		ifs >> rmatch;
		ASSERT_TRUE(ifs.eof()) << "File: " << fileName;
		for (const RegexMatchCase& rmcase : rmatch.vec) {
			try {
				RE::Regexp re{ rmcase.re }; COUNT;
				for (const RE::REstring rs : rmcase.valid) {
					EXPECT_TRUE(re.Match(rs)) << "File: " << fileName << std::endl
						<< "rs: " << RE::GetGlyph(rs) << std::endl << rmcase; COUNT;
				}
				for (const RE::REstring rs : rmcase.invalid) {
					EXPECT_FALSE(re.Match(rs)) << "File: " << fileName << std::endl
						<< "rs: " << RE::GetGlyph(rs) << std::endl << rmcase; COUNT;
				}
			}
			catch (const Error::InvalidRegex& e) {
				std::cout << rmcase;
				FAIL() << "File: " << fileName << std::endl << "Ctor threw 'InvalidRegex' exception";
			}
			catch (...) {
				std::cout << rmcase;
				FAIL() << "File: " << fileName << std::endl << "Someone threw an exception";
			}
		}
		PRINT_COUNTER;
	}
} // namespace RegexTest
