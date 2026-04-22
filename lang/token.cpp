#include "token.h"

#include <stdexcept>
#include <sstream>

namespace lx
{
	void TokenStream::load(std::vector<Token>&& tokens)
	{
		_tokens = std::move(tokens);
		_ptr = 0;
	}

	bool TokenStream::eof() const
	{
		return _ptr >= _tokens.size();
	}

	void TokenStream::advance(size_t n)
	{
		if (n > tokens_left())
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": cannot advance by " << n << " - only " << tokens_left() << " token(s) left";
			throw std::out_of_range(ss.str());
		}

		_ptr += n;
	}

	const Token& TokenStream::peek(size_t n) const
	{
		if (n >= tokens_left())
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": cannot peek by " << n << " - only " << tokens_left() << " token(s) left";
			throw std::out_of_range(ss.str());
		}

		return _tokens[_ptr + n];
	}

	void TokenStream::seek(size_t i)
	{
		if (i > _tokens.size())
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": cannot seek to position " << i << " - out of range " << _tokens.size();
			throw std::out_of_range(ss.str());
		}

		_ptr = i;
	}

	size_t TokenStream::tokens_left() const
	{
		return _ptr < _tokens.size() ? _tokens.size() - _ptr : 0;
	}
}
