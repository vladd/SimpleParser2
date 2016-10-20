#ifndef INSTALLED_FUNCTIONS_H
#define INSTALLED_FUNCTIONS_H

#include <vector>
#include <memory>
#include <iostream>
#include <chrono>
#include <functional>
#include <unordered_map>

#include "value.h"
#include "exc.h"

using namespace std;

inline void f_pause(const activation_record&, const vector<shared_ptr<value>>& args)
{
    auto parg = dynamic_cast<typed_value<chrono::seconds>*>(args[0].get());
    if (!parg)
        throw runtime_exception("argument type mismatch in function pause");
    auto duration = parg->value;
    cout << "pause: " << duration.count() << " seconds" << endl;
}

inline void f_click(const activation_record&, const vector<shared_ptr<value>>& args)
{
    auto parg1 = dynamic_cast<typed_value<int>*>(args[0].get());
    auto parg2 = dynamic_cast<typed_value<int>*>(args[1].get());
    if (!parg1 || !parg2)
        throw runtime_exception("argument type mismatch in function click");
    auto x = parg1->value;
    auto y = parg2->value;
    cout << "click: (" << x << ", " << y << ")" << endl;
}

#endif