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
#include <unordered_map>
#include <unordered_set>
#include "routeDef.h"

using namespace std;

typedef vector<unsigned> UintList;
typedef vector< pair<unsigned, unsigned> > BlkgList;


//---------------------
// class Definition
//---------------------

extern RouteMgr *routeMgr;

//--------------------------
// Self-Defined Hasher& Key
//--------------------------
//self defined struct for sameGrid & adjGrid MC pairs & nonDefault
struct MCTri{
  unsigned idx1, idx2, layNum;
  MCTri(const unsigned n1, const unsigned n2, const unsigned layN){
    idx1 = n1 < n2 ? n1 : n2 ;
    idx2 = n1 < n2 ? n2 : n1 ;
    layNum = layN;
  }
  bool operator == (const MCTri& p ) const{
    return (this->idx1==p.idx1) && (this->idx2==p.idx2) && 
            (this->layNum==p.layNum);
  }
};
// hasher for triple
struct TriHash{
  size_t operator()(const MCTri& p) const{
    return ((p.idx1<<27)+(p.idx2<<27)+p.layNum);
  }
};
// hasher for ordered pair
struct PairHash{
  size_t operator() (const pair<unsigned, unsigned>& p) const{
    return ((p.first<<32)+p.second);
  }
};


//--------------------
// MasterCell class
//--------------------
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


//-------------------
// CellInst class
//-------------------
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
//-----------------------
// Layer & gGrid class
//----------------------
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
        yMin = rBeg;
        xMin = cBeg;
        yMax = rEnd;
        xMax = cEnd;
    }
    static unsigned xMin;
    static unsigned yMin;
    static unsigned xMax;
    static unsigned yMax;
private:
    Pos        pos;
    LayerList  layerList;
};

//---------------
//   Net Class
//---------------

// TODO
class Net
{
public:
    Net();
    ~Net();
private:
    unsigned minLayCons; // minimum layer Constraint
};





















#endif // ROUTE_NET_H
