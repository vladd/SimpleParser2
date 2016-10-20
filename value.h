#ifndef ARGS_H
#define ARGS_H

#include <chrono>
#include <vector>
#include <memory>

struct value
{
    virtual ~value() { }
};

template<typename T>
struct typed_value : public value
{
    typed_value(T value) : value(value) { }
    T value;
};

struct arglist
{
    std::vector<std::shared_ptr<value>> args;
};

#endif