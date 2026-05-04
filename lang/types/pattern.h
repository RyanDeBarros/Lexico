#pragma once

#include "declarations.h"
#include "basic.h"

#include <memory>

namespace lx
{
	class SubpatternNode
	{
	public:
		virtual ~SubpatternNode() = default;
	};

	class SubpatternArray : public SubpatternNode
	{
		std::vector<SubpatternNode*> _array;

	public:
		void append(SubpatternNode& node);
	};

	class SubpatternRoot : public SubpatternArray
	{
	};

	class SubpatternString : public SubpatternNode
	{
		std::string _string;

	public:
		SubpatternString(const std::string& string);
		SubpatternString(std::string&& string);
	};

	class SubpatternCatenation : public SubpatternArray
	{
	};

	class SubpatternDisjunction : public SubpatternArray
	{
	};

	class SubpatternException : public SubpatternNode
	{
		SubpatternNode* _exception;

	public:
		SubpatternException(SubpatternNode& exception);
	};

	class SubpatternRepetition : public SubpatternNode
	{
		IRange _range;

	public:
		SubpatternRepetition(const IRange& range);
		SubpatternRepetition(PatternSimpleRepeatOperator op);
	};

	enum class LookaroundMode
	{
		Ahead,
		Behind,
		NotAhead,
		NotBehind,
	};

	extern LookaroundMode lookaround_mode(PrefixOperator op);

	class SubpatternLookaround : public SubpatternNode
	{
		LookaroundMode _mode;

	public:
		SubpatternLookaround(LookaroundMode mode);
	};

	class SubpatternOptional : public SubpatternNode
	{
		SubpatternNode* _optional;

	public:
		SubpatternOptional(SubpatternNode& optional);
	};

	class SubpatternBackRef : public SubpatternNode
	{
		CapId _capid;

	public:
		SubpatternBackRef(CapId capid);
	};

	class SubpatternCapture : public SubpatternNode
	{
		CapId _capid;
		SubpatternNode* _captured;

	public:
		SubpatternCapture(CapId capid, SubpatternNode& captured);
	};

	class SubpatternLazy : public SubpatternNode
	{
		SubpatternNode* _lazy;

	public:
		SubpatternLazy(SubpatternNode& lazy);
	};

	class Pattern
	{
		std::vector<std::unique_ptr<SubpatternNode>> _subnodes;
		std::unique_ptr<SubpatternRoot> _root;

	public:
		Pattern();

		static Pattern make_from(const Int& v);
		static Pattern make_from(const Float& v);
		static Pattern make_from(const Bool& v);
		static Pattern make_from(const String& v);
		static Pattern make_from(String&& v);
		static Pattern make_from(const SRange& v);

		const SubpatternRoot& root() const;
		SubpatternRoot& root();

	private:
		void impl_add(std::unique_ptr<SubpatternNode>&& node);

	public:
		template<std::derived_from<SubpatternNode> T>
		T& add(std::unique_ptr<T>&& node)
		{
			T* ptr = node.get();
			impl_add(std::move(node));
			return *ptr;
		}

		void append(Pattern&& pattern);
	};
}
