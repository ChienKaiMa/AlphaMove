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
RouteMgr* routeMgr;

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
    _laySupply.resize(tmpCnt);
    for(unsigned i=0; i<tmpCnt; ++i)
    {
        // Layer* lay = new Layer;
        ifs >> buffer; // Lay
        ifs >> buffer; // layerName
        ifs >> buffer; // Idx
        ifs >> buffer; // RoutingDirection
        ifs >> supply; // defaultSupplyOfOneGGrid
        _laySupply[i] = supply;
    }
    myUsage.report(true, true);cout << "\n";
    ifs >> buffer; // NumNonDefaultSupplyGGrid or NumMasterCell
    cout << "Reading nondefaultSupply\n";
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
    myUsage.report(true, true);cout << "\n";
    cout << "genGridList\n";
    genGridList();
    myUsage.report(true, true);cout << "\n";
    cout << "Init 2D supply\n";
    init2DSupply();
    myUsage.report(true, true);cout << "\n";
    cout << "Init 3D supply\n";
    init3DSupply();
    myUsage.report(true, true);cout << "\n";
    cout << "Reading masterCell and demand\n";
    // MasterCell and demand
    ifs >> tmpCnt; // masterCellCount
    _mcList.resize(tmpCnt);

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
        _mcList[i-1] = Laren;
    }
    myUsage.report(true, true);cout << "\n";
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
            MCTri dent2(tmpCnt2, tmpCnt1, tmpCnt3);
            ifs >> tmpCnt1; // demand

            pair<MCTri, unsigned> sGd(dent, tmpCnt1);
            pair<MCTri, unsigned> sGd2(dent2, tmpCnt1);
            if (tempQ == true) {
                this->_sameGridDemand.insert(sGd);
                this->_sameGridDemand.insert(sGd2);
            } else {
                this->_adjHGridDemand.insert(sGd);
                this->_adjHGridDemand.insert(sGd2);
            }
        }
        ifs >> buffer; // NumCellInst
    }
    myUsage.report(true, true);cout << "\n";
    cout << "Reading CellInst...\n";
    // CellInst and initial placement
    ifs >> tmpCnt; // cellInstCount
    int mcNum;
    _instList.resize(tmpCnt);

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
        _instList[i] = love;

        OutputCell cell(i+1, tmpCnt1, tmpCnt2);
        _initCells.push_back(cell);
    }
    myUsage.report(true, true);cout << "\n";
    cout << "Reading nets\n";
    ifs >> buffer; // NumNets
    ifs >> tmpCnt; // netCount
    _netList.resize(tmpCnt);

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
        _netList[i] = brook;
        brook->avgPinLayer();
    }
    myUsage.report(true, true);cout << "\n";

    cout << "Reading init route\n";
    // Initial routing data
    ifs >> buffer; // NumRoutes
    ifs >> _initTotalSegNum; // routeSegmentCount
    //_initRouteSegs.resize(_initTotalSegNum);
    _bestRouteSegs.resize(_initTotalSegNum);

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
        //_initRouteSegs[i] = seg;
        _bestRouteSegs[i] = seg;
    }
    myUsage.report(true, true); cout << "\n";

    cout << "initialize cell instance list of gGrids\n";
    initCellInstList();
    /*for(unsigned i=Ggrid::rBeg-1;i<Ggrid::rEnd;++i){
        for(unsigned j=Ggrid::cBeg-1;j<Ggrid::cEnd;++j){
            cout << "gGid(" << i+1 << "," << j+1 << ") has cellInst";
            for(unsigned k=0;k<_gridList[i][j]->cellInstList.size();++k){
                cout << _gridList[i][j]->cellInstList[k]->getId() << " ";
            }
            cout << "\n";
        }
    }*/
    myUsage.report(true, true);cout << "\n";
    cout << "adding 2D demand\n";
    for(auto& m : _netList){
        add2DDemand(m);
    }
    myUsage.report(true, true);cout << "\n";
    cout << "adding 3D demand\n";
    for(auto& m : _netList){
        add3DDemand(m);
    }
    myUsage.report(true, true);cout << "\n";
    cout << "adding 2D Blkg demand\n";
    for(auto& m : _instList){
        add2DBlkDemand(m);
    }
    myUsage.report(true, true);cout << "\n";
    cout << "adding 3D Blkg demand\n";
    for(auto& m : _instList){
        add3DBlkDemand(m);
    }
    myUsage.report(true, true);cout << "\n";
    cout << "adding sameGGrid/adjHGrid demand\n";
    initNeighborDemand();
    myUsage.report(true, true);cout << "\n";
    cout << "initialize associated cell instances of nets\n";
    for(auto& m : _netList){
        m->initAssoCellInst();
        
        //cout << "Net " << m->_netId << "\n";
        //m->printAssoCellInst();
    }
    myUsage.report(true, true);cout << "\n";
    _netRank = new NetRank;
    _netRank->init();
    myUsage.report(true, true);cout << "\n";
    _netRank->showTopTen();
    _initTotalWL = evaluateWireLen();
    myUsage.report(true, true);cout << "\n";
    _bestTotalWL = _initTotalWL;
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
RouteMgr::writeDemand(ostream& outfile) const{
    outfile << "row col lay supply demand\n";
    //cout << "row col lay supply demand\n";
    for(unsigned k=1; k<=_laySupply.size(); ++k){
        for(unsigned i=Ggrid::rBeg; i<=Ggrid::rEnd; ++i){
            for(unsigned j=Ggrid::cBeg; j<=Ggrid::cEnd; ++j){
                //cout << i << " " << j << " " << k << " " << (*_gridList[i-1][j-1])[k]->getSupply() << " " << (*_gridList[i-1][j-1])[k]->getDemand() << "\n";
                outfile << i << " " << j << " " << k << " " << (*_gridList[i-1][j-1])[k]->getSupply() << " " << (*_gridList[i-1][j-1])[k]->getDemand() << "\n";
            }
        }
    }
}

void
RouteMgr::storeBestResult(){
    _bestMovedCells.resize(0);
    std::set<CellInst*>::iterator ite = _curMovedSet.begin();
    for(unsigned i=0;i<_curMovedSet.size();++i){
        OutputCell cell((*ite)->getId(),(*ite)->getPos().first,(*ite)->getPos().second);
        _bestMovedCells.push_back(cell);
        ++ite;
    }

    _bestRouteSegs.resize(0);
    for(unsigned i=0;i<_netList.size();++i){
        for(unsigned j=0;j<_netList[i]->_netSegs.size();++j){
            OutputSeg seg(*(_netList[i]->_netSegs[j]),i+1);
            _bestRouteSegs.push_back(seg);
        }
    }
}

void
RouteMgr::genGridList()
{
    _gridList.resize(Ggrid::rEnd, vector<Ggrid*>(Ggrid::cEnd));
    for (unsigned i=1; i<=Ggrid::rEnd; ++i) {
        //vector<Ggrid*> bar;
        for (unsigned j=1; j<=Ggrid::cEnd; ++j) {
            Ggrid* g = new Ggrid(Pos(i, j), _laySupply.size());
            _gridList[i-1][j-1] = g;
            //bar.push_back(g);
        }
        //_gridList.push_back(bar);
    }
}

void
RouteMgr::init2DSupply()
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
RouteMgr::init3DSupply()
{
    for (unsigned i=1; i<=Ggrid::rEnd; ++i) {
        for (unsigned j=1; j<=Ggrid::cEnd; ++j) {
            for (unsigned k=1; k<=_laySupply.size(); ++k) {
                Layer* grid = _gridList[i-1][j-1]->operator[](k);
                // Find grid supply
                unsigned total_supply = _laySupply[k-1];
                MCTri good = MCTri(i, j, k);
                auto yeah = _nonDefaultSupply.find(good);
                if (yeah != _nonDefaultSupply.cend())
                {
                    total_supply += yeah->second;
                }
                grid->setSupply(total_supply);
            }
        }
    }
    
}

void
RouteMgr::initCellInstList(){
    for(unsigned i=0;i<_instList.size();++i){
        Ggrid* g = _instList[i]->getGrid();
        g->cellInstList.push_back(_instList[i]);
    }
}

void
RouteMgr::initNeighborDemand(){
    //same gGrid demand
    for(unsigned i=Ggrid::rBeg-1;i<Ggrid::rEnd;++i){
        for(unsigned j=Ggrid::cBeg-1;j<Ggrid::cEnd;++j){
            map<unsigned,unsigned> mcMap; //key : id of MC; value : num of MC of the id in the Ggrid
            for(unsigned k=0;k<_gridList[i][j]->cellInstList.size();++k){
                pair<std::map<unsigned,unsigned>::iterator, bool> ret = mcMap.insert(pair<unsigned,unsigned>(_gridList[i][j]->cellInstList[k]->getMC()->_mcId,1));
                if(ret.second == false){
                    ++ret.first->second;
                }
            }
            std::map<unsigned,unsigned>::iterator ite = mcMap.begin();
            for(unsigned k=0;k<mcMap.size();++k){
                for(unsigned l=1;l<mcMap.size()-k;++l){
                    for(unsigned m=0;m<min((*ite).second,(*(next(ite,l))).second);++m){
                        addNeighborDemand(_mcList[(*ite).first-1],_mcList[(*(next(ite,l))).first-1],_gridList[i][j],0);
                    }
                }
                ++ite;
            }
        }
    }

    //adj. gGrid demand
    for(unsigned i=Ggrid::rBeg-1;i<Ggrid::rEnd;++i){
        for(unsigned j=Ggrid::cBeg;j<Ggrid::cEnd-1;++j){
            map<unsigned,unsigned> prevMcMap, curMcMap, nxtMcMap;
            for(unsigned k=0;k<_gridList[i][j]->cellInstList.size();++k){
                pair<std::map<unsigned,unsigned>::iterator, bool> ret = curMcMap.insert(pair<unsigned,unsigned>(_gridList[i][j]->cellInstList[k]->getMC()->_mcId,1));
                if(ret.second == false){
                    ++ret.first->second;
                }
            }
            for(unsigned k=0;k<_gridList[i][j-1]->cellInstList.size();++k){
                pair<std::map<unsigned,unsigned>::iterator, bool> ret = prevMcMap.insert(pair<unsigned,unsigned>(_gridList[i][j-1]->cellInstList[k]->getMC()->_mcId,1));
                if(ret.second == false){
                    ++ret.first->second;
                }
            }
            for(unsigned k=0;k<_gridList[i][j+1]->cellInstList.size();++k){
                pair<std::map<unsigned,unsigned>::iterator, bool> ret = nxtMcMap.insert(pair<unsigned,unsigned>(_gridList[i][j+1]->cellInstList[k]->getMC()->_mcId,1));
                if(ret.second == false){
                    ++ret.first->second;
                }
            }
            std::map<unsigned,unsigned>::iterator cur_ite = curMcMap.begin();
            for(unsigned k=0;k<curMcMap.size();++k){
                std::map<unsigned,unsigned>::iterator prev_ite = prevMcMap.begin();
                for(unsigned l=0;l<prevMcMap.size();++l){
                    for(unsigned m=0;m<min((*cur_ite).second,(*prev_ite).second);++m){
                        addNeighborDemand(_mcList[(*cur_ite).first-1],_mcList[(*prev_ite).first-1],_gridList[i][j],1);
                    }
                    ++prev_ite;
                }
                std::map<unsigned,unsigned>::iterator nxt_ite = nxtMcMap.begin();
                for(unsigned l=0;l<nxtMcMap.size();++l){
                    for(unsigned m=0;m<min((*cur_ite).second,(*nxt_ite).second);++m){
                        addNeighborDemand(_mcList[(*cur_ite).first-1],_mcList[(*nxt_ite).first-1],_gridList[i][j],1);
                    }
                    ++nxt_ite;
                }
                ++cur_ite;
            }
        }

        unsigned j = Ggrid::cBeg-1;
        map<unsigned,unsigned> curMcMap, nxtMcMap;
        for(unsigned k=0;k<_gridList[i][j]->cellInstList.size();++k){
            pair<std::map<unsigned,unsigned>::iterator, bool> ret = curMcMap.insert(pair<unsigned,unsigned>(_gridList[i][j]->cellInstList[k]->getMC()->_mcId,1));
            if(ret.second == false){
                ++ret.first->second;
            }
        }
        for(unsigned k=0;k<_gridList[i][j+1]->cellInstList.size();++k){
            pair<std::map<unsigned,unsigned>::iterator, bool> ret = nxtMcMap.insert(pair<unsigned,unsigned>(_gridList[i][j+1]->cellInstList[k]->getMC()->_mcId,1));
            if(ret.second == false){
                ++ret.first->second;
            }
        }
        std::map<unsigned,unsigned>::iterator cur_ite = curMcMap.begin();
        for(unsigned k=0;k<curMcMap.size();++k){
            std::map<unsigned,unsigned>::iterator nxt_ite = nxtMcMap.begin();
            for(unsigned l=0;l<nxtMcMap.size();++l){
                for(unsigned m=0;m<min((*cur_ite).second,(*nxt_ite).second);++m){
                    addNeighborDemand(_mcList[(*cur_ite).first-1],_mcList[(*nxt_ite).first-1],_gridList[i][j],1);
                }
                ++nxt_ite;
            }
            ++cur_ite;
        }

        j = Ggrid::cEnd-1;
        map<unsigned,unsigned> prevMcMap;
        curMcMap.clear();
        for(unsigned k=0;k<_gridList[i][j]->cellInstList.size();++k){
            pair<std::map<unsigned,unsigned>::iterator, bool> ret = curMcMap.insert(pair<unsigned,unsigned>(_gridList[i][j]->cellInstList[k]->getMC()->_mcId,1));
            if(ret.second == false){
                ++ret.first->second;
            }
        }
        for(unsigned k=0;k<_gridList[i][j-1]->cellInstList.size();++k){
            pair<std::map<unsigned,unsigned>::iterator, bool> ret = prevMcMap.insert(pair<unsigned,unsigned>(_gridList[i][j-1]->cellInstList[k]->getMC()->_mcId,1));
            if(ret.second == false){
                ++ret.first->second;
            }
        }
        cur_ite = curMcMap.begin();
        for(unsigned k=0;k<curMcMap.size();++k){
            std::map<unsigned,unsigned>::iterator prev_ite = prevMcMap.begin();
            for(unsigned l=0;l<prevMcMap.size();++l){
                for(unsigned m=0;m<min((*cur_ite).second,(*prev_ite).second);++m){
                    addNeighborDemand(_mcList[(*cur_ite).first-1],_mcList[(*prev_ite).first-1],_gridList[i][j],1);
                }
                ++prev_ite;
            }
            ++cur_ite;
        }
    }
}

void
RouteMgr::passGrid(Net* net, set<Layer*>& alpha) const
{
    if (net->_netSegs.empty()) {
        PinPair firstPin = *(net->_pinSet.begin());
        Pos pinPos = getPinPos(firstPin);
        int layer = getPinLay(firstPin);
        alpha.insert((*_gridList[pinPos.first-1][pinPos.second-1])[layer]);
    }
    for(auto& seg : net->_netSegs) {
        seg->passGrid(net, alpha);
    }
}

void
RouteMgr::add3DDemand(Net* net)
{
    set<Layer*> alpha;
    passGrid(net, alpha);
    for (auto& m : alpha) {
        m->addDemand(1);
    }
}

void
RouteMgr::remove3DDemand(Net* net)
{
    set<Layer*> alpha;
    passGrid(net, alpha);
    for (auto& m : alpha) {
        m->removeDemand(1);
    }
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

void
RouteMgr::add2DBlkDemand(CellInst* cell){
    const MC* mc = cell->getMC();
    Ggrid* grid = cell->getGrid();
    for(unsigned i=0;i<mc->_blkgList.size();++i){
        grid->update2dDemand(mc->_blkgList[i].second);
    }
}

void
RouteMgr::remove2DBlkDemand(CellInst* cell){
    const MC* mc = cell->getMC();
    Ggrid* grid = cell->getGrid();
    for(unsigned i=0;i<mc->_blkgList.size();++i){
        grid->update2dDemand(-(int)(mc->_blkgList[i].second));
    }
}

void
RouteMgr::add3DBlkDemand(CellInst* cell){
    const MC* mc = cell->getMC();
    Ggrid* grid = cell->getGrid();
    for(unsigned i=0;i<mc->_blkgList.size();++i){
        (*grid)[mc->_blkgList[i].first]->addDemand(mc->_blkgList[i].second);
    }
}

void
RouteMgr::remove3DBlkDemand(CellInst* cell){
    const MC* mc = cell->getMC();
    Ggrid* grid = cell->getGrid();
    for(unsigned i=0;i<mc->_blkgList.size();++i){
        (*grid)[mc->_blkgList[i].first]->removeDemand(mc->_blkgList[i].second);
    }
}

void
RouteMgr::addNeighborDemand(MC* mc_a, MC* mc_b, Ggrid* grid, bool type){
    if(type == 0){
        for(unsigned i=0;i<_laySupply.size();++i){
            MCTri t(mc_a->_mcId, mc_b->_mcId, i+1);
            std::unordered_map<MCTri, unsigned, TriHash>::iterator it = _sameGridDemand.find(t);
            if(it!=_sameGridDemand.end()){
                (*grid)[it->first.layNum]->addDemand(it->second);
                grid->update2dDemand(it->second);
                #ifdef DEBUG
                cout << "MC " << mc_a->_mcId
                    << " and MC " << mc_b->_mcId
                    << " generate sameGGrid demand " << it->second << " on layer " << it->first.layNum << "\n";
                #endif
            }
        }
    }
    else{
        for(unsigned i=0;i<_laySupply.size();++i){
            MCTri t(mc_a->_mcId, mc_b->_mcId, i+1);
            std::unordered_map<MCTri, unsigned, TriHash>::iterator it = _adjHGridDemand.find(t);
            if(it!=_adjHGridDemand.end()){
                (*grid)[it->first.layNum]->addDemand(it->second);
                grid->update2dDemand(it->second);
                #ifdef DEBUG
                cout << "MC " << mc_a->_mcId
                    << " and MC " << mc_b->_mcId
                    << " generate adjHGrid demand " << it->second << " on layer " << it->first.layNum << "\n";
                #endif
            }
        }
    }
}

void
RouteMgr::removeNeighborDemand(MC* mc_a, MC* mc_b, Ggrid* grid, bool type){
    if(type == 0){
        for(unsigned i=0;i<_laySupply.size();++i){
            MCTri t(mc_a->_mcId, mc_b->_mcId, i+1);
            std::unordered_map<MCTri, unsigned, TriHash>::iterator it = _sameGridDemand.find(t);
            if(it!=_sameGridDemand.end()){
                (*grid)[it->first.layNum]->removeDemand(it->second);
                grid->update2dDemand(-(int)(it->second));
                #ifdef DEBUG
                cout << "MC " << mc_a->_mcId
                    << " and MC " << mc_b->_mcId
                    << " remove sameGGrid demand " << it->second << " on layer " << it->first.layNum << "\n";
                #endif
            }
        }
    }
    else{
        for(unsigned i=0;i<_laySupply.size();++i){
            MCTri t(mc_a->_mcId, mc_b->_mcId, i+1);
            std::unordered_map<MCTri, unsigned, TriHash>::iterator it = _adjHGridDemand.find(t);
            if(it!=_adjHGridDemand.end()){
                (*grid)[it->first.layNum]->removeDemand(it->second);
                grid->update2dDemand(-(int)(it->second));
                #ifdef DEBUG
                cout << "MC " << mc_a->_mcId
                    << " and MC " << mc_b->_mcId
                    << " generate adjHGrid demand " << it->second << " on layer " << it->first.layNum << "\n";
                #endif
            }
        }
    }
}


GridStatus
RouteMgr::check3dOverflow(unsigned i, unsigned j, unsigned k) {
    //cout << "Checking grid (" << i << ", " << j << ", " << k << ")\n";
    assert(i >= 1);
    assert(j >= 1);
    assert(k >= 1);
    assert(i <= Ggrid::rEnd);
    assert(j <= Ggrid::cEnd);
    assert(k <= _laySupply.size());
    Layer* grid = _gridList[i-1][j-1]->operator[](k);
    
    GridStatus myStatus = grid->checkOverflow();
    #ifdef DEBUG
    if (myStatus == GRID_FULL_CAP) {
        //cerr << "(" << i << ", " << j << ", "
        // << k << ") is full!\n";
    } else if (myStatus == GRID_OVERFLOW) {
        cerr << "(" << i << ", " << j << ", "
        << k << ") is overflow!\n";
    }
    #endif
    return myStatus;
}

bool
RouteMgr::checkOverflow()
{
    int OVCNT = 0;	
    for (unsigned k=1; k<=routeMgr->getLayerCnt(); ++k) {
        for (unsigned i=1; i<=Ggrid::rEnd; ++i) {
            for (unsigned j=1; j<=Ggrid::cEnd; ++j) {
                if (routeMgr->check3dOverflow(i, j, k) == GRID_OVERFLOW) {
                    ++OVCNT;
                }
            }
        }
    }
    cout << OVCNT << " grids are overflown!\n";
    return OVCNT;
}

unsigned 
RouteMgr::evaluateWireLen() const{
    unsigned newWL = 0;
    // cout << "evalueateWireLen" << endl;
    for (auto n : _netList){
        set<Layer*> alpha;
        passGrid(n, alpha);
        newWL += alpha.size();
    }
    // newWL+=_netList.size();
    cout << "Wire length : " << newWL << endl;
    return newWL;
}

void 
RouteMgr::replaceBest(){
    cout << "Evaluating WL ..." << endl;
    unsigned newWL = evaluateWireLen();
    if(newWL < _bestTotalWL){
    //if (true) {
        //cout << "CHeating myself...\n";
        storeBestResult();
        _bestTotalWL = newWL;
        cout << _bestTotalWL << " is a Better Solution!!\n";
    }
}

void
RouteMgr::printRank() const
{
    _netRank->update();
    _netRank->showTopTen();
}
