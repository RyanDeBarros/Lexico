#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace lx
{
	enum class TokenType
	{
		// General
		Identifier,
		BuiltinSymbol,
		Percent,
		Newline,
		Runoff,
		EndOfFile,
		Comma,
		Arrow,
		Assign,
		Dot,

		// Literals
		Integer,
		Float,
		String,
		Bool,
		CapId,

		// Punctuation
		Asterisk,
		EqualTo,
		GreaterThan,
		GreaterThanOrEqualTo,
		LBracket,
		LessThan,
		LessThanOrEqualTo,
		LParen,
		Minus,
		NotEqualTo,
		Plus,
		RBracket,
		RParen,
		Slash,
	};

	enum class Keyword
	{
		_None = 0,

		// Types
		IntType,
		FloatType,
		BoolType,
		StringType,
		StringViewType,
		VoidType,
		PatternType,
		MatchType,
		MatchesType,
		CapIdType,
		CapType,
		IRangeType,
		SRangeType,
		ListType,

		// Keywords
		Ahead,
		And,
		Append,
		Apply,
		As,
		Behind,
		Break,
		Capture,
		Color,
		Continue,
		Delete,
		Elif,
		Else,
		End,
		Except,
		Filter,
		FindAll,
		For,
		Fn,
		Greedy,
		Highlight,
		If,
		In,
		Lazy,
		Let,
		Log,
		Max,
		Min,
		Mod,
		Not,
		NotAhead,
		NotBehind,
		Optional,
		Or,
		Page,
		Pop,
		Push,
		Ref,
		Repeat,
		Replace,
		Return,
		Search,
		Scope,
		To,
		Var,
		While,
		With,
	};

	enum class Precedence
	{
		None = -1,
		Lowest,
		Assign,
		Comma,
		Or,
		And,
		Equality,
		Compare,
		Except,
		Sum,
		Mult,
		Prefix,
		Repeat,
		To,
		Dot,
	};

	extern Precedence operator+(Precedence a, Precedence b);
	extern Precedence operator+(Precedence a, int b);

	struct ScriptSegment
	{
		const std::vector<std::string_view>& script_lines;
		
		unsigned int start_line = 0;
		unsigned int end_line = 0;
		unsigned int start_column = 0;
		unsigned int end_column = 0;
		
		ScriptSegment(const std::vector<std::string_view>& script_lines);
		ScriptSegment(const ScriptSegment&);
		ScriptSegment(ScriptSegment&&) noexcept;
		ScriptSegment& operator=(const ScriptSegment&);
		ScriptSegment& operator=(ScriptSegment&&) noexcept;

		std::string_view first_line() const;
		std::string line_number_prefix() const;

		std::string underline(unsigned int tabs = 1) const;
		std::string message(const std::string_view header) const;
		static std::string batch_message(const std::vector<ScriptSegment>& segments, const std::string_view header);

		ScriptSegment combined_right(ScriptSegment right) const;
	};

	struct Token
	{
		TokenType type = TokenType::EndOfFile;

	private:
		mutable std::optional<Keyword> _keyword = std::nullopt;

	public:
		std::string_view lexeme;
		ScriptSegment segment;

		Token(const ScriptSegment& segment);

		std::string resolved() const;
		Keyword keyword() const;
		void set_keyword(Keyword kw);

	private:
		std::optional<Keyword> impl_keyword() const;

	public:
		bool is_datatype() const;
		bool is_literal() const;

		bool is_binary_operator() const;
		bool is_prefix_operator() const;
		bool is_postfix_operator() const;

		Precedence precedence() const;
		bool is_right_associative() const;
	};

	class TokenStream
	{
		std::vector<Token> _tokens;
		size_t _ptr = 0;

	public:
		void load(std::vector<Token>&& tokens);

		bool eof() const;
		void advance(size_t n = 1);
		const Token& peek(size_t n = 0) const;
		Token& ref(size_t n = 0);
		void seek(size_t i = 0);
		size_t tokens_left() const;
	};
}
