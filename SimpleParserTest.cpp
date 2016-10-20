#include "stdafx.h"

#include <string>
#include <iostream>
#include <memory>

#include "parser.h"
#include "installed_functions.h"

using namespace std;

string text = "repeat (1)\n"
              "{\n"
              "click(10,10)\n"
              "pause(100s)\n"
              "click(10,10)\n"
              "def f(x) { click(1, 1) if (x) { f(false) } }\n"
              "repeat(2)\n"
              "   {\n"
              "   pause(10s)\n"
              "   f(true)\n"
              "   }\n"
              "}";

int main(int argc, char* argv[])
{
    activation_record r;
    r.install_function(f_pause, 1, "pause");
    r.install_function(f_click, 2, "click");

    auto& ns = r.get_ns();

    parser p(text);
    unique_ptr<program> tree;
    try
    {
        tree.reset(p.parse(ns));
        cout << "executing:" << endl;
        tree->execute(r);
    }
    catch (const parse_exception& ex)
    {
        cerr << "parse exception at line " << ex.row << ", char " << ex.col << ": " << ex.text << endl;
    }
    catch (const runtime_exception& ex)
    {
        cerr << "runtime exception: " << ex.text << endl;
    }

    return 0;
}

