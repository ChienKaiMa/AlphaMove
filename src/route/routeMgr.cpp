/****************************************************************************
  FileName     [ routeMgr.cpp ]
  PackageName  [ route ]
  Synopsis     [ Define route manager functions ]
  Author       [ Chien-Kai Ma, Kai-Chun Chang, Yu-Wei Fan ]
  Copyright    [ Copyleft(c) 2020-present NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include <math.h>
#include "routeMgr.h"
#include "routeNet.h"
#include "../util/util.h"

using namespace std;

// TODO: Implement member functions for class RouteMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
RouteMgr* routeMgr = 0;

/**************************************************************/
/*   class RouteMgr member functions for circuit construction   */
/**************************************************************/
bool
RouteMgr::readCircuit(const string& fileName)
{
    ifstream ifs(fileName.c_str());
    if (!ifs) {
        cerr << "Error: \"" << fileName << "\" does not exist!!" << endl;
        return false;
    }

    // Buffer variables
    // TODO: increase readability
    uint tmpCnt;
    uint tmpCnt1;
    uint tmpCnt2;
    uint tmpCnt3;
    int supply; // To save defaultSupply
    string buffer;
    bool tempQ;
    unsigned bndCoord[4];

    // Start parsing
    ifs >> buffer; // MaxCellMove
    ifs >> _maxMoveCnt; 
    ifs >> buffer; // GGridBoundaryIdx
    for(unsigned i=0; i<4; ++i)
        ifs >> bndCoord[i];
    Ggrid::setBoundary(bndCoord[0], bndCoord[1], bndCoord[2], bndCoord[3]);

    // Layers and supply
    ifs >> buffer; // NumLayers
    ifs >> tmpCnt; // LayerCount
    for(unsigned i=0; i<tmpCnt; ++i)
    {
        // Layer* lay = new Layer;
        ifs >> buffer; // Lay
        ifs >> buffer; // layerName
        ifs >> buffer; // Idx
        ifs >> buffer; // RoutingDirection
        ifs >> supply; // defaultSupplyOfOneGGrid
        _laySupply.push_back(supply);
    }
    ifs >> buffer; // NumNonDefaultSupplyGGrid or NumMasterCell
    if (buffer == "NumNonDefaultSupplyGGrid")
    {
        ifs >> tmpCnt; // nonDefaultSupplyGGridCount
        for(unsigned i=0; i<tmpCnt; ++i)
        {
            // if yes return offset if no return 0
            ifs >> tmpCnt1; // rowIdx
            ifs >> tmpCnt2; // colIdx
            ifs >> tmpCnt3; // LayIdx
            MCTri dent(tmpCnt1, tmpCnt2, tmpCnt3);
            int supply;
            ifs >> buffer; // incrOrDecrValue
            if (buffer[0] == '+') {
                tempQ = true;
            } else {
                tempQ = false;
            }
            if (myStr2Int(buffer.substr(1), supply)) {
                if (tempQ == true) {
                    pair<MCTri, unsigned> sGd(dent, supply);
                    this->_nonDefaultSupply.insert(sGd);
                } else {
                    pair<MCTri, unsigned> sGd(dent, -supply);
                    this->_nonDefaultSupply.insert(sGd);
                }
            }
        }
        ifs >> buffer; // NumMasterCell
    }

    genGridList();
    initSupply();

    // MasterCell and demand
    ifs >> tmpCnt; // masterCellCount
    for(unsigned i=1; i<tmpCnt+1; ++i)
    {
        ifs >> buffer; // MasterCell
        ifs >> buffer; // masterCellName
        ifs >> tmpCnt1; // pinCount
        ifs >> tmpCnt2; // blockageCount
        MC* Laren = new MC(i, tmpCnt1, tmpCnt2);
        int layer;
        for(unsigned j=1; j<tmpCnt1+1; ++j)
        {
            ifs >> buffer; // Pin
            ifs >> buffer; // pinName
            ifs >> buffer; // pinLayer
            if (myStr2Int(buffer.substr(1), layer))
            {
                Laren->addPin(j, layer);
            }
        }
        for(unsigned j=1; j<tmpCnt2+1; ++j)
        {
            ifs >> buffer; // Blkg
            ifs >> buffer; // blockageName
            ifs >> buffer; // blockageLayer
            ifs >> tmpCnt3; // demand
            if (myStr2Int(buffer.substr(1), layer))
            {
                Laren->addBlkg(j, layer, tmpCnt3);
            }
        }
        _mcList.push_back(Laren);
    }
    ifs >> buffer; // NumNeighborCellExtraDemand or NumCellInst
    if (buffer == "NumNeighborCellExtraDemand")
    {
        
        ifs >> tmpCnt; // count
        for(unsigned i=1; i<tmpCnt+1; ++i)
        {
            ifs >> buffer; // sameGGrid or adjHGGrid
            tempQ = buffer == "sameGGrid" ? true : false;
            ifs >> buffer; // masterCellName1
            tmpCnt1 = stoi(buffer.substr(2));
            ifs >> buffer; // masterCellName2
            tmpCnt2 = stoi(buffer.substr(2));
            ifs >> buffer; // layerName
            tmpCnt3 = stoi(buffer.substr(1));
            MCTri dent(tmpCnt1, tmpCnt2, tmpCnt3);
            ifs >> tmpCnt1; // demand

            pair<MCTri, unsigned> sGd(dent, tmpCnt1);
            if (tempQ == true) {
                this->_sameGridDemand.insert(sGd);
            } else {
                this->_adjHGridDemand.insert(sGd);
            }
        }
        ifs >> buffer; // NumCellInst
    }
    // CellInst and initial placement
    ifs >> tmpCnt; // cellInstCount
    int mcNum;
    for(unsigned i=0; i<tmpCnt; ++i)
    {
        ifs >> buffer; // CellInst
        ifs >> buffer; // instName
        ifs >> buffer; // masterCellName
        myStr2Int(buffer.substr(2), mcNum);
        ifs >> tmpCnt1; // gGridRowIdx
        ifs >> tmpCnt2; // gGridColIdx
        ifs >> buffer; // movableCstr
        tempQ = (buffer == "Movable") ? true : false;
        // Ggrid* cool = new Ggrid(Pos(tmpCnt1, tmpCnt2));
        CellInst* love = new CellInst(i+1, _gridList[tmpCnt1-1][tmpCnt2-1], this->_mcList[mcNum-1], tempQ);
        _instList.push_back(love);

        OutputCell cell(i+1, tmpCnt1, tmpCnt2);
        _initCells.push_back(cell);
    }
    ifs >> buffer; // NumNets
    ifs >> tmpCnt; // netCount
    for(unsigned i=0; i<tmpCnt; ++i)
    {
        ifs >> buffer; // Net
        ifs >> buffer; // netName
        ifs >> tmpCnt1; // numPins
        ifs >> buffer; // minRoutingLayConstraint
        Net* brook;
        if (buffer != "NoCstr") {
            int layCons = stoi(buffer.substr(1));
            brook = new Net(i+1, layCons);
        } else {
            brook = new Net(i+1, 0); // [Important] NoCstr
        }
        for(unsigned j=0; j<tmpCnt1; ++j)
        {
            ifs >> buffer; // Pin
            ifs >> buffer; // instName/masterPinName
            tmpCnt2 = buffer.find_first_of('/');
            string instName = buffer.substr(0, tmpCnt2);
            string masterPinName = buffer.substr(tmpCnt2);
            int inst = stoi(instName.substr(1));
            int mstrPin = stoi(masterPinName.substr(2));
            brook->addPin(PinPair(inst, mstrPin));
            // TODO: genAssoNet
            _instList[inst-1]->assoNet.push_back(i+1);
        }
        _netList.push_back(brook);
    }

    // Initial routing data
    ifs >> buffer; // NumRoutes
    ifs >> _initTotalSegNum; // routeSegmentCount
    for(unsigned i=0; i<_initTotalSegNum; ++i)
    {
        Segment* damn = new Segment();
        for(unsigned j=0; j<3; ++j)
            ifs >> damn->startPos[j];
        for(unsigned j=0; j<3; ++j)
            ifs >> damn->endPos[j];
        ifs >> buffer; // netName
        int netIdx = stoi(buffer.substr(1));
        _netList[netIdx-1]->addSeg(damn);

        OutputSeg seg(*damn, (unsigned)netIdx);
        _initRouteSegs.push_back(seg);
        _bestRouteSegs.push_back(seg);
    }
    
    for(auto& m : _netList){
        add2DDemand(m);
    }

    for(auto& m : _netList){
        for(unsigned i=0;i<_instList.size();++i)
            m->_assoCellInst.push_back(0);
        m->initAssoCellInst();
        //cout << "Net " << m->_netId << "\n";
        //m->printAssoCellInst();
    }

    return true;
}

void
RouteMgr::writeCircuit(ostream& outfile) const
{
    outfile << "NumMovedCellInst " << _bestMovedCells.size() << "\n";
    for(auto m : _bestMovedCells)
    {
        outfile << "CellInst C" << get<0>(m) << " " << get<1>(m) << " " << get<2>(m) << "\n";
    }
    // TODO: output routes
    outfile << "NumRoutes " << _bestRouteSegs.size() << "\n";
    for (unsigned i=0; i<_bestRouteSegs.size()-1; ++i) {
        outfile << _bestRouteSegs[i].first.startPos[0] << " " << _bestRouteSegs[i].first.startPos[1] << " " << _bestRouteSegs[i].first.startPos[2] << " "
                << _bestRouteSegs[i].first.endPos[0]   << " " << _bestRouteSegs[i].first.endPos[1]   << " " << _bestRouteSegs[i].first.endPos[2]   << " N"
                << _bestRouteSegs[i].second << "\n";
    }
    outfile << _bestRouteSegs[_bestRouteSegs.size()-1].first.startPos[0] << " " << _bestRouteSegs[_bestRouteSegs.size()-1].first.startPos[1] << " " << _bestRouteSegs[_bestRouteSegs.size()-1].first.startPos[2] << " "
            << _bestRouteSegs[_bestRouteSegs.size()-1].first.endPos[0]   << " " << _bestRouteSegs[_bestRouteSegs.size()-1].first.endPos[1]   << " " << _bestRouteSegs[_bestRouteSegs.size()-1].first.endPos[2]   << " N"
            << _bestRouteSegs[_bestRouteSegs.size()-1].second;
}

void
RouteMgr::genGridList()
{
    for (unsigned i=1; i<=Ggrid::rEnd; ++i) {
        vector<Ggrid*> bar;
        for (unsigned j=1; j<=Ggrid::cEnd; ++j) {
            Ggrid* g = new Ggrid(Pos(i, j));
            bar.push_back(g);
        }
        _gridList.push_back(bar);
    }
}

void
RouteMgr::initSupply()
{
    int total_default_supply = 0;
    for (auto const m : _laySupply) {
        total_default_supply += m;
    }
    int total_supply;

    for (unsigned i=1; i<=Ggrid::rEnd; ++i) {
        for (unsigned j=1; j<=Ggrid::cEnd; ++j) {
            total_supply = total_default_supply;
            for (unsigned k=1; k<=_laySupply.size(); ++k)
            {
                MCTri good = MCTri(i, j, k);
                auto yeah = _nonDefaultSupply.find(good);
                if (yeah != _nonDefaultSupply.cend())
                {
                    total_supply += yeah->second;
                }
            }
            _gridList[i-1][j-1]->set2dSupply(total_supply);
        }
    }
    /*
    Pseudo code
    for i=1 to gridList.length
        gridList[i]._2dSupply = total_default_supply;
        for i=1 to _layerSupply.length
            use unorder_map to add/sub nondefault supply
    */
}

void
RouteMgr::add2DDemand(Net* net) //Initialize after each route
{
    unsigned availale_layer = _laySupply.size() - net->getMinLayCons() + 1;
    double constraint = ((double)_laySupply.size() / (double)availale_layer);
    //cout << "Net " << net->_netId << "\n";
    //cout << "rEnd: " << Ggrid::rEnd << " cEnd: " << Ggrid::cEnd << "\n";
    //cout << "NetSeg.size()" << net->_netSegs.size() << endl;
    if(net->_netSegs.size() == 0) {
        //cout << "Net" << net->_netId << " is empty!\n";
    }
    for(auto& s : net->_netSegs) {
        if(s->startPos[2] == s->endPos[2]){
            if(s->startPos[0] != s->endPos[0]){
                int max = s->endPos[0];
                int min = s->startPos[0];
                if(s->startPos[0] > s->endPos[0]){
                    max = s->startPos[0];
                    min = s->endPos[0];
                }
                for(int j=min;j<=max;++j){
                    _gridList[j-1][(s->startPos[1])-1]->update2dDemand(constraint);
                }
            }
            else if(s->startPos[1] != s->endPos[1]){
                int max = s->endPos[1];
                int min = s->startPos[1];
                if(s->startPos[1] > s->endPos[1]){
                    max = s->startPos[1];
                    min = s->endPos[1];
                }
                for(int j=min; j<=max; ++j){
                    //cout << "Net " << net->_netId << " " << s->startPos[0]-1 << " " << j-1 << "\n";
                    _gridList[(s->startPos[0])-1][j-1]->update2dDemand(constraint);
                }
            }
        }
        else{
            int num_of_layer = abs((int)(s->startPos[2]) - (int)(s->endPos[2]));
            _gridList[(s->startPos[0])-1][(s->startPos[1])-1]->update2dDemand(num_of_layer*constraint);
        }
    }
    /*Psuedo Code
    unsigned available_layer = _layerSupply.length - net->_minLayCons;
    double constraint = _layerSupply.length/available_layer;

    go through net, for every passing grid g, call g.updateDemand(constraint)
    */
}

void 
RouteMgr::remove2DDemand(Net* net) //before each route
{
    unsigned availale_layer = _laySupply.size() - net->getMinLayCons() + 1;
    double constraint = (double) (_laySupply.size() / availale_layer);

    for(auto& s : net->_netSegs) {
        if(s->startPos[2] == s->endPos[2]){
            if(s->startPos[0] != s->endPos[0]){
                int max = s->endPos[0];
                int min = s->startPos[0];
                if(s->startPos[0] > s->endPos[0]){
                    max = s->startPos[0];
                    min = s->endPos[0];
                }
                for(int j=min;j<=max;++j){
                    _gridList[j-1][(s->startPos[1])-1]->update2dDemand(-constraint);
                }
            }
            else if(s->startPos[1] != s->endPos[1]){
                int max = s->endPos[1];
                int min = s->startPos[1];
                if(s->startPos[1] > s->endPos[1]){
                    max = s->startPos[1];
                    min = s->endPos[1];
                }
                for(int j=min;j<=max;++j){
                    _gridList[(s->startPos[0])-1][j-1]->update2dDemand(-constraint);
                }
            }
        }
        else{
            int num_of_layer = abs((int)(s->startPos[2]) - (int)(s->endPos[2]));
            _gridList[(s->startPos[0])-1][(s->startPos[1])-1]->update2dDemand(-num_of_layer*constraint);
        }
    }
    /*Psuedo Code
    unsigned available_layer = _layerSupply.length - net->_minLayCons;
    double constraint = _layerSupply.length/available_layer;

    go through net, for every passing grid g, call g.updateDemand(-constraint)
    */
}

unsigned 
RouteMgr::evaluateWireLen() const{
    unsigned newWL = 0;
    for (auto n : _netList){
        for(auto seg: n->_netSegs){
            newWL+=seg->getWL();
        }
    }
    return newWL;
}
