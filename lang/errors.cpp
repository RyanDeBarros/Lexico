#include "errors.h"

#include <sstream>

namespace lx
{
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
		return LxError(type, segment.message(cause));
	}

	LxError LxError::batch_error(const std::vector<ScriptSegment>& segments, ErrorType type, const std::string_view cause)
	{
		return LxError(type, ScriptSegment::batch_message(segments, cause));
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
		return LxWarning(type, segment.message(cause));
	}

	LxWarning LxWarning::batch_warning(const std::vector<ScriptSegment>& segments, ErrorType type, const std::string_view cause)
	{
		return LxWarning(type, ScriptSegment::batch_message(segments, cause));
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
