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

static bool CompareWL(PinPair a, PinPair b)
{
    return a.second > b.second;
}

/********************************/
/* class Layer member functions */
/********************************/
void Layer::printSummary() const
{
    cout << "Supply: " << _supply << ", Demand: " << _supply - _capacity << " ";
    switch (checkOverflow())
    {
    case GRID_FULL_CAP:
        cout << "Full";
        break;
    case GRID_HEALTHY:
        cout << "Healthy";
        break;
    case GRID_OVERFLOW:
        cout << "Overflow";
        break;
    case GRID_RISKY:
        cout << "Risky";
        break;
    default:
        cout << "Unknown";
        break;
    }
}

GridStatus
Layer::checkOverflow() const
{
    if (_capacity > 3)
        return GRID_HEALTHY;
    else if (_capacity == 0)
        return GRID_FULL_CAP;
    else if (_capacity < 0)
        return GRID_OVERFLOW;
    else
        return GRID_RISKY;
}

/********************************/
/* class Ggrid member functions */
/********************************/
unsigned
Ggrid::getOverflowCount() const {
    unsigned OVCNT = 0;
    for (auto& grid : _layerList)
    {
        if (grid->checkOverflow() == GRID_OVERFLOW)
        {
            ++OVCNT;
        }
    }
    return OVCNT;
}

void
Ggrid::printSummary() const
{
    cout << "\n2D Supply: " << _2dSupply
         << "\n2D Demand: " << _2dDemand
         << "\n2D Congestion: " << _2dCongestion << "\n";
    cout << "CellInst on Ggrid: ";
    for (auto& m : cellInstList)
    {
        cout << m->getId() << setw(7);
    }
    cout << "\n";
    for (unsigned i = 1; i <= _layerList.size(); ++i)
    {
        cout << "(" << _pos.first << "," << _pos.second << "," << i << ") ";
        _layerList[i - 1]->printSummary();
        cout << "\n";
    }
    cout << getOverflowCount() << " grids overflow!\n";
}

void Ggrid::printCapacity() const
{
    for (auto &m : _layerList)
    {
        cout << m->_capacity << setw(5);
    }
    cout << endl;
}

void Ggrid::printDemand() const
{
    for (auto &m : _layerList)
    {
        cout << m->_supply - m->_capacity << setw(5);
    }
    cout << endl;
}

/*****************************/
/* class MC member functions */
/*****************************/
void MC::printMC() const
{
    cout << "MasterCell MC" << _mcId << "\n";
    cout << "Pin count: " << _layerOfPin.size() << "\n";
    cout << "Blkg count: " << _blkgList.size() << "\n";
    for (unsigned i = 0; i < _layerOfPin.size(); ++i)
        cout << "Pin "
             << "P" << i + 1 << " "
             << "M" << _layerOfPin[i] << endl;
    for (unsigned i = 0; i < _blkgList.size(); ++i)
        cout << "Blkg "
             << "B" << i + 1 << " "
             << "M" << _blkgList[i].first << " " << _blkgList[i].second << endl;
}

/***********************************/
/* class CellInst member functions */
/***********************************/
Pos CellInst::getPos()
{
    return _grid->_pos;
}

Pos CellInst::getPos() const
{
    return _grid->_pos;
}

Pos CellInst::getInitPos() const
{
    return _initGrid->_pos;
}

void CellInst::printPos(ostream &outfile) const
{
    outfile << " " << getPos().first << " " << getPos().second << endl;
}

void CellInst::printPos() const
{
    cout << "Orig Pos: (" << _initGrid->getPos().first << "," << _initGrid->getPos().second << ")\n";
    cout << "Position: (" << getPos().first << "," << getPos().second << ")\n";
}

void CellInst::printCell() const
{
    cout << "CellInst " << _cellId << endl;
    if (_movable)
    {
        cout << "Movable" << endl;
    }
    else
    {
        cout << "Not movable" << endl;
    }
    printPos();
    cout << "Associated nets: ";
    printAssoNet();
    _mc->printMC();
}

void CellInst::printAssoNet() const
{
    for (auto const m : assoNet)
    {
        cout << "N" << m << " ";
    }
    cout << endl;
}

void CellInst::move(Pos newPos)
{
    _grid = routeMgr->_gridList[newPos.first - 1][newPos.second - 1];
}

unsigned
CellInst::getPinLay(unsigned idx) const
{
    return _mc->_layerOfPin[idx - 1];
}

/**********************************/
/* class Segment member functions */
/**********************************/
void Segment::print() const
{
    cout << startPos[0] << " " << startPos[1] << " " << startPos[2];
    cout << " " << endPos[0] << " " << endPos[1] << " " << endPos[2];
}

void Segment::print(ostream &outfile) const
{
    outfile << startPos[0] << " " << startPos[1] << " " << startPos[2];
    outfile << " " << endPos[0] << " " << endPos[1] << " " << endPos[2];
}

bool Segment::isValid() const
{
    if (startPos[0] < Ggrid::rBeg || startPos[0] > Ggrid::rEnd || endPos[0] < Ggrid::rBeg || endPos[0] > Ggrid::rEnd || startPos[1] < Ggrid::cBeg || startPos[1] > Ggrid::cEnd || endPos[1] < Ggrid::cBeg || endPos[1] > Ggrid::cEnd || startPos[2] < 1 || startPos[2] > routeMgr->getLayerCnt() || endPos[2] < 1 || endPos[2] > routeMgr->getLayerCnt())
    {
        //print();
        //cout << " is not valid!\n";
        return false;
    }
    else if (startPos[0] == endPos[0] && startPos[1] == endPos[1] && startPos[2] == endPos[2])
    {
        //cout << "[WARN] 0-width segment\n";
        return true;
    }
    else
    {
        return true;
    }
}

bool Segment::isZero() const
{
    return startPos[0] == endPos[0] && startPos[1] == endPos[1] && startPos[2] == endPos[2];
}

SegDirection
Segment::checkDir() const
{
    if (startPos[0] == endPos[0])
    {
        if (startPos[1] == endPos[1])
        {
            return DIR_Z;
        }
        else
        {
            return DIR_H;
        }
    }
    else
    {
        return DIR_V;
    }
}

unsigned
Segment::getWL() const
{
    return abs((int)startPos[0] - (int)endPos[0]) + abs((int)startPos[1] - (int)endPos[1]) + abs((int)startPos[2] - (int)endPos[2]);
}

void Segment::passGrid(Net *net, set<Layer *> &alpha) const
{
    if (!isValid())
    {
        return;
    }
    unsigned i0 = startPos[0];
    unsigned j0 = startPos[1];
    unsigned k0 = startPos[2];
    unsigned i1 = endPos[0];
    unsigned j1 = endPos[1];
    unsigned k1 = endPos[2];

    if (checkDir() == DIR_H)
    {
        if (j0 > j1)
        {
            unsigned tmp = j0;
            j0 = j1;
            j1 = tmp;
        }
        for (unsigned x = j0; x <= j1; ++x)
            alpha.insert((*(routeMgr->_gridList[i0 - 1][x - 1]))[k0]);
    }
    else if (checkDir() == DIR_V)
    {
        if (i0 > i1)
        {
            unsigned tmp = i0;
            i0 = i1;
            i1 = tmp;
        }
        for (unsigned x = i0; x <= i1; ++x)
            alpha.insert((*(routeMgr->_gridList[x - 1][j0 - 1]))[k0]);
    }
    else
    {
        if (k0 > k1)
        {
            unsigned tmp = k0;
            k0 = k1;
            k1 = tmp;
        }
        for (unsigned x = k0; x <= k1; ++x)
        {
            alpha.insert((*(routeMgr->_gridList[i0 - 1][j0 - 1]))[x]);
        }
    }
}

set<Layer *>
Segment::newGrid(Net *net, set<Layer *> &alpha) const
{
    if (!isValid())
    {
        return set<Layer *>();
    }
    unsigned i0 = startPos[0];
    unsigned j0 = startPos[1];
    unsigned k0 = startPos[2];
    unsigned i1 = endPos[0];
    unsigned j1 = endPos[1];
    unsigned k1 = endPos[2];

    set<Layer *> myBoy;
    if (checkDir() == DIR_H)
    {
        if (j0 > j1)
        {
            unsigned tmp = j0;
            j0 = j1;
            j1 = tmp;
        }
        for (unsigned x = j0; x <= j1; ++x)
        {
            if ((alpha.find((*(routeMgr->_gridList[i0 - 1][x - 1]))[k0])) == alpha.end())
            {
                myBoy.insert((*(routeMgr->_gridList[i0 - 1][x - 1]))[k0]);
            }
        }
    }
    else if (checkDir() == DIR_V)
    {
        if (i0 > i1)
        {
            unsigned tmp = i0;
            i0 = i1;
            i1 = tmp;
        }
        for (unsigned x = i0; x <= i1; ++x)
        {
            if ((alpha.find((*(routeMgr->_gridList[x - 1][j0 - 1]))[k0])) == alpha.end())
            {
                myBoy.insert((*(routeMgr->_gridList[x - 1][j0 - 1]))[k0]);
            }
        }
    }
    else
    {
        if (k0 > k1)
        {
            unsigned tmp = k0;
            k0 = k1;
            k1 = tmp;
        }
        for (unsigned x = k0; x <= k1; ++x)
        {
            if ((alpha.find((*(routeMgr->_gridList[i0 - 1][j0 - 1]))[x])) == alpha.end())
            {
                myBoy.insert((*(routeMgr->_gridList[i0 - 1][j0 - 1]))[x]);
            }
        }
    }
    return myBoy;
}

void Segment::rearrange()
{
    unsigned temp = 1;
    for (auto i : {0, 1, 2})
    {
        if (startPos[i] > endPos[i])
        {
            temp = endPos[i];
            endPos[i] = startPos[i];
            startPos[i] = temp;
        }
    }
}

void Segment::assignLayer(unsigned l)
{
    startPos[2] = l;
    endPos[2] = l;
}

bool Segment::checkOverflow()
{
    int OVCNT = 0;
    int FULLCNT = 0;
    rearrange();
    for (unsigned i = startPos[0]; i <= endPos[0]; ++i)
    {
        for (unsigned j = startPos[1]; j <= endPos[1]; ++j)
        {
            for (unsigned k = startPos[2]; k <= endPos[2]; ++k)
            {
                if (routeMgr->check3dOverflow(i, j, k) == GRID_OVERFLOW)
                {
                    ++OVCNT;
                }
                else if (routeMgr->check3dOverflow(i, j, k) == GRID_FULL_CAP)
                {
                    ++FULLCNT;
                }
            }
        }
    }
#ifdef DEBUG
    if (OVCNT)
    {
        cout << OVCNT << " grids in ";
        print();
        cout << " is overflown!\n";
    }
    if (FULLCNT)
    {
        cout << FULLCNT << " grids in ";
        print();
        cout << " is full!\n";
    }
#endif
    return OVCNT;
}

/******************************/
/* class Net member functions */
/******************************/
unsigned Net::getWirelength()
{
    return routeMgr->evaluateWireLen(this);
}

void Net::reduceSeg()
{
    vector<Segment*> toDel;
    auto it = _netSegs.begin();
    while (it != _netSegs.end())
    {
        if ((*it)->isZero() || !(*it)->isValid())
        {
            toDel.push_back(*it);
            it = _netSegs.erase(it);
        } else {
            ++it;
        }
    }
    for (auto seg : toDel)
    {
        delete seg;
    }
}

void Net::printSummary() const
{
    cout << "N" << _netId << endl;
    cout << "MinLayerConstr " << _minLayCons << endl;
    cout << "Routable " << _routable << "\n";
    printPinSet();
    printAssoCellInst();
    printAllSeg();
}

void Net::printPinSet() const
{
    for (auto it = _pinSet.begin(); it != _pinSet.end(); ++it)
    {
        auto a = *it;
        cout << a.first << " " << a.second << endl;
    }
}

set<PinPair>
Net::sortPinSet()
{
    if (_pinSet.empty())
    {
        return _pinSet;
    }
    set<PinPair> sorted;
    set<PinPair> connected;
    sorted.insert(*_pinSet.begin());
    connected.insert(*_pinSet.begin());
    auto pin = *_pinSet.begin();
    while (connected.size() != _pinSet.size())
    {
        Pos pos1 = routeMgr->getPinPos(pin);
        int lay1 = routeMgr->getPinLay(pin);
        int bestDist = INT32_MAX;
        PinPair bestPin;
        for (auto pin_2 : _pinSet)
        {
            if (connected.find(pin_2) == connected.end())
            {
                Pos pos2 = routeMgr->getPinPos(pin_2);
                int lay2 = routeMgr->getPinLay(pin_2);
                int curDist = abs(int(pos1.first) - int(pos2.first)) + abs(int(pos1.second) - int(pos2.second)) + abs(int(lay1) - int(lay2));
                if (curDist < bestDist)
                {
                    bestDist = curDist;
                    bestPin = pin_2;
                }
            }
        }
        sorted.insert(bestPin);
        connected.insert(bestPin);
        assert(pin != bestPin);
        pin = bestPin;
    }

    return sorted;
}

RouteExecStatus
Net::layerAssign()
{
    addPinDemand();
    for (auto seg : _netSegs)
    {
        if (seg->checkDir() == DIR_H)
        {
            assignH(seg);
        }
        else if (seg->checkDir() == DIR_V)
        {
            assignV(seg);
        }
        else if (seg->checkDir() == DIR_Z)
        {
            assignZ(seg);
        }
        seg->isValid();
    }
    checkOverflow();
    return ROUTE_EXEC_DONE;
}

void Net::addPinDemand()
{
}

void Net::removePinDemand()
{
}

void Net::assignH(Segment *)
{
}

void Net::assignV(Segment *)
{
}

void Net::assignZ(Segment *)
{
}

bool Net::checkOverflow()
{
    bool isOV = false;
    set<Layer *> alpha;
    routeMgr->passGrid(this, alpha);
    for (auto &grid : alpha)
    {
        if (grid->checkOverflow() == GRID_OVERFLOW)
        {
#ifdef DEBUG
            cout << "Net " << _netId << " causes overflow!\n";
#endif
            isOV = true;
            break;
        }
    }
    return isOV;
}

void Net::printAllSeg() const
{
    for (unsigned i = 0; i < _netSegs.size(); ++i)
    {
        _netSegs[i]->print();
        cout << " N" << _netId << endl;
    }
}

void Net::printAllSeg(ostream &outfile) const
{
    for (unsigned i = 0; i < _netSegs.size(); ++i)
    {
        _netSegs[i]->print(outfile);
        outfile << " N" << _netId << endl;
    }
}

void Net::ripUp()
{
    for (auto seg : _netSegs)
    {
        delete seg;
    }
    _netSegs.clear();
}

void Net::initAssoCellInst()
{
    std::set<PinPair>::iterator ite = _pinSet.begin();
    for (unsigned i = 0; i < _pinSet.size(); ++i)
    {
        //++_assoCellInst[ite->first-1];
        std::pair<std::map<unsigned, unsigned>::iterator, bool> ret;
        ret = _assoCellInstMap.insert(pair<unsigned, unsigned>(ite->first, 1));
        if (ret.second == false)
        {
            ++ret.first->second;
        }
        ++ite;
    }
}

void Net::avgPinLayer()
{
    double totalPinLayer = 0;
    double pinCnt = 0;
    for (auto &p : _pinSet)
    {
        totalPinLayer += double(p.second);
        ++pinCnt;
    }
    _avgPinLayer = totalPinLayer / pinCnt;
#ifdef DEBUG
    cout << "AvgPinLayer of N" << _netId << " is "
         << _avgPinLayer << "\n";
#endif
}

void Net::printAssoCellInst() const
{
    auto ite = _assoCellInstMap.begin();
    for (unsigned i = 0; i < _assoCellInstMap.size(); ++i)
    {
        cout << "CellInst " << ite->first << " has " << ite->second << " associated pins.\n";
        ++ite;
    }
}

bool Net::findVCand(vector<int> &candsV)
{
    /* Find layer candidates for layer assignment */
    int maxLayer = routeMgr->getLayerCnt();
    int minLayer = _minLayCons ? _minLayCons : 1;
    int minV = minLayer + (minLayer % 2);
    for (int i = minV; i <= maxLayer; i += 2)
    {
        candsV.push_back(i);
    }
    return !candsV.empty();
}

bool Net::findHCand(vector<int> &candsH)
{
    /* Find layer candidates for layer assignment */
    int maxLayer = routeMgr->getLayerCnt();
    int minLayer = _minLayCons ? _minLayCons : 1;
    int minH = minLayer + (1 - minLayer % 2);
    for (int i = minH; i <= maxLayer; i += 2)
    {
        candsH.push_back(i);
    }
    return !candsH.empty();
}

/**********************************/
/* class NetRank member functions */
/**********************************/
void NetRank::init()
{
    for (auto net : routeMgr->_netList)
    {
        set<Layer *> alpha;
        routeMgr->passGrid(net, alpha);
        unsigned WL = alpha.size();
        PinPair newpair = PinPair(net->_netId, WL);
        NetWLpairs.push_back(newpair);
    }
    ::sort(NetWLpairs.begin(), NetWLpairs.end(), CompareWL);
}

void NetRank::update()
{
    NetWLpairs.clear();
    init();
    // TODO: less memory cost
}

vector<unsigned>
NetRank::getTopTen() const
{
    vector<unsigned> nets;
    if (NetWLpairs.size() < 30)
    {
        for (auto &nwPair : NetWLpairs)
        {
            nets.push_back(nwPair.first);
        }
    }
    else
    {
        int cnt = 0;
        for (auto &nwPair : NetWLpairs)
        {
            if (cnt > 30)
            {
                break;
            }
            nets.push_back(nwPair.first);
            ++cnt;
        }
    }
    return nets;
}

void NetRank::showTopTen() const
{
    if (NetWLpairs.size() < 10)
    {
        for (auto &nwPair : NetWLpairs)
        {
            cout << "Net " << nwPair.first << " : "
                 << nwPair.second << "\n";
        }
    }
    else
    {
        int cnt = 0;
        for (auto &nwPair : NetWLpairs)
        {
            if (cnt > 10)
            {
                break;
            }
            cout << "Net " << nwPair.first << " : "
                 << nwPair.second << "\n";
            ++cnt;
        }
    }
}
