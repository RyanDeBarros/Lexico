#pragma once

#include <stdexcept>

#include "token.h"

namespace lx
{
	enum class ErrorType
	{
		Syntax,
		Semantic,
		Runtime,
		Internal,
	};

	class LxStatusMessage
	{
		ErrorType _type;
		std::string _message;

	public:
		LxStatusMessage(ErrorType type, const std::string& message);
		LxStatusMessage(ErrorType type, std::string&& message);

		ErrorType type() const;
		std::string message() const;

		virtual const char* what() const = 0;

		static std::string underline(const ScriptSegment& segment, unsigned int tabs = 1);
		static std::string segment_message(const ScriptSegment& segment, const std::string_view cause);
	};

	class LxError : public LxStatusMessage, public std::runtime_error
	{
	public:
		LxError(ErrorType type, std::string&& message = "");

		const char* what() const override;

		static LxError segment_error(const ScriptSegment& segment, ErrorType type, const std::string_view cause);
	};

	class LxWarning : public LxStatusMessage
	{
		std::string _what;

	public:
		LxWarning(ErrorType type, std::string&& message = "");

		const char* what() const override;

		static LxWarning segment_warning(const ScriptSegment& segment, ErrorType type, const std::string_view cause);
		static LxWarning batch_warning(const std::vector<ScriptSegment>& segments, ErrorType type, const std::string_view cause);
	};
}
