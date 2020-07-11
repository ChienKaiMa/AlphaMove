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
#include <tuple>
#include <ctime>
#include "routeNet.h"

using namespace std;

extern RouteMgr *routeMgr;


class RouteMgr
{
friend CellInst;
friend Net;
public:
    RouteMgr() : _placeStrategy(0) { _startTime = clock(); }
    ~RouteMgr() { // TODO: reset();
    } 
    bool    readCircuit(const string&);
    void    writeCircuit(ostream&) const;
    void    setRouteLog(ofstream *logFile) { _tempRoute = logFile; }
    void    genGridList();

    void    printRouteSummary() const;
    void    printNetlist() const;
    bool    printNet(int idx) const;
    void    printMCList() const;
    bool    printMCList(unsigned idx) const;

    void    printLaySupply() const;
    void    printNonDefaultSupply() const;
    void    print2DSupply() const;

    void    printGridDemand() const;
    void    printExtraDemand() const;
    void    print2DDemand() const;

    void    print2DCongestion() const;

    void    printCellInst() const;
    void    printAssoNet() const;
    bool    printAssoNet(unsigned idx) const;
    void    printInitSegs() const;
    void    replaceBest();
    double  getCongestion(Pos pos) const { 
      if( pos.first<Ggrid::rBeg  || pos.first>Ggrid::rEnd || 
          pos.second<Ggrid::cBeg || pos.second>Ggrid::cEnd ){ 
        return -CONGEST_MIN;  
      }
      else { 
        return -_gridList[pos.first-1][pos.second-1]->get2dCongestion(); 
      }
    }
    unsigned getLayerCnt() const {
      return _laySupply.size();
    }

    unsigned evaluateWireLen() const;

    
    /**********************************/
    /*        Placement&Routing       */
    /**********************************/      
    void     place();
    void     netbasedPlace();
    void     forcedirectedPlace ();
    unsigned Share(Net*,Net*);
    pair<double,double> Move(Net*,Net*,double);
    void     mainPnR();

    void    route2D(NetList&);
    bool    route2DAll();
    bool    route();
    void    koova_place();
    void    change_notifier(CellInst*);
    void    koova_route();
    
    bool    check3dOverflow(unsigned, unsigned, unsigned);
    bool    findCand(unsigned min, unsigned max, vector<int>&);
    bool    layerassign(NetList&);

    //Demand & Supply
    void    init2DSupply();
    void    add3DDemand(Net*);
    void    remove3DDemand(Net*);
    void    add3DBlkDemand(CellInst*);
    void    remove3DBlkDemand(CellInst*);
    void    add3DNeighborDemand(CellInst*, CellInst*, bool type);
    void    remove3DNeighborDemand(CellInst*, CellInst*, bool type);
    void    add2DDemand(Net*);
    void    remove2DDemand(Net*);
    void    add2DBlkDemand(CellInst*);
    void    remove2DBlkDemand(CellInst*);
    void    add2DNeighborDemand(CellInst*, CellInst*, bool type); //type=0: same gGrid, type=1: adj gGrid
    void    remove2DNeighborDemand(CellInst*, CellInst*, bool type);

    void    initCellInstList();
    /**********************************/
    /*            Output              */
    /**********************************/ 
    void    storeBestResult();

private:
    // Initial
    unsigned          _maxMoveCnt;
    unsigned          _initTotalSegNum; // segment num
    vector<OutputSeg> _initRouteSegs; // TODO: check redundancy
    vector<OutputCell>_initCells;
    MCList            _mcList; // id->MC*
    InstList          _instList; // 1D array
    GridList          _gridList; // 2D array
    NetList           _netList;  // Net
    vector<bool>      _layDir; // layId -> Horizontal or Vertical
    vector<unsigned>  _laySupply; // layId -> default supply
    unordered_map<MCTri, unsigned, TriHash>   _sameGridDemand;
    unordered_map<MCTri, unsigned, TriHash>   _adjHGridDemand;
    unordered_map<MCTri, int, TriHash>        _nonDefaultSupply; // supply offset row,col,lay
    
    // Current
    bool              _placeStrategy; // 0 for force-directed, 1 for congestion-based move
    clock_t           _startTime;
    unsigned          _curMoveCnt = 0;
    unsigned          _curTotalWL;
    InstSet           _curMovedSet;

    // Results
    vector<OutputCell>_bestMovedCells;
    vector<OutputSeg> _bestRouteSegs; // TODO: check redundancy
    unsigned          _bestTotalWL;
    ofstream*         _tempRoute;


    //Routing Helper function
    bool route2Pin(Pos p1, Pos p2, Net* net, double demand, unsigned lay1, unsigned lay2);
    Pos getPinPos(const PinPair) const; // 2D
    unsigned getPinLay(const PinPair) const;
};




#endif // ROUTE_MGR_H
