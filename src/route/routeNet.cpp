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

/*****************************/
/* class MC member functions */
/*****************************/

void 
MC::printMC() const{
    cout << "MasterCell MC" << mcId << " " << layerOfPin.size() << " " << blkgList.size() << endl;
    for (unsigned i; i<layerOfPin.size(); ++i)
        cout << "Pin " << "P" << i << " " << "M" << layerOfPin[i] << endl; 
    for (unsigned i; i<blkgList.size(); ++i)
        cout << "Blkg " << "B" << i << " " << "M" << blkgList[i].first << " " << blkgList[i].second << endl; 
}
















































