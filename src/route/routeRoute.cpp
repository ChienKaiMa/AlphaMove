#include <cassert>
#include <algorithm>
#include "routeRoute.h"
#include "routeMgr.h"
#include "util.h"

// #define DEBUG

using namespace std;

// TODO: Please keep "RouteMgr::optimize()" for route cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/
extern RouteMgr* routeMgr;

/**************************************/
/*   Static variables and functions   */
/**************************************/

bool netCompare(Net* n1, Net* n2) // greater than , decsending order
{
    return (*n1) > (*n2);
}

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/


void RouteMgr::route()
{
    // 1.   for each to-be routed net , sorted by #Pins
    //      route the largest Net first with Bounds(initially bounding box)
    // 2.   performing A*search based on congestion + manhattan distance(s,t) (routability&routing length)
    //      dynamically update the congestion 
    // 3.   if can't route, rip-up the pre-exist segments , enlarge the bound, and goto 2.
    // 4.   iteratively untill all net is routed in 2D Grid graph.      
    NetList toRouteNet = NetList();
    for (auto m : _netList){
        if (m->shouldReroute())
            toRouteNet.push_back(m);
    }
    // sorted by #pins
    sort( toRouteNet.begin(), toRouteNet.end(), netCompare);

    for (auto n : toRouteNet){
        auto pinSet = n->_pinSet;
        cout << "Currently routing Net : " << n->_netId << endl;
        for(auto it=pinSet.begin(); it != --pinSet.end();){
            Pos pos1 = getPinPos(*it);
            Pos pos2 = getPinPos(*(++it));
            if (!route2Pin(pos1, pos2)) {
                cout << "route2Pin("
                << pos1.first << " " << pos1.second << ", " 
                << pos2.first << " " << pos2.second
                << " ) failed!" << endl;
            }
        }
    }



    // cout << "Route..." << "(Not function-ready!)" << endl;
}


bool RouteMgr::route2Pin(Pos p1, Pos p2)
{
    AStarSearch<MapSearchNode> searchSolver;
    MapSearchNode s = MapSearchNode(p1.first, p1.second); // start node
    MapSearchNode t = MapSearchNode(p2.first, p2.second); // terminal node
    searchSolver.SetStartAndGoalStates(s, t);

    unsigned searchState;
    unsigned searchSteps = 0;
    do{
        searchState = searchSolver.SearchStep();
        searchSteps++;
        #ifdef DEBUG
            cout << "Step: " << searchSteps << endl;
            int len = 0;
            // open lists
            cout << "Open:\n";
            MapSearchNode* p = searchSolver.GetOpenListStart();
            while(p){
                len++;
                p->PrintNodeInfo();
                p = searchSolver.GetOpenListNext();
            }
            cout << "Open list has " << len <<  " nodes\n";
            len = 0;

            // closed list
            cout << "Closed:\n";
            p = searchSolver.GetClosedListStart();
            while(p){
                len++;
                p->PrintNodeInfo();
                p = searchSolver.GetClosedListNext();
            }
            cout << "Closed list has " << len << " nodes\n";
        #endif
    }
    while(searchState == AStarSearch<MapSearchNode>::SEARCH_STATE_SEARCHING);

    if(searchState == AStarSearch<MapSearchNode>::SEARCH_STATE_SUCCEEDED){
        cout << "Search found goal state\n";
        MapSearchNode* node =  searchSolver.GetSolutionStart();
        cout << "Displaying solution\n";
        int steps = 0;
        while(node){
            node->PrintNodeInfo();
            node = searchSolver.GetSolutionNext();
            steps++;
        }
        cout << "Solution steps " << steps << endl;
        searchSolver.FreeSolutionNodes();
    }
    else if(searchState == AStarSearch<MapSearchNode>::SEARCH_STATE_FAILED){
        cout << "Search terminated. Failed to find goal state" << endl;
    }
    else{
        cout << "Unexpected search states!!" << endl;
    }

    cout << "SearchSteps : " << searchSteps << endl;

    searchSolver.EnsureMemoryFreed();
    return true;
}

Pos
RouteMgr::getPinPos(const PinPair pin) const
{
    return _instList[pin.first-1]->getPos();
}