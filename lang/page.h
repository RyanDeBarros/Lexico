#pragma once

#include "types/variable.h"

#include <optional>

namespace lx
{
	class Runtime;
	class Page;

	class Snippet
	{
		Variable _text;
		unsigned int _start;
		unsigned int _length;

	public:
		Snippet(const Page& page, unsigned int start, unsigned int length);

		std::string_view page_content() const;
		unsigned int absolute(unsigned int i) const;

		bool operator==(const Snippet&) const = default;
	};

	class Page
	{
		Variable _text;

	public:
		Page(Runtime& runtime, std::string&& text);

		Variable text() const;
		std::vector<Snippet> snippets(std::optional<unsigned int> lines) const;
	};
}
