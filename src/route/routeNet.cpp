/****************************************************************************
  FileName     [ cirNet.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class in routeNet.h member functions ]
  Author       [ TBD ]
****************************************************************************/ 

#include <iostream>
#include <iomanip>
#include "routeNet.h"
#include "routeMgr.h"

using namespace std;



unsigned Ggrid::xMax = 0;
unsigned Ggrid::yMax = 0;
unsigned Ggrid::xMin = 0;
unsigned Ggrid::yMin = 0;


/*****************************/
/* class MC member functions */
/*****************************/
void 
MC::printMC() const{
    cout << "MasterCell MC" << mcId << " " << layerOfPin.size() << " " << blkgList.size() << endl;
    for (unsigned i=0; i<layerOfPin.size(); ++i)
        cout << "Pin " << "P" << i << " " << "M" << layerOfPin[i] << endl; 
    for (unsigned i=0; i<blkgList.size(); ++i)
        cout << "Blkg " << "B" << i << " " << "M" << blkgList[i].first << " " << blkgList[i].second << endl; 
}



/***********************************/
/* class CellInst member functions */
/***********************************/
Pos
CellInst::getPos(){
	return grid->pos; 
}












































