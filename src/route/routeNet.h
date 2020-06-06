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
#include <set>
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
    MC(unsigned id, unsigned pinCnt, unsigned blkgCnt) : _mcId(id) {
        _layerOfPin.resize(pinCnt);
        _blkgList.resize(blkgCnt);
    }
    ~MC(){}
    void printMC() const;
    bool addPin(unsigned id, unsigned layer) {
        // TODO Error handling
        _layerOfPin[id-1] = layer;
        return true;
    }
    bool addBlkg(unsigned id, unsigned layer, unsigned demand){
        // TODO Error handling
        _blkgList[id-1] = pair<unsigned, unsigned>(layer, demand);
        return true;
    }
    unsigned getId() { return _mcId; }
private:
    unsigned    _mcId;
    UintList    _layerOfPin; // idx: pin  value: layerNum 
    BlkgList    _blkgList;   // value pair < LayerNum, demand >
};


//-------------------
// CellInst class
//-------------------
class CellInst
{
    friend MC;
public:
    CellInst(unsigned id, Ggrid* grid, MC* mc, bool move): 
    _cellId(id), _grid(grid), _mc(mc), _movable(move) {}
    ~CellInst(){}
    Pos getPos();
private:
    unsigned _cellId;
    bool     _movable;
    Ggrid*   _grid;   // in which grid;
    MC*      _mc; // storing MasterCell Info.
};



// Multi Layer in a gGrid , i.e. layerGrid
//-----------------------
// Layer & gGrid class
//----------------------
class Layer 
{
    friend Ggrid;
public:
    Layer(){ _demand = 0;}
    ~Layer(){}
    inline void addDemand(int offset){ _demand+=offset; }
private:
    unsigned _demand;
};

class Ggrid
{
    friend CellInst;
public:
    Ggrid(Pos coord): _pos(coord) {}
    ~Ggrid(){}
    const Layer& operator [] (unsigned layId) { return *_layerList[layId]; }
    inline void initLayer( unsigned layNum ){ _layerList.resize(layNum); }
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
    Pos        _pos;
    LayerList  _layerList;
};

//---------------
//   Net Class
//---------------

// TODO
class Net
{
public:
    Net(unsigned layCons): _minLayCons(layCons) {};
    ~Net();
    inline void addPin(pair<unsigned, unsigned> pin){ _pinSet.insert(pin); }
private:
    unsigned _minLayCons; // minimum layer Constraints
    set<pair<unsigned,unsigned>> _pinSet; // a set of pins i.e. <instance id, pin id>  pair
};


#endif // ROUTE_NET_H
