
// Copyright (c) 2021 Vitaly Dikov
// 
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

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

    struct RegexSearchCase {
        RE::REstring text;
        RE::REstring re;
        unsigned int nMatches;
        std::vector<RE::REstring> valid;
        std::vector<unsigned int> lineNumber;
        std::vector<unsigned int> charNumber;
    };

    struct RegexSearch {
        std::vector<RegexSearchCase> vec;
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
    std::basic_istream<char32_t>& operator>>(std::basic_istream<char32_t>& is, RegexSearch& rsearch);
    std::ostream& operator<<(std::ostream& os, const RegexMatch& rmatch);
    std::ostream& operator<<(std::ostream& os, const RegexMatchCase& rmcase);
    std::basic_ostream<char32_t>& operator<<(std::basic_ostream<char32_t>& os, const RegexSearchCase& rscase);
    std::basic_ostream<char32_t>& operator<<(std::basic_ostream<char32_t>& os, const RE::MatchResults& results);

    inline void PrintNumberOfTests(std::ostream& os, const size_t n);
    const std::string& ErrorReport(const std::string& reportFileName, const RegexSearchCase& rscase);
    const std::string& ErrorReport(const std::string& reportFileName, const RegexSearchCase& rscase,
        const std::vector<RE::MatchResults>& results);
    void RegexMatchTest(const std::string& fileName);
    void RegexSearchTest(const std::string& fileName);
}

#endif // REGEXPR_TEST_HPP
