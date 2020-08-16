/**************************************
  FileName      [ routeNet.h ]
  PackageName   [ route ]
  Synopsis      [ Define basic routing-net data structures ]
  Author        [ TBD ]
***************************************/

#ifndef ROUTE_NET_H
#define ROUTE_NET_H
#define CONGESTION_PARAMETER 0.7

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <utility>
#include <unordered_map>
#include <set>
#include <map>
#include <cassert>
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
    return (this->idx1 == p.idx1) && (this->idx2 == p.idx2) && 
            (this->layNum == p.layNum);
  }
};
// hasher for triple
struct TriHash{
  size_t operator()(const MCTri& p) const{
    return ((p.idx1 << 13)+(p.idx2 << 27)+p.layNum);
  }
};
// hasher for ordered pair
struct PairHash{
  size_t operator() (const pair<unsigned, unsigned>& p) const{
    return ((p.first << 16)+p.second);
  }
};

enum SegDirection
{
    DIR_H   = 0,
    DIR_V   = 1,
    DIR_Z   = 2,

    // dummy
    UNDEFINED
};

//----------------------------------------------------------------------
//    Grid overflow status
//----------------------------------------------------------------------
enum GridStatus
{
    GRID_HEALTHY    = 0,
    GRID_RISKY      = 2,
    GRID_FULL_CAP   = 3,
    GRID_OVERFLOW   = 6,

    // dummy
    GRID_TOT
};

//--------------------
// MasterCell class
//--------------------
class MC  // MasterCell 
{
    friend CellInst;
    friend RouteMgr;
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
    _cellId(id), _grid(grid), _mc(mc), _movable(move), _initGrid(grid) {}
    ~CellInst(){}
    Pos getPos();
    Pos getPos() const;
    Pos getInitPos() const;
    Ggrid* getGrid() {return _grid;}
    unsigned getId() const { return _cellId; }
    MC* getMC() {return (MC*)_mc;}
    unsigned getPinLay(unsigned idx) const;
    bool is_movable() { return _movable; }
    vector<int> assoNet; // associated net index // Koova
    void printPos(ostream&) const;
    void printPos() const;
    void printCell() const;
    void printAssoNet() const;
    void move(Pos);
    bool     _hasmovedbyfd = false;
    bool     min_layer_constraint = false;
private:
    const unsigned  _cellId;
    Ggrid*          _grid;   // in which grid;
    const MC*       _mc; // storing MasterCell Info.
    const bool      _movable;
    const Ggrid*    _initGrid; // Use it to check if the cell is moved.
};



// Multi Layer in a gGrid , i.e. layerGrid
//-----------------------
// Layer & gGrid class
//----------------------
class Layer 
{
    friend Ggrid;
public:
    Layer() : _supply(0) { _capacity = _supply; }
    ~Layer(){}
    inline void setSupply(unsigned supply) { _supply = supply; _capacity = supply; }
    inline void addDemand(int offset) { _capacity -= offset; }
    inline void removeDemand(int offset) { _capacity += offset; }
    inline unsigned getSupply() { return _supply; }
    inline int getDemand() { return (((int)_supply) - _capacity);}

    void printSummary() const;
    GridStatus checkOverflow() const;
private:
    unsigned _supply;
    int _capacity;  // supply - demand
};

class Ggrid
{
    friend CellInst;
public:
    Ggrid(Pos coord, unsigned layNum): _pos(coord), _2dSupply(0), _2dDemand(0), _2dCongestion(1) {
        
        initLayer(layNum);
    }
    ~Ggrid(){}
    Layer*& operator [] (unsigned layId) {
        assert(!_layerList.empty());
         return _layerList[layId-1]; }
    inline void initLayer( unsigned layNum ){ 
        _layerList.resize(layNum); 
        for (unsigned i=0; i<layNum; ++i) {
            Layer* newL = new Layer();
            _layerList[i] = newL;
        }
    }
    static void setBoundary(unsigned rrBeg, unsigned ccBeg, unsigned rrEnd, unsigned ccEnd){ // [row][col]
        rBeg = rrBeg;
        cBeg = ccBeg;
        rEnd = rrEnd;
        cEnd = ccEnd;
    }
    void set2dSupply(int supply) { _2dSupply = supply; }
    unsigned get2dSupply() const { return _2dSupply; }
    unsigned get2dSupply() { return _2dSupply; }
    double get2dDemand() const { return _2dDemand; }
    double get2dDemand() { return _2dDemand; }
    //double get2dCongestion() const {return _2dCongestion;}
    double get2dCongestion() {return _2dCongestion;}

    void printSummary() const;
    void printCapacity() const;
    void printDemand() const;
    void update2dDemand( double deltaDemand ) { 
        assert(_2dSupply > 0);
        _2dDemand = _2dDemand + deltaDemand;
        #ifdef DEBUG
        cout << "Grid (" << _pos.first << "," << _pos.second << ") delta demand " << deltaDemand << "\n"; 
        #endif
        _2dCongestion = ((double)(_2dSupply) - (double)(_2dDemand)  * CONGESTION_PARAMETER) / (double)(_2dSupply); 
    }
    unsigned getOverflowCount() const;
    double koovaCongParam() {
        double gotcha = 0;
        for (auto m : _layerList) {
            gotcha += m->checkOverflow();
        }
        // TODO: Maybe average?
        return gotcha / _layerList.size();
    }
    
    void updatePos( Pos newpos ){
        _pos = newpos;
    }
    Pos getPos() const { return _pos; }

    static unsigned rBeg;
    static unsigned rEnd;
    static unsigned cBeg;
    static unsigned cEnd;

    vector<CellInst*> cellInstList;
private:
    Pos        _pos;
    LayerList  _layerList;
    InstList   _cellOnGridList;
    unsigned   _2dSupply;
    double     _2dDemand;
    double     _2dCongestion;
};

//-------------------
//   Segment Class
//-------------------

// TODO
class Segment
{
    friend Net;
public:
    Segment() {}
    Segment(Segment*& seg) {
        startPos[0] = seg->startPos[0];
        startPos[1] = seg->startPos[1];
        startPos[2] = seg->startPos[2];
        endPos[0] = seg->endPos[0];
        endPos[1] = seg->endPos[1];
        endPos[2] = seg->endPos[2];
    }
    ~Segment() {}
    Segment(unsigned srow, unsigned scol, unsigned slay, unsigned erow, unsigned ecol, unsigned elay)
    {
        startPos[0] = srow;
        startPos[1] = scol;
        startPos[2] = slay;
        endPos[0] = erow;
        endPos[1] = ecol;
        endPos[2] = elay;
    }
    void print() const;
    void print(ostream&) const;
    unsigned getWL() const ; // Manhattan Distance
    void passGrid(Net* net, set<Layer*>& alpha) const;
    set<Layer*> newGrid(Net* net, set<Layer*>& alpha) const;
    void extend(); // TODO or I'm crazy
    void assignLayer(unsigned);
    bool checkOverflow();
    void rearrange();
    unsigned startPos[3];
    unsigned endPos[3];
    SegDirection    checkDir() const;
    bool            isValid() const;
    bool            isZero() const;
};


//---------------
//   Net Class
//---------------

// TODO
// How to get segment's position
class Net
{
    friend RouteMgr;
    friend NetRank;
public:
    Net(unsigned id, unsigned layCons): _netId(id), _minLayCons(layCons){};
    ~Net();
    inline void addPin(PinPair pin){ _pinSet.insert(pin); }
    void addSeg(Segment*& s) { _netSegs.push_back(s); }
    void shouldReroute(bool q) { _toReroute = q; }
    bool operator > (const Net& net ) const { return this->_pinSet.size() > net._pinSet.size(); }
    void ripUp();
    void initAssoCellInst();
    void avgPinLayer();
    set<PinPair> sortPinSet();

    // TODO
    // Layer assignment
    RouteExecStatus layerAssign();
    void addPinDemand();
    void removePinDemand();
    void assignH(Segment*);
    void assignV(Segment*);
    void assignZ(Segment*);
    bool checkOverflow();
    
    //Accessing functions
    unsigned getMinLayCons() { return _minLayCons; }
    set<PinPair> getPinSet() { return _pinSet; }
    bool shouldReroute() { return _toReroute; }
    bool findVCand(vector<int>&);
    bool findHCand(vector<int>&);
    bool findZCand(vector<int>&);
    unsigned getWirelength();
    void reduceSeg();

    //Printing functions
    void printPinSet() const;
    void printSummary() const;
    void printAllSeg() const;
    void printAllSeg(ostream&) const;
    void printAssoCellInst() const;
    
private:
    unsigned            _netId;
    unsigned            _minLayCons; // minimum layer Constraints
    set<PinPair>        _pinSet; // a set of pins i.e. <instance id, pin id>  pair
    vector<Segment*>    _netSegs; //TODO pointer?
    // unordered_map< unsigned, Pos > _pinPos; // a map from instance id->Pos(current placement);
    bool                _toReroute = false; // TODO: decide whether true or false

    //bounding box
    unsigned            _centerRow;
    unsigned            _centerCol;
    double              _avgCongestion;
    double              _avgPinLayer;

    //associated cell instances
    //vector<unsigned>    _assoCellInst; //Represents the pin numbers of the cell instances in the net
    map<unsigned, unsigned> _assoCellInstMap;
    bool                _toRemoveDemand = false;
    bool                _hasmovedbynb = false;
};

class NetRank
{
    friend RouteMgr;
    friend Net;
public:
    void init();
    void update();
    void showTopTen() const;
    vector<unsigned> getTopTen() const;
private:
    vector<PinPair> NetWLpairs; // <netId, netTotWL>
};

#endif // ROUTE_NET_H
