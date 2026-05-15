#include "page.h"

#include "runtime.h"

namespace lx
{
	Snippet::Snippet(const Page& page, unsigned int start, unsigned int length)
		: _text(page.text()), _start(start), _length(length)
	{
	}

	std::string_view Snippet::page_content() const
	{
		return outer_page().substr(_start, _length);
	}

	std::string_view Snippet::outer_page() const
	{
		return _text.ref().as<String>()->value();
	}

	unsigned int Snippet::absolute(unsigned int i) const
	{
		return _start + i;
	}

	unsigned int Snippet::relative(unsigned int i) const
	{
		return i - _start;
	}

	bool Snippet::placement_equals(const Snippet& other, unsigned int my_start, unsigned int my_length, unsigned int other_start, unsigned int other_length) const
	{
		if (my_length != other_length)
			return false;

		if (absolute(my_start) != other.absolute(other_start))
			return false;

		if (&_text.ref() == &other._text.ref())
			return true;
		else
		{
			std::string_view v1 = outer_page().substr(absolute(my_start), my_length);
			std::string_view v2 = other.outer_page().substr(other.absolute(other_start), other_length);
			return v1 == v2;
		}
	}

	Page::Page(Runtime& runtime, std::string&& text)
		: _text(runtime.unbound_variable(String(std::move(text))))
	{
	}

	Variable Page::text() const
	{
		return _text;
	}

	std::vector<Snippet> Page::snippets(std::optional<unsigned int> lines) const
	{
		const std::string_view text = _text.ref().as<String>()->value();
		if (text.empty() || !lines || *lines == 0)
			return { Snippet(*this, 0, text.size()) };

		std::vector<size_t> starts;
		starts.push_back(0);

		for (size_t i = 0; i < text.size(); ++i)
			if (text[i] == '\n' && i + 1 < text.size())
				starts.push_back(i + 1);

		if (starts.size() < *lines)
			return {};

		std::vector<Snippet> snippets;
		for (size_t i = 0; i + *lines - 1 < starts.size(); ++i)
		{
			unsigned int begin = starts[i];
			unsigned int end;

			if (i + *lines < starts.size())
			{
				end = starts[i + *lines] - 1;
				if (end > begin && text[end - 1] == '\r')
					--end;
			}
			else
				end = text.size();

			snippets.emplace_back(*this, begin, end - begin);
		}

		return snippets;
	}

	void Page::replace(const EvalContext& env, Variable match_var, Variable string_var)
	{
		Match match = std::move(match_var).consume_as<Match>(env);
		StringView string = std::move(string_var).consume_as<StringView>(env);
		if (!_text.is(string.string_variable()))
			throw env.runtime_error("match does not reference current page");

		const auto range = match.highlight_range();
		size_t index = range.start;
		size_t from_length = range.length;
		size_t to_length = string.size();

		_text.ref().get<String>().replace(index, from_length, std::move(string).consume_value(env));
		env.runtime.global_matches().adjust_indexes(index, from_length, to_length);
	}

	String SnippetSection::str() const
	{
		return String(std::string(snippet.page_content().substr(start, length)));
	}

	unsigned int SnippetSection::absolute_start() const
	{
		return snippet.absolute(start);
	}

	void SnippetSection::adjust_indexes(size_t index, size_t from_length, size_t to_length)
	{
		size_t abs_start = snippet.absolute(start);
		if (abs_start + length <= index)
			return;

		if (abs_start >= index + from_length)
		{
			if (to_length > from_length)
				abs_start += to_length - from_length;
			else
				abs_start -= from_length - to_length;
		}
		else if (abs_start >= index)
		{
			if (abs_start + length <= index + from_length)
				length = 0;
			else
				length -= index + from_length - abs_start;

			abs_start = index + to_length;
		}
		else if (abs_start + length > index + from_length)
		{
			if (to_length > from_length)
				length += to_length - from_length;
			else
				length -= from_length - to_length;
		}
		else
			length -= abs_start + length - index;

		start = snippet.relative(abs_start);
	}

	size_t SnippetSection::hash() const
	{
		size_t h = 0;
		h = hash_combine(h, std::hash<unsigned int>{}(snippet.absolute(start)));
		h = hash_combine(h, std::hash<unsigned int>{}(start));
		h = hash_combine(h, std::hash<unsigned int>{}(length));
		return h;
	}

	bool SnippetSection::operator==(const SnippetSection& o) const
	{
		return snippet.placement_equals(o.snippet, start, length, o.start, o.length);
	}
}
