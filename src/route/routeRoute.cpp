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

bool
RouteMgr::route()
{
    cout << "Route..." << endl;
    NetList targetNet = NetList();
    for (auto m : _netList){
        if (m->shouldReroute()){
            targetNet.push_back(m);
            m->ripUp();
            // cout << m->_netSegs.size() << " " << m->_netSegs.capacity() << endl;
        }
    }
    // sorted by #pins
    sort( targetNet.begin(), targetNet.end(), netCompare);

    route2D(targetNet);
    cout << endl;
    return layerassign(targetNet);
}

void RouteMgr::route2D(NetList& toRouteNet)
{
    // 1.   for each to-be routed net , sorted by #Pins
    //      route the largest Net first with Bounds(initially bounding box)
    // 2.   performing A*search based on congestion + manhattan distance(s,t) (routability&routing length)
    //      dynamically update the congestion 
    // 3.   if can't route, rip-up the pre-exist segments , enlarge the bound, and goto 2.
    // 4.   iteratively untill all net is routed in 2D Grid graph.      
    cout << "2D-Routing..." << endl;
    
    for (auto n : toRouteNet){
        auto pinSet = n->_pinSet;
        cout << "Routing N" << n->_netId << endl;
        unsigned availale_layer = _laySupply.size() - n->getMinLayCons() + 1;
        double demand = ((double)_laySupply.size() / (double)availale_layer);
        for(auto it=pinSet.begin(); it != --pinSet.end();){
            Pos pos1 = getPinPos(*it);
            Pos pos2 = getPinPos(*(++it));
            unsigned lay1 = getPinLay(*(--it));
            unsigned lay2 = getPinLay(*(++it));
            if (!route2Pin(pos1, pos2, n, demand, lay1, lay2)) {
                cout << "route2Pin("
                << pos1.first << " " << pos1.second << ", " 
                << pos2.first << " " << pos2.second
                << " ) failed!" << endl;
            }
        }
        n->shouldReroute(false);
    }
}


bool RouteMgr::route2Pin(Pos p1, Pos p2, Net* net, double demand, unsigned lay1, unsigned lay2)
{
    AStarSearch<MapSearchNode> searchSolver;
    MapSearchNode s = MapSearchNode(p1.first, p1.second); // start node
    MapSearchNode t = MapSearchNode(p2.first, p2.second); // terminal node
    searchSolver.SetStartAndGoalStates(s, t);
    cout << "route2Pin from : " << p1.first << " " << p1.second << ", to "
                                << p2.first << " " << p2.second << "." << endl;
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
        // cout << "Search found goal state\n";
        MapSearchNode* goal = searchSolver.GetSolutionEnd();
        MapSearchNode* node =  searchSolver.GetSolutionStart(); 
        MapSearchNode* next = searchSolver.GetSolutionNext();
        bool dir = 0;
        if(next){
            dir = (node->x - next->x)==0; // 1:col 0:row
        }else{
            Segment* news = new Segment(node->x, node->y, lay1,
                                        node->x, node->y, lay2);
            cout << "New Segment!! : " << node->x << " " << node->y << " " << lay1 << ", "
                                       << node->x << " " << node->y << " " << lay2 << endl; 
            net->addSeg(news);
        }
        Pos segStart = Pos(node->x, node->y);
        (routeMgr->_gridList[node->x-1][node->y-1])->update2dDemand(1);

        int steps = 0;
        int dirCnt = 0;
        while(next){
            if( dir != ((node->x-next->x)==0) ) { // changing direction
                Segment* news = new Segment(segStart.first, segStart.second, dirCnt==0 ? lay1 : 0,
                                            node->x       , node->y        , 0);
                cout << "New Segment!! : " << segStart.first << " " << segStart.second << " " << ( dirCnt==0 ? lay1 : 0 ) << " , "
                                           << node->x        << " " << node->y         << " " << 0 << endl; 
                net->addSeg(news);
                segStart.first = node->x;
                segStart.second = node->y;
                dir = (node->x-next->x)==0 ;
                dirCnt++;
            } 
            if( next==goal ){
                Segment* news = new Segment(segStart.first, segStart.second, dirCnt==0 ? lay1 : 0,
                                            next->x       , next->y        , lay2 );
                cout << "New Segment!! : " << segStart.first << " " << segStart.second << " " << (dirCnt==0 ? lay1 : 0) << " , "
                                           << next->x        << " " << next->y         << " " << lay2 << endl; 
                net->addSeg(news);                           
            }
            (routeMgr->_gridList[next->x-1][next->y-1])->update2dDemand(demand);
            // node->PrintNodeInfo();
            // next->PrintNodeInfo();
            node = next;
            next = searchSolver.GetSolutionNext();
            steps++;
        }
        // cout << "Solution steps " << steps << endl;
        searchSolver.FreeSolutionNodes();
    }
    else if(searchState == AStarSearch<MapSearchNode>::SEARCH_STATE_FAILED){
        cout << "Search terminated. Failed to find goal state" << endl;
    }
    else{
        cout << "Unexpected search states!!" << endl;
    }

    // cout << "SearchSteps : " << searchSteps << endl;

    searchSolver.EnsureMemoryFreed();
    return true;
}

Pos
RouteMgr::getPinPos(const PinPair pin) const
{
    return _instList[pin.first-1]->getPos();
}

unsigned
RouteMgr::getPinLay(const PinPair pin) const
{
    CellInst* cell = _instList[pin.first-1];
    return cell->getPinLay(pin.second);
}