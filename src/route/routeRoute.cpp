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
RouteExecStatus
RouteMgr::route()
{
    cout << "Route..." << endl;
    NetList targetNet = NetList();
    RouteExecStatus myStatus = ROUTE_EXEC_DONE;
    //for (auto m : _netList){
    for (auto nPair : _netRank->NetWLpairs){
        Net* m = _netList[nPair.first-1];
        if (m->shouldReroute()){
            targetNet.push_back(m);
            remove3DDemand(m);
            m->ripUp();
            // cout << m->_netSegs.size() << " " << m->_netSegs.capacity() << endl;
        }
    }
    for (auto n : targetNet) {
        if (route2D(n) == ROUTE_EXEC_ERROR) {
            n->_routable = false;
            myStatus = ROUTE_EXEC_ERROR;
        } else {
            if (layerassign(n) == ROUTE_EXEC_ERROR) {
                n->_routable = false;
                myStatus = ROUTE_EXEC_ERROR;
            }
            else n->_routable = true;
        }
    }
    // sorted by #pins
    //sort( targetNet.begin(), targetNet.end(), netCompare);

    //route2D(targetNet);
    cout << endl;
    if (checkOverflow() == ROUTE_EXEC_ERROR) { myStatus = ROUTE_EXEC_ERROR; }
    return myStatus;
    //return layerassign(targetNet);
}

RouteExecStatus RouteMgr::route2D(Net* n)
{
    // 1.   for each to-be routed net , sorted by #Pins
    //      route the largest Net first with Bounds(initially bounding box)
    // 2.   performing A*search based on congestion + manhattan distance(s,t) (routability&routing length)
    //      dynamically update the congestion 
    // 3.   if can't route, rip-up the pre-exist segments , enlarge the bound, and goto 2.
    // 4.   iteratively untill all net is routed in 2D Grid graph.      
    //cout << "\n2D-Routing...\n";
    
    //auto pinSet = n->_pinSet;
    //cout << "Routing N" << n->_netId << endl;
    unsigned availale_layer = _laySupply.size() - n->getMinLayCons() + 1;
    double demand = ((double)_laySupply.size() / (double)availale_layer);
    auto pinSet = n->sortPinSet();
    for(auto it=pinSet.begin(); it != --pinSet.end();)
    {
        Pos pos1 = getPinPos(*it);
        Pos pos2 = getPinPos(*(++it));
        unsigned lay1 = getPinLay(*(--it));
        unsigned lay2 = getPinLay(*(++it));
        if (!route2Pin(pos1, pos2, n, demand, lay1, lay2)) {
            cout << "route2Pin("
            << pos1.first << " " << pos1.second << ", " 
            << pos2.first << " " << pos2.second
            << " ) failed!" << endl;
            n->shouldReroute(true);
            return ROUTE_EXEC_ERROR;
        }
    }
    n->shouldReroute(false);
    return ROUTE_EXEC_DONE;
}
RouteExecStatus RouteMgr::reroute()
{
    cout << "\nRerouting...\n";
    RouteExecStatus myStatus = ROUTE_EXEC_DONE;
    _numOverflowNet1 = _numOverflowNet2 = _numOverflowNet3 = _numValidNet1 = _numValidNet2 = 0;
    _targetNetList.clear();
    _targetNetList.resize(0);
    for (unsigned i=0; i<_netList.size(); ++i)
    {
        if (reroute(_netList[i]) == ROUTE_EXEC_ERROR) {
            _netList[i]->_hasmovedbynb = true;
            for (auto cellPair : _netList[i]->_assoCellInstMap) {
                routeMgr->_instList[cellPair.first-1]->_hasmovedbyfd = true;
            }
        }
        if (i % 10000 == 0) {
            cout << i << "\n";
            replaceBest();
        }
    }
    cout << "\nReroute done!\n";
    replaceBest();
    _netRank->update();
    _netRank->showTopTen();
    routeMgr->printRouteSummary();
    myUsage.report(true, false);
    #ifndef DEBUG
    cout << "\nNet type:\n"
         << "Cannot route       : " << _numOverflowNet1 << "\n" 
         << "Cannot layerassign : " << _numOverflowNet2 << "\n"
         << "Overflow           : " << _numOverflowNet3 << "\n"
         << "Valid but longer   : " << _numValidNet1 << "\n"
         << "Valid and shorter  : " << _numValidNet2 << "\n\n";
    #endif
    return myStatus;
}

RouteExecStatus RouteMgr::reroute(Net* n)
{
    RouteExecStatus myStatus = ROUTE_EXEC_DONE;
    //cout << "Rerouting N" << n->_netId << "\n";
    vector<Segment> OOrigSegs;
    for (auto s : n->_netSegs) { OOrigSegs.push_back(*s); }
    unsigned origWL = n->getWirelength();
    remove3DDemand(n);
    n->ripUp();
    n->shouldReroute(false);
    if (route2D(n) == ROUTE_EXEC_ERROR) {
        ++_numOverflowNet1;
        n->shouldReroute(false);
        myStatus = ROUTE_EXEC_ERROR;
        n->ripUp();
        for (auto s : OOrigSegs) {
            Segment* seg = new Segment(s);
            n->_netSegs.push_back(seg);
        }
        add3DDemand(n);
        n->_routable = false;
        return myStatus;
    }
    if (layerassign(n) == ROUTE_EXEC_ERROR) {
        ++_numOverflowNet2;
        myStatus = ROUTE_EXEC_ERROR;
        remove3DDemand(n);
        n->ripUp();
        for (auto s : OOrigSegs) {
            Segment* seg = new Segment(s);
            n->_netSegs.push_back(seg);
        }
        add3DDemand(n);
        n->_routable = false;
        return myStatus;
    }
    if (n->checkOverflow())
    {
        ++_numOverflowNet3;
        myStatus = ROUTE_EXEC_ERROR;
        remove3DDemand(n);
        n->ripUp();
        for (auto s : OOrigSegs) {
            Segment* seg = new Segment(s);
            n->_netSegs.push_back(seg);
        }
        add3DDemand(n);
        n->_routable = false;
        return myStatus;
    }
    unsigned newWL = n->getWirelength();
    if (newWL > origWL)
    {
        ++_numValidNet1;
        //myStatus = ROUTE_EXEC_ERROR;
        remove3DDemand(n);
        n->ripUp();
        for (auto s : OOrigSegs) {
            Segment* seg = new Segment(s);
            n->_netSegs.push_back(seg);
        }
        add3DDemand(n);
        //cout << "Net N" << n->_netId << " has no wirelength reduction!\n";
        _targetNetList.push_back(n);
        return myStatus;
    }
    else {
        ++_numValidNet2;
        //cout << "Net N" << n->_netId << " Reduce wirelength by " << (origWL - newWL) << "\n";
    }
    return myStatus;
}

bool RouteMgr::route2Pin(Pos p1, Pos p2, Net* net, double demand, unsigned lay1, unsigned lay2)
{
    AStarSearch<MapSearchNode> searchSolver;
    MapSearchNode s = MapSearchNode(p1.first, p1.second); // start node
    MapSearchNode t = MapSearchNode(p2.first, p2.second); // terminal node
    searchSolver.SetStartAndGoalStates(s, t);
    #ifdef DEBUG
    cout << "route2Pin from : " << p1.first << " " << p1.second << ", to "
                                << p2.first << " " << p2.second << "." << endl;
    #endif
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
            #ifdef DEBUG
            cout << "New Segment!! : " << node->x << " " << node->y << " " << lay1 << ", "
                                       << node->x << " " << node->y << " " << lay2 << endl; 
            #endif
            net->addSeg(news);
        }
        Pos segStart = Pos(node->x, node->y);
        #ifdef DEBUG
        cout << "Net " << net->_netId << "\n";
        #endif
        (routeMgr->_gridList[node->x-1][node->y-1])->update2dDemand(demand);

        int steps = 0;
        int dirCnt = 0;
        while(next){
            if( dir != ((node->x-next->x)==0) ) { // changing direction
                Segment* news = new Segment(segStart.first, segStart.second, dirCnt==0 ? lay1 : 0,
                                            node->x       , node->y        , 0);
                #ifdef DEBUG
                cout << "New Segment!! : " << segStart.first << " " << segStart.second << " " << ( dirCnt==0 ? lay1 : 0 ) << " , "
                                           << node->x        << " " << node->y         << " " << 0 << endl; 
                #endif
                net->addSeg(news);
                segStart.first = node->x;
                segStart.second = node->y;
                dir = (node->x-next->x)==0 ;
                dirCnt++;
            } 
            if( next==goal ){
                Segment* news = new Segment(segStart.first, segStart.second, dirCnt==0 ? lay1 : 0,
                                            next->x       , next->y        , lay2 );
                #ifdef DEBUG
                cout << "New Segment!! : " << segStart.first << " " << segStart.second << " " << (dirCnt==0 ? lay1 : 0) << " , "
                                           << next->x        << " " << next->y         << " " << lay2 << endl; 
                #endif
                net->addSeg(news);                           
            }
            #ifdef DEBUG
            cout << "Net " << net->_netId << "\n";
            #endif
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
        searchSolver.EnsureMemoryFreed();
        return false;
    }
    else{
        cout << "Unexpected search states!!" << endl;
        searchSolver.EnsureMemoryFreed();
        return false;
    }

    //cout << "SearchSteps : " << searchSteps << endl;

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