#include "resolution.h"

#include "errors.h"

#include <sstream>

namespace lx
{
	std::optional<VariableSignature> SemanticVariableTable::registered_variable(const std::string_view identifier) const
	{
		auto it = _map.find(identifier);
		if (it != _map.end())
			return it->second;
		else
			return std::nullopt;
	}

	void SemanticVariableTable::register_variable(const std::string_view identifier, DataType type, unsigned int line_number)
	{
		if (_map.count(identifier))
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": variable already registered: " << identifier;
			throw LxError(ErrorType::Internal, ss.str());
		}

		_map[std::string(identifier)] = { .decl_line_number = line_number, .type = type };
	}

	bool VarConsistencyTest::seen(const FunctionDefinition& fn) const
	{
		return _seen_functions.count(&fn);
	}
	
	void VarConsistencyTest::see(const FunctionDefinition& fn)
	{
		_seen_functions.insert(&fn);
	}

	void VarConsistencyTest::clear_functions()
	{
		_seen_functions.clear();
	}

	void VarConsistencyTest::declare(const std::string_view var_identifier)
	{
		_declared_vars.insert(std::string(var_identifier));
	}

	void VarConsistencyTest::test(SemanticContext& ctx, const Token& var) const
	{
		if (!_declared_vars.count(var.lexeme))
			// TODO v0.2 print call stack that caused this
			ctx.add_semantic_error(var.segment, "global variable may not be defined at this point in function call stack");
	}

	std::vector<LxError>& SemanticContext::errors()
	{
		return _errors;
	}

	void SemanticContext::add_semantic_error(const ScriptSegment& segment, const std::string_view cause) 
	{
		_errors.push_back(LxError::segment_error(segment, ErrorType::Semantic, cause));
	}

	void SemanticContext::add_semantic_error(const Token& token, const std::string_view cause)
	{
		add_semantic_error(token.segment, cause);
	}

	std::vector<LxWarning>& SemanticContext::warnings()
	{
		return _warnings;
	}

	void SemanticContext::add_semantic_warning(const ScriptSegment& segment, const std::string_view cause)
	{
		_warnings.push_back(LxWarning::segment_warning(segment, ErrorType::Semantic, cause));
	}

	void SemanticContext::add_semantic_warning(const Token& token, const std::string_view cause)
	{
		add_semantic_warning(token.segment, cause);
	}

	void SemanticContext::push_local_scope(bool isolated)
	{
		_scope_stack.push_back({ .isolated = isolated });
	}

	void SemanticContext::pop_local_scope()
	{
		if (_scope_stack.empty())
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": scope stack is empty";
			throw LxError(ErrorType::Internal, ss.str());
		}
		_scope_stack.pop_back();
	}

	SemanticContext::LocalScope::LocalScope(SemanticContext& context, bool isolated)
		: _context(context), _alive(true)
	{
		_context.push_local_scope(isolated);
	}

	SemanticContext::LocalScope::LocalScope(LocalScope&& other) noexcept
		: _context(other._context), _alive(other._alive)
	{
		other._alive = false;
	}

	SemanticContext::LocalScope::~LocalScope()
	{
		if (_alive)
			_context.pop_local_scope();
	}

	bool SemanticContext::in_local_scope() const
	{
		return !_scope_stack.empty();
	}

	VarConsistencyTest& SemanticContext::var_consistency_test()
	{
		return _var_consistency_test;
	}

	static std::optional<unsigned int> first_variable_line_number(const SemanticVariableTable& variable_table, const std::string_view identifier)
	{
		if (auto var = variable_table.registered_variable(identifier))
			return var->decl_line_number;
		else
			return std::nullopt;
	}

	static std::optional<unsigned int> first_function_line_number(const SemanticFunctionTable& function_table, const std::string_view identifier)
	{
		auto calls = function_table.registered_function_calls(identifier);
		if (!calls.empty())
		{
			unsigned int first_line_number = std::numeric_limits<unsigned int>::max();
			for (const auto& call : calls)
				first_line_number = std::min(first_line_number, function_table.known_registered_function(call.identifier, call.arg_types)->decl_line_number);

			return first_line_number;
		}
		else
			return std::nullopt;
	}

	std::optional<unsigned int> SemanticContext::identifier_first_decl_line_number(const std::string_view identifier, Namespace ns)
	{
		try
		{
			if (ns == Namespace::Global)
			{
				if (auto vln = first_variable_line_number(_global_variable_table, identifier))
					return *vln;
				else if (auto fln = first_function_line_number(_function_table, identifier))
					return *fln;
				else
					return std::nullopt;
			}

			std::optional<unsigned int> line_number = std::nullopt;

			if (ns == Namespace::Unknown || _scope_stack.empty())
			{
				if (auto vln = first_variable_line_number(_global_variable_table, identifier))
					line_number = *vln;
				else if (auto fln = first_function_line_number(_function_table, identifier))
					line_number = *fln;
			}

			for (auto it = _scope_stack.rbegin(); it != _scope_stack.rend(); ++it)
			{
				if (auto ln = first_variable_line_number(it->table, identifier))
				{
					if (line_number)
						line_number = std::min(*line_number, *ln);
					else
						line_number = ln;
				}

				if (it->isolated)
					break;
			}

			return line_number;
		}
		catch (const LxError& error)
		{
			_errors.push_back(error);
		}

		return {};
	}

	std::optional<VariableSignature> SemanticContext::registered_variable(const std::string_view identifier, Namespace ns)
	{
		try
		{
			if (ns == Namespace::Global)
				return _global_variable_table.registered_variable(identifier);
			
			if (ns == Namespace::Unknown || _scope_stack.empty())
			{
				if (auto sig = _global_variable_table.registered_variable(identifier))
					return sig;
			}

			for (auto it = _scope_stack.rbegin(); it != _scope_stack.rend(); ++it)
			{
				if (auto sig = it->table.registered_variable(identifier))
					return sig;
				else if (it->isolated)
					break;
			}

			if (ns == Namespace::Unknown)
			{
			}
		}
		catch (const LxError& error)
		{
			_errors.push_back(error);
		}
		
		return std::nullopt;
	}

	void SemanticContext::register_variable(const std::string_view identifier, DataType type, unsigned int line_number, Namespace ns)
	{
		switch (ns)
		{
		case lx::Namespace::Global:
			_global_variable_table.register_variable(identifier, type, line_number);
			break;
		case lx::Namespace::Local:
			if (!_scope_stack.empty())
				_scope_stack.back().table.register_variable(identifier, type, line_number);
			else
			{
				std::stringstream ss;
				ss << __FUNCTION__ << ": local scope stack is empty";
				_errors.push_back(LxError(ErrorType::Internal, ss.str()));
			}
			break;
		default:
			std::stringstream ss;
			ss << __FUNCTION__ << ": cannot register variable to unknown/isolated namespace";
			_errors.push_back(LxError(ErrorType::Internal, ss.str()));
		}
	}

	std::vector<FunctionSignature> SemanticContext::registered_functions(const std::string_view identifier, const std::vector<DataType>& arg_types) const
	{
		return _function_table.registered_functions(identifier, arg_types);
	}

	std::optional<FunctionSignature> SemanticContext::known_registered_function(const std::string_view identifier, const std::vector<DataType>& arg_types) const
	{
		return _function_table.known_registered_function(identifier, arg_types);
	}

	FunctionCallSet SemanticContext::registered_function_calls(const std::string_view identifier) const
	{
		return _function_table.registered_function_calls(identifier);
	}

	void SemanticContext::register_function(FunctionDefinition& decl_node, const std::string_view identifier,
		DataType return_type, std::vector<DataType>&& arg_types, unsigned int line_number)
	{
		_function_table.register_function(decl_node, identifier, return_type, std::move(arg_types), line_number);
	}

	SemanticFunctionTable& SemanticContext::ftable()
	{
		return _function_table;
	}
}
