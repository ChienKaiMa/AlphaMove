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

using namespace std;

#include "routeDef.h"

extern RouteMgr *routeMgr;

class RouteMgr
{
public:
    RouteMgr() {}
    ~RouteMgr() { // TODO: reset();
    } 
    bool readCircuit(const string&);
    void optimize();
private:
    unsigned maxMoveCnt;
};

#endif // ROUTE_MGR_H
