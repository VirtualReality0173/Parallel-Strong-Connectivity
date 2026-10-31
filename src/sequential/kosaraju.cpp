#include <iostream>
#include <stack>
#include <vector>
using namespace std;

void dfs1(int node, vector<vector<int>>&adj, vector<bool>&visited,stack<int>&st){
    visited[node]=true;
    for(int neighbour:adj[node]){
        if(!visited[neighbour]){
            dfs1(neighbour,adj,visited,st);
        }
    }
    st.push(node);
}


void dfs2 (int node, vector<vector<int>>&transpose,vector<bool>&visited){
    visited[node]=true;
    cout<<node<<" ";
    for(int neighbour : transpose[node]){
        if(!visited[neighbour]){
            dfs2(neighbour,transpose,visited);
        }
    }

}


void kosaraju(int V,vector<vector<int>>&adj){
    vector<bool> visited(V,false);
    stack<int> st;
    for(int i=0;i<V;i++){
        if(!visited[i]){
            dfs1(i,adj,visited,st);
        }
    }

    vector<vector<int>> transpose(V);

    for(int i=0;i<V;i++){
        for(int neighbour : adj[i]){
            transpose[neighbour].push_back(i);
        }
    }

    fill(visited.begin(),visited.end(),false);
    cout << "Strongly Connected Components: \n";

    while(!st.empty()){
        int node = st.top();
        st.pop();

        if(!visited[node]){
            dfs2(node,transpose,visited);
            cout<<endl;
        }
    }

}

int main(){
    int V,E;
    cout << "Enter Number of Vertices and Edges: ";
    cin >>V >>E;
    vector<vector<int>> adj(V);
    cout << "Enter directed edges: \n";
    for(int i=0;i<E;i++){
        int u,v;
        cin>>u>>v;
        adj[u].push_back(v);

    }
    kosaraju(V,adj);
    return 0;
}