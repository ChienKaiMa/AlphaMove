#include <cassert>
#include "routeMgr.h"
#include "util.h"
#include <algorithm>
#include "routeRoute.h"


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
        for(auto it=pinSet.begin(); it != --pinSet.end(); it++ ){
            Pos pos1 = getPinPos(*it);
            Pos pos2 = getPinPos(*(++it));
            bool canRoute = route2Pin(pos1, pos2);
        }
    }



    cout << "Route..." << "(Not function-ready!)" << endl;
}


bool RouteMgr::route2Pin(Pos p1, Pos p2)
{
    AStarSearch<MapSearchNode> searchSolver;
    MapSearchNode s = MapSearchNode(p1.first, p1.second); // start node
    MapSearchNode t = MapSearchNode(p2.first, p2.second); // terminal node
    searchSolver.SetStartAndGoalStates(s, t);
}

Pos
RouteMgr::getPinPos(const PinPair pin) const
{
    return _instList[pin.first-1]->getPos();
}