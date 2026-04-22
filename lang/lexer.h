#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace lx
{
	enum class TokenType
	{
		// Basic
		Identifier,
		BuiltinSymbol,
		Percent,
		Newline,
		Runoff,
		EndOfFile,

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
		And,
		Apply,
		As,
		Break,
		Color,
		Continue,
		Delete,
		Elif,
		Else,
		End,
		Filter,
		Find,
		Fn,
		Highlight,
		If,
		In,
		Let,
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
		Comma,
		Except,
		Lazy,
		Max,
		Min,
		Mod,
		Not,
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
		Arrow,
		Assign,
		Asterisk,
		Dot,
		EqualTo,
		GreaterThan,
		GreaterThanOrEqualTo,
		LessThan,
		LessThanOrEqualTo,
		Minus,
		Plus,
		Slash,
	};

	struct Token
	{
		TokenType type;

		std::string lexeme;
		size_t line;
		size_t column;
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
		size_t tokens_left() const;
	};

	class Lexer
	{
		TokenStream _stream;

	public:
		void tokenize(const std::string_view script);
	};
}
