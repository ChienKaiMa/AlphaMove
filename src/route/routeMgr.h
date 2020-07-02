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
friend CellInst;
public:
    RouteMgr() : _placeStrategy(0) {}
    ~RouteMgr() { // TODO: reset();
    } 
    bool    readCircuit(const string&);
    void    writeCircuit(ostream&) const;
    void    setRouteLog(ofstream *logFile) { _tempRoute = logFile; }
    void    genGridList();

    void    printRouteSummary() const;
    void    printNetlist() const;
    void    printMCList() const;

    void    printLaySupply() const;
    void    printNonDefaultSupply() const;
    void    print2DSupply() const;

    void    printGridDemand() const;
    void    printExtraDemand() const;
    void    print2DDemand() const;

    void    print2DCongestion() const;

    void    printCellInst() const;
    void    printAssoNet() const;
    void    printInitSegs() const;
    double  getCongestion(Pos pos) const { 
      if( pos.first<Ggrid::rBeg  || pos.first>Ggrid::rEnd || 
          pos.second<Ggrid::cBeg || pos.second>Ggrid::cEnd ){ 
        return -CONGEST_MIN;  
      }
      else { 
        return -_gridList[pos.first-1][pos.second-1]->get2dCongestion(); 
      }
    }
    
    /**********************************/
    /*        Placement&Routing       */
    /**********************************/
    void     place();
    void     netbasedPlace();
    void     forcedirectedPlace ();
    unsigned Share(Net*,Net*);
    pair<double,double> Move(Net*,Net*,double);

    void    route2D(NetList&);
    void    route();
    void    koova_place();
    void    change_notifier(CellInst*);
    void    koova_route();
    void    layerassign(NetList&);
    void    initSupply();
    void    add2DDemand(Net*);
    void    remove2DDemand(Net*);

private:
    unsigned          maxMoveCnt;
    unsigned          initTotalWL; // wirelength
    vector<Segment*>  initRouteSegs; // TODO: check redundancy
    MCList            _mcList; // id->MC*
    InstList          _instList; // 1D array
    GridList          _gridList; // 2D array
    NetList           _netList;  // Net
    vector<bool>      _layDir; // layId -> Horizontal or Vertical
    vector<unsigned>  _laySupply; // layId -> default supply
    unordered_map<MCTri, unsigned, TriHash>   _sameGridDemand;
    unordered_map<MCTri, unsigned, TriHash>   _adjHGridDemand;
    unordered_map<MCTri, int, TriHash>        _nonDefaultSupply; // supply offset row,col,lay
    bool              _placeStrategy; // 0 for force-directed, 1 for congestion-based move

    // Results
    // TODO: maintain and prepare for output
    unsigned          _curMoveCnt = 0;
    unsigned          _curTotalWL;
    InstSet           _movedSet;
    vector<Segment*>  _curRouteSegs; // TODO: check redundancy
    ofstream*         _tempRoute;


    //Routing Helper function
    bool route2Pin(Pos p1, Pos p2, Net* net);
    Pos getPinPos(const PinPair) const; // 2D
};

#endif // ROUTE_MGR_H
