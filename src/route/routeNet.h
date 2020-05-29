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


class MC  // MasterCell 
{
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
private:
    unsigned    mcId;
    UintList    layerOfPin; // idx: pin  value: layer 
    BlkgList    blkgList;   // value pair < LayerNum, demand >
};

class CellInst
{
public:
    CellInst(){}
    ~CellInst(){}
private:
    unsigned cellId;
    bool     movable;
    Ggrid*   grid;   // in which grid;
    MC*      mcType; // storing MasterCell Info.
};

class Ggrid
{
public:
    Ggrid(){}
    ~Ggrid(){}
private:
    pair<unsigned, unsigned> pos;
};



































#endif // ROUTE_NET_H
