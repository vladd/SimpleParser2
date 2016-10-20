#ifndef EXC_H
#define EXC_H

#include <exception>
#include <string>

#include "tokenizer.h"

using namespace std;

struct parse_exception : exception
{
    int row;
    int col;
    string text;

    parse_exception(string text, token t) : text(text), row(t.lineno), col(t.colno)
    {
    }
};

struct runtime_exception : exception
{
    string text;

    runtime_exception(string text) : text(text)
    {
    }
};

#endif