/****************************************************************************
  FileName     [ routeDef.h ]
  PackageName  [ route ]
  Synopsis     [ Define basic data or var for route package ]
  Author       [ Chien-Kai Ma, Kai-Chun Chang, Yu-Wei Fan ]
  Copyright    [ Copyleft(c) 2020-present NTU, Taiwan ]
****************************************************************************/

#ifndef ROUTE_DEF_H
#define ROUTE_DEF_H
#define CONGEST_MIN -2147483647 // INT_MIN

#include <vector>
#include <tuple>
#include <set>

using namespace std;

// TODO: define your own typedef or enum

class RouteMgr;
class MC;
class Ggrid;
class Net;
class CellInst;
class Layer;
class Segment;
class NetRank;
typedef vector<CellInst*> InstList;
typedef set<CellInst*> InstSet;
typedef vector<MC*> MCList;
typedef vector<vector<Ggrid*>> GridList;
typedef vector<Net*> NetList;
typedef pair<unsigned, unsigned> Pos;
typedef pair<unsigned, unsigned> PinPair;
typedef vector<Layer*> LayerList;
typedef tuple<unsigned,unsigned,unsigned> OutputCell; //CellID, row, col
typedef pair<Segment,unsigned> OutputSeg; //Segment, netID

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

//----------------------------------------------------------------------
//    placement strategy
//----------------------------------------------------------------------
enum PlaceStrategy
{
   FORCE_DIRECTED,
   CONGESTION_BASED,
};

//----------------------------------------------------------------------
//    routing execution status
//----------------------------------------------------------------------
enum RouteExecStatus
{
   ROUTE_EXEC_DONE  = 0,
   ROUTE_EXEC_ERROR = 1,
   ROUTE_EXEC_QUIT  = 2,
   ROUTE_EXEC_NOP   = 3,

   // dummy
   ROUTE_EXEC_TOT
};

enum RouteExecError
{
   ROUTE_OVERFLOW     = 0,
   ROUTE_DIR_ILLEGAL  = 1,

   // dummy
   ROUTE_EXEC_ERROR_TOT
};

#endif // ROUTE_DEF_H