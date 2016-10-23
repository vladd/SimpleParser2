#ifndef NODES_H
#define NODES_H

#include <vector>
#include "value.h"
#include "function.h"
#include "namescope.h"
#include "exc.h"

using namespace std;

struct expr
{
    virtual shared_ptr<value> evaluate(activation_record& r) = 0;
    virtual ~expr() { }
};

template<typename T>
struct const_expr : public expr
{
    shared_ptr<typed_value<T>> pv;
    const_expr(typed_value<T> v) : pv(new typed_value<T>(v.value)) { }
    virtual shared_ptr<value> evaluate(activation_record& r) { return pv; }
};

struct var : public expr
{
    string name;
    var(string name) : name(name) { }
    virtual shared_ptr<value> evaluate(activation_record& r)
    {
        auto pvar = r.get_var(name);
        if (pvar == nullptr)
            throw runtime_exception("impossible: cannot find variable in name scope");
        return pvar;
    }
};

struct paramlist
{
    std::vector<std::unique_ptr<expr>> params;
    arglist evaluate(activation_record& r)
    {
        arglist l;
        for (auto& param : params)
            l.args.emplace_back(param->evaluate(r));
        return l;
    }
};

struct namelist
{
    std::vector<std::string> names;
};

struct statement
{
    virtual void execute(activation_record& r) = 0;
    virtual ~statement() { }
};

struct function_call : public statement
{
    string function_name;
    unique_ptr<paramlist> p_params;

    virtual void execute(activation_record& r)
    {
        auto p_function = r.get_func(function_name);
        if (p_function == nullptr)
            throw runtime_exception("impossible: cannot find function in name scope");
        arglist arglist(p_params->evaluate(r));
        p_function->function(r, arglist.args);
    }
};

struct compound_statement : public statement
{
    vector<unique_ptr<statement>> statements;
    virtual void execute(activation_record& r)
    {
        activation_record inner(&r);
        for (auto& p_statement : statements)
            p_statement->execute(inner);
    }
};

struct repeat_statement : public statement
{
    unique_ptr<statement> p_statement;
    long num_repeat;
    virtual void execute(activation_record& r)
    {
        for (long i = 0; i < num_repeat; i++)
            p_statement->execute(r);
    }
};

struct if_statement : public statement
{
    unique_ptr<expr> p_expression;
    unique_ptr<statement> p_statement;
	virtual void execute(activation_record& r)
	{
        auto& boolval = dynamic_pointer_cast<typed_value<bool>>(p_expression->evaluate(r));
        if (!boolval)
            throw runtime_exception("type mismatch for if condition, must be bool");
		if (boolval->value)
			p_statement->execute(r);
	}
};

struct def_statement : public statement
{
    string name;
    vector<string> argnames;
    shared_ptr<statement> p_statement;
    virtual void execute(activation_record& lexical_record)
    {
        // we can catch lexical_record by reference, since it the current design a function never leaves its
        // lexical scope, so the lexical activation record is guaranteed to be alive during the function's lifetime.
        // if it will be possible to escape the definition scope (e.g. by returning a function from a function),
        // we would need to make this perhaps a strong reference
        auto func = [=, &lexical_record](const activation_record& execution_record, const vector<shared_ptr<value>>& args)
        {
            // the referenced outer variables belong to a lexical scope, not a dynamic scope
            // (arbitrary language design desision, but most of languages do it this way)
            activation_record inner(&lexical_record);
            int argnum = args.size();
            if (argnum != argnames.size())
                throw runtime_exception("impossible: number of arguments mismatch for function call");
            for (int i = 0; i < argnum; i++)
                inner.install_var(args[i], argnames[i]);
            p_statement->execute(inner);
        };
        lexical_record.install_function(func, argnames.size(), name);
    }
};

struct program : public compound_statement
{
};

#endif