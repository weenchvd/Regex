#include<iostream>
#include<iomanip>
#include<string>
#include<vector>
#include"error.hpp"

namespace Error
{
	const std::string prefixLevel{ "\t[ " };
	const std::string suffixLevel{ " ] " };
	const std::vector<std::string> level
	{
		{ "EXCEPTION" },
		{ "ERROR" },
		{ "WARNING" },
		{ "NOTICE" }
	};

	const std::vector<std::string> type
	{
		{ "Can't open input file" },
		{ "Can't open output file" }
	};

	void ErrPrint(std::ostream& os, Level errLevel, const std::string& message)
	{
		os << prefixLevel << level[int(errLevel)] << suffixLevel << message << std::endl;
	}

	void ErrPrint(std::ostream& os, Level errLevel, Type errType)
	{
		os << prefixLevel << level[int(errLevel)] << suffixLevel << type[int(errType)] << std::endl;
	}

	void ErrPrint(std::ostream& os, Level errLevel, Type errType, const std::string& message)
	{
		os << prefixLevel << level[int(errLevel)] << suffixLevel << type[int(errType)];
		if (message.size() > 0) {
			os << ". " << message;
		}
		os << std::endl;
	}
}
