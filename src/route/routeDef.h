/****************************************************************************
  FileName     [ routeDef.h ]
  PackageName  [ route ]
  Synopsis     [ Define basic data or var for route package ]
  Author       [ Chien-Kai Ma, Kai-Chun Chang, Yu-Wei Fan ]
  Copyright    [ Copyleft(c) 2020-present NTU, Taiwan ]
****************************************************************************/

#ifndef ROUTE_DEF_H
#define ROUTE_DEF_H
#define H 0 // horizontal
#define V 1 // Vertical
#define CONGEST_MAX 2147483647 // INT_MAX

#include <vector>

using namespace std;

// TODO: define your own typedef or enum

class RouteMgr;
class MC;
class Ggrid;
class Net;
class CellInst;
class Layer;
typedef vector<CellInst*> InstList;
typedef vector<MC*> MCList;
typedef vector<vector<Ggrid*>> GridList;
typedef vector<Net*> NetList;
typedef pair<unsigned, unsigned> Pos;
typedef pair<unsigned, unsigned> PinPair;
typedef vector<Layer*> LayerList;

#endif // ROUTE_DEF_H