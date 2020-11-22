#include "NetlistParser.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>

Parser::Parser(string path)
{
    this->netlist_path = path;
}

Parser::~Parser()
{
    if (this->G)
        delete this->G;
}

bool Parser::cleanExp(string &line)
{
    // skip line comment
    line = line.substr(0, line.find("//"));
    // skip comment before "*/"
    size_t pos = line.find("*/");
    line.erase(0, (pos != string::npos) ? pos + 2 : 0);
    // clean left string
    line.erase(0, line.find_first_not_of(" "));
    line.erase(0, line.find_first_not_of("\t"));
    // skip comment after "/*"
    line = line.substr(0, line.find("/*"));
    // clean right string
    line.erase(line.find_last_not_of(" ") + 1);
    line.erase(line.find_last_not_of("\t") + 1);
    return (line[line.length() - 1] == ';');
}

vector<string> Parser::splitStr(string str, string delimiter)
{
    size_t pos = 0;
    vector<string> splited;
    while ((pos = str.find(delimiter)) != string::npos) {
        splited.push_back(str.substr(0, pos));
        str.erase(0, pos + delimiter.length());
    }
    splited.push_back(str);
    return splited;
}

void Parser::parseVar(string exp, vector<string> &vars)
{
    exp.erase(remove(exp.begin(), exp.end(), ' '), exp.end());
    vector<string> splited = splitStr(exp, ",");
    vars.resize(splited.size());
    copy(splited.begin(), splited.end(), vars.begin());
}

void Parser::parseGate(string exp, string type, vector<LogicCell> &gates)
{
    // parse logic cell
    string gate_name = exp.substr(0, exp.find("("));
    gate_name = gate_name.substr(0, gate_name.find_last_not_of(" ") + 1);
    string params = exp.substr(exp.find("(") + 1,
                               exp.find_last_of(")") - exp.find("(") - 1);
    params.erase(remove(params.begin(), params.end(), ' '), params.end());
    LogicCell gate(gate_name, type);
    regex gate_pin_reg("\\.([A-Z][A-Z]*[0-9]*)\\(([^)]+)\\),*");
    smatch m;
    vector<string> pin, name;
    while (regex_search(params, m, gate_pin_reg)) {
        pin.push_back(m.str(1));
        name.push_back(m.str(2));
        params = m.suffix();
    }
    if (type.compare("INVX1") == 0) {
        if (pin[0].compare("I") == 0)
            gate.addInput(name[0]);
        else
            gate.addOutput(name[0]);

        if (pin[1].compare("I") == 0)
            gate.addInput(name[1]);
        else
            gate.addOutput(name[1]);
    } else {
        for (int i = 0; i < 3; i++) {
            if (pin[i].compare("A1") == 0) {
                gate.addInput(name[i]);
                break;
            }
        }
        for (int i = 0; i < 3; i++) {
            if (pin[i].compare("A2") == 0) {
                gate.addInput(name[i]);
                break;
            }
        }
        for (int i = 0; i < 3; i++) {
            if (pin[i].compare("ZN") == 0) {
                gate.addOutput(name[i]);
                break;
            }
        }
    }
    gates.push_back(gate);
};

void Parser::parse()
{
    ifstream netlist;
    netlist.open(this->netlist_path);
    if (!netlist) {
        cerr << "[Parser] Error: cannot open file \"" << this->netlist_path
             << "\"" << endl;
        return;
    }
    string line;
    bool prev_is_end = true;
    string prev_line;
    while (getline(netlist, line)) {
        bool is_end = cleanExp(line);
        if (!is_end) {
            prev_is_end = false;
            prev_line += line;
            continue;
        }
        if (!prev_is_end) {
            line = prev_line + line;
            prev_line.clear();
        }
        string keyword = line.substr(0, line.find(" "));
        string exp =
            line.substr(line.find(" ") + 1, line.length() - line.find(" ") - 2);

        if (keyword.compare("//") == 0) {
            continue;
        } else if (keyword.compare("input") == 0)
            parseVar(exp, this->inputs);
        else if (keyword.compare("output") == 0)
            parseVar(exp, this->outputs);
        else if (keyword.compare("wire") == 0)
            parseVar(exp, this->wires);
        else if (keyword.compare("NOR2X1") == 0)
            parseGate(exp, "NOR2X1", this->gates);
        else if (keyword.compare("INVX1") == 0)
            parseGate(exp, "INVX1", this->gates);
        else if (keyword.compare("NANDX1") == 0)
            parseGate(exp, "NANDX1", this->gates);
    }

    netlist.close();
}

void Parser::buildGraph()
{
    size_t size = this->inputs.size() + this->outputs.size() +
                  this->wires.size() + this->gates.size() + 2;
    if (size == 2) {
        cerr << "[Parser] Error: the netlist graph is empty" << endl;
        return;
    }
    vector<vertex> vertexes(size);

    // add vertexes
    int idx = 0;
    vertexes[idx] = {
        .id = idx, .name = "START", .type = vertex_type::S, .cell_type = ""};
    idx++;
    for (auto i = this->inputs.begin(); i != this->inputs.end(); i++) {
        vertexes[idx] = {
            .id = idx, .name = *i, .type = vertex_type::INPUT, .cell_type = ""};
        idx++;
    }
    for (auto i = this->gates.begin(); i != this->gates.end(); i++) {
        vertexes[idx] = {.id = idx,
                         .name = i->getName(),
                         .type = vertex_type::GATE,
                         .cell_type = i->getType()};
        idx++;
    }
    for (auto i = this->outputs.begin(); i != this->outputs.end(); i++) {
        vertexes[idx] = {.id = idx,
                         .name = *i,
                         .type = vertex_type::OUTPUT,
                         .cell_type = ""};
        idx++;
    }
    vertexes[idx] = {.id = idx, .name = "END", .type = vertex_type::E};
    idx++;
    for (auto i = this->wires.begin(); i != this->wires.end(); i++) {
        vertexes[idx] = {
            .id = idx, .name = *i, .type = vertex_type::WIRE, .cell_type = ""};
        idx++;
    }
    this->G = new Graph(vertexes);


    // add edges
    for (auto i = this->inputs.begin(); i != this->inputs.end(); i++)
        this->G->addEdge("START", *i, *i, "");
    for (auto i = this->gates.begin(); i != this->gates.end(); i++) {
        if (i->getType().compare("NANDX1") == 0 ||
            i->getType().compare("NOR2X1") == 0) {
            this->G->addEdge(i->getNInput(0), i->getName(), i->getNInput(0),
                             "A1");
            this->G->addEdge(i->getNInput(1), i->getName(), i->getNInput(1),
                             "A2");
        } else if (i->getType().compare("INVX1") == 0)
            this->G->addEdge(i->getNInput(0), i->getName(), i->getNInput(0),
                             "I");
        this->G->addEdge(i->getName(), i->getNOutput(0), i->getNOutput(0),
                         "ZN");
    }

    for (auto i = this->outputs.begin(); i != this->outputs.end(); i++)
        this->G->addEdge(*i, "END", *i, "");

    // delete wire
    for (auto i = size - this->wires.size(); i < size; i++)
        this->G->deleteVertex(i);
}

Graph *Parser::getGraph()
{
    return (this->G != NULL) ? (this->G) : NULL;
}

vector<string> Parser::getInputs()
{
    return this->inputs;
}

vector<string> Parser::getOutputs()
{
    return this->outputs;
}

vector<string> Parser::getWires()
{
    return this->wires;
}

vector<LogicCell> Parser::getGates()
{
    return this->gates;
}