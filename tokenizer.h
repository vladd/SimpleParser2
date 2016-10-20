#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
using namespace std; // never do this

enum token_type
{
    // keywords
    tt_repeat,
	tt_if,
	tt_def,

    tt_ident,
    tt_number,
    tt_duration,
	tt_boolval,

    // punctuation
    tt_lparen,
    tt_rparen,
    tt_lbrace,
    tt_rbrace,
    tt_comma,

    // end of file
    tt_eof,

    // parse error
    tt_error
};

struct token
{
    token_type type;
    string string_value;
    long num_value;
	bool bool_value;
    int lineno;
    int colno;
};

class tokenizer
{
    const string text;
    int curridx;
    int endidx;
    int currline, currcol;

    token lookahead;

    void set_lookahead();

public:
    tokenizer(string text) : text(text), curridx(0), endidx(text.length()), currline(1), currcol(1)
    {
        lookahead.num_value = 0;
        lookahead.string_value = "";
        set_lookahead();
    }

    token peek_next() { return lookahead; }
    void move_ahead() { set_lookahead(); }
};

#endif