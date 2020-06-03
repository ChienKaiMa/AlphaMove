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
    void optimize();
    void printRouteSummary();
    void printMCList();
    void printLaySupply();
private:
    unsigned maxMoveCnt;
    MCList mcList; // id->MC*
    vector<bool> layDir; // layId -> Horizontal or Vertical
    vector<unsigned> laySupply; // layId -> default supply
    unordered_map<MCTri, unsigned, TriHash> sameGridDemand;
    unordered_map<MCTri, unsigned, TriHash> adjGridDemand;
    unordered_map<MCTri, int, TriHash> nonDefaultSupply; // supply offset row,col,lay
};

#endif // ROUTE_MGR_H
