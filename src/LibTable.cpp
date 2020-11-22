#include "LibTable.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>

void DTable::print()
{
    cout << table_name;
    for (size_t i = 0; i < value.size(); i++) {
        cout << "\n";
        for (size_t j = 0; j < value[i].size(); j++)
            cout << fixed << setprecision(6) << value[i][j] << " ";
    }
    cout << "\n" << endl;
}

void LibPin::print(void)
{
    cout << "name: " << pin_name << endl;
    cout << "direction: " << direction << endl;
    cout << "capacitance: " << capacitance << endl;
    rise_power.print();
    fall_power.print();
    cell_rise.print();
    cell_fall.print();
    rise_transition.print();
    fall_transition.print();
    // cout << "@"<< endl;
}

void LibCell::print()
{
    cout << "cell type: " << cell_name << "\n";
    for (auto it = pins.begin(); it != pins.end(); it++) {
        it->second.print();
    }
    // cout << "#"<< endl;
}

void lib_table_template::print()
{
    cout << "name: " << name << "\n";
    cout << "variable_1: " << variable_1 << "\n";
    cout << "variable_2: " << variable_2 << "\n";
    for (size_t i = 0; i < index_1.size(); i++)
        cout << index_1[i] << " ";
    cout << "\n";
    for (size_t i = 0; i < index_2.size(); i++)
        cout << index_2[i] << " ";
    cout << endl;
}

void CellLib::parse(string lib_path)
{
    ifstream lib;
    lib.open(lib_path);
    if (!lib) {
        cerr << "[LibParser] Error: cannot open file \"" << lib_path << "\""
             << endl;
        return;
    }
    regex one_line_comment("\\s*/\\*.*\\*/");
    regex one_line_attribute("\\s*(\\S+)\\s*:\\s*(\\S+)\\s*;");
    regex lu_table_start("\\s*lu_table_template\\s*\\(\\s*(\\S+)\\s*\\)");
    regex block_end("\\s*\\}");
    regex one_line_value("\\s*(\\S+)\\s*\\(\\s*(.*?)\\)");
    regex cell_start("\\s*cell\\s*\\(\\s*(\\S+)\\s*\\)");
    regex pin_start("\\s*pin\\s*\\(\\s*(\\S+)\\s*\\)");
    regex table_start("\\s*(\\S+)\\s*\\(\\s*(\\S*)\\s*\\)");
    string input_buffer;
    while (getline(lib, input_buffer)) {
        smatch m;
        if (regex_search(input_buffer, m, one_line_comment)) {  // /*....*/
            continue;
        } else if (regex_search(input_buffer, m,
                                one_line_attribute)) {  // time_unit : "1ns";
            lib_attr.insert(make_pair(m.str(1), m.str(2)));
            // for( int i=0; i<m.size(); i++)
            // cout << m.str(i)<<endl;
        } else if (regex_search(
                       input_buffer, m,
                       lu_table_start)) {  // lu_table_template(table10) {
            lib_table_template ltt;
            // for( int i=0; i<m.size(); i++)
            // cout << m.str(i)<<endl;
            ltt.name = m.str(1);
            string tmp_buffer;
            smatch tmp_m;
            while (getline(lib, tmp_buffer)) {
                if (regex_search(tmp_buffer, tmp_m, block_end)) {
                    break;
                } else if (regex_search(
                               tmp_buffer, tmp_m,
                               one_line_attribute)) {  // variable_1 : ...;
                    if (tmp_m.str(1) == "variable_1") {
                        ltt.variable_1 = tmp_m.str(2);
                    } else if (tmp_m.str(1) == "variable_2") {
                        ltt.variable_2 = tmp_m.str(2);
                    } else {
                        cout << "[LibParser] Error: one_line_attribute "
                             << tmp_m.str(0) << endl;
                    }
                } else if (regex_search(tmp_buffer, tmp_m,
                                        one_line_value)) {  // index_1 ();
                    if (tmp_m.str(1) == "index_1") {
                        string tmp_str = tmp_m.str(2);
                        replace(tmp_str.begin(), tmp_str.end(), '"', ' ');
                        replace(tmp_str.begin(), tmp_str.end(), ',', ' ');
                        stringstream ss(tmp_str);
                        double value;
                        while (ss >> value) {
                            ltt.index_1.push_back(value);
                        }
                    } else if (tmp_m.str(1) == "index_2") {
                        string tmp_str = tmp_m.str(2);
                        replace(tmp_str.begin(), tmp_str.end(), '"', ' ');
                        replace(tmp_str.begin(), tmp_str.end(), ',', ' ');
                        stringstream ss(tmp_str);
                        double value;
                        while (ss >> value) {
                            ltt.index_2.push_back(value);
                        }
                    } else
                        cout << "[LibParser] Error: one_line_value" << endl;
                }
                // cout<<tmp_buffer<<endl;
            }
            lib_template.insert(make_pair(m.str(1), ltt));
            // ltt.print();
        } else if (regex_search(input_buffer, m, cell_start)) {  // cell
                                                                 // (NOR2x1)
            cell lc;
            lc.cell_name = m.str(1);
            string tmp_buffer;
            smatch tmp_m;
            while (getline(lib, tmp_buffer)) {
                if (regex_search(tmp_buffer, tmp_m, block_end)) {
                    break;
                } else if (regex_search(tmp_buffer, tmp_m,
                                        pin_start)) {  // pin(A1)
                    pin lp;
                    lp.pin_name = tmp_m.str(1);
                    string tmp_buffer_pin;
                    smatch tmp_m_pin;

                    while (getline(lib, tmp_buffer_pin)) {
                        if (regex_search(tmp_buffer_pin, tmp_m_pin,
                                         block_end)) {
                            break;
                        } else if (regex_search(
                                       tmp_buffer_pin, tmp_m_pin,
                                       one_line_attribute)) {  // direction :
                                                               // output
                            if (tmp_m_pin.str(1) == "direction") {
                                lp.direction = tmp_m_pin.str(2);
                            } else if (tmp_m_pin.str(1) == "capacitance") {
                                lp.capacitance = stod(tmp_m_pin.str(2));
                            }
                        } else if (regex_search(
                                       tmp_buffer_pin, tmp_m_pin,
                                       table_start)) {  // internal_power,
                                                        // timing
                            string tmp_buffer2;
                            smatch tmp_m2;

                            while (getline(lib, tmp_buffer2)) {
                                if (regex_search(tmp_buffer2, tmp_m2,
                                                 block_end)) {
                                    break;
                                } else if (regex_search(tmp_buffer2, tmp_m2,
                                                        table_start)) {
                                    // rise power, fall power, cell rise/fall,
                                    // output transition parse table value
                                    table tb;
                                    tb.table_name = tmp_m2.str(1);
                                    string table_buffer;
                                    smatch table_m;

                                    getline(lib, table_buffer, ';');
                                    regex one_row_reg(
                                        "\"\\s*(\\S+?)\\s*\"");  // "0,1,2,3"

                                    while (regex_search(table_buffer, table_m,
                                                        one_row_reg)) {
                                        // cout << table_m.str(1) <<endl;
                                        vector<double> row;
                                        // cout << table_m.str(1) <<endl;
                                        string tmp_string = table_m.str(1);
                                        replace(tmp_string.begin(),
                                                tmp_string.end(), ',', ' ');
                                        stringstream ss(tmp_string);
                                        double tmp_double;
                                        while (ss >> tmp_double) {
                                            row.push_back(tmp_double);
                                        }
                                        tb.value.push_back(row);
                                        table_buffer = table_m.suffix();
                                    }

                                    if (tmp_m2.str(1) == "rise_power")
                                        lp.rise_power = tb;
                                    else if (tmp_m2.str(1) == "fall_power")
                                        lp.fall_power = tb;
                                    else if (tmp_m2.str(1) == "cell_rise")
                                        lp.cell_rise = tb;
                                    else if (tmp_m2.str(1) == "cell_fall")
                                        lp.cell_fall = tb;
                                    else if (tmp_m2.str(1) == "rise_transition")
                                        lp.rise_transition = tb;
                                    else if (tmp_m2.str(1) == "fall_transition")
                                        lp.fall_transition = tb;

                                    while (getline(lib, table_buffer)) {
                                        if (regex_search(table_buffer, table_m,
                                                         block_end))
                                            break;
                                    }
                                }
                            }
                        }
                    }
                    lc.pins.insert(make_pair(tmp_m.str(1), lp));
                }
            }

            // lc.print();
            lib_cells.insert(make_pair(m.str(1), lc));
        }
    }
    lib.close();
}

void CellLib::print()
{
    cout << "Cell Library\n";
    cout << "unit attributes:\n";
    for (auto it = lib_attr.begin(); it != lib_attr.end(); it++)
        cout << it->first << ":" << it->second << "\n";
    cout << "\n";
    for (auto it = lib_template.begin(); it != lib_template.end(); it++) {
        cout << it->first << "\n";
        it->second.print();
    }
    cout << "\n";
    for (auto it = lib_cells.begin(); it != lib_cells.end(); it++) {
        it->second.print();
    }
    cout << endl;
}
