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
RouteMgr::printRouteSummary() const
{
    cout << "MaxCellMove" << setw(8) << maxMoveCnt << endl;
    // TODO
    cout << "CurrentMove" << setw(8) << 0 << endl;
    cout << "Boundary" << setw(5) << Ggrid::xMin << " " << Ggrid::yMin << " " << Ggrid::xMax
    << " " << Ggrid::yMax << endl;
    cout << "Total layer" << setw(8) << _laySupply.size() << endl;
    cout << "Total MC" << setw(11) << _mcList.size() << endl;
    cout << "CellInst" << setw(11) << _instList.size() << endl;
    cout << "Net" << setw(16) << _netList.size() << endl;
    cout << "OrigRoute" << setw(10) << initTotalWL << endl;
    // TODO: Support more information
    // cout << "CurrRoute" << setw(10) << numRoute << endl;
}

void
RouteMgr::printNetlist() const
{
    for(unsigned i=0; i<_netList.size(); ++i)
    {
        cout << endl;
        cout << "MinLayerConstr " << _netList[i]->getMinLayCons() << endl;
        _netList[i]->printPinSet();
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

void
RouteMgr::printLaySupply() const
{
    for(unsigned i=0; i<_laySupply.size(); ++i)
    {
        cout << _laySupply[i] << endl;
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
RouteMgr::printCellInst() const
{
    for(unsigned i=0; i<_instList.size(); ++i)
    {
        cout << endl;
        _instList[i]->printCell();
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
