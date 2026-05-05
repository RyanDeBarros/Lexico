#pragma once

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

		// Types
		IntType,
		FloatType,
		BoolType,
		StringType,
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
		Apply,
		Break,
		Color,
		Continue,
		Delete,
		Elif,
		Else,
		End,
		Filter,
		Find,
		For,
		Fn,
		Highlight,
		If,
		In,
		Let,
		Log,
		Page,
		Pop,
		Push,
		Replace,
		Return,
		Scope,
		Var,
		While,
		With,

		// Pattern grammar
		Append,
		Ahead,
		Behind,
		Capture,
		Except,
		Lazy,
		Max,
		Min,
		Not,
		NotAhead,
		NotBehind,
		Optional,
		Or,
		Ref,
		Repeat,
		To,

		// Punctuation
		LParen,
		RParen,
		LBracket,
		RBracket,

		// Operators
		And,
		As,
		Asterisk,
		EqualTo,
		GreaterThan,
		GreaterThanOrEqualTo,
		LessThan,
		LessThanOrEqualTo,
		Minus,
		Mod,
		NotEqualTo,
		Plus,
		Slash,
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

		ScriptSegment combined_right(ScriptSegment right) const;
	};

	struct Token
	{
		TokenType type = TokenType::EndOfFile;

		std::string_view lexeme;
		ScriptSegment segment;

		std::string resolved() const;

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
