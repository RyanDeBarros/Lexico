#include "errors.h"

#include <sstream>

namespace lx
{
	static const char* SPACED_TAB = "    ";

	static std::string error_prefix(ErrorType type)
	{
		switch (type)
		{
		case lx::ErrorType::Syntax:
			return "[Syntax Error] ";
		case lx::ErrorType::Semantic:
			return "[Semantic Error] ";
		case lx::ErrorType::Runtime:
			return "[Runtime Error] ";
		case lx::ErrorType::Internal:
			return "[Internal Error] ";
		default:
			return "";
		}
	}

	static std::string warning_prefix(ErrorType type)
	{
		switch (type)
		{
		case lx::ErrorType::Syntax:
			return "[Syntax Warning] ";
		case lx::ErrorType::Semantic:
			return "[Semantic Warning] ";
		case lx::ErrorType::Runtime:
			return "[Runtime Warning] ";
		case lx::ErrorType::Internal:
			return "[Internal Warning] ";
		default:
			return "";
		}
	}

	LxStatusMessage::LxStatusMessage(ErrorType type, const std::string& message)
		: _type(type), _message(message)
	{
	}

	LxStatusMessage::LxStatusMessage(ErrorType type, std::string&& message)
		: _type(type), _message(std::move(message))
	{
	}

	ErrorType LxStatusMessage::type() const
	{
		return _type;
	}

	std::string LxStatusMessage::message() const
	{
		return _message;
	}

	std::string LxStatusMessage::underline(const ScriptSegment& segment, unsigned int tabs)
	{
		std::stringstream ss;

		while (tabs > 0)
		{
			ss << SPACED_TAB;
			--tabs;
		}

		for (unsigned int i = 1; i < segment.start_column; ++i)
			ss << ' ';

		ss << '^';

		unsigned int end_column = segment.end_column;
		if (segment.start_line != segment.end_line)
		{
			size_t line_length = segment.script_lines[segment.start_line].size();
			end_column = line_length > 0 ? line_length - 1 : 0;
		}

		for (unsigned int i = segment.start_column + 1; i <= end_column; ++i)
			ss << '~';

		return ss.str();
	}

	std::string LxStatusMessage::segment_message(const ScriptSegment& segment, const std::string_view cause)
	{
		std::stringstream ss;
		ss << cause;
		ss << ":\n";
		std::string line_number = segment.line_number_prefix();
		unsigned int tabs = 1 + line_number.size() / 4;
		ss << SPACED_TAB << line_number << segment.first_line() << '\n' << LxError::underline(segment, tabs);
		return ss.str();
	}

	LxError::LxError(ErrorType type, std::string&& message)
		: LxStatusMessage(type, message), std::runtime_error(error_prefix(type) + std::move(message))
	{
	}

	LxError::LxError(ErrorType type, std::string&& message, DirectCtor)
		: LxStatusMessage(type, message), std::runtime_error(std::move(message))
	{
	}

	const char* LxError::what() const
	{
		return std::runtime_error::what();
	}

	LxError LxError::segment_error(const ScriptSegment& segment, ErrorType type, const std::string_view cause)
	{
		return LxError(type, segment_message(segment, cause));
	}

	LxWarning::LxWarning(ErrorType type, std::string&& message)
		: LxStatusMessage(type, message), _what(warning_prefix(type) + std::move(message))
	{
	}

	const char* LxWarning::what() const
	{
		return _what.c_str();
	}

	LxWarning LxWarning::segment_warning(const ScriptSegment& segment, ErrorType type, const std::string_view cause)
	{
		return LxWarning(type, segment_message(segment, cause));
	}

	LxWarning LxWarning::batch_warning(const std::vector<ScriptSegment>& segments, ErrorType type, const std::string_view cause)
	{
		std::stringstream ss;
		ss << cause;
		ss << ":";

		unsigned int max_tabs = 0;
		std::vector<std::string> line_numbers;

		for (const ScriptSegment& segment : segments)
		{
			std::string line_number = segment.line_number_prefix();
			const unsigned int tabs = 1 + line_number.size() / 4;
			max_tabs = std::max(max_tabs, tabs);
			line_numbers.push_back(std::move(line_number));
		}

		for (size_t i = 0; i < segments.size(); ++i)
		{
			const unsigned int tabs = 1 + line_numbers[i].size() / 4;
			ss << '\n' << SPACED_TAB << line_numbers[i];
			for (unsigned int j = 0; j < max_tabs - tabs; ++j)
				ss << SPACED_TAB;
			ss << segments[i].first_line();
		}

		return LxWarning(type, ss.str());
	}

	static ErrorType common_type(const std::vector<LxError>& errors)
	{
		ErrorType type = ErrorType::Internal;
		for (const LxError& error : errors)
			type = std::max(type, error.type());
		return type;
	}

	static std::string common_message(const std::vector<LxError>& errors)
	{
		std::stringstream ss;
		for (size_t i = 0; i < errors.size(); ++i)
		{
			ss << errors[i].what();
			if (i + 1 < errors.size())
				ss << "\n\n";
		}
		return ss.str();
	}

	LxErrorList::LxErrorList(const std::vector<LxError>& errors)
		: LxError(common_type(errors), common_message(errors), DirectCtor{})
	{
	}
}
