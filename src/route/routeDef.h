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