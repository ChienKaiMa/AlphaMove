/****************************************************************************
  FileName     [ routeNet.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class in routeNet.h member functions ]
  Author       [ TBD ]
****************************************************************************/ 

#include <iostream>
#include <iomanip>
#include <math.h>
#include <algorithm>
#include "routeNet.h"
#include "routeMgr.h"

using namespace std;

unsigned Ggrid::rEnd = 0;
unsigned Ggrid::cEnd = 0;
unsigned Ggrid::rBeg = 0;
unsigned Ggrid::cBeg = 0;

static bool CompareWL(PinPair a, PinPair b) {
    return a.second > b.second;
}

/********************************/
/* class Ggrid member functions */
/********************************/
void
Ggrid::printCapacity() const
{
    for (auto& m : _layerList)
    {
        cout << m->_capacity << setw(5);
    }
    cout << endl;
}

void
Ggrid::printDemand() const {
    for (auto& m : _layerList)
    {
        cout << m->_supply - m->_capacity << setw(5);
    }
    cout << endl;
}

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

Pos
CellInst::getInitPos() const
{
	return _initGrid->_pos; 
}

void
CellInst::printPos(ostream& outfile) const
{
    outfile << " " << getPos().first << " " << getPos().second << endl;
}

void
CellInst::printPos() const
{
    cout << "row: " << getPos().first << ", col: " << getPos().second << endl;
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
    _grid = routeMgr->_gridList[newPos.first-1][newPos.second-1];
}

unsigned
CellInst::getPinLay(unsigned idx) const
{
    return _mc->_layerOfPin[idx-1];
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

bool
Segment::isValid() const
{
    if (startPos[0] < Ggrid::rBeg || startPos[0] > Ggrid::rEnd
        || endPos[0] < Ggrid::rBeg || endPos[0] > Ggrid::rEnd
        || startPos[1] < Ggrid::cBeg || startPos[1] > Ggrid::cEnd
        || endPos[1] < Ggrid::cBeg || endPos[1] > Ggrid::cEnd
        || startPos[2] < 1 || startPos[2] > routeMgr->getLayerCnt()
        || endPos[2] < 1 || endPos[2] > routeMgr->getLayerCnt()) {
        print();    cout << " is not valid!\n";
        return false;
    } else { return true; }
}

SegDirection
Segment::checkDir() const
{
    if (startPos[0] == endPos[0]) {
        if (startPos[1] == endPos[1]) {
            return DIR_Z;
        } else {
            return DIR_H;
        }
    } else {
        return DIR_V;
    }
}

unsigned
Segment::getWL() const {
    return abs((int)startPos[0]-(int)endPos[0]) + abs((int)startPos[1]-(int)endPos[1]) + abs((int)startPos[2]-(int)endPos[2]);
}

void
Segment::passGrid(Net* net, set<Layer*>& alpha) const
{
    if (!isValid()) { return; }
    unsigned i0 = startPos[0];
    unsigned j0 = startPos[1];
    unsigned k0 = startPos[2];
    unsigned i1 = endPos[0];
    unsigned j1 = endPos[1];
    unsigned k1 = endPos[2];

    if (checkDir() == DIR_H) {
        if (j0 > j1) {
            unsigned tmp = j0;
            j0 = j1;
            j1 = tmp;
        }
        for (unsigned x=j0; x<=j1; ++x)
            alpha.insert((*(routeMgr->_gridList[i0-1][x-1]))[k0]);
    } else if (checkDir() == DIR_V) {
        if (i0 > i1) {
            unsigned tmp = i0;
            i0 = i1;
            i1 = tmp;
        }
        for (unsigned x=i0; x<=i1; ++x)
            alpha.insert((*(routeMgr->_gridList[x-1][j0-1]))[k0]);
    } else {
        if (k0 > k1) {
            unsigned tmp = k0;
            k0 = k1;
            k1 = tmp;
        }
        for (unsigned x=k0; x<=k1; ++x) {
            alpha.insert((*(routeMgr->_gridList[i0-1][j0-1]))[x]);
        }
    }
}

void
Segment::rearrange()
{
    unsigned temp = 1;
    for (auto i : {0, 1, 2}) {
        if (startPos[i] > endPos[i]) {
            temp = endPos[i];
            endPos[i] = startPos[i];
            startPos[i] = temp;
        }
    }
}

bool
Segment::checkOverflow()
{
    int OVCNT = 0;
    int FULLCNT = 0;
    rearrange();
    for (unsigned i=startPos[0]; i<=endPos[0]; ++i) {
        for (unsigned j=startPos[1]; j<=endPos[1]; ++j) {
            for (unsigned k=startPos[2]; k<=endPos[2]; ++k) {
                if (routeMgr->check3dOverflow(i, j, k) == GRID_OVERFLOW) {
                    ++OVCNT;
                } else if (routeMgr->check3dOverflow(i, j, k) == GRID_FULL_CAP) {
                    ++FULLCNT;
                }
            }
        }
    }
    cout << OVCNT << " grids in "; print(); cout << " is overflown!\n";
    cout << FULLCNT << " grids in "; print(); cout << " is full!\n";
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
    printAssoCellInst();
    printAllSeg();
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

void
Net::ripUp(){
    for(auto seg: _netSegs){
        delete seg;
    }
    _netSegs.clear();
}

void 
Net::initAssoCellInst(){
    std::set<PinPair>::iterator ite = _pinSet.begin();
    for(unsigned i=0;i<_pinSet.size();++i){
        //++_assoCellInst[ite->first-1];
        std::pair<std::map<unsigned,unsigned>::iterator, bool> ret;
        ret = _assoCellInstMap.insert(pair<unsigned,unsigned>(ite->first, 1));
        if(ret.second == false){
            ++ret.first->second;
        }
        ++ite;
    }
}

void
Net::avgPinLayer() {
    double totalPinLayer = 0;
    double pinCnt = 0;
    for (auto& p : _pinSet) {
        totalPinLayer += double(p.second);
        ++pinCnt;
    }
    _avgPinLayer = totalPinLayer / pinCnt;
    #ifdef DEBUG
    cout << "AvgPinLayer of N" << _netId << " is "
    << _avgPinLayer << "\n";
    #endif
}

void
Net::printAssoCellInst() const
{
    auto ite = _assoCellInstMap.begin();
    for(unsigned i=0;i<_assoCellInstMap.size();++i){
        cout << "CellInst " << ite->first << " has " << ite->second << " associated pins.\n";
        ++ite;
    }
}

unsigned
Net::passGrid() const{
    #ifdef DEBUG
    cout << "In Net" << _netId << "..." << endl;
    #endif
    unsigned layNum = routeMgr->_laySupply.size();
    bool gridGraph[Ggrid::rEnd][Ggrid::cEnd][layNum] ;
    for (unsigned i=0; i<Ggrid::rEnd; ++i){
        for(unsigned j=0; j<Ggrid::cEnd;++j){
            for(unsigned k=0; k<layNum; ++k){
                gridGraph[i][j][k] = 0;
            }
        }
    }
    unsigned gridNum = 0;
    for(const auto& seg: _netSegs){
        unsigned rStart = seg->startPos[0];
        unsigned cStart = seg->startPos[1];
        unsigned lStart = seg->startPos[2];
        unsigned rEnd   = seg->endPos[0];
        unsigned cEnd   = seg->endPos[1];
        unsigned lEnd   = seg->endPos[2];
        // cout << rStart << " " << cStart << " " << lStart << endl;
        // cout << rEnd << " " << cEnd << " " << lEnd << endl;
        assert(rEnd);
        assert(cEnd);
        assert(lEnd);
        assert(rStart);
        assert(cStart);
        assert(lStart);
        if(rStart!=rEnd){
            for(unsigned i = (rStart<rEnd ? rStart : rEnd); i<= (rStart<rEnd ? rEnd : rStart) ; i++){
                // cout << "GridGraph " << gridGraph[i-1][cEnd-1][lEnd-1] << endl;
                if(!gridGraph[i-1][cEnd-1][lEnd-1]){
                    ++gridNum;
                    gridGraph[i-1][cEnd-1][lEnd-1] = 1;
                }
            }
        }
        else if(cStart!=cEnd){
            for(unsigned i = (cStart<cEnd ? cStart : cEnd); i<= (cStart<cEnd ? cEnd : cStart); i++){
                // cout << "GridGraph " << gridGraph[rEnd-1][i-1][lEnd-1] << endl;
                if(!gridGraph[rEnd-1][i-1][lEnd-1]){
                    ++gridNum;
                    gridGraph[rEnd-1][i-1][lEnd-1] = 1;
                }
            }
        }
        else if(lStart!=lEnd){
            for(unsigned i = (lStart<lEnd ? lStart : lEnd); i<= (lStart<lEnd ? lEnd : lStart); i++){
                // cout << "GridGraph " << gridGraph[rEnd-1][cEnd-1][i-1] << endl;
                if(!gridGraph[rEnd-1][cEnd-1][i-1]){
                    ++gridNum;
                    gridGraph[rEnd-1][cEnd-1][i-1] = 1;
                }
            }
        }
        else{
            gridGraph[rEnd-1][cEnd-1][lEnd-1] = 1;
            ++gridNum;
        }
    }
    if(_netSegs.empty()){
        gridNum = 1;
    }
    #ifdef DEBUG
    cout << "Net n" << this->_netId << " passed " << gridNum << "grids" << endl;
    #endif
    return gridNum;
}

/**********************************/
/* class NetRank member functions */
/**********************************/
void
NetRank::init()
{
    for (auto net : routeMgr->_netList)
    {
        set<Layer*> alpha;
        routeMgr->passGrid(net, alpha);
        unsigned WL = alpha.size();
        PinPair newpair = PinPair(net->_netId, WL);
        NetWLpairs.push_back(newpair);
    }
    ::sort(NetWLpairs.begin(), NetWLpairs.end(), CompareWL);
}

void
NetRank::update()
{
    NetWLpairs.clear();
    init();
    // TODO: less memory cost
}

vector<unsigned>
NetRank::getTopTen() const
{
    vector<unsigned> nets;
    if (NetWLpairs.size() < 30) {
        for (auto& nwPair : NetWLpairs)
        {
            nets.push_back(nwPair.first);
        }
    } else {
        int cnt = 0;
        for (auto& nwPair : NetWLpairs)
        {
            if (cnt>30) {
                break;
            }
            nets.push_back(nwPair.first);
            ++cnt;
        }
    }
    return nets;
}

void
NetRank::showTopTen() const
{
    if (NetWLpairs.size() < 10) {
        for (auto& nwPair : NetWLpairs)
        {
            cout << "Net " << nwPair.first << " : "
                << nwPair.second << "\n";
        }
    } else {
        int cnt = 0;
        for (auto& nwPair : NetWLpairs)
        {
            if (cnt>10) {
                break;
            }
            cout << "Net " << nwPair.first << " : "
                << nwPair.second << "\n";
            ++cnt;
        }
    }
}































