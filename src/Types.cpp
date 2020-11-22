#include "Types.h"

using namespace std;

LogicCell::LogicCell(string name, string type)
{
    setName(name);
    setType(type);
}

void LogicCell::setName(string name)
{
    this->name = name;
}

void LogicCell::setType(string type)
{
    this->type = type;
}

void LogicCell::addInput(string input)
{
    this->inputs.push_back(input);
}

void LogicCell::addOutput(string output)
{
    this->outputs.push_back(output);
}

string LogicCell::getName()
{
    return this->name;
};

string LogicCell::getType()
{
    return this->type;
};

size_t LogicCell::getInputSize()
{
    return this->inputs.size();
};

size_t LogicCell::getOutputSize()
{
    return this->outputs.size();
};

string LogicCell::getNInput(size_t n)
{
    return (n >= this->inputs.size()) ? NULL : this->inputs.at(n);
};

string LogicCell::getNOutput(size_t n)
{
    return (n >= this->outputs.size()) ? NULL : this->outputs.at(n);
};