
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
        RegexValidTest("RegexValid.txt");
    }

    TEST(RegexpTest, InvalidRegexes) {
        RegexInvalidTest("RegexInvalid.txt");
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
        RE::REstring p_RE;                  // prefix
        RE::REstring b_validMatch;          // border
        RE::REstring b_invalidMatch;        // border
        std::basic_istringstream<char32_t> iss{ secondLine };
        iss >> p_RE >> b_validMatch >> b_invalidMatch;
        if (!iss) {
            is.setstate(std::ios_base::failbit);
            return is;
        }
        InputBuffer buf;
        RegexMatch rm;
        while (is) {
            RegexMatchCase rmcase;
            RE::REstring separator;
            if (buf) {
                separator = buf.str;
                buf.full = false;
            }
            else {
                is >> separator;
                if (!is) return is;
            }
            // get 're'
            if (separator != p_RE) {
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
                is >> separator;
                if (!is) {
                    if (is.eof()) break;
                    else return is;
                }
                if (separator != b_validMatch && separator != b_invalidMatch) {
                    if (separator == p_RE) {
                        buf.str = separator;
                        buf.full = true;
                        break;
                    }
                    is.setstate(std::ios_base::failbit);
                    return is;
                }
                // get 'valid' or 'invalid'
                is.get();
                RE::REstring s;
                getline(is, s, separator[0]);
                if (!is) {
                    if (is.eof()) break;
                    else return is;
                }
                if (separator == b_validMatch) {
                    rmcase.valid.push_back(s);
                }
                else if (separator == b_invalidMatch) {
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
        InputBuffer buf;
        RegexSearch rs;
        while (is) {
            RegexSearchCase rscase;
            RE::REstring separator;
            if (buf) {
                separator = buf.str;
                buf.full = false;
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
                        buf.str = separator;
                        buf.full = true;
                        break;
                    }
                    is.setstate(std::ios_base::failbit);
                    return is;
                }
                rscase.valid.push_back(RE::REstring{});
                rscase.lineNumber.push_back(0);
                rscase.posInLine.push_back(0);
                // get 'valid'
                is.get();
                getline(is, rscase.valid[rscase.valid.size() - 1], separator[0]);
                if (!is) {
                    is.setstate(std::ios_base::failbit);
                    return is;
                }
                // get 'lineNumber' and 'posInLine'
                is >> separator;
                if (!is || separator != b_LCNumbers) {
                    is.setstate(std::ios_base::failbit);
                    return is;
                }
                is.get();
                /*is >> rscase.lineNumber[rscase.lineNumber.size() - 1]
                    >> rscase.posInLine[rscase.posInLine.size() - 1];*/
                { std::basic_string<char32_t> temp; is >> temp;
                    ToUInt(temp, rscase.lineNumber[rscase.lineNumber.size() - 1]); }
                { std::basic_string<char32_t> temp; getline(is, temp, separator[0]); is.unget();
                    ToUInt(temp, rscase.posInLine[rscase.posInLine.size() - 1]); }
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

    const char32_t* dl{ U"--------------------------------------------------" };

    std::basic_ostream<char32_t>& operator<<(std::basic_ostream<char32_t>& os, const RegexVector& rvector)
    {
        std::basic_string<char32_t> sp{ U"  " };
        for (int i = 0; i < rvector.vec.size(); ++i) {
            os << sp << U"RegexVector.vec[" << /*i*/ ToChar(i) << "]: " << rvector.vec[i] << std::endl;
        }
        return os;
    }

    std::basic_ostream<char32_t>& operator<<(std::basic_ostream<char32_t>& os, const RegexMatchCase& rmcase)
    {
        std::basic_string<char32_t> sp{ U"  " };
        os << sp << U"RegexMatchCase.re: " << rmcase.re << std::endl;
        for (int i = 0; i < rmcase.valid.size(); ++i) {
            os << sp + sp << U"RegexMatchCase.valid[" << /*i*/ ToChar(i) << "]: " << rmcase.valid[i] << std::endl;
        }
        for (int i = 0; i < rmcase.invalid.size(); ++i) {
            os << sp + sp << "RegexMatchCase.invalid[" << /*i*/ ToChar(i) << "]: " << rmcase.invalid[i] << std::endl;
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
            os << sp << U"RegexSearchCase[" << /*i*/ ToChar(i) << U"]: " << std::endl;
            os << sp + sp << U"RegexSearchCase.valid: " << rscase.valid[i] << std::endl;
            os << sp + sp << U"RegexSearchCase.lineNumber: "
                << /*rscase.lineNumber[i]*/ ToChar(rscase.lineNumber[i]) << std::endl;
            os << sp + sp << U"RegexSearchCase.posInLine: "
                << /*rscase.posInLine[i]*/ ToChar(rscase.posInLine[i]) << std::endl;
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
        os << "               Number of passed subtests in this test: " << n << std::endl;
    }

    void ErrorReport(const std::string& reportFileName, const RegexVector& rvector,
        const RE::REstring& errorList)
    {
        std::locale loc(std::locale(), new std::codecvt_utf8<char32_t>);
        std::basic_ofstream<char32_t> ofs{ reportFileName, std::ofstream::out | std::ofstream::app };
        if (!ofs) {
            Error::ErrPrint(std::cerr, Error::Level::ERROR, Error::Type::OUTFILE,
                "ErrorReport(). File: " + reportFileName);
            return;
        }
        ofs.imbue(loc);
        ofs << errorList << std::endl;
        ofs << U"RegexVector:" << std::endl;
        ofs << rvector << std::endl << dl << std::endl << std::endl;
    }

    void ErrorReport(const std::string& reportFileName, const RegexMatchCase& rmcase,
        const RE::REstring& errorList)
    {
        std::locale loc(std::locale(), new std::codecvt_utf8<char32_t>);
        std::basic_ofstream<char32_t> ofs{ reportFileName, std::ofstream::out | std::ofstream::app };
        if (!ofs) {
            Error::ErrPrint(std::cerr, Error::Level::ERROR, Error::Type::OUTFILE,
                "ErrorReport(). File: " + reportFileName);
            return;
        }
        ofs.imbue(loc);
        ofs << errorList << std::endl;
        ofs << U"RegexMatchCase:" << std::endl;
        ofs << rmcase << std::endl << dl << std::endl << std::endl;
    }

    void ErrorReport(const std::string& reportFileName, const RegexSearchCase& rscase,
        const RE::REstring& errorList)
    {
        std::locale loc(std::locale(), new std::codecvt_utf8<char32_t>);
        std::basic_ofstream<char32_t> ofs{ reportFileName, std::ofstream::out | std::ofstream::app };
        if (!ofs) {
            Error::ErrPrint(std::cerr, Error::Level::ERROR, Error::Type::OUTFILE,
                "ErrorReport(). File: " + reportFileName);
            return;
        }
        ofs.imbue(loc);
        ofs << errorList << std::endl;
        ofs << U"RegexSearchCase:" << std::endl;
        ofs << rscase << std::endl << dl << std::endl << std::endl;
    }

    void ErrorReport(const std::string& reportFileName, const RegexSearchCase& rscase,
        const std::vector<RE::MatchResults>& results, const RE::REstring& errorList)
    {
        std::locale loc(std::locale(), new std::codecvt_utf8<char32_t>);
        std::basic_ofstream<char32_t> ofs{ reportFileName, std::ofstream::out | std::ofstream::app };
        if (!ofs) {
            Error::ErrPrint(std::cerr, Error::Level::ERROR, Error::Type::OUTFILE,
                "ErrorReport(). File: " + reportFileName);
            return;
        }
        ofs.imbue(loc);
        ofs << errorList << std::endl;
        ofs << U"MatchResults:" << std::endl;
        for (size_t i = 0; i < results.size(); ++i) {
            ofs << U"  MatchResults[" << /*i*/ ToChar(i) << U"]:" << std::endl << results[i] << std::endl;
        }
        ofs << U"RegexSearchCase:" << std::endl;
        ofs << rscase << std::endl << dl << std::endl << std::endl;
    }

    void RegexValidTest(const std::string& fileName)
    {
        INIT_COUNTER;
        const std::string reportFileName{ "ErrorReport_" + fileName };
        std::remove(reportFileName.c_str());
        std::locale loc(std::locale(), new std::codecvt_utf8<char32_t>);
        std::basic_ifstream<char32_t> ifs{ fileName };
        ASSERT_TRUE(ifs) << "File: " << fileName;
        ifs.imbue(loc);
        RegexVector valid;
        ifs >> valid;
        ASSERT_TRUE(ifs.eof()) << "File: " << fileName;
        bool printReport{ false };
        std::basic_ostringstream<char32_t> oss;
        for (size_t i = 0; i < valid.vec.size(); ++i) {
            const bool noException{ true };
            const bool exception{ true };
            bool fail{ false };
            try {
                RE::Regexp re{ valid.vec[i] }; COUNT;
                EXPECT_TRUE(noException);
            }
            catch (const Error::InvalidRegex& e) {
                oss << U">>> Error: RE::Regexp{ \"" << valid.vec[i]
                    << U"\" }. Expect: No exception. Actual: Ctor threw 'InvalidRegex' exception" << std::endl
                    << U"      Case --> RegexVector.vec[" << /*i*/ ToChar(i) << U"]" << std::endl;
                printReport = true;
                fail = true;
                EXPECT_FALSE(exception);
            }
            catch (...) {
                oss << U">>> Error: RE::Regexp{ \"" << valid.vec[i]
                    << U"\" }. Expect: No exception. Actual: Someone threw an exception" << std::endl
                    << U"      Case --> RegexVector.vec[" << /*i*/ ToChar(i) << U"]" << std::endl;
                printReport = true;
                fail = true;
                EXPECT_FALSE(exception);
            }
            try {
                RE::Regexp re{ RE::REstring{'a'} }; COUNT;
                re.PutRE(valid.vec[i]);
                EXPECT_TRUE(noException);
            }
            catch (const Error::InvalidRegex& e) {
                oss << U">>> Error: RE::Regexp.PutRE(\"" << valid.vec[i]
                    << U"\"). Expect: No exception. Actual: Ctor threw 'InvalidRegex' exception" << std::endl
                    << U"      Case --> RegexVector.vec[" << /*i*/ ToChar(i) << U"]" << std::endl;
                printReport = true;
                fail = true;
                EXPECT_FALSE(exception);
            }
            catch (...) {
                oss << U">>> Error: RE::Regexp.PutRE(\"" << valid.vec[i]
                    << U"\"). Expect: No exception. Actual: Someone threw an exception" << std::endl
                    << U"      Case --> RegexVector.vec[" << /*i*/ ToChar(i) << U"]" << std::endl;
                printReport = true;
                fail = true;
                EXPECT_FALSE(exception);
            }
            if (!fail) {
                RE::Regexp re1{ valid.vec[i] };
                RE::Regexp re2{ RE::REstring{'a'} };
                re2.PutRE(valid.vec[i]);
                const bool equal{ re1 == re2 };
                EXPECT_TRUE(equal); COUNT;
                if (!equal) {
                    printReport = true;
                    oss << U">>> Error: re1 == re2. Expect: true. Actual: false" << std::endl;
                }
            }
        }
        if (printReport) {
            std::cout << ">>>> ERROR REPORT FILE: " << reportFileName << std::endl;
            ErrorReport(reportFileName, valid, oss.str());
        }
        PRINT_COUNTER;
    }

    void RegexInvalidTest(const std::string& fileName)
    {
        INIT_COUNTER;
        const std::string reportFileName{ "ErrorReport_" + fileName };
        std::remove(reportFileName.c_str());
        std::locale loc(std::locale(), new std::codecvt_utf8<char32_t>);
        std::basic_ifstream<char32_t> ifs{ fileName };
        ASSERT_TRUE(ifs) << "File: " << fileName;
        ifs.imbue(loc);
        RegexVector invalid;
        ifs >> invalid;
        ASSERT_TRUE(ifs.eof()) << "File: " << fileName;
        bool printReport{ false };
        std::basic_ostringstream<char32_t> oss;
        for (size_t i = 0; i < invalid.vec.size(); ++i) {
            const bool noException{ true };
            const bool exception{ true };
            try {
                RE::Regexp re{ invalid.vec[i] }; COUNT;
                oss << U">>> Error: RE::Regexp{ \"" << invalid.vec[i]
                    << U"\" }. Expect: InvalidRegex exception. Actual: No exception" << std::endl
                    << U"      Case --> RegexVector.vec[" << /*i*/ ToChar(i) << U"]" << std::endl;
                printReport = true;
                EXPECT_FALSE(noException);
            }
            catch (const Error::InvalidRegex& e) {
                EXPECT_TRUE(exception);
            }
            catch (...) {
                oss << U">>> Error: RE::Regexp{ \"" << invalid.vec[i]
                    << U"\" }. Expect: InvalidRegex exception. Actual: Someone threw an exception" << std::endl
                    << U"      Case --> RegexVector.vec[" << /*i*/ ToChar(i) << U"]" << std::endl;
                printReport = true;
                EXPECT_FALSE(exception);
            }
            try {
                RE::Regexp re{ RE::REstring{'a'} }; COUNT;
                re.PutRE(invalid.vec[i]);
                oss << U">>> Error: RE::Regexp.PutRE(\"" << invalid.vec[i]
                    << U"\"). Expect: InvalidRegex exception. Actual: No exception" << std::endl
                    << U"      Case --> RegexVector.vec[" << /*i*/ ToChar(i) << U"]" << std::endl;
                printReport = true;
                EXPECT_FALSE(noException);
            }
            catch (const Error::InvalidRegex& e) {
                EXPECT_TRUE(exception);
            }
            catch (...) {
                oss << U">>> Error: RE::Regexp.PutRE(\"" << invalid.vec[i]
                    << U"\"). Expect: InvalidRegex exception. Actual: Someone threw an exception" << std::endl
                    << U"      Case --> RegexVector.vec[" << /*i*/ ToChar(i) << U"]" << std::endl;
                printReport = true;
                EXPECT_FALSE(exception);
            }
        }
        if (printReport) {
            std::cout << ">>>> ERROR REPORT FILE: " << reportFileName << std::endl;
            ErrorReport(reportFileName, invalid, oss.str());
        }
        PRINT_COUNTER;
    }

    void RegexMatchTest(const std::string& fileName)
    {
        INIT_COUNTER;
        const std::string reportFileName{ "ErrorReport_" + fileName };
        std::remove(reportFileName.c_str());
        std::locale loc(std::locale(), new std::codecvt_utf8<char32_t>);
        std::basic_ifstream<char32_t> ifs{ fileName };
        ASSERT_TRUE(ifs) << "File: " << fileName;
        ifs.imbue(loc);
        RegexMatch rmatch;
        ifs >> rmatch;
        ASSERT_TRUE(ifs.eof()) << "File: " << fileName;
        bool printName{ false };
        for (size_t i = 0; i < rmatch.vec.size(); ++i) {
            const RegexMatchCase& rmcase = rmatch.vec[i];
            std::basic_ostringstream<char32_t> oss;
            oss << U"RegexMatchCase #" << /*i + 1*/ ToChar(i + 1) << std::endl << std::endl;
            try {
                RE::Regexp re1{ rmcase.re }; COUNT;
                RE::Regexp re2{ RE::REstring{'a'} };
                re2.PutRE(rmcase.re); COUNT;
                bool printReport{ false };
                for (size_t j = 0; j < rmcase.valid.size(); ++j) {
                    const bool match1{ re1.Match(rmcase.valid[j]) };
                    EXPECT_TRUE(match1); COUNT;
                    const bool match2{ re2.Match(rmcase.valid[j]) };
                    EXPECT_TRUE(match2); COUNT;
                    if (!match1 || !match2) {
                        printReport = true;
                        oss << U">>> Error: Regexp::Match(\"" << rmcase.valid[j]
                            << U"\"). Expect: true. Actual: false" << std::endl
                            << U"      Case --> RegexMatchCase.valid[" << /*j*/ ToChar(j) << U"]" << std::endl;
                    }
                }
                for (size_t j = 0; j < rmcase.invalid.size(); ++j) {
                    const bool match1{ re1.Match(rmcase.invalid[j]) };
                    EXPECT_FALSE(match1); COUNT;
                    const bool match2{ re2.Match(rmcase.invalid[j]) };
                    EXPECT_FALSE(match2); COUNT;
                    if (match1 || match2) {
                        printReport = true;
                        oss << U">>> Error: Regexp::Match(\"" << rmcase.invalid[j]
                            << U"\"). Expect: false. Actual: true" << std::endl
                            << U"      Case --> RegexMatchCase.invalid[" << /*j*/ ToChar(j) << U"]" << std::endl;
                    }
                }
                if (printReport) {
                    printName = true;
                    ErrorReport(reportFileName, rmcase, oss.str());
                }
            }
            catch (const Error::InvalidRegex& e) {
                oss << U">>> Error: Ctor threw 'InvalidRegex' exception" << std::endl;
                ErrorReport(reportFileName, rmcase, oss.str());
                bool exception{ true };
                EXPECT_FALSE(exception);
            }
            catch (...) {
                oss << U">>> Error: Someone threw an exception" << std::endl;
                ErrorReport(reportFileName, rmcase, oss.str());
                bool exception{ true };
                EXPECT_FALSE(exception);
            }
        }
        if (printName) {
            std::cout << ">>>> ERROR REPORT FILE: " << reportFileName << std::endl;
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
        bool printName{ false };
        for (size_t i = 0; i < rsearch.vec.size(); ++i) {
            const RegexSearchCase& rscase = rsearch.vec[i];
            std::basic_ostringstream<char32_t> oss;
            oss << U"RegexSearchCase #" << /*i + 1*/ ToChar(i + 1) << std::endl << std::endl;
            try {
                RE::Regexp re1{ rscase.re }; COUNT;
                RE::Regexp re2{ RE::REstring{'a'} };
                re2.PutRE(rscase.re); COUNT;
                bool printReport{ false };
                const bool equal{ re1 == re2 };
                EXPECT_TRUE(equal); COUNT;
                if (!equal) {
                    printReport = true;
                    oss << U">>> Error: re1 == re2. Expect: true. Actual: false" << std::endl;
                }
                std::vector<RE::MatchResults> results{ re1.Search(rscase.text) };
                bool sizes{ results.size() == rscase.nMatches };
                EXPECT_TRUE(sizes); COUNT;
                if (!sizes) {
                    printReport = true;
                    oss << U">>> Error: results.size() == rscase.nMatches. Expect: true. Actual: false" << std::endl;
                }
                for (size_t j = 0; j < rscase.valid.size(); ++j) {
                    bool found{ false };
                    for (const RE::MatchResults& mr : results) {
                        if (RE::REstring{ mr.str.first, mr.str.second } == rscase.valid[j]
                            && mr.ln == rscase.lineNumber[j]
                            && mr.pos == rscase.posInLine[j]) {
                            found = true;
                            break;
                        }
                    }
                    EXPECT_TRUE(found); COUNT;
                    if (!found) {
                        printReport = true;
                        oss << U">>> Error: String \"" << rscase.valid[j] << "\" not found AND/OR "
                            << U"Line number \"" << /*rscase.lineNumber[j]*/ ToChar(rscase.lineNumber[j])
                            << U"\" not found AND/OR "
                            << U"Position in the line \"" << /*rscase.posInLine[j]*/ ToChar(rscase.posInLine[j])
                            << U"\" not found" << std::endl
                            << U"      Case --> RegexSearchCase[" << /*j*/ ToChar(j) << U"]" << std::endl;
                    }
                }
                if (printReport) {
                    printName = true;
                    ErrorReport(reportFileName, rscase, results, oss.str());
                }
            }
            catch (const Error::InvalidRegex& e) {
                oss << U">>> Error: Ctor threw 'InvalidRegex' exception" << std::endl;
                ErrorReport(reportFileName, rscase, oss.str());
                bool exception{ true };
                EXPECT_FALSE(exception);
            }
            catch (...) {
                oss << U">>> Error: Someone threw an exception" << std::endl;
                ErrorReport(reportFileName, rscase, oss.str());
                bool exception{ true };
                EXPECT_FALSE(exception);
            }
        }
        if (printName) {
            std::cout << ">>>> ERROR REPORT FILE: " << reportFileName << std::endl;
        }
        PRINT_COUNTER;
    }
} // namespace RegexTest
