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
#include"../Error/error.hpp"
#include"regexpr.hpp"


int main()
{
	try
	{
		std::locale loc(std::locale(), new std::codecvt_utf8<char32_t>);
		
		const std::string inputFileName{ "Regexes.txt" };
		std::basic_ifstream<char32_t> ifs{ inputFileName };
		if (!ifs) {
			Error::ErrPrint(std::cerr, Error::Level::ERROR, Error::Type::INFILE, inputFileName);
			return 1;
		}
		ifs.imbue(loc);

		RE::REstring rs;
		std::getline(ifs, rs);
		RE::Regexp regex{ rs };

		std::cout << std::endl << "Success!" << std::endl;
		return 0;
	}
	catch (const Error::RuntimeError& e)
	{
		Error::ErrPrint(std::cerr, Error::Level::EXCEPTION, Error::Type::RUNTIME, e.what());
		return -1;
	}
	catch (const std::exception& e)
	{
		Error::ErrPrint(std::cerr, Error::Level::EXCEPTION, Error::Type::STD, e.what());
		return -2;
	}
}
