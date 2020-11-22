#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <vector>

using namespace std;

class LogicCell
{
public:
    LogicCell(string, string);
    void setName(string);
    void setType(string);
    void addInput(string);
    void addOutput(string);
    string getName();
    string getType();
    size_t getInputSize();
    size_t getOutputSize();
    string getNInput(size_t);
    string getNOutput(size_t);

private:
    string name;
    string type;
    vector<string> inputs, outputs;
};

#endif