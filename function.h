#ifndef FUNCTION_H
#define FUNCTION_H

#include <functional>
#include <vector>
#include <memory>

#include "value.h"

using namespace std;

class activation_record;

struct installed_function
{
    std::function<void(const activation_record&, const std::vector<shared_ptr<value>>&)> function;
    int argnum;
};

#endif