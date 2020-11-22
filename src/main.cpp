#include <assert.h>
#include <getopt.h>
#include <string.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "Graph.h"
#include "LibTable.h"
#include "NetlistParser.h"

using namespace std;

vector<string> inputs;
vector<unordered_map<string, int>> patterns;

void parsePattern(string);
void printPat(vector<unordered_map<string, int>>);
double linearPolation1D(double, double, double, double, double);
double linearPolation2D(double,
                        double,
                        double,
                        double,
                        double,
                        double,
                        double,
                        double,
                        double,
                        double);
vector<edge> getInEdge(vector<vector<edge>>, int);
void updateEdgeWeight(vector<vector<edge>> &, int, int, double);
void getIdxAndVal(int &, int &, double &, double &, vector<double>, double);
vector<double> FindLongestPath(vector<vector<edge>>,
                               int,
                               vector<int> &,
                               bool,
                               ofstream &);
edge getEdge(vector<vector<edge>>, int, int);

int main(int argc, char *argv[])
{
    char n_path[1024] = "testdata/c17.v", p_path[1024] = "testdata/c17.pat",
         l_path[1024] = "lib/test_lib.lib";
    int opt;
    while ((opt = getopt(argc - 1, argv + 1, "p:l:")) != -1) {
        switch (opt) {
        case 'p':
            strncpy(p_path, optarg, sizeof(p_path));
            break;
        case 'l':
            strncpy(l_path, optarg, sizeof(l_path));
            break;
        default:
            fprintf(
                stderr,
                "usage: %s netlist_file [-h] [-p input.pat] [-l testlib.lib]\n",
                argv[0]);
            fprintf(stderr,
                    "\tnetlist_file: set the netlist path, default = "
                    "./testdata/c17.v\n");
            fprintf(stderr, "\t-h: this help text\n");
            fprintf(
                stderr,
                "\t-p: set the pattern path, default = ./testdata/c17.pat\n");
            fprintf(
                stderr,
                "\t-p: set the library path, default = ./lib/test_lib.lib\n");
            exit(EXIT_FAILURE);
        }
    }

    if (argc != 6 || optind != 5) {
        fprintf(stderr,
                "usage: %s netlist_file [-p input.pat] [-l testlib.lib]\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }
    strncpy(n_path, argv[1], sizeof(p_path));

    string netlist_path(n_path), pattern_path(p_path), lib_path(l_path);

    // parse cell library
    cell_lib lib;
    lib.parse(lib_path);

    // parse netlist and build graph
    Parser netlist(netlist_path);
    netlist.parse();
    netlist.buildGraph();
    Graph *G = netlist.getGraph();
    vector<vertex> cells;
    vector<vector<edge>> adj;
    assert(G != NULL);
    G->topologicalSort();
    cells = G->getVertex();
    adj = G->getAdjList();
    vector<string> id2v;
    id2v.resize(cells.size());
    unordered_map<string, int> v2id;
    unordered_map<string, int> v2tid;
    for (unsigned int i = 0; i < cells.size(); i++) {
        id2v[cells[i].id] = cells[i].name;
        v2id.insert(make_pair(cells[i].name, cells[i].id));
        v2tid.insert(make_pair(cells[i].name, i));
    }

    // parse pattern
    parsePattern(pattern_path);

    // compute delay for each pattern and output
    // because the problem of sensitzable path, it needs the delay from the
    // input reg of a gate using a longest path to find the delay, and label the
    // edge which could be discard finally, do longest path again and get the
    // answer

    ofstream output_stream(
        "Results/0856090_" +
        netlist_path.substr(netlist_path.find_last_of("/") + 1,
                            netlist_path.find_last_of(".") -
                                netlist_path.find_last_of("/") - 1) +
        ".txt");
    unordered_map<string, double> pd;    // propagation delay
    unordered_map<string, double> orft;  // output rising/falling time
    for (unsigned int n = 0; n < patterns.size(); n++) {
        pd.clear();
        orft.clear();
        vector<vector<edge>> n_adj = adj;

        for (unsigned int i = 0; i < cells.size(); i++) {
            if (cells[i].type == vertex_type::INPUT) {
                pd.insert(make_pair(cells[i].name, 0.0));
                orft.insert(make_pair(cells[i].name, 0.0));
            } else if (cells[i].type == vertex_type::OUTPUT) {
                // output regs could be connected by other gates
                // Need to handle this case
                string in_name = getInEdge(n_adj, cells[i].id)[0].u_name;
                pd.insert(make_pair(cells[i].name, pd.at(in_name)));
                orft.insert(make_pair(cells[i].name, orft.at(in_name)));
                int out_signal =
                    patterns[n].at(getInEdge(n_adj, cells[i].id)[0].u_name);
                patterns[n].insert(
                    make_pair(cells[i].name, abs(out_signal % 2)));
                for (unsigned int j = 0;
                     j < n_adj[v2id.at(cells[i].name)].size(); j++) {
                    if (n_adj[v2id.at(cells[i].name)][j].v_name.compare(
                            "END") == 0)
                        continue;
                    n_adj[v2id.at(cells[i].name)][j].delay = pd[cells[i].name];
                }
            } else if (cells[i].type == vertex_type::GATE) {
                double cap = 0.0, itt = 0.0;
                vector<edge> i_adj = n_adj[cells[i].id];
                for (unsigned int j = 0; j < i_adj.size(); j++) {
                    string fo_input = i_adj[j].v_name;
                    vertex_type fo_type = cells[v2tid.at(fo_input)].type;
                    if (fo_type == vertex_type::OUTPUT) {
                        for (unsigned int k = 0;
                             k < n_adj[v2id.at(fo_input)].size() - 1; k++) {
                            string type =
                                cells[v2tid.at(
                                          n_adj[v2id.at(fo_input)][k].v_name)]
                                    .cell_type;
                            string pin = n_adj[v2id.at(fo_input)][k].pin_name;
                            cap +=
                                lib.lib_cells.at(type).pins.at(pin).capacitance;
                        }
                        cap += 0.03;
                    } else {
                        string type = cells[v2tid.at(fo_input)].cell_type;
                        string pin = i_adj[j].pin_name;
                        cap += lib.lib_cells.at(type).pins.at(pin).capacitance;
                    }
                }
                // update weight of input edge to sensitizable delay
                // calulate itt and gate output signal
                vector<edge> i_in = getInEdge(n_adj, cells[i].id);
                int out_signal = 0;
                if (cells[i].cell_type.compare("NANDX1") == 0) {
                    string fin0 = i_in[0].u_name;
                    string fin1 = i_in[1].u_name;
                    itt = max(orft.at(fin0), orft.at(fin1));
                    out_signal = ~(patterns[n].at(fin0) & patterns[n].at(fin1));

                    if (patterns[n].at(fin0) == 0 &&
                        patterns[n].at(fin1) == 1) {
                        updateEdgeWeight(n_adj, i_in[1].u, i_in[1].v, -1.0);
                    } else if (patterns[n].at(fin0) == 1 &&
                               patterns[n].at(fin1) == 0) {
                        updateEdgeWeight(n_adj, i_in[0].u, i_in[0].v, -1.0);
                    }
                } else if (cells[i].cell_type.compare("NOR2X1") == 0) {
                    string fin0 = i_in[0].u_name;
                    string fin1 = i_in[1].u_name;
                    itt = max(orft.at(fin0), orft.at(fin1));
                    out_signal = ~(patterns[n].at(fin0) | patterns[n].at(fin1));
                    if (patterns[n].at(fin0) == 1 &&
                        patterns[n].at(fin1) == 0) {
                        updateEdgeWeight(n_adj, i_in[1].u, i_in[1].v, -1.0);
                    } else if (patterns[n].at(fin0) == 0 &&
                               patterns[n].at(fin1) == 1) {
                        updateEdgeWeight(n_adj, i_in[0].u, i_in[0].v, -1.0);
                    }
                } else if (cells[i].cell_type.compare("INVX1") == 0) {
                    string fin = i_in[0].u_name;
                    itt = orft[fin];
                    out_signal = ~(patterns[n].at(fin));
                }
                bool rise = (abs(out_signal % 2) == 1) ? true : false;
                // find value in the table of cell library
                vector<double> idx1 = lib.lib_template.at("table10").index_1;
                vector<double> idx2 = lib.lib_template.at("table10").index_2;
                int x1_idx, x2_idx, y1_idx, y2_idx;
                double x1, x2, y1, y2;
                double x1y1_pd_val, x1y2_pd_val, x2y1_pd_val, x2y2_pd_val;
                double x1y1_orft_val, x1y2_orft_val, x2y1_orft_val,
                    x2y2_orft_val;

                getIdxAndVal(y1_idx, y2_idx, y1, y2, idx1, cap);
                getIdxAndVal(x1_idx, x2_idx, x1, x2, idx2, itt);
                if (rise == true) {
                    x1y1_pd_val = lib.lib_cells.at(cells[i].cell_type)
                                      .pins.at("ZN")
                                      .cell_rise.value[x1_idx][y1_idx];
                    x1y2_pd_val = lib.lib_cells.at(cells[i].cell_type)
                                      .pins.at("ZN")
                                      .cell_rise.value[x1_idx][y2_idx];
                    x2y1_pd_val = lib.lib_cells.at(cells[i].cell_type)
                                      .pins.at("ZN")
                                      .cell_rise.value[x2_idx][y1_idx];
                    x2y2_pd_val = lib.lib_cells.at(cells[i].cell_type)
                                      .pins.at("ZN")
                                      .cell_rise.value[x2_idx][y2_idx];
                    x1y1_orft_val = lib.lib_cells.at(cells[i].cell_type)
                                        .pins.at("ZN")
                                        .rise_transition.value[x1_idx][y1_idx];
                    x1y2_orft_val = lib.lib_cells.at(cells[i].cell_type)
                                        .pins.at("ZN")
                                        .rise_transition.value[x1_idx][y2_idx];
                    x2y1_orft_val = lib.lib_cells.at(cells[i].cell_type)
                                        .pins.at("ZN")
                                        .rise_transition.value[x2_idx][y1_idx];
                    x2y2_orft_val = lib.lib_cells.at(cells[i].cell_type)
                                        .pins.at("ZN")
                                        .rise_transition.value[x2_idx][y2_idx];
                } else {
                    x1y1_pd_val = lib.lib_cells.at(cells[i].cell_type)
                                      .pins.at("ZN")
                                      .cell_fall.value[x1_idx][y1_idx];
                    x1y2_pd_val = lib.lib_cells.at(cells[i].cell_type)
                                      .pins.at("ZN")
                                      .cell_fall.value[x1_idx][y2_idx];
                    x2y1_pd_val = lib.lib_cells.at(cells[i].cell_type)
                                      .pins.at("ZN")
                                      .cell_fall.value[x2_idx][y1_idx];
                    x2y2_pd_val = lib.lib_cells.at(cells[i].cell_type)
                                      .pins.at("ZN")
                                      .cell_fall.value[x2_idx][y2_idx];
                    x1y1_orft_val = lib.lib_cells.at(cells[i].cell_type)
                                        .pins.at("ZN")
                                        .fall_transition.value[x1_idx][y1_idx];
                    x1y2_orft_val = lib.lib_cells.at(cells[i].cell_type)
                                        .pins.at("ZN")
                                        .fall_transition.value[x1_idx][y2_idx];
                    x2y1_orft_val = lib.lib_cells.at(cells[i].cell_type)
                                        .pins.at("ZN")
                                        .fall_transition.value[x2_idx][y1_idx];
                    x2y2_orft_val = lib.lib_cells.at(cells[i].cell_type)
                                        .pins.at("ZN")
                                        .fall_transition.value[x2_idx][y2_idx];
                }

                // do interpolation/exterapolation
                pd.insert(make_pair(
                    cells[i].name,
                    linearPolation2D(x1, x2, y1, y2, x1y1_pd_val, x1y2_pd_val,
                                     x2y1_pd_val, x2y2_pd_val, itt, cap)));
                orft.insert(
                    make_pair(cells[i].name,
                              linearPolation2D(x1, x2, y1, y2, x1y1_orft_val,
                                               x1y2_orft_val, x2y1_orft_val,
                                               x2y2_orft_val, itt, cap)));
                patterns[n].insert(
                    make_pair(cells[i].name, abs(out_signal % 2)));
                // add propagation delay to ADJ list as weight of edge
                for (unsigned int j = 0;
                     j < n_adj[v2id.at(cells[i].name)].size(); j++) {
                    n_adj[v2id.at(cells[i].name)][j].delay = pd[cells[i].name];
                }
            }
        }

        // get the input delay by using longest path algo.
        vector<double> delay(n_adj.size());
        vector<int> prev;
        delay = FindLongestPath(n_adj, v2id.at("START"), prev, false,
                                output_stream);

        // apply the sensitizable rule
        for (unsigned int i = 0; i < cells.size(); i++) {
            if (cells[i].type == VERTEX_TYPE::GATE &&
                cells[i].cell_type.compare("INVX1") != 0) {
                vector<edge> i_in = getInEdge(n_adj, v2id[cells[i].name]);
                int prev_id = prev[v2id[cells[i].name]];
                string fin0 = i_in[0].u_name;
                string fin1 = i_in[1].u_name;

                double delay_in0 = abs(delay[v2id.at(fin0)]) +
                                   getEdge(n_adj, i_in[0].u, i_in[0].v).delay;
                double delay_in1 = abs(delay[v2id.at(fin1)]) +
                                   getEdge(n_adj, i_in[1].u, i_in[1].v).delay;
                if (cells[i].cell_type.compare("NANDX1") == 0) {
                    if (patterns[n].at(fin0) == 0 &&
                        patterns[n].at(fin1) == 0) {
                        if (delay_in0 > delay_in1) {
                            if (prev_id == i_in[0].u) {
                                delay[i_in[0].v] -= (-1.0) * i_in[0].delay;
                                delay[i_in[0].v] += (-1.0) * i_in[1].delay;
                            }
                            updateEdgeWeight(n_adj, i_in[0].u, i_in[0].v, -1.0);
                        } else if (delay_in0 < delay_in1) {
                            if (prev_id == i_in[1].u) {
                                delay[i_in[1].v] -= (-1.0) * i_in[1].delay;
                                delay[i_in[1].v] += (-1.0) * i_in[0].delay;
                            }
                            updateEdgeWeight(n_adj, i_in[1].u, i_in[1].v, -1.0);
                        }
                    } else if (patterns[n].at(fin0) == 1 &&
                               patterns[n].at(fin1) == 1) {
                        if (delay_in0 > delay_in1) {
                            if (prev_id == i_in[1].u) {
                                delay[i_in[1].v] -= (-1.0) * i_in[1].delay;
                                delay[i_in[1].v] += (-1.0) * i_in[0].delay;
                            }
                            updateEdgeWeight(n_adj, i_in[1].u, i_in[1].v, -1.0);
                        } else if (delay_in0 < delay_in1) {
                            if (prev_id == i_in[0].u) {
                                delay[i_in[0].v] -= (-1.0) * i_in[0].delay;
                                delay[i_in[0].v] += (-1.0) * i_in[1].delay;
                            }
                            updateEdgeWeight(n_adj, i_in[0].u, i_in[0].v, -1.0);
                        }
                    }
                } else if (cells[i].cell_type.compare("NOR2X1") == 0) {
                    if (patterns[n].at(fin0) == 1 &&
                        patterns[n].at(fin1) == 1) {
                        if (delay_in0 > delay_in1) {
                            if (prev_id == i_in[0].u) {
                                delay[i_in[0].v] -= (-1.0) * i_in[0].delay;
                                delay[i_in[0].v] += (-1.0) * i_in[1].delay;
                            }
                            updateEdgeWeight(n_adj, i_in[0].u, i_in[0].v, -1.0);
                        } else if (delay_in0 < delay_in1) {
                            if (prev_id == i_in[1].u) {
                                delay[i_in[1].v] -= (-1.0) * i_in[1].delay;
                                delay[i_in[1].v] += (-1.0) * i_in[0].delay;
                            }
                            updateEdgeWeight(n_adj, i_in[1].u, i_in[1].v, -1.0);
                        }
                    } else if (patterns[n].at(fin0) == 0 &&
                               patterns[n].at(fin1) == 0) {
                        if (delay_in0 > delay_in1) {
                            if (prev_id == i_in[1].u) {
                                delay[i_in[1].v] -= (-1.0) * i_in[1].delay;
                                delay[i_in[1].v] += (-1.0) * i_in[0].delay;
                            }
                            updateEdgeWeight(n_adj, i_in[1].u, i_in[1].v, -1.0);
                        } else if (delay_in0 < delay_in1) {
                            if (prev_id == i_in[0].u) {
                                delay[i_in[0].v] -= (-1.0) * i_in[0].delay;
                                delay[i_in[0].v] += (-1.0) * i_in[1].delay;
                            }
                            updateEdgeWeight(n_adj, i_in[0].u, i_in[0].v, -1.0);
                        }
                    }
                }
            }
        }

        // get the longest sensitizable path and delay
        FindLongestPath(n_adj, v2id.at("START"), prev, true, output_stream);
        output_stream << endl;
        vector<string> sorted_gate;
        for (unsigned int i = 0; i < n_adj.size(); i++) {
            if (cells[v2tid[id2v[i]]].type == VERTEX_TYPE::GATE) {
                sorted_gate.push_back(id2v[i]);
            }
        }
        sort(sorted_gate.begin(), sorted_gate.end(),
             [&](string a, string b) -> bool {
                 return stoi(a.substr(1)) < stoi(b.substr(1));
             });
        for (auto i = sorted_gate.begin(); i != sorted_gate.end(); i++)
            output_stream << *i << " " << patterns[n].at(*i) << " " << pd[*i]
                          << " " << orft[*i] << "\n";
        output_stream << endl;
    }
    output_stream.close();
    return 0;
}

void parsePattern(string pat_path)
{
    ifstream pat;
    pat.open(pat_path);
    if (!pat) {
        cerr << "[main] Error: cannot open .pat: \"" << pat_path << "\""
             << endl;
        return;
    }
    string line;
    Parser p("");
    getline(pat, line);
    line = line.substr(line.find(" ") + 1, line.length() - line.find(" ") - 1);
    p.parseVar(line, inputs);

    vector<string> signals;
    while (getline(pat, line)) {
        p.cleanExp(line);
        if (line.compare(".end") == 0)
            break;
        signals = p.splitStr(line, " ");
        unordered_map<string, int> pattern;
        for (size_t i = 0; i < inputs.size(); i++)
            pattern.insert(make_pair(inputs[i], stoi(signals[i])));
        patterns.push_back(pattern);
    }
    pat.close();
}

void printPat(vector<unordered_map<string, int>> patterns)
{
    for (size_t i = 0; i < patterns.size(); i++) {
        for (auto j = patterns[i].begin(); j != patterns[i].end(); j++)
            cout << j->first << ":" << j->second << " ";
        cout << "\n";
    }
    cout << endl;
}

double linearPolation1D(double u,
                        double v,
                        double u_val,
                        double v_val,
                        double x)
{
    return (x - u) * abs(u_val - v_val) / abs(u - v) + u_val;
}

double linearPolation2D(double u1,
                        double u2,
                        double v1,
                        double v2,
                        double u1v1_val,
                        double u1v2_val,
                        double u2v1_val,
                        double u2v2_val,
                        double x,
                        double y)
{
    double tmp1 = linearPolation1D(u1, u2, u1v1_val, u2v1_val, x);
    double tmp2 = linearPolation1D(u1, u2, u1v2_val, u2v2_val, x);
    return linearPolation1D(v1, v2, tmp1, tmp2, y);
}

// get in-edges of an vertex
vector<edge> getInEdge(vector<vector<edge>> adj, int id)
{
    vector<edge> ret;
    for (int i = 0; i < (int) adj.size(); i++) {
        if (i == id)
            continue;
        for (unsigned int j = 0; j < adj[i].size(); j++) {
            if (adj[i][j].v == id)
                ret.push_back(adj[i][j]);
        }
    }
    return ret;
}

// update weight of the edge
void updateEdgeWeight(vector<vector<edge>> &adj, int u, int v, double new_weght)
{
    for (unsigned int i = 0; i < adj[u].size(); i++) {
        if (adj[u][i].v == v) {
            adj[u][i].delay = new_weght;
            break;
        }
    }
}

void getIdxAndVal(int &x,
                  int &y,
                  double &x_val,
                  double &y_val,
                  vector<double> table_row,
                  double target)
{
    if (target < table_row.front()) {
        x = 0;
        y = 1;
    } else if (target >= table_row.back()) {
        x = table_row.size() - 1;
        y = table_row.size() - 2;
    } else {
        for (unsigned int i = 0; i < table_row.size() - 1; i++) {
            if (target >= table_row[i] && target < table_row[i + 1]) {
                x = i;
                y = i + 1;
                break;
            }
        }
    }
    x_val = table_row[x];
    y_val = table_row[y];
}

vector<double> FindLongestPath(vector<vector<edge>> adj,
                               int s,
                               vector<int> &prev,
                               bool display,
                               ofstream &fout)
{
    vector<double> distance(adj.size());
    vector<int> pre_vertex(adj.size());
    vector<int> repeat_vertex;
    for (unsigned int i = 0; i < adj.size(); i++) {
        distance[i] = 100.0;
        pre_vertex[i] = -1;
    }

    typedef pair<double, int> iPair;
    priority_queue<iPair, vector<iPair>, greater<iPair>> pq;
    pq.push(make_pair(0, s));
    distance[s] = 0;

    while (!pq.empty()) {
        int u = pq.top().second;
        pq.pop();

        for (auto x : adj[u]) {
            int v = x.v;
            double weight = x.delay * (-1.0);
            if (weight != -1.0 && distance[v] > distance[u] + weight) {
                distance[v] = distance[u] + weight;
                pq.push(make_pair(distance[v], v));
                pre_vertex[v] = u;
            }
        }
    }

    // output
    if (display == true) {
        vector<string> path;
        edge prev_e = getEdge(adj, pre_vertex[adj.size() - 1], adj.size() - 1);
        path.push_back(prev_e.name);
        vector<double> repeat;
        for (int i = pre_vertex[adj.size() - 1]; i != 0; i = pre_vertex[i]) {
            edge e = getEdge(adj, pre_vertex[i], i);
            if (prev_e.name.compare(e.name) == 0) {
                repeat.push_back(e.delay);
                prev_e = e;
                continue;
            }
            path.push_back(e.name);
            prev_e = e;
        }
        for (unsigned int i = 1; i < repeat.size() - 1; i++)
            distance[adj.size() - 1] += repeat[i];
        fout << "Longest delay = ";
        fout << abs(distance[adj.size() - 1]) << ", the path is:\n";
        for (int i = path.size() - 1; i > 0; i--) {
            fout << path[i] << " -> ";
        }
        fout << path[0];
        fout << endl;
    }
    prev.clear();
    prev = pre_vertex;
    return distance;
}

// get the edge from u to v
edge getEdge(vector<vector<edge>> adj, int u, int v)
{
    edge ret;
    for (unsigned int i = 0; i < adj[u].size(); i++)
        if (adj[u][i].v == v) {
            ret = adj[u][i];
            break;
        }
    return ret;
}
