#include <cassert>
#include <algorithm>
#include <math.h>
#include <iostream>
#include "routeDef.h"
#include "stlastar.h"
#include "routeMgr.h"
#include "util.h"

class MapSearchNode;
class CubeSearchNode;

extern RouteMgr* routeMgr;

class MapSearchNode
{
public:
	int x;	 // the (x,y) positions of the node
	int y;	 // x->row, y->col
	
	MapSearchNode() { x = y = 0; }
	MapSearchNode( int px, int py ) { x=px; y=py; }

    // Heuristic function that estimate the distance to Goal
	float GoalDistanceEstimate( MapSearchNode &nodeGoal )
    {
        return abs(x-nodeGoal.x) + abs(y-nodeGoal.y);
    }

	bool IsGoal( MapSearchNode &nodeGoal )
    {
        return (x==nodeGoal.x) && (y==nodeGoal.y);
    }

    double GetMap(int x, int y){
        if (x <= 0) return INT32_MAX;
        if (y <= 0) return INT32_MAX;
        return routeMgr->getCongestion(Pos(x, y));
    }


	bool GetSuccessors( AStarSearch<MapSearchNode> *astarsearch, unordered_map<MCTri, unsigned, TriHash>* parentNodes )
    {
        MapSearchNode newNode;

        // push each possible node 
        if( GetMap( x-1, y ) < -CONGEST_MIN ) 
	    {
            MCTri t (x-1, y, 0);
            auto it = parentNodes->find(t);
            if (it == parentNodes->end() || true) {
                pair<MCTri, unsigned> yeah (t, 0);
                parentNodes->insert(yeah);
                newNode = MapSearchNode( x-1, y );
		        astarsearch->AddSuccessor( newNode );
            }
	    }

        if( GetMap( x, y-1 ) < -CONGEST_MIN )
	    {
            MCTri t (x, y-1, 0);
            auto it = parentNodes->find(t);
            if (it == parentNodes->end() || true) {
                pair<MCTri, unsigned> yeah (t, 0);
                parentNodes->insert(yeah);
                newNode = MapSearchNode( x, y-1 );
		        astarsearch->AddSuccessor( newNode );
            }
	    }

        if( GetMap( x+1, y ) < -CONGEST_MIN ) 
	    {
            MCTri t (x+1, y, 0);
            auto it = parentNodes->find(t);
            if (it == parentNodes->end() || true) {
                pair<MCTri, unsigned> yeah (t, 0);
                parentNodes->insert(yeah);
                newNode = MapSearchNode( x+1, y );
		        astarsearch->AddSuccessor( newNode );
            }
	    }

        if( GetMap( x, y+1 ) < -CONGEST_MIN ) 
	    {
            MCTri t (x, y+1, 0);
            auto it = parentNodes->find(t);
            if (it == parentNodes->end() || true) {
                pair<MCTri, unsigned> yeah (t, 0);
                parentNodes->insert(yeah);
                newNode = MapSearchNode( x, y+1 );
		        astarsearch->AddSuccessor( newNode );
            }
	    }
        return true;
    }

	double GetCost( MapSearchNode &successor )
    {
        return routeMgr->getCongestion(Pos(x, y));
    }

	bool IsSameState( MapSearchNode &rhs ) // to be improved
    {
        return (x==rhs.x) && (y==rhs.y);
    }

	void PrintNodeInfo()
    {
        std::cout << "Node position : " << x << " " << y << std::endl ; 
    } 

};

class CubeSearchNode
{
public:
	int x;	 // the (x,y) positions of the node
	int y;	 // x->row, y->col
    int z;
	
	CubeSearchNode() { x = y = z = 0; }
	CubeSearchNode( int px, int py, int pz ) { x=px; y=py; z=pz; }

    // Heuristic function that estimate the distance to Goal
	float GoalDistanceEstimate( CubeSearchNode &nodeGoal )
    {
        return abs(x-nodeGoal.x) + abs(y-nodeGoal.y) + abs(z-nodeGoal.z);
    }

	bool IsGoal( CubeSearchNode &nodeGoal )
    {
        return (x==nodeGoal.x) && (y==nodeGoal.y) && (z==nodeGoal.z);
    }

    double GetMap(int x, int y, int z){
        if (x <= 0 || x > Ggrid::rEnd) return INT32_MAX;
        if (y <= 0 || x > Ggrid::cEnd) return INT32_MAX;
        if (z <= 0 || z > routeMgr->getLayerCnt()) return INT32_MAX;
        return (*(routeMgr->getGrid(Pos(x, y))))[z]->getCapacity();
    }


	bool GetSuccessors( AStarSearch<CubeSearchNode> *astarsearch, unordered_map<MCTri, unsigned, TriHash>* parentNodes )
    {
        CubeSearchNode newNode;

        // push each possible node 
        if( (GetMap( x-1, y, z ) < -CONGEST_MIN) && z%2==0) 
	    {
            MCTri t (x-1, y, z);
            auto it = parentNodes->find(t);
            if (it == parentNodes->end() || true) {
                pair<MCTri, unsigned> yeah (t, 0);
                parentNodes->insert(yeah);
                newNode = CubeSearchNode( x-1, y, z );
		        astarsearch->AddSuccessor( newNode );
            }
	    }

        if( (GetMap( x, y-1, z ) < -CONGEST_MIN) && z%2==1) 
	    {
            MCTri t (x, y-1, z);
            auto it = parentNodes->find(t);
            if (it == parentNodes->end() || true) {
                pair<MCTri, unsigned> yeah (t, 0);
                parentNodes->insert(yeah);
                newNode = CubeSearchNode( x, y-1, z );
		        astarsearch->AddSuccessor( newNode );
            }
	    }

        if( (GetMap( x+1, y, z ) < -CONGEST_MIN) && z%2==0) 
	    {
            MCTri t (x+1, y, z);
            auto it = parentNodes->find(t);
            if (it == parentNodes->end() || true) {
                pair<MCTri, unsigned> yeah (t, 0);
                parentNodes->insert(yeah);
                newNode = CubeSearchNode( x+1, y, z );
		        astarsearch->AddSuccessor( newNode );
            }
	    }

        if( (GetMap( x, y+1, z ) < -CONGEST_MIN) && z%2==1) 
	    {
            MCTri t (x, y+1, z);
            auto it = parentNodes->find(t);
            if (it == parentNodes->end() || true) {
                pair<MCTri, unsigned> yeah (t, 0);
                parentNodes->insert(yeah);
                newNode = CubeSearchNode( x, y+1, z );
		        astarsearch->AddSuccessor( newNode );
            }
	    }

        if( (GetMap( x, y, z-1 ) < -CONGEST_MIN)) 
	    {
            MCTri t (x, y, z-1);
            auto it = parentNodes->find(t);
            if (it == parentNodes->end() || true) {
                pair<MCTri, unsigned> yeah (t, 0);
                parentNodes->insert(yeah);
                newNode = CubeSearchNode( x, y, z-1 );
		        astarsearch->AddSuccessor( newNode );
            }
	    }

        if( (GetMap( x, y, z+1 ) < -CONGEST_MIN)) 
	    {
            MCTri t (x, y, z+1);
            auto it = parentNodes->find(t);
            if (it == parentNodes->end() || true) {
                pair<MCTri, unsigned> yeah (t, 0);
                parentNodes->insert(yeah);
                newNode = CubeSearchNode( x, y, z+1 );
		        astarsearch->AddSuccessor( newNode );
            }
	    }

        return true;
    }

	double GetCost( CubeSearchNode &successor )
    {
        if (x <= 0 || x > Ggrid::rEnd) return INT32_MAX;
        if (y <= 0 || x > Ggrid::cEnd) return INT32_MAX;
        if (z <= 0 || z > routeMgr->getLayerCnt()) return INT32_MAX;
        return (*(routeMgr->getGrid(Pos(x, y))))[z]->getCapacity();
    }

	bool IsSameState( CubeSearchNode &rhs ) // to be improved
    {
        return (x==rhs.x) && (y==rhs.y) && (z==rhs.z);
    }

	void PrintNodeInfo()
    {
        std::cout << "Node position : " << x << " " << y << " " << z << std::endl ; 
    } 

};