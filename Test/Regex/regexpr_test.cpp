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
#include "Regex/regexpr.hpp"
#include "Error/error.hpp"
#include "gtest/gtest.h"
#include "regexpr_test.hpp"

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
		const std::string fileName{ "ValidRegexes.txt" };
		std::ifstream ifs{ fileName };
		ASSERT_TRUE(ifs);
		RegexVector valid;
		ifs >> valid;
		ASSERT_TRUE(ifs.eof());
		for (RE::REstring& s : valid.vec) {
			ASSERT_NO_THROW(RE::Regexp{ s }) << "RE: " << s;
		}
	}

	TEST(RegexpTest, InvalidRegexes) {
		const std::string fileName{ "InvalidRegexes.txt" };
		std::ifstream ifs{ fileName };
		ASSERT_TRUE(ifs);
		RegexVector invalid;
		ifs >> invalid;
		ASSERT_TRUE(ifs.eof());
		for (RE::REstring& s : invalid.vec) {
			ASSERT_THROW(RE::Regexp{ s }, Error::InvalidRegex) << "RE: " << s;
		}
	}



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

} // namespace RegexTest