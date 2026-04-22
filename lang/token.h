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
		Comma,

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
		NotEqualTo,
		Plus,
		Slash,
	};

	struct Token
	{
		TokenType type = TokenType::EndOfFile;

		std::string lexeme;
		size_t line = 0;
		size_t column = 0;
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
		void seek(size_t i = 0);
		size_t tokens_left() const;
	};
}
