/****************************************************************************
  FileName     [ routePrint.cpp ]
  PackageName  [ route ]
  Synopsis     [ Define route manager printing functions ]
  Author       [ Chien-Kai Ma, Kai-Chun Chang, Yu-Wei Fan ]
  Copyright    [ Copyleft(c) 2020-present NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "routeMgr.h"
#include "../util/util.h"

using namespace std;

void
RouteMgr::printInputSummary() const
{
    cout << "MaxCellMove" << setw(8) << _maxMoveCnt << endl;
    cout << "Boundary" << setw(5) << Ggrid::rBeg << " " << Ggrid::cBeg << " " << Ggrid::rEnd
    << " " << Ggrid::cEnd << endl;
    cout << "Total layer" << setw(8) << _laySupply.size() << endl;
    cout << "Total MC" << setw(11) << _mcList.size() << endl;
    cout << "CellInst" << setw(11) << _instList.size() << endl;
    cout << "Net" << setw(16) << _netList.size() << endl;
    cout << "OrigRoute" << setw(10) << _initTotalSegNum << endl;
    cout << "InitWL" << setw(13) << _initTotalWL << "\n";
    // TODO: Support more information
}

void
RouteMgr::printRouteSummary() const
{
    cout << "MaxCellMove" << setw(8) << _maxMoveCnt << endl;
    cout << "CurrentMove" << setw(8) << getCurMoveCnt() << endl;
    cout << "InitWL" << setw(13) << _initTotalWL << "\n";
    cout << "bestWL" << setw(13) << _bestTotalWL << "\n";
}


void
RouteMgr::printNetlist() const
{
    for(unsigned i=0; i<_netList.size(); ++i)
    {
        _netList[i]->printSummary();
        cout << endl;
    }
}

bool
RouteMgr::printNet(int idx) const
{
    if (size_t(idx) > _netList.size() || idx <= 0) {
        return false;
    } else {
        cout << endl;
        _netList[idx-1]->printSummary();
        return true;
    }
}


bool
RouteMgr::printAssoInst(unsigned netId) const
{
    if (netId > _netList.size() || netId <= 0) {
        return false;
    } else {
        cout << endl;
        _netList[netId-1]->printAssoCellInst();
        return true;
    }
}

void
RouteMgr::printMCList() const
{
    for(unsigned i=0; i<_mcList.size(); ++i)
    {
        cout << endl;
        _mcList[i]->printMC();
    }
}

bool
RouteMgr::printMC(unsigned mcId) const
{
    if (mcId > _mcList.size() || mcId <= 0) {
        return false;
    } else {
        cout << endl;
        _mcList[mcId-1]->printMC();
        return true;
    }
}

void
RouteMgr::printLaySupply() const
{
    for(unsigned i=0; i<_laySupply.size(); ++i)
    {
        cout << _laySupply[i] << endl;
    }
}

void
RouteMgr::print2DDemand() const
{
    for (auto const rows : _gridList)
    {
        for (auto const g : rows)
        {
            cout << left << setw(7) << setprecision(3) << g->get2dDemand();
        }
        cout << endl;
    }
}

void
RouteMgr::printExtraDemand() const
{
    for(auto const& pair : _sameGridDemand)
    {
        cout << endl;
        cout << "sameGGrid MC" << pair.first.idx1 << " MC" << pair.first.idx2
        << " M" << pair.first.layNum << endl;
        cout << "demand " << pair.second << endl;
    }
    for(auto const& pair : _adjHGridDemand)
    {
        cout << endl;
        cout << "adjHGGrid MC" << pair.first.idx1 << " MC" << pair.first.idx2
        << " M" << pair.first.layNum << endl;
        cout << "demand " << pair.second << endl;
    }
}

void
RouteMgr::printGridDemand() const
{
    for (auto const rows : _gridList)
    {
        for (auto const g : rows)
        {
            g->printDemand();
        }
        cout << endl;
    }
}

void
RouteMgr::printNonDefaultSupply() const
{
    for(auto const& pair : _nonDefaultSupply)
    {
        cout << endl;
        cout << pair.first.idx1 << " " << pair.first.idx2
        << " " << pair.first.layNum << endl;
        cout << "supply " << pair.second << endl;
    }
}

void
RouteMgr::print2DSupply() const
{
    for (auto const rows : _gridList)
    {
        for (auto const g : rows)
        {
            cout << left << setw(5) << g->get2dSupply();
        }
        cout << endl;
    }
}

void
RouteMgr::print2DCongestion() const
{
    for (auto const rows : _gridList)
    {
        for (auto const g : rows)
        {
            cout << left << setw(8) << setprecision(3) << g->get2dCongestion();
        }
        cout << endl;
    }
}

void
RouteMgr::printCellSummary() const
{
    cout << "\nMaxCellMove " << setw(12) << _maxMoveCnt << "\n";
    cout << "CurrentMove " << setw(12) << getCurMoveCnt() << "\n";
    cout << "Total CellInst " << setw(9) << _instList.size() << "\n";
    cout << "Movable CellInst " << setw(7);
    unsigned mvbCnt = 0;
    for (auto& ci : _instList) { if (ci->is_movable()) { ++mvbCnt; } }
    cout << mvbCnt << "\n";
    // TODO: stacked cellInst, cellInst with most pins, most used masterCell...
}

void
RouteMgr::printCellInst() const
{
    for(unsigned i=0; i<_instList.size(); ++i)
    {
        cout << endl;
        _instList[i]->printCell();
    }
}

bool
RouteMgr::printCellInst(unsigned instIdx) const
{
    if (instIdx > _instList.size() || instIdx <= 0) {
        return false;
    } else {
        cout << endl;
        _instList[instIdx-1]->printCell();
        return true;
    }
}

void
RouteMgr::printAssoNet() const
{
    for (auto& m : _instList)
    {
        m->printAssoNet();
    }
}

bool
RouteMgr::printAssoNet(unsigned instIdx) const
{
    if (instIdx > _instList.size() || instIdx <= 0) {
        return false;
    } else {
        cout << endl;
        _instList[instIdx-1]->printAssoNet();
        return true;
    }
}

void
RouteMgr::printInitSegs() const
{
    for (unsigned i=0; i<_netList.size(); ++i) {
        cout << endl;
        cout << "N" << i+1 << endl;
        _netList[i]->printAllSeg();
    }
}
