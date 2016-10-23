#include "stdafx.h"

#include <string>
#include <iostream>
#include <memory>

#include "parser.h"
#include "installed_functions.h"

using namespace std;

string text = 
    R"%(
        def g(x, y)
        {
            def f(x) { dump(y) }
            def h(y) { f(y) }
            h(true)
        }
        g(1, 2)
        repeat (3)
        {
            def f(x)
            {
                click(1, 1)
                dump(x)
                if (x) { f(false) }
            }
            click(10,10)
            pause(100s)
            click(10,10)
            repeat(2)
            {
               pause(10s)
               f(true)
            }
        })%";

int main(int argc, char* argv[])
{
    activation_record r;
    r.install_function(f_pause, 1, "pause");
    r.install_function(f_click, 2, "click");
    r.install_function(f_dump, 1, "dump");

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

