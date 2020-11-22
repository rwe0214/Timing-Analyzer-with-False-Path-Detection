#ifndef NETLIST_PARSER_H
#define NETLIST_PARSER_H

#include <vector>

#include "Graph.h"
#include "Types.h"

using namespace std;

class Parser
{
public:
    Parser(string);
    ~Parser();
    void parse();
    void buildGraph();
    Graph *getGraph();
    bool cleanExp(string &);
    vector<string> splitStr(string, string);
    void parseVar(string, vector<string> &);
    vector<string> getInputs();
    vector<string> getOutputs();
    vector<string> getWires();
    vector<LogicCell> getGates();

private:
    void parseGate(string, string, vector<LogicCell> &);
    string netlist_path;
    Graph *G = NULL;
    vector<string> inputs, outputs, wires;
    vector<LogicCell> gates;
};

#endif