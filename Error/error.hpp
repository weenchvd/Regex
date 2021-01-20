#ifndef ERROR_HPP
#define ERROR_HPP

namespace Error
{
	enum class Level {
		EXCEPTION,
		ERROR,
		WARNING,
		NOTICE,
	};

	enum class Type {
		INFILE,
		OUTFILE
	};
	
	void ErrPrint(std::ostream& os, Level errLevel, const std::string& message);
	void ErrPrint(std::ostream& os, Level errLevel, Type errType);
	void ErrPrint(std::ostream& os, Level errLevel, Type errType, const std::string& message);
}

#endif // ERROR_HPP
