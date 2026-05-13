#include "runtime.h"

#include "constants.h"

namespace lx
{
	void RuntimeSymbolTable::register_variable(const std::string_view identifier, Variable&& dp)
	{
		if (_variable_table.contains(identifier))
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": variable already registered: " << identifier;
			throw LxError(ErrorType::Runtime, ss.str());
		}

		_variable_table.try_emplace(std::string(identifier), std::move(dp));
	}

	std::optional<Variable> RuntimeSymbolTable::registered_variable(const std::string_view identifier) const
	{
		auto it = _variable_table.find(identifier);
		if (it != _variable_table.end())
			return it->second;
		else
			return std::nullopt;
	}

	RuntimeScopeContext::RuntimeScopeContext(bool isolated)
		: isolated(isolated)
	{
	}

	Runtime::Runtime(const std::string_view input, SemanticFunctionTable&& ftable)
		: _input(input), _global_matches(_heap.add(Matches())), _search_scope(std::nullopt), _function_table(std::move(ftable))
	{
		push_local_scope(true);
		_root_page = std::make_unique<Page>(*this, std::string(input));
	}

	const std::stringstream& Runtime::output() const
	{
		return _output;
	}

	std::stringstream& Runtime::output()
	{
		return _output;
	}

	const std::stringstream& Runtime::log() const
	{
		return _log;
	}

	std::stringstream& Runtime::log()
	{
		return _log;
	}

	void Runtime::push_local_scope(bool isolated)
	{
		_scope_stack.emplace_back(isolated);
	}

	void Runtime::pop_local_scope()
	{
		if (_scope_stack.empty())
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": scope stack is empty";
			throw LxError(ErrorType::Internal, ss.str());
		}
		_scope_stack.pop_back();
	}

	Runtime::LocalScope::LocalScope(Runtime& runtime, bool isolated)
		: _runtime(runtime), _alive(true)
	{
		_runtime.push_local_scope(isolated);
	}

	Runtime::LocalScope::LocalScope(LocalScope&& other) noexcept
		: _runtime(other._runtime), _alive(other._alive)
	{
		other._alive = false;
	}

	Runtime::LocalScope::~LocalScope()
	{
		if (_alive)
			_runtime.pop_local_scope();
	}

	void Runtime::register_variable(const std::string_view identifier, DataPoint&& dp, Namespace ns)
	{
		switch (ns)
		{
		case lx::Namespace::Global:
			_global_variable_table.register_variable(identifier, _heap.add(std::move(dp)));
			break;
		case lx::Namespace::Local:
			if (!_scope_stack.empty())
				_scope_stack.back().table.register_variable(identifier, _heap.add(std::move(dp)));
			else
			{
				std::stringstream ss;
				ss << __FUNCTION__ << ": local scope stack is empty";
				throw LxError(ErrorType::Runtime, ss.str());
			}
			break;
		default:
			std::stringstream ss;
			ss << __FUNCTION__ << ": cannot register variable to unknown/isolated namespace";
			throw LxError(ErrorType::Runtime, ss.str());
		}
	}

	Variable Runtime::registered_variable(const std::string_view identifier, Namespace ns, const ScriptSegment& segment) const
	{
		if (ns == Namespace::Global)
		{
			if (auto dp = _global_variable_table.registered_variable(identifier))
				return *dp;
			else
				throw LxError::segment_error(segment, ErrorType::Runtime, "variable does not exist in current scope");
		}

		for (auto it = _scope_stack.rbegin(); it != _scope_stack.rend(); ++it)
		{
			if (auto sig = it->table.registered_variable(identifier))
				return *sig;
			else if (it->isolated)
				break;
		}

		if (ns == Namespace::Unknown || _scope_stack.empty())
		{
			if (auto sig = _global_variable_table.registered_variable(identifier))
				return *sig;
		}

		throw LxError::segment_error(segment, ErrorType::Runtime, "variable does not exist in current scope");
	}

	Variable Runtime::unbound_variable(DataPoint&& dp)
	{
		return _heap.add(std::move(dp));
	}

	const FunctionDefinition& Runtime::registered_function(const std::string_view identifier, const std::vector<DataType>& arg_types, const ScriptSegment& segment) const
	{
		if (const FunctionDefinition* fn = _function_table.registered_function(identifier, arg_types))
			return *fn;
		else
		{
			std::stringstream ss;
			ss << "no declaration matches the argument types ";
			print_list(ss, arg_types);
			throw LxError::segment_error(segment, ErrorType::Runtime, ss.str());
		}
	}

	void Runtime::declare_pattern(std::string_view identifier)
	{
		auto it = _declared_patterns.find(identifier);
		if (it != _declared_patterns.end())
			_focused_pattern = it->second;
		else
		{
			// TODO v0.2 allow for passing initial pattern expression in pattern declaration
			Variable var = _heap.add(Pattern());
			_declared_patterns.try_emplace(std::string(identifier), var);
			_focused_pattern = var;
		}
	}
	
	void Runtime::delete_pattern(std::string_view identifier)
	{
		auto it = _declared_patterns.find(identifier);
		if (it != _declared_patterns.end())
		{
			if (_focused_pattern && *_focused_pattern == it->second)
				_focused_pattern.reset();
			_declared_patterns.erase(it);
		}
	}
	
	Variable Runtime::focused_pattern(const ScriptSegment& segment) const
	{
		if (_focused_pattern)
			return *_focused_pattern;
		else
			throw LxError::segment_error(segment, ErrorType::Runtime, "no pattern currently declared");
	}

	void Runtime::find_all(const ScriptSegment& segment)
	{
		do_find(segment, [](const EvalContext& env, Matches& matches, const Pattern& pattern, const Snippet& snippet) { matches.append(pattern.find_all(env, snippet)); });
	}

	void Runtime::search(const ScriptSegment& segment)
	{
		do_find(segment, [](const EvalContext& env, Matches& matches, const Pattern& pattern, const Snippet& snippet) { matches.append(pattern.search(env, snippet)); });
	}

	void Runtime::do_find(const ScriptSegment& segment, void(*func)(const EvalContext&, Matches&, const Pattern&, const Snippet&))
	{
		if (Pattern* pattern = focused_pattern(segment).ref().as<Pattern>())
		{
			const Page& page = focused_page();
			const Scope& scope = search_scope();
			const auto snippets = page.snippets(scope.lines());
			global_matches() = Matches();
			EvalContext env{ .runtime = *this, .segment = &segment };
			for (const Snippet& snippet : snippets)
				func(env, global_matches(), *pattern, snippet);
			// TODO remove duplicates
		}
		else
			throw LxError::segment_error(segment, ErrorType::Runtime, "no pattern currently declared");
	}

	void Runtime::add_highlight(const Color& color, std::optional<Variable> format, const ScriptSegment& segment)
	{
		do_highlight(color, std::move(format), segment, [](HighlightSet& set, Highlight h) { set.insert(h); });
	}

	void Runtime::remove_highlight(const Color& color, std::optional<Variable> format, const ScriptSegment& segment)
	{
		do_highlight(color, std::move(format), segment, [](HighlightSet& set, Highlight h) { set.remove(h); });
	}

	void Runtime::do_highlight(const Color& color, std::optional<Variable> format, const ScriptSegment& segment, void(*func)(HighlightSet&, Highlight))
	{
		HighlightSet& set = _highlights[color.color()];

		if (format)
		{
			if (const IRange* range = format->ref().as<IRange>())
			{
				const Matches& matches = global_matches();

				const int min = range->min() ? *range->min() : 0;
				const int max = range->max() ? *range->max() : matches.size() - 1;

				// TODO v0.2 allow negative indexing
				if (min < 0 || min >= matches.size() || max < 0 || max >= matches.size())
				{
					std::stringstream ss;
					range->print(EvalContext{ .runtime = *this, .segment = &segment }, ss);
					ss << " out of range: '%' has size " << matches.size();
					throw LxError::segment_error(segment, ErrorType::Runtime, ss.str());
				}

				for (size_t i = std::min(min, max); i <= std::max(min, max); ++i)
					func(set, matches.match(i).highlight_range());
			}
			else if (const Match* match = format->ref().as<Match>())
				func(set, match->highlight_range());
			else if (const Matches* matches = format->ref().as<Matches>())
			{
				for (size_t i = 0; i < matches->size(); ++i)
					func(set, matches->match(i).highlight_range());
			}
			else
				throw LxError::segment_error(segment, ErrorType::Runtime, "expression is not highlightable");
		}
		else
		{
			const Matches& matches = global_matches();
			for (size_t i = 0; i < matches.size(); ++i)
				func(set, matches.match(i).highlight_range());
		}
	}

	const HighlightMap& Runtime::highlights() const
	{
		return _highlights;
	}

	HighlightMap& Runtime::highlights()
	{
		return _highlights;
	}

	void Runtime::push_page(Variable page_desc, const ScriptSegment& segment)
	{
		_page_stack.push(Page(*this, page_desc.ref().page_content(EvalContext{ .runtime = *this, .segment = &segment })));
	}

	void Runtime::pop_page(const ScriptSegment& segment)
	{
		if (!_page_stack.empty())
			_page_stack.pop();
		else
			_log << LxWarning::segment_warning(segment, ErrorType::Runtime, "page stack is empty").what();
	}

	void Runtime::clear_page_stack()
	{
		while (!_page_stack.empty())
			_page_stack.pop();
	}

	const Page& Runtime::focused_page() const
	{
		return _page_stack.empty() ? *_root_page : _page_stack.top();
	}

	const Matches& Runtime::global_matches() const
	{
		return _global_matches.ref().get<Matches>();
	}

	Matches& Runtime::global_matches()
	{
		return _global_matches.ref().get<Matches>();
	}

	Variable Runtime::global_matches_var() const
	{
		return _global_matches;
	}

	const Scope& Runtime::search_scope() const
	{
		return _search_scope;
	}

	Scope& Runtime::search_scope()
	{
		return _search_scope;
	}

	CapId Runtime::capture_id(const std::string_view id)
	{
		auto it = _capture_ids.find(id);
		if (it != _capture_ids.end())
			return CapId(it->second);

		unsigned int uid = _capture_ids.size();
		_capture_ids[std::string(id)] = uid;
		return CapId(uid);
	}
}
