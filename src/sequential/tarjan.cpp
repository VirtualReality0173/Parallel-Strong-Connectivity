/* #include<iostream>
#include<vector>
#include<stack>
using namespace std;

class TarjanSCC{
public:
    int V,timer = 0,sccCount = 0;
    vector<vector<int>> adj;
    vector<int> disc,low,comp;
    vector<bool> onStack;
    stack<int> st;
    vector<vector<int>> groups;

    TarjanSCC(int V) : 
        V(V), adj(V), disc(V,-1), low(V,0), comp(V,-1), onStack(V,false) {}

    void addEdge(int u, int v){adj[u].push_back(v);}

    void dfs(int node){
        disc[node] = low[node] = timer++;
        st.push(node);
        onStack[node] = true;

        for(auto v : adj[node]){
            if(disc[v]==-1){
                dfs(v);
                low[node] = min(low[node],low[v]);
            }else if(onStack[v]){
                low[node] = min(low[node],disc[v]);
            }
        }

        if(low[node]==disc[node]){
            groups.push_back({});
            while(true){
                int v = st.top();
                st.pop();
                onStack[v] = false;
                comp[v] = sccCount;
                groups.back().push_back(v);
                if(v==node) break;
            }
            sccCount++;
        }
    }
    void run(){
        for(int i = 0;i<V;i++){
            if(disc[i]==-1) dfs(i);
        }
    }
    void printSCC(){
        if(disc[0]==-1) run();
        cout << "Number of SCCs: " << sccCount << "\n";
        for (int i = 0; i < sccCount; i++) {
            cout << "SCC " << i << ":";
            for (int v : groups[i]) cout << " " << v;
            cout << "\n";
        }
    }
};


int main(){
    int V,E;
    cin >> V >> E;
    TarjanSCC tarjan{V};
    for(int i = 0;i<E;i++){
        int a,b;
        cin >> a >> b;
        tarjan.addEdge(a,b);
    }

    tarjan.printSCC();

    return 0;
}


*/



#include "scc/tarjan.hpp"
#include <algorithm>

TarjanSCC::TarjanSCC(const Graph& g)
    : graph(g),
      disc(g.V, -1),
      low(g.V, 0),
      onStack(g.V, false) {}

void TarjanSCC::dfs(int u) {
    disc[u] = low[u] = timer++;
    st.push_back(u);
    onStack[u] = true;

    for (int v : graph.adj[u]) {
        if (disc[v] == -1) {
            dfs(v);
            low[u] = std::min(low[u], low[v]);
        }
        else if (onStack[v]) {
            low[u] = std::min(low[u], disc[v]);
        }
    }

    if (low[u] == disc[u]) {
        std::vector<int> component;

        while (true) {
            int v = st.back();
            st.pop_back();
            onStack[v] = false;

            component.push_back(v);

            if (v == u) break;
        }

        groups.push_back(component);
    }
}

std::vector<std::vector<int>> TarjanSCC::run() {
    for (int u = 0; u < graph.V; u++) {
        if (disc[u] == -1) {
            dfs(u);
        }
    }

    return groups;
}
