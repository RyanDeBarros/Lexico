#pragma once

#include "types/declarations.h"

namespace lx
{
	class SRange
	{
		std::optional<char> _min;
		std::optional<char> _max;

	public:
		SRange(std::optional<char> min, std::optional<char> max);
		SRange(std::optional<std::string> min, std::optional<std::string> max);
		SRange(std::optional<std::string_view> min, std::optional<std::string_view> max);
		SRange(std::optional<std::string> min, const ScriptSegment* min_segment, std::optional<std::string> max, const ScriptSegment* max_segment);
		SRange(std::optional<std::string_view> min, const ScriptSegment* min_segment, std::optional<std::string_view> max, const ScriptSegment* max_segment);

		static DataType data_type();
		TypeVariant cast_copy(const EvalContext& env, const DataType& type) const;
		TypeVariant cast_move(const EvalContext& env, const DataType& type) &&;
		void print(const EvalContext& env, std::stringstream& ss) const;

		Variable data_member(VarContext& ctx, const std::string_view member) const;
		Variable invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const;
		void assign(const EvalContext& env, SRange&& o);
		bool equals(const EvalContext& env, const SRange& o) const;

		size_t iterlen(const EvalContext& env) const;
		DataPoint iterget(const EvalContext& env, size_t i) const;

		std::optional<char> min() const;
		std::optional<char> max() const;
		std::string string() const;
	};
}
