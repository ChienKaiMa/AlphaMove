/**************************************
  FileName      [ routeNet.h ]
  PackageName   [ route ]
  Synopsis      [ Define basic routing-net data structures ]
  Author        [ TBD ]
***************************************/

#ifndef ROUTE_NET_H
#define ROUTE_NET_H

#include <string>
#include <vector>
#include <iostream>
#include <utility>
#include "routeDef.h"

using namespace std;

typedef vector<unsigned> UintList;
typedef vector< pair<unsigned, unsigned> > BlkgList;


//---------------------
// class Definition
//---------------------

extern RouteMgr *routeMgr;

class MC  // MasterCell 
{
    friend CellInst;
public:
    MC(unsigned id, unsigned pinCnt, unsigned blkgCnt) : mcId(id) {
        layerOfPin.resize(pinCnt);
        blkgList.resize(blkgCnt);
    }
    ~MC(){}
    void printMC() const;
    bool addPin(unsigned id, unsigned layer) {
        // TODO Error handling
        layerOfPin[id-1] = layer;
        return true;
    }
    bool addBlkg(unsigned id, unsigned layer, unsigned demand){
        // TODO Error handling
        blkgList[id-1] = pair<unsigned, unsigned>(layer, demand);
        return true;
    }
    unsigned getId() { return mcId; }
private:
    unsigned    mcId;
    UintList    layerOfPin; // idx: pin  value: layerNum 
    BlkgList    blkgList;   // value pair < LayerNum, demand >
};

class CellInst
{
    friend MC;
public:
    CellInst(unsigned id, Ggrid* grid, MC* mc, bool move): 
    cellId(id), grid(grid), mc(mc), movable(move) {}
    ~CellInst(){}
    Pos getPos();
private:
    unsigned cellId;
    bool     movable;
    Ggrid*   grid;   // in which grid;
    MC*      mc; // storing MasterCell Info.
};


// Multi Layer in a gGrid , i.e. layerGrid
class Layer 
{
    friend Ggrid;
public:
    Layer(){ demand = 0;}
    ~Layer(){}
private:
    unsigned demand;
};

class Ggrid
{
    friend CellInst;
public:
    Ggrid(Pos coord): pos(coord) {}
    ~Ggrid(){}
    const Layer& operator [] (unsigned layId) { return *layerList[layId]; }
    void initLayer( unsigned layNum ){ layerList.resize(layNum); }
    static void setBoundary(unsigned rBeg, unsigned cBeg, unsigned rEnd, unsigned cEnd){
        xMin = rBeg;
        yMin = cBeg;
        xMax = rEnd;
        yMax = cEnd;
    }
    static unsigned xMin;
    static unsigned yMin;
    static unsigned xMax;
    static unsigned yMax;
private:
    Pos        pos;
    LayerList  layerList;
};



































#endif // ROUTE_NET_H
