#ifndef NAMESCOPE_H
#define NAMESCOPE_H

#include <unordered_map>
#include <unordered_set>

class namescope
{
    const namescope* p_outer;
    unordered_map<string, int> function_signatures;
    unordered_set<string> vars;
    bool owns_outer_scope = false;

public:
    namescope() : p_outer(nullptr) { }
    namescope(const namescope* outer) : p_outer(outer) { }
    namescope(const namescope&) = delete;

    enum class lookup_result { not_found, wrong_signature, found };

    lookup_result lookup_func(string name, int nargs) const
    {
        auto psig = function_signatures.find(name);
        auto found = psig != function_signatures.end();
        if (found && psig->second == nargs)
            return lookup_result::found;
        if (p_outer != nullptr)
        {
            auto outer_result = p_outer->lookup_func(name, nargs);
            if (outer_result != lookup_result::not_found)
                return outer_result;
        }
        return found ? lookup_result::wrong_signature : lookup_result::not_found;
    }

    lookup_result lookup_var(string name) const
    {
        if (vars.find(name) != vars.end())
            return lookup_result::found;
        if (p_outer != nullptr)
            return p_outer->lookup_var(name);
        return lookup_result::not_found;
    }

    void install_function(string name, int argnum)
    {
        function_signatures.insert(make_pair(name, argnum));
    }

    void install_var(string name)
    {
        vars.insert(name);
    }

    namescope* clone() const
    {
        namescope* r = new namescope();
        if (p_outer != nullptr)
            r->p_outer = p_outer->clone();
        r->function_signatures = function_signatures;
        r->owns_outer_scope = true;
        return r;
    }

    ~namescope()
    {
        if (owns_outer_scope)
            delete p_outer;
    }
};

class activation_record
{
    const activation_record* p_outer;
    unordered_map<string, shared_ptr<installed_function>> functions;
    unordered_map<string, shared_ptr<value>> vars;
    namescope ns;

public:
    activation_record() { }
    activation_record(const activation_record* outer) : p_outer(outer) { }
    activation_record(const activation_record&) = delete;

    void install_function(function<void(const activation_record&, const vector<shared_ptr<value>>&)> f, int argnum, string name)
    {
        installed_function* pf = new installed_function;
        pf->function = f;
        pf->argnum = argnum;
        functions.insert(make_pair(name, shared_ptr<installed_function>(pf)));
        ns.install_function(name, argnum);
    }

    void install_var(shared_ptr<value> v, string name)
    {
        vars.insert(make_pair(name, v));
        ns.install_var(name);
    }

    shared_ptr<installed_function> get_func(string name) const
    {
        auto pfunc = functions.find(name);
        if (pfunc != functions.end())
            return pfunc->second;
        if (p_outer != nullptr)
            return p_outer->get_func(name);
        return nullptr;
	}

    shared_ptr<value> get_var(string name) const
    {
        auto pvar = vars.find(name);
        if (pvar != vars.end())
            return pvar->second;
        if (p_outer != nullptr)
            return p_outer->get_var(name);
        return nullptr;
    }

    const namescope& get_ns()
    {
        return ns;
    }
};

#endif