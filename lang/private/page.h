#pragma once

#include "types/variable.h"
#include "types/declarations.h"

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
		unsigned int relative(unsigned int i) const;
		std::string_view outer_page() const;

		bool operator==(const Snippet&) const = default;
		bool placement_equals(const Snippet& other, unsigned int my_start, unsigned int my_length, unsigned int other_start, unsigned int other_length) const;
	};

	class Page
	{
		Variable _text;

	public:
		Page(Runtime& runtime, std::string&& text);

		Variable text() const;
		std::vector<Snippet> snippets(std::optional<unsigned int> lines) const;

		void replace(const EvalContext& env, Variable match, Variable string);
	};

	struct SnippetSection
	{
		Snippet snippet;
		unsigned int start = 0;
		unsigned int length = 0;

		unsigned int absolute_start() const;
		String str() const;
		void adjust_indexes(size_t index, size_t from_length, size_t to_length);

		size_t hash() const;
		bool operator==(const SnippetSection& o) const;
	};
}
