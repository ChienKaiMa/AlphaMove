#include <cassert>
#include "routeMgr.h"
#include "util.h"
#include <algorithm>
#include "stlastar.h"

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

    }
	bool IsGoal( MapSearchNode &nodeGoal )
    {

    }
	bool GetSuccessors( AStarSearch<MapSearchNode> *astarsearch, MapSearchNode *parent_node )
    {

    }
	float GetCost( MapSearchNode &successor )
    {

    }
	bool IsSameState( MapSearchNode &rhs )
    {

    }

	void PrintNodeInfo()
    {

    } 

};