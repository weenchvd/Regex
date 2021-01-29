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
		STD,
		RUNTIME,
		INFILE,
		OUTFILE
	};
	
	void ErrPrint(std::ostream& os, Level errLevel, const std::string& message);
	void ErrPrint(std::ostream& os, Level errLevel, Type errType);
	void ErrPrint(std::ostream& os, Level errLevel, Type errType, const std::string& message);

	class RuntimeError : public std::exception {
	public:
		using ExceptionBase = std::exception;

		explicit RuntimeError(const std::string& message) : ExceptionBase(message.c_str()) {}
		explicit RuntimeError(const char* message) : ExceptionBase(message) {}
	};

	class InvalidRegex : public RuntimeError {
	public:
		using ExceptionBase = RuntimeError;

		explicit InvalidRegex(const std::string& message) : ExceptionBase(message.c_str()) {}
		explicit InvalidRegex(const char* message) : ExceptionBase(message) {}
	};
}

#endif // ERROR_HPP
