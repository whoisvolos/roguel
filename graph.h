//
// Created by whoisvolos on 12/08/15.
//

#ifndef PLAYGROUND_GRAPH_H
#define PLAYGROUND_GRAPH_H

#include <map>
#include <vector>
#include <set>

class Graph {
private:
    std::map<int, std::map<int, int>> adjacency;
    std::set<int> vertices;

public:
    inline int size() {
        return vertices.size();
    }

    bool add_adge(int v, int w, int l) {
        auto it = adjacency.insert(std::pair<int, std::map<int, int>>(v, {{ w, l }}));
        bool result = false;
        if (it.second) {
            result = true;
        } else {
            result = (it.first->second.insert(std::pair<int, int>(w, l))).second;
        }
        vertices.insert(v);
        vertices.insert(w);
        return result;
    }

    bool connected() {
        // DFS
        std::set<int> visited;
        std::vector<int> stack;

        for (auto i : adjacency) {
            // Push 1st vertex to visited
            visited.insert(i.first);
            // Push all successors of 1st vertex to stack
            for (auto j : i.second) {
                stack.push_back(j.first);
            }
            break;
        }

        while(!stack.empty()) {
            auto successor = stack.back();
            stack.pop_back();
            if (visited.find(successor) != visited.end()) {
                // Already in visited set
                continue;
            } else {
                visited.insert(successor);
                std::map<int, std::map<int, int>>::iterator adj = adjacency.find(successor);
                if (adj != adjacency.end()) {
                    // Leaf elem
                    for (auto adj_adj : adj->second) {
                        if (visited.find(adj_adj.first) == visited.end()) {
                            stack.push_back(adj_adj.first);
                        }
                    }
                }
            }
        }

        return visited.size() == vertices.size();
    }
};

/*
template <class T>
struct Vertex {
    T value;
    std::map<Vertex<T>*, int> adjacency;
};

template <class T>
struct Graph {
    std::vector<Vertex<T>*> vertices;

    bool is_connected() {
        if (vertices.empty()) {
            return false;
        }

        std::set<Vertex<T>*> visited;
        auto u = vertices.at(0);
        visited.insert(u);
        std::vector<std::pair<Vertex<T>*, Vertex<T>*>> stack;
        for (auto w : u->adjacency) {
            stack.push_back(std::pair<Vertex<T>*, Vertex<T>*>(u, w.first));
        }
        while (!stack.empty()) {
            auto adj = stack.back();
            stack.pop_back();
            if (visited.find(adj.second) != visited.end()) {
                continue;
            }
            for (auto w : adj.second->adjacency) {
                if (visited.find(w.first) != visited.end()) {
                    continue;
                }
                visited.insert(w.first);
                stack.push_back(std::pair<Vertex<T>*, Vertex<T>*>(adj.second, w.first));
            }
        }

        return visited.size() == vertices.size();
    }
};
*/

#endif //PLAYGROUND_GRAPH_H
