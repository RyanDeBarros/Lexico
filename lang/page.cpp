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
		return _text.ref().as<String>()->value().substr(_start, _length);
	}

	unsigned int Snippet::absolute(unsigned int i) const
	{
		return _start + i;
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
}
