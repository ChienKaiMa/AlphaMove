#include <cassert>
#include <algorithm>
#include <math.h>
#include <iostream>
#include "stlastar.h"
#include "routeMgr.h"
#include "util.h"

class MapSearchNode;

extern RouteMgr* routeMgr;

class MapSearchNode
{
public:
	int x;	 // the (x,y) positions of the node
	int y;	
	
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
        return routeMgr->getCongestion(Pos(x, y));
    }


	bool GetSuccessors( AStarSearch<MapSearchNode> *astarsearch, MapSearchNode *parent_node )
    {
        int parentX = -1;
        int parentY = -1;
        if(parent_node){
            parentX = parent_node->x;
            parentY = parent_node->y;
        }
        else {
            //return false;
        }

        MapSearchNode newNode;

        // push each possible node 
        if( (GetMap( x-1, y ) < -CONGEST_MIN) && 
            !((parentX == x-1) && (parentY == y)) ) 
	    {
		    newNode = MapSearchNode( x-1, y );
		    astarsearch->AddSuccessor( newNode );
	    }

        if( (GetMap( x, y-1 ) < -CONGEST_MIN) && 
            !((parentX == x) && (parentY == y-1)) ) 
	    {
		    newNode = MapSearchNode( x, y-1 );
		    astarsearch->AddSuccessor( newNode );
	    }

        if( (GetMap( x+1, y ) < -CONGEST_MIN) && 
            !((parentX == x+1) && (parentY == y)) ) 
	    {
		    newNode = MapSearchNode( x+1, y );
		    astarsearch->AddSuccessor( newNode );
	    }

        if( (GetMap( x, y+1 ) < -CONGEST_MIN) && 
            !((parentX == x) && (parentY == y+1)) ) 
	    {
		    newNode = MapSearchNode( x, y+1 );
		    astarsearch->AddSuccessor( newNode );
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