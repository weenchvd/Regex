
// Copyright (c) 2021 Vitaly Dikov
// 
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

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
#include<cstdio>
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

    TEST(RegexpTest, RegexSearch001) {
        RegexSearchTest("RegexSearch_001.txt");
    }

    TEST(RegexpTest, RegexSearch002) {
        RegexSearchTest("RegexSearch_002.txt");
    }

    ///----------------------------------------------------------------------------------------------------

    std::basic_string<char32_t> ToChar(unsigned int x)
    {
        std::ostringstream oss;
        oss << x;
        std::basic_string<char32_t> s;
        for (char ch : oss.str()) {
            s += char32_t(ch);
        }
        return s;
    }

    void ToUInt(const std::basic_string<char32_t>& source, unsigned int& x)
    {
        std::string s;
        for (char32_t ch : source) {
            s += char(ch);
        }
        std::istringstream iss{ s };
        iss >> x;
    }

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
        RE::REstring secondLine;            // line of symbols
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

    std::basic_istream<char32_t>& operator>>(std::basic_istream<char32_t>& is, RegexSearch& rsearch)
    {
        is.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // first line is comment, skip it
        RE::REstring secondLine;            // line of symbols
        getline(is, secondLine);
        if (!is) return is;
        RE::REstring b_text;                // border
        RE::REstring p_RE;                  // prefix
        RE::REstring p_nMatches;            // prefix
        RE::REstring b_validMatch;          // border
        RE::REstring b_LCNumbers;           // border
        std::basic_istringstream<char32_t> iss{ secondLine };
        iss >> b_text >> p_RE >> p_nMatches >> b_validMatch >> b_LCNumbers;
        if (!iss) {
            is.setstate(std::ios_base::failbit);
            return is;
        }
        InputBuffer ibuff;
        RegexSearch rs;
        while (is) {
            RegexSearchCase rscase;
            RE::REstring separator;
            if (ibuff) {
                separator = ibuff.buf;
                ibuff.full = false;
            }
            else {
                is >> separator;
                if (!is) return is;
            }
            // get 'text'
            if (separator != b_text) {
                is.setstate(std::ios_base::failbit);
                return is;
            }
            is.get();
            getline(is, rscase.text, separator[0]);
            if (!is) {
                is.setstate(std::ios_base::failbit);
                return is;
            }
            // get 're'
            is >> separator;
            if (!is || separator != p_RE) {
                is.setstate(std::ios_base::failbit);
                return is;
            }
            is.get();
            getline(is, rscase.re);
            if (!is) {
                is.setstate(std::ios_base::failbit);
                return is;
            }
            // get 'nMatches'
            is >> separator;
            if (!is || separator != p_nMatches) {
                is.setstate(std::ios_base::failbit);
                return is;
            }
            is.get();
            /*is >> rscase.nMatches;*/
            { std::basic_string<char32_t> temp; is >> temp; ToUInt(temp, rscase.nMatches); }
            if (!is) {
                is.setstate(std::ios_base::failbit);
                return is;
            }
            while (true) {
                is >> separator;
                if (!is) {
                    if (is.eof()) break;
                    else return is;
                }
                if (separator != b_validMatch) {
                    if (separator == b_text) {
                        ibuff.buf = separator;
                        ibuff.full = true;
                        break;
                    }
                    is.setstate(std::ios_base::failbit);
                    return is;
                }
                rscase.valid.push_back(RE::REstring{});
                rscase.lineNumber.push_back(0);
                rscase.charNumber.push_back(0);
                // get 'valid'
                is.get();
                getline(is, rscase.valid[rscase.valid.size() - 1], separator[0]);
                if (!is) {
                    is.setstate(std::ios_base::failbit);
                    return is;
                }
                // get 'lineNumber' and 'charNumber'
                is >> separator;
                if (!is || separator != b_LCNumbers) {
                    is.setstate(std::ios_base::failbit);
                    return is;
                }
                is.get();
                /*is >> rscase.lineNumber[rscase.lineNumber.size() - 1]
                    >> rscase.charNumber[rscase.charNumber.size() - 1];*/
                { std::basic_string<char32_t> temp; is >> temp;
                    ToUInt(temp, rscase.lineNumber[rscase.lineNumber.size() - 1]); }
                { std::basic_string<char32_t> temp; getline(is, temp, separator[0]); is.unget();
                    ToUInt(temp, rscase.charNumber[rscase.charNumber.size() - 1]); }
                if (!is) {
                    is.setstate(std::ios_base::failbit);
                    return is;
                }
                is >> separator;
                if (!is || separator != b_LCNumbers) {
                    is.setstate(std::ios_base::failbit);
                    return is;
                }
            }
            rs.vec.push_back(rscase);
        }
        rsearch = rs;
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

    std::basic_ostream<char32_t>& operator<<(std::basic_ostream<char32_t>& os, const RegexSearchCase& rscase)
    {
        std::basic_string<char32_t> sp{ U"  " };
        os << sp << U"RegexSearchCase.text:" << std::endl << rscase.text << std::endl;
        os << sp << U"RegexSearchCase.re: " << rscase.re << std::endl;
        os << sp << U"RegexSearchCase.nMatches: " << /*rscase.nMatches*/ ToChar(rscase.nMatches) << std::endl;
        for (size_t i = 0; i < rscase.valid.size(); ++i) {
            os << sp + sp << U"RegexSearchCase.valid[" << /*i*/ ToChar(i) << U"]: " << rscase.valid[i] << std::endl;
            os << sp + sp << U"RegexSearchCase.lineNumber[" << /*i*/ ToChar(i) << U"]: "
                << /*rscase.lineNumber[i]*/ ToChar(rscase.lineNumber[i]) << std::endl;
            os << sp + sp << U"RegexSearchCase.charNumber[" << /*i*/ ToChar(i) << U"]: "
                << /*rscase.charNumber[i]*/ ToChar(rscase.charNumber[i]) << std::endl;
        }
        return os;
    }

    std::basic_ostream<char32_t>& operator<<(std::basic_ostream<char32_t>& os, const RE::MatchResults& results)
    {
        std::basic_string<char32_t> sp{ U"  " };
        os << sp + sp << U"MatchResults.str: "
            << std::basic_string<char32_t>{ results.str.first, results.str.second } << std::endl;
        os << sp + sp << U"MatchResults.ln: " << /*results.ln*/ ToChar(results.ln) << std::endl;
        os << sp + sp << U"MatchResults.pos: " << /*results.pos*/ ToChar(results.pos) << std::endl;
        return os;
    }

    inline void PrintNumberOfTests(std::ostream& os, const size_t n)
    {
        os << "               Number of subtests in this test: " << n << std::endl;
    }

    const std::string& ErrorReport(const std::string& reportFileName, const RegexSearchCase& rscase)
    {
        std::locale loc(std::locale(), new std::codecvt_utf8<char32_t>);
        std::basic_ofstream<char32_t> ofs{ reportFileName, std::ofstream::out | std::ofstream::app };
        if (!ofs) {
            Error::ErrPrint(std::cerr, Error::Level::ERROR, Error::Type::OUTFILE,
                "ErrorReport(). File: " + reportFileName);
            return std::string{};
        }
        ofs.imbue(loc);
        ofs << U"RegexSearchCase" << std::endl;
        ofs << rscase << std::endl;
        return reportFileName;
    }

    const std::string& ErrorReport(const std::string& reportFileName, const RegexSearchCase& rscase,
        const std::vector<RE::MatchResults>& results)
    {
        std::locale loc(std::locale(), new std::codecvt_utf8<char32_t>);
        std::basic_ofstream<char32_t> ofs{ reportFileName, std::ofstream::out | std::ofstream::app };
        if (!ofs) {
            Error::ErrPrint(std::cerr, Error::Level::ERROR, Error::Type::OUTFILE,
                "ErrorReport(). File: " + reportFileName);
            return std::string{};
        }
        ofs.imbue(loc);
        ofs << U"MatchResults" << std::endl;
        for (size_t i = 0; i < results.size(); ++i) {
            ofs << U"  MatchResults[" << /*i*/ ToChar(i) << U"]:" << std::endl << results[i] << std::endl;
        }
        ofs << U"RegexSearchCase" << std::endl;
        ofs << rscase << std::endl;
        return reportFileName;
    }

    void RegexMatchTest(const std::string& fileName)
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
                for (const RE::REstring& rs : rmcase.valid) {
                    EXPECT_TRUE(re.Match(rs)) << "File: " << fileName << std::endl
                        << "rs: " << RE::GetGlyph(rs) << std::endl << rmcase; COUNT;
                }
                for (const RE::REstring& rs : rmcase.invalid) {
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

    void RegexSearchTest(const std::string& fileName)
    {
        INIT_COUNTER;
        const std::string reportFileName{ "ErrorReport_" + fileName };
        std::remove(reportFileName.c_str());
        std::locale loc(std::locale(), new std::codecvt_utf8<char32_t>);
        std::basic_ifstream<char32_t> ifs{ fileName };
        ASSERT_TRUE(ifs) << "File: " << fileName;
        ifs.imbue(loc);
        RegexSearch rsearch;
        ifs >> rsearch;
        ASSERT_TRUE(ifs.eof()) << "File: " << fileName;
        for (const RegexSearchCase& rscase : rsearch.vec) {
            try {
                RE::Regexp re{ rscase.re }; COUNT;
                std::vector<RE::MatchResults> results{ re.Search(rscase.text) };
                int condition{ 1 };
                condition &= (results.size() == rscase.nMatches ? 1 : 0);
                EXPECT_TRUE(results.size() == rscase.nMatches); COUNT;
                for (size_t i = 0; i < rscase.valid.size(); ++i) {
                    bool found{ false };
                    for (const RE::MatchResults& mr : results) {
                        if (RE::REstring{ mr.str.first, mr.str.second } == rscase.valid[i]
                            && mr.ln == rscase.lineNumber[i]
                            && mr.pos == rscase.charNumber[i]) {
                            found = true;
                            break;
                        }
                    }
                    condition &= (found ? 1 : 0);
                    EXPECT_TRUE(found); COUNT;
                }
                if (!condition) {
                    std::cout << "Error report file: " << ErrorReport(reportFileName, rscase, results) << std::endl;
                }
            }
            catch (const Error::InvalidRegex& e) {
                FAIL() << "Ctor threw 'InvalidRegex' exception" << std::endl
                    << "File: " << fileName << std::endl
                    << "Error report file: " << ErrorReport(reportFileName, rscase);
            }
            catch (...) {
                FAIL() << "Someone threw an exception" << std::endl
                    << "File: " << fileName << std::endl
                    << "Error report file: " << ErrorReport(reportFileName, rscase);
            }
        }
        PRINT_COUNTER;
    }
} // namespace RegexTest
