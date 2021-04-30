
// Copyright (c) 2021 Vitaly Dikov
// 
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef REGEXPR_TEST_HPP
#define REGEXPR_TEST_HPP

namespace RegexTest
{
    struct RegexVector {
        std::vector<RE::UString> vec;
    };

    struct RegexMatchCase {
        RE::UString re;
        std::vector<RE::UString> valid;
        std::vector<RE::UString> invalid;
    };

    struct RegexMatch {
        std::vector<RegexMatchCase> vec;
    };

    struct RegexSearchCase {
        RE::UString text;
        RE::UString re;
        unsigned int nMatches;
        std::vector<RE::UString> valid;
        std::vector<unsigned int> lineNumber;
        std::vector<unsigned int> posInLine;
    };

    struct RegexSearch {
        std::vector<RegexSearchCase> vec;
    };

    struct InputBuffer {
        RE::UString str;
        bool full;

        InputBuffer()
            : full{ false } {}
        
        operator bool() { return full; }
    };

    extern const char32_t* dl;              // dash line

    std::basic_istream<char32_t>& operator>>(std::basic_istream<char32_t>& is, RegexVector& rvector);
    std::basic_istream<char32_t>& operator>>(std::basic_istream<char32_t>& is, RegexMatch& rmatch);
    std::basic_istream<char32_t>& operator>>(std::basic_istream<char32_t>& is, RegexSearch& rsearch);
    std::basic_ostream<char32_t>& operator<<(std::basic_ostream<char32_t>& os, const RegexVector& rvector);
    std::basic_ostream<char32_t>& operator<<(std::basic_ostream<char32_t>& os, const RegexMatchCase& rmcase);
    std::basic_ostream<char32_t>& operator<<(std::basic_ostream<char32_t>& os, const RegexSearchCase& rscase);
    std::basic_ostream<char32_t>& operator<<(std::basic_ostream<char32_t>& os, const RE::MatchResults& results);

    inline void PrintNumberOfTests(std::ostream& os, const size_t n);

    void ErrorReport(const std::string& reportFileName, const RegexVector& rvector,
        const RE::UString& errorList);

    void ErrorReport(const std::string& reportFileName, const RegexMatchCase& rmcase,
        const RE::UString& errorList);

    void ErrorReport(const std::string& reportFileName, const RegexSearchCase& rscase,
        const RE::UString& errorList);

    void ErrorReport(const std::string& reportFileName, const RegexSearchCase& rscase,
        const std::vector<RE::MatchResults>& results, const RE::UString& errorList);

    void RegexValidTest(const std::string& fileName);
    void RegexInvalidTest(const std::string& fileName);
    void RegexMatchTest(const std::string& fileName);
    void RegexSearchTest(const std::string& fileName);
}

#endif // REGEXPR_TEST_HPP
