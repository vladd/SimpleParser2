#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <memory>
#include <unordered_map>

#include "tokenizer.h"
#include "nodes.h"
#include "installed_functions.h"
#include "exc.h"

using namespace std;

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
    namelist ::= EMPTY | ident ["," ident]*
*/

class parser
{
    expr* try_parse_expr(namescope* pns);
    expr* try_parse_param(namescope* pns);
    paramlist* try_parse_paramlist_until_rparen(namescope* pns);
    namelist* try_parse_namelist_until_rparen(namescope* pns);
    function_call* try_parse_function_call(namescope* pns);
    repeat_statement* try_parse_repeat_statement(namescope* pns);
    statement* try_parse_statement(namescope* pns);
    compound_statement* try_parse_compound_statement(namescope* pns);
    if_statement* try_parse_if_statement(namescope* pns);
    def_statement* try_parse_def_statement(namescope* pns);

    tokenizer tokenizer;

public:
    program* parse(const namescope& initialns);

public:
    parser(string input) : tokenizer(input)
    {
    }
};

#endif