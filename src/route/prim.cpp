/*Layman806-zz/MST-Prim/prim.cpp*/

/*
 * This code is bugfree.
 * Started off with pseudocode from C, L, R, S 's book, but later changed
 * implementation without deviating from the original algorithm.
 * This is my cleanest attempt on Prim's algorithm yet.
 */

#include <iostream>
#include <vector>
#include <queue>
#include <utility>
#include <functional>
#include <unordered_set>   //required for queue
#define p pair < int, int >
#define pa pair < int, p >
#define infinite 32767
#define nil 32767

using namespace std;
bool debug;

unordered_set <int> Q; 
int N, E;
vector<int> pie, key, V; //In CLRS, pie depicts 'parent of'

/*
 * minE is a max priority_queue holding pairs of vertices and their costs
 * in format:
 *   pair< cost, pair <v1, v2> >
 */
priority_queue < pa, vector<pa>, greater<pa> > minE;

/*
 * Graph and costs/weights of edges are held in 2D matrices
 */
vector < vector <int> > w, G;

void init(){
	w.resize(N);
	G.resize(N);
	for(int i=0; i<N; i++){
		V.push_back(i);
		w[i].resize(N);
	}
	pie.resize(N);
	key.resize(N);
}

void addToQueue(int x){
	for(int y : G[x]){
		minE.push(pa(w[x][y], p(x, y))); //pushes (edge-cost connecting to MST, vertex pair)
		if(debug == true){
			cout<<"\t added "<<y+1<<" from"<<x+1;
		}
	}
}

/*
 * The MST-PRIM(r) funtion finds the MST
 */
void MST_PRIM(int r=0){
	for(auto u : V){   //for each vertex in G
		key[u]=infinite;
		pie[u]=nil;
	}
	
	key[r]=0; addToQueue(r);
	
	while(minE.empty()!=true){
		
		if(debug==true) cout<<"\tInside loop\n";
		
		int u=minE.top().second.first;
		int v=minE.top().second.second;
		int c=minE.top().first;
		minE.pop();
		
		if(c<key[v]){
			pie[v]=u;
			key[v]=c;
			addToQueue(v);
		}
	}
}


int main(){
	debug=false;
	cout<<"Prim's MST algo.\n Enter number of Vertices and Edges: " ;
	cin>>N>>E;
	cout<<"\nEnter all edges in the graph in format - v1 v2 edge-cost : \n [starting from 1, the root]";
	int tmp1, tmp2, tmp3;
	init();
	for(int i=0; i<E; i++){
		cin>>tmp1>>tmp2>>tmp3;
		G[tmp1-1].push_back(tmp2-1);
		G[tmp2-1].push_back(tmp1-1);
		w[tmp1-1][tmp2-1]=tmp3;
		w[tmp2-1][tmp1-1]=tmp3;
	}
	int cost=0;
	pie[0]=0;
	MST_PRIM();
	cout<<" ::MST::\nRoot is : 1";
	for(int i=1; i<N; i++){
		cout<<"( "<<i+1<<", "<<pie[i]+1<<" ) : "<<key[i]<<"\n";
		cost+=key[i];
	}
	cout<<"Minimum Cost = "<<cost;
	return 0;
}