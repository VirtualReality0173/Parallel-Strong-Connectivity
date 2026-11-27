#include<iostream>
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