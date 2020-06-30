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

Pos
CellInst::getPos() const
{
	return _grid->_pos; 
}

void
CellInst::printPos(ostream& outfile) const
{
    outfile << " " << getPos().first << " " << getPos().second << endl;
}

void
CellInst::printPos() const
{
    cout << "x: " << getPos().first << ", y: " << getPos().second << endl;
}

void
CellInst::printCell() const
{
    cout << _cellId << endl;
    if (_movable) {
        cout << "Movable" << endl;
    } else {
        cout << "Not movable" << endl;
    }
    printPos();
    _mc->printMC();
}

void
CellInst::printAssoNet() const
{
    cout << "CellInst" << _cellId << endl;
    for (auto const m : assoNet)
    {
        cout << "N" << m << " ";
    }
    cout << endl;
}

void
CellInst::move(Pos newPos)
{
    _grid->updatePos(newPos);
}

/**********************************/
/* class Segment member functions */
/**********************************/
void
Segment::print() const
{
    cout << startPos[0] << " " << startPos[1] << " " << startPos[2];
    cout << " " << endPos[0] << " " << endPos[1] << " " << endPos[2];
}

void
Segment::print(ostream& outfile) const
{
    outfile << startPos[0] << " " << startPos[1] << " " << startPos[2];
    outfile << " " << endPos[0] << " " << endPos[1] << " " << endPos[2];
}


/******************************/
/* class Net member functions */
/******************************/
void
Net::printSummary() const
{
    cout << "N" << _netId << endl;
    cout << "MinLayerConstr " << _minLayCons << endl;
    printPinSet();
}

void
Net::printPinSet() const
{
    for(auto it = _pinSet.begin(); it != _pinSet.end(); ++it)
    {
        auto a = *it;
        cout << a.first << " " << a.second << endl;
    }
}

void
Net::printAllSeg() const
{
    for (unsigned i=0; i<_netSegs.size(); ++i) {
        _netSegs[i]->print();
        cout << " N" << _netId << endl;
    }
}

void
Net::printAllSeg(ostream& outfile) const
{
    for (unsigned i=0; i<_netSegs.size(); ++i) {
        _netSegs[i]->print(outfile);
        outfile << " N" << _netId << endl;
    }
}





































