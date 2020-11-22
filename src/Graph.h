#ifndef MY_GRAPH_H
#define MY_GRAPH_H

#include <map>
#include <string>
#include <vector>

#include "Types.h"
using namespace std;

typedef enum class VERTEX_TYPE {
    UNKNOWN,
    INPUT,
    WIRE,
    OUTPUT,
    GATE,
    S,
    E
} vertex_type;

typedef enum class VERTEX_ATTR {
    UNKNOWN,
    PROPAGATION_DELAY,
    RF_DELAY,
    CAPACITANCE
} vertex_attr;

typedef struct VERTEX {
    int id;
    string name;
    vertex_type type;
    string cell_type;
} vertex;

// edge from u to v
typedef struct EDGE {
    string name;
    int u, v;
    string u_name, v_name;
    string pin_name;
    bool back;
    double delay;
} edge;

class Graph
{
public:
    Graph(vector<vertex>);  // constructor with n vertexes
    ~Graph();
    int size();
    vertex start();
    vertex end();
    void addEdge(string, string, string, string);
    void setEdgeName(edge &, string);
    void deleteEdgeWithVertex(int);
    void deleteVertex(int);
    void addVertexAttr(int, double, vertex_attr);
    double getVertexAttr(int, vertex_attr);
    vector<vertex> getVertex();
    vector<vector<edge> > getAdjList();
    void compute_delay();
    void printADJ();
    void DFS(vertex);
    void DFSVisit(int, int &);
    void printDFS();
    void topologicalSort();
    void print();


private:
    int n;  // number of vertexes
    vector<edge> *adj = NULL;
    vector<vertex> *vertexes = NULL;
    vector<int> *color = NULL,  // 0:white, 1:gray, 2:black
        *prev = NULL, *discover = NULL, *finish = NULL;
    map<string, int> *v2id = NULL;
    map<int, string> *id2v = NULL;
    map<int, double> *id2ptime = NULL, *id2rftime = NULL, *id2cap = NULL;
};

#endif