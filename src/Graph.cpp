#include "Graph.h"

#include <assert.h>

#include <algorithm>
#include <iostream>

#define V2ID(vertex) (*(this->v2id))[vertex]
#define ID2V(vertex) (*(this->id2v))[vertex]
#define ID2PTIME(vertex) (*(this->id2ptime))[vertex]
#define ID2RFTIME(vertex) (*(this->id2rftime))[vertex]
#define ID2CAP(vertex) (*(this->id2cap))[vertex]
#define ADJ(id) (this->adj)[id]
#define VERTEX(id) (*(this->vertexes))[id]
#define COLOR(id) (*(this->color))[id]
#define PREV(id) (*(this->prev))[id]
#define DISCOVER(id) (*(this->discover))[id]
#define FINISH(id) (*(this->finish))[id]

Graph::Graph(vector<vertex> vertexes)
{
    this->n = vertexes.size();
    this->vertexes = new vector<vertex>(this->n);
    this->adj = new vector<edge>[this->n];
    this->v2id = new map<string, int>;
    this->id2v = new map<int, string>;
    this->id2ptime = new map<int, double>;
    this->id2rftime = new map<int, double>;
    this->id2cap = new map<int, double>;
    for (int i = 0; i < n; i++) {
        VERTEX(i).id = vertexes[i].id;
        VERTEX(i).name = vertexes[i].name;
        VERTEX(i).type = vertexes[i].type;
        VERTEX(i).cell_type = vertexes[i].cell_type;
        this->v2id->insert(pair<string, int>(vertexes[i].name, i));
        this->id2v->insert(pair<int, string>(i, vertexes[i].name));
    }
}

Graph::~Graph()
{
    delete this->vertexes;
    delete[] this->adj;
    delete this->v2id;
    delete this->id2v;
    delete this->id2ptime;
    delete this->id2rftime;
    delete this->id2cap;
    if (this->color != NULL)
        delete this->color;
    if (this->prev != NULL)
        delete this->prev;
    if (this->discover != NULL)
        delete this->discover;
    if (this->finish != NULL)
        delete this->finish;
}

int Graph::size()
{
    return this->n;
}

vertex Graph::start()
{
    return VERTEX(0);
}

vertex Graph::end()
{
    return VERTEX(this->n - 1);
}

void Graph::addEdge(string u, string v, string name, string pin_name)
{
    int u_id = V2ID(u);
    int v_id = V2ID(v);
    assert(u_id >= 0 && v_id >= 0);
    edge e = {.name = name,
              .u = u_id,
              .v = v_id,
              .u_name = u,
              .v_name = v,
              .pin_name = pin_name,
              .back = false,
              .delay = 0.0};
    ADJ(u_id).push_back(e);
}

void Graph::setEdgeName(edge &e, string name)
{
    e.name = name;
}

// delete wire vertex and add it to edge's name after xth index(include x)
void Graph::deleteEdgeWithVertex(int del_id)
{
    for (auto i = 0; i < this->n; i++) {
        if (i == del_id)
            continue;
        for (auto e = ADJ(i).begin(); e != ADJ(i).end(); e++) {
            if (e->v == del_id) {
                ADJ(i).erase(e);
                e--;
                for (auto j = ADJ(del_id).begin(); j != ADJ(del_id).end();
                     j++) {
                    addEdge(ID2V(i), ID2V(j->v), ID2V(del_id), j->pin_name);
                }
                break;
            }
        }
    }
    ADJ(del_id).erase(ADJ(del_id).begin(), ADJ(del_id).end());
}

void Graph::deleteVertex(int del_id)
{
    deleteEdgeWithVertex(del_id);
    for (auto i = this->vertexes->begin(); i != this->vertexes->end(); i++) {
        if (i->name.compare(ID2V(del_id)) == 0) {
            this->vertexes->erase(i);
            break;
        }
    }
    this->n--;
}

void Graph::addVertexAttr(int id, double val, vertex_attr info)
{
    if (id >= this->n) {
        cerr << "[Graph] Warning: " << id
             << " is out of the size of vertexes, skip the operation" << endl;
        return;
    }

    switch (info) {
    case vertex_attr::CAPACITANCE:
        this->id2cap->insert(pair<int, double>(id, val));
        break;
    case vertex_attr::PROPAGATION_DELAY:
        this->id2ptime->insert(pair<int, double>(id, val));
        break;
    case vertex_attr::RF_DELAY:
        this->id2rftime->insert(pair<int, double>(id, val));
        break;
    default:
        cerr << "[Graph] Warning: UNKNOWN vertex_info is detected, skip the "
                "operation"
             << endl;
        break;
    }
}

double Graph::getVertexAttr(int id, vertex_attr info)
{
    if (id >= this->n) {
        cerr << "[Graph] Warning: " << id
             << " is out of the size of vertexes, return -1.0" << endl;
        return -1.0;
    }

    switch (info) {
    case vertex_attr::CAPACITANCE:
        return ID2CAP(id);
    case vertex_attr::PROPAGATION_DELAY:
        return ID2PTIME(id);
    case vertex_attr::RF_DELAY:
        return ID2RFTIME(id);
    default:
        cerr << "[Graph] Warning: UNKNOWN vertex_info is detected, return -1.0"
             << endl;
        return -1.0;
    }
}

vector<vertex> Graph::getVertex()
{
    return *(this->vertexes);
}
vector<vector<edge> > Graph::getAdjList()
{
    vector<vector<edge> > ret_adj;
    for (auto i = 0; i < this->n; i++)
        ret_adj.push_back(this->adj[i]);
    return ret_adj;
}

void Graph::printADJ()
{
    cout << "Adjacency list:\n\t";
    for (auto i = 0; i < this->n; i++) {
        cout << ID2V(i) << ": ";
        for (auto e = ADJ(i).begin(); e != ADJ(i).end(); e++)
            cout << e->v_name << "(" << e->name << ") ";
        cout << "\n\t";
    }
    cout << endl;
}

void Graph::DFS(vertex s)
{
    this->color = new vector<int>(this->n);
    this->prev = new vector<int>(this->n);
    this->discover = new vector<int>(this->n);
    this->finish = new vector<int>(this->n);
    int time = 0;
    int s_id = s.id;
    for (auto i = 0; i < this->n; i++) {
        COLOR(i) = DISCOVER(i) = FINISH(i) = 0;
        PREV(i) = -1;
    }
    int i = s_id;
    for (auto j = 0; j < this->n; j++) {
        if (COLOR(i) == 0)
            DFSVisit(i, time);
        i = j;
    }
}

void Graph::DFSVisit(int u, int &time)
{
    COLOR(u) = 1;
    DISCOVER(u) = ++time;
    for (auto i = ADJ(u).begin(); i != ADJ(u).end(); i++) {
        if (COLOR((*i).v) == 0) {
            PREV((*i).v) = u;
            DFSVisit((*i).v, time);
        }
    }
    COLOR(u) = 2;
    FINISH(u) = ++time;
}

void Graph::printDFS()
{
    cout << "DFS Result:" << endl;
    assert(this->color && this->prev && this->discover && this->finish);
    cout << "\tprev\n\t";
    for (auto i = 0; i != this->n; i++)
        cout << PREV(i) << " ";
    cout << endl;
    cout << "\tdiscover\n\t";
    for (auto i = 0; i != this->n; i++)
        cout << DISCOVER(i) << " ";
    cout << endl;
    cout << "\tfinish\n\t";
    for (auto i = 0; i != this->n; i++)
        cout << FINISH(i) << " ";
    cout << endl;
}

void Graph::topologicalSort()
{
    DFS(VERTEX(V2ID("START")));
    assert(this->color && this->prev && this->discover && this->finish);
    sort(this->vertexes->begin(), this->vertexes->end(),
         [&](vertex a, vertex b) -> bool {
             return FINISH(a.id) > FINISH(b.id);
         });
}

void Graph::print()
{
    cout << "Graph vertex:\nid:\t";
    for (auto i = 0; i < this->n; i++)
        cout << VERTEX(i).id << " ";
    cout << endl << "name:\t";
    for (auto i = 0; i < this->n; i++)
        cout << VERTEX(i).name << " ";
    cout << "\n" << endl;
}