#ifndef LIBTABLE_H
#define LIBTABLE_H


#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

typedef class DTable
{
public:
    string table_name;
    vector<vector<double>> value;
    void print();
} table;

typedef class LibPin
{
public:
    string pin_name;
    string direction;
    double capacitance;
    table rise_power;
    table fall_power;
    table cell_rise;
    table cell_fall;
    table rise_transition;
    table fall_transition;
    void print(void);
} pin;

typedef class LibCell
{
public:
    string cell_name;
    unordered_map<string, pin> pins;
    void print();
} cell;

typedef class lib_table_template
{
public:
    string name;
    string variable_1;
    string variable_2;
    vector<double> index_1;
    vector<double> index_2;
    void print(void);
} table_template;

typedef class CellLib
{
public:
    unordered_map<string, string> lib_attr;
    unordered_map<string, table_template> lib_template;
    unordered_map<string, cell> lib_cells;
    void parse(string);
    void print();
} cell_lib;

#endif