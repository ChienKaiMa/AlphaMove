/****************************************************************************
  FileName     [ routeMgr.h ]
  PackageName  [ route ]
  Synopsis     [ Define routing manager ]
  Author       [ Chien-Kai Ma, Kai-Chun Chang, Yu-Wei Fan ]
  Copyright    [ Copyleft(c) 2020-present NTU, Taiwan ]
****************************************************************************/

#ifndef ROUTE_MGR_H
#define ROUTE_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include "routeNet.h"

using namespace std;

extern RouteMgr *routeMgr;


class RouteMgr
{
public:
    RouteMgr() {}
    ~RouteMgr() { // TODO: reset();
    } 
    bool readCircuit(const string&);
    void printRouteSummary();
    void printMCList();
    void printLaySupply();
    void printNonDefaultSupply();
    void printExtraDemand();
    void place();
    void route();
    void layerassign();
    void initSupply();
    void add2DDemand(Net*);
    void remove2DDemand(Net*);
private:
    unsigned maxMoveCnt;
    MCList   _mcList; // id->MC*
    InstList _instList; // 1D array
    GridList _gridList; // 2D array
    NetList  _netList;  // Net
    vector<bool> _layDir; // layId -> Horizontal or Vertical
    vector<unsigned> _laySupply; // layId -> default supply
    unordered_map<MCTri, unsigned, TriHash> _sameGridDemand;
    unordered_map<MCTri, unsigned, TriHash> _adjHGridDemand;
    unordered_map<MCTri, int, TriHash> _nonDefaultSupply; // supply offset row,col,lay
    bool _placeStrategy;//0 for force-directed, 1 for congestion-based move
};

#endif // ROUTE_MGR_H
