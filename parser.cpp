#include "stdafx.h"
#include "parser.h"

/*
grammar:
    program ::= statement* EOF
    compound-statement ::= '{' statement* '}'
    statement ::= repeat-statement | function-call | compound-statement | if-statement | def-statement
    repeat-statement ::= "repeat" "(" expr ")" compound-statement
    if-statement ::= "if" "(" expr ")" compound-statement
    def-statement ::= "def" ident "(" namelist ")" compound-statement
    function-call ::= ident "(" paramlist ")"
    paramlist ::= EMPTY | param ["," param]*
    expr ::= number-constant | duration-constant | bool-constant | arg
    param ::= expr
*/

// program ::= statement* EOF
program* parser::parse(const namescope& initialns)
{
    unique_ptr<namescope> pns(initialns.clone());
    unique_ptr<program> p(new program());
    while (true)
    {
        statement* s = try_parse_statement(pns.get());
        if (!s)
            break;
        p->statements.emplace_back(s);
    }

    token t = tokenizer.peek_next();
    if (t.type != tt_eof)
        throw parse_exception("extra characters after program end", t);
    return p.release();
}

// compound-statement ::= '{' statement* '}'
compound_statement* parser::try_parse_compound_statement(namescope* pns)
{
    token t = tokenizer.peek_next();
    if (t.type != tt_lbrace)
        return nullptr;
    tokenizer.move_ahead();
    unique_ptr<compound_statement> p(new compound_statement());
    namescope inner(pns);
    while (true)
    {
        statement* s = try_parse_statement(&inner);
        if (!s)
            break;
        p->statements.emplace_back(s);
    }
    t = tokenizer.peek_next();
    if (t.type != tt_rbrace)
        throw parse_exception("expected closing brace after compound statement", t);
    tokenizer.move_ahead();
    return p.release();
}

// statement ::= repeat-statement | function-call | compound-statement | if-statement | def-statement
statement* parser::try_parse_statement(namescope* pns)
{
    statement* result = nullptr;
    result = try_parse_repeat_statement(pns);
    if (result)
        return result;

    result = try_parse_function_call(pns);
    if (result)
        return result;

    result = try_parse_compound_statement(pns);
    if (result)
        return result;

    result = try_parse_if_statement(pns);
    if (result)
        return result;

    result = try_parse_def_statement(pns);
    if (result)
        return result;

    return nullptr;
}

// repeat-statement ::= "repeat" "(" number-constant ")" compound-statement
repeat_statement* parser::try_parse_repeat_statement(namescope* pns)
{
    token t = tokenizer.peek_next();
    if (t.type != tt_repeat)
        return nullptr;
    tokenizer.move_ahead();

    t = tokenizer.peek_next();
    if (t.type != tt_lparen)
        throw parse_exception("opening parenthesis expected", t);
    tokenizer.move_ahead();

    t = tokenizer.peek_next();
    if (t.type != tt_number)
        throw parse_exception("number expected", t);
    tokenizer.move_ahead();

    long num = t.num_value;

    t = tokenizer.peek_next();
    if (t.type != tt_rparen)
        throw parse_exception("closing parenthesis expected", t);
    tokenizer.move_ahead();

    unique_ptr<compound_statement> s(try_parse_compound_statement(pns));
    if (!s)
        throw parse_exception("compound statement expected after repeat", tokenizer.peek_next());

    repeat_statement* rs = new repeat_statement();
    rs->num_repeat = num;
    rs->p_statement = move(s);
    return rs;
}

// if-statement ::= "if" "(" expr ")" compound-statement
if_statement* parser::try_parse_if_statement(namescope* pns)
{
    token t = tokenizer.peek_next();
    if (t.type != tt_if)
        return nullptr;
    tokenizer.move_ahead();

    t = tokenizer.peek_next();
    if (t.type != tt_lparen)
        throw parse_exception("opening parenthesis expected", t);
    tokenizer.move_ahead();

    unique_ptr<expr> condition(try_parse_expr(pns));
    if (!condition)
        throw parse_exception("expression expected", tokenizer.peek_next());

    t = tokenizer.peek_next();
    if (t.type != tt_rparen)
        throw parse_exception("closing parenthesis expected", t);
    tokenizer.move_ahead();

    unique_ptr<compound_statement> s(try_parse_compound_statement(pns));
    if (!s)
        throw parse_exception("compound statement expected after repeat", tokenizer.peek_next());

    if_statement* is = new if_statement();
    is->p_expression = move(condition);
    is->p_statement = move(s);
    return is;
}

// def-statement ::= "def" ident "(" namelist ")" compound-statement
// namelist ::= EMPTY | ident ["," ident]*
def_statement* parser::try_parse_def_statement(namescope* pns)
{
    token t = tokenizer.peek_next();
    if (t.type != tt_def)
        return nullptr;
    tokenizer.move_ahead();

    t = tokenizer.peek_next();
    if (t.type != tt_ident)
        throw parse_exception("identifier for function name expected", t);
    tokenizer.move_ahead();

    string name = t.string_value;

    t = tokenizer.peek_next();
    if (t.type != tt_lparen)
        throw parse_exception("opening parenthesis expected", t);
    tokenizer.move_ahead();

    unique_ptr<namelist> args(try_parse_namelist_until_rparen(pns));
    if (!args)
        throw parse_exception("expected argument list for function definition", tokenizer.peek_next());

    t = tokenizer.peek_next();
    if (t.type != tt_rparen)
        throw parse_exception("closing parenthesis expected", t);
    tokenizer.move_ahead();

    pns->install_function(name, args->names.size());
    namescope inner(pns);
    for (auto& argname : args->names)
        inner.install_var(argname);

    unique_ptr<compound_statement> s(try_parse_compound_statement(&inner));
    if (!s)
        throw parse_exception("compound statement expected for function body", tokenizer.peek_next());

    def_statement* ds = new def_statement();
    ds->name = name;
    ds->argnames = args->names;
    ds->p_statement = move(s);
    return ds;
}

// function-call ::= ident "(" arglist ")"
function_call* parser::try_parse_function_call(namescope* pns)
{
    token ft = tokenizer.peek_next();
    if (ft.type != tt_ident)
        return nullptr;
    tokenizer.move_ahead();

    string name = ft.string_value;

    token t = tokenizer.peek_next();
    if (t.type != tt_lparen)
        throw parse_exception("left parenthesis expected for function call", t);
    tokenizer.move_ahead();

    unique_ptr<paramlist> args(try_parse_paramlist_until_rparen(pns));
    if (!args)
        throw parse_exception("argument list not found", tokenizer.peek_next());

    t = tokenizer.peek_next();
    if (t.type != tt_rparen)
        throw parse_exception("right parenthesis expected after function call arg list", t);
    tokenizer.move_ahead();

    auto lookup = pns->lookup_func(name, args->params.size());
    if (lookup == namescope::lookup_result::not_found)
        throw parse_exception("unknown function", ft);
    if (lookup == namescope::lookup_result::wrong_signature)
        throw parse_exception("signature mismatch for function", ft);

    function_call* fc = new function_call();
    fc->function_name = name;
    fc->p_params = move(args);
    return fc;
}

// expr ::= number-constant | duration-constant | bool-constant | arg
expr* parser::try_parse_expr(namescope* pns)
{
    expr* result = nullptr;
    token t = tokenizer.peek_next();
    if (t.type == tt_number)
    {
        result = new const_expr<int>(typed_value<int>(t.num_value));
    }
    else if (t.type == tt_duration)
    {
        result = new const_expr<chrono::seconds>(typed_value<chrono::seconds>(chrono::seconds(t.num_value)));
    }
    else if (t.type == tt_boolval)
    {
        result = new const_expr<bool>(typed_value<bool>(t.bool_value));
    }
    else if (t.type == tt_ident)
    {
        if (pns->lookup_var(t.string_value) != namescope::lookup_result::found)
            throw parse_exception("unknown variable", t);
        result = new var(t.string_value);
    }
    if (result != nullptr)
        tokenizer.move_ahead();
    return result;
}

// param ::= expr
expr* parser::try_parse_param(namescope* pns)
{
    return try_parse_expr(pns);
}

// arglist ::= EMPTY | arg ["," arg]*
paramlist* parser::try_parse_paramlist_until_rparen(namescope* pns)
{
    unique_ptr<paramlist> result(new paramlist());
    token t = tokenizer.peek_next();
    if (t.type == tt_rparen)
        return result.release();

    while (true)
    {
        expr* p = try_parse_param(pns);
        if (!p)
            throw parse_exception("expected argument", tokenizer.peek_next());
        result->params.emplace_back(p);

        t = tokenizer.peek_next();
        if (t.type == tt_rparen)
            return result.release();
        if (t.type != tt_comma)
            throw parse_exception("comma expected between arguments", t);
        tokenizer.move_ahead();
    }
}

// namelist ::= EMPTY | ident ["," ident]*
namelist* parser::try_parse_namelist_until_rparen(namescope* pns)
{
    unique_ptr<namelist> result(new namelist());
    token t = tokenizer.peek_next();
    if (t.type == tt_rparen)
        return result.release();

    while (true)
    {
        if (t.type != tt_ident)
            throw parse_exception("expected identifier for argument name", t);
        result->names.emplace_back(t.string_value);
        tokenizer.move_ahead();

        t = tokenizer.peek_next();
        if (t.type == tt_rparen)
            return result.release();
        if (t.type != tt_comma)
            throw parse_exception("comma expected between argument names", t);
        tokenizer.move_ahead();
    }
}