/**************************************
  FileName      [ routeNet.h ]
  PackageName   [ route ]
  Synopsis      [ Define basic routing-net data structures ]
  Author        [ TBD ]
***************************************/

#ifndef ROUTE_NET_H
#define ROUTE_NET_H
#define CONGESTION_PARAMETER 1.0

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
    idx1 = n1;
    idx2 = n2;
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
    return ((p.idx1<<13)+(p.idx2<<27)+p.layNum);
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
    Pos getPos() const;
    Ggrid* getGrid() {return _grid;}
    unsigned getId() const { return _cellId; }
    vector<int> assoNet; // associated net index // Koova
    void printPos(ostream&) const;
    void printPos() const;
    void printCell() const;
    void printAssoNet() const;
    void move(Pos);
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
    Ggrid(Pos coord): _pos(coord), _2dDemand(0), _2dSupply(0), _2dCongestion(0) {}
    ~Ggrid(){}
    const Layer& operator [] (unsigned layId) { return *_layerList[layId]; }
    inline void initLayer( unsigned layNum ){ _layerList.resize(layNum); }
    static void setBoundary(unsigned rBeg, unsigned cBeg, unsigned rEnd, unsigned cEnd){
        yMin = rBeg;
        xMin = cBeg;
        yMax = rEnd;
        xMax = cEnd;
    }
    void set2dSupply(int supply) { _2dSupply = supply; }
    unsigned get2dSupply() const { return _2dSupply; }
    unsigned get2dSupply() { return _2dSupply; }
    void updateDemand( int deltaDemand ) { 
        _2dDemand = _2dDemand + deltaDemand;
        _2dCongestion = (double)((_2dSupply - _2dDemand * CONGESTION_PARAMETER) / _2dSupply); 
    }
    double getCongestion() {return _2dCongestion;}
    void updatePos( Pos newpos ){
        _pos = newpos;
    }
    Pos getPos() const { return _pos; }

    static unsigned xMin;
    static unsigned yMin;
    static unsigned xMax;
    static unsigned yMax;

private:
    Pos        _pos;
    LayerList  _layerList;
    InstList   _cellOnGridList;
    unsigned   _2dSupply;
    unsigned   _2dDemand;
    double     _2dCongestion;
};

//-------------------
//   Segment Class
//-------------------

// TODO
class Segment
{
public:
    Segment() {}
    ~Segment() {}
    Segment(unsigned scol, unsigned srow, unsigned slay, unsigned ecol, unsigned erow, unsigned elay)
    {
        startPos[0] = scol;
        startPos[1] = srow;
        startPos[2] = slay;
        endPos[0] = ecol;
        endPos[1] = erow;
        endPos[2] = elay;
    }
    void print() const;
    void print(ostream&) const;
    unsigned startPos[3];
    unsigned endPos[3];
};


//---------------
//   Net Class
//---------------

// TODO
// How to get segment's position
class Net
{
    friend RouteMgr;
public:
    Net(unsigned id, unsigned layCons): _netId(id), _minLayCons(layCons){};
    ~Net();
    inline void addPin(PinPair pin){ _pinSet.insert(pin); }
    void addSeg(Segment* s) { _netSegs.push_back(s); }
    unsigned getMinLayCons() { return _minLayCons; }
    set<PinPair> getPinSet() { return _pinSet; }
    void printPinSet() const;
    void shouldReroute(bool q) { _toReroute = q; }
    bool shouldReroute() { return _toReroute; }
    void printSummary() const;
    void printAllSeg() const;
    void printAllSeg(ostream&) const;
    bool operator > (const Net& net ) const { return this->_pinSet.size() > net._pinSet.size(); }
private:
    unsigned            _netId;
    unsigned            _minLayCons; // minimum layer Constraints
    set<PinPair>        _pinSet; // a set of pins i.e. <instance id, pin id>  pair
    vector<Segment*>    _netSegs; //TODO pointer?
    // unordered_map< unsigned, Pos > _pinPos; // a map from instance id->Pos(current placement);
    bool                _toReroute = false;
};


#endif // ROUTE_NET_H
