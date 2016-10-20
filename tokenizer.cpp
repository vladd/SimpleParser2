#include "stdafx.h"
#include "tokenizer.h"

void tokenizer::set_lookahead()
{
    while (curridx < endidx && isspace(text[curridx]))
    {
        if (text[curridx] == '\n')
        {
            currcol = 1;
            currline++;
        }
        else
        {
            currcol++;
        }
        curridx++;
    }

    lookahead.lineno = currline;
    lookahead.colno = currcol;
    if (curridx == endidx)
    {
        lookahead.type = tt_eof;
        return;
    }

    char c = text[curridx];

    if (c == '(' || c == ')' || c == '{' || c == '}' || c == ',')
    {
        lookahead.type = (c == '(') ? tt_lparen :
                            (c == ')') ? tt_rparen :
                            (c == '{') ? tt_lbrace :
                            (c == '}') ? tt_rbrace :
                            tt_comma;
        curridx++;
        currcol++;
        return;
    }

    if (isalpha(c)) // ident
    {
        string result;
        while (curridx < endidx && isalpha(text[curridx]))
        {
            result += text[curridx];
            curridx++;
            currcol++;
        }

		if (result == "repeat")
		{
			lookahead.type = tt_repeat;
			return;
		}
		if (result == "if")
		{
			lookahead.type = tt_if;
			return;
		}
		if (result == "def")
		{
			lookahead.type = tt_def;
			return;
		}
        // check other keywords here

		if (result == "true" || result == "false")
		{
			lookahead.type = tt_boolval;
			lookahead.bool_value = result == "true";
			return;
		}

        lookahead.type = tt_ident;
        lookahead.string_value = result;
        return;
    }

    if (isdigit(c)) // numeric
    {
        string result;
        while (curridx < endidx && isalnum(text[curridx]))
        {
            result += text[curridx];
            curridx++;
            currcol++;
        }
        auto c_str = result.c_str();
        char* last_char;
        long converted = strtol(c_str, &last_char, 10);
        if (*last_char == 0)
        {
            lookahead.type = tt_number;
            lookahead.num_value = converted;
            return;
        }
        if (*last_char == 's' && *(last_char + 1) == 0)
        {
            lookahead.type = tt_duration;
            lookahead.num_value = converted;
            return;
        }
    }

    lookahead.type = tt_error;
}
