/****************************************************************************
  FileName     [ routeNet.cpp ]
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
    cout << "MasterCell MC" << _mcId << " " << _layerOfPin.size() << " " << _blkgList.size() << endl;
    for (unsigned i=0; i<_layerOfPin.size(); ++i)
        cout << "Pin " << "P" << i+1 << " " << "M" << _layerOfPin[i] << endl; 
    for (unsigned i=0; i<_blkgList.size(); ++i)
        cout << "Blkg " << "B" << i+1 << " " << "M" << _blkgList[i].first << " " << _blkgList[i].second << endl; 
}



/***********************************/
/* class CellInst member functions */
/***********************************/
Pos
CellInst::getPos(){
	return _grid->_pos; 
}












































