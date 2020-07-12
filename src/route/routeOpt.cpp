/****************************************************************************
  FileName     [ routeOpt.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define route optimization functions ]
  Author       [ Chien-Kai Ma, Kai-Chun Chang, Yu-Wei Fan ]
  Copyright    [ Copyleft(c) 2020-present NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "routeMgr.h"
#include "util.h"
#include <algorithm>
#include <csignal>
#include "stlastar.h"
#include "math.h"

using namespace std;

// TODO: Please keep "RouteMgr::optimize()" for route cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/
extern RouteMgr* routeMgr;


/**************************************/
/*   Static variables and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/


// TODO DEBUG!!!!!
// routeMgr becomes 0 when calling cellInst->move() in Forcedirected 
void RouteMgr::mainPnR()
{
    cout << "Initial WL : " << _bestTotalWL << endl;
    while(true){
        this->place();
        cout << "End of Placing..." << endl;
        if( _curMoveCnt > _maxMoveCnt ){
            cout << "Maximum cell-movements!!" << endl;
            cout << "P&R terminates..." << endl;
            return;
        }
        bool canRoute = this->route();
        cout << "End of Routing..." << endl;
        if(canRoute){
            unsigned newWL = evaluateWireLen();// evaluate total wirelength
            if( newWL<_bestTotalWL){
                _bestTotalWL = newWL;
                storeBestResult();
            }
        }
        this->_placeStrategy = !canRoute;
        printRouteSummary();
    }
}



void RouteMgr::place()
{
    cout << "Place...\n";
    if(_placeStrategy){ //Congestion-based
        netbasedPlace();
    }
    else{ //force-directed
        forcedirectedPlace();
    }
}

void RouteMgr::netbasedPlace(){
    cout << "Net-based placement...\n";

    Net* moveNet;
    double moveNetCongestion;
    unsigned minRow = Ggrid::rEnd;
    unsigned minCol = Ggrid::cEnd;
    unsigned maxRow = Ggrid::rBeg;
    unsigned maxCol = Ggrid::cBeg;

    //Store the net congestions and bounding boxes
    for(unsigned i=0;i<_netList.size();++i){
        Net* net = _netList[i];
        double netcongestion = 0;
        if(!(net->_netSegs.empty())){
            minRow = Ggrid::rEnd;
            minCol = Ggrid::cEnd;
            maxRow = Ggrid::rBeg;
            maxCol = Ggrid::cBeg;
            for(unsigned j=0;j<net->_netSegs.size();++j){
                if(net->_netSegs[j]->startPos[0] < minRow)
                    minRow = net->_netSegs[j]->startPos[0];
                if(net->_netSegs[j]->startPos[0] > maxRow)
                    maxRow = net->_netSegs[j]->startPos[0];
                if(net->_netSegs[j]->endPos[0] < minRow)
                    minRow = net->_netSegs[j]->endPos[0];
                if(net->_netSegs[j]->endPos[0] > maxRow)
                    maxRow = net->_netSegs[j]->endPos[0];

                if(net->_netSegs[j]->startPos[1] < minCol)
                    minCol = net->_netSegs[j]->startPos[1];
                if(net->_netSegs[j]->startPos[1] > maxCol)
                    maxCol = net->_netSegs[j]->startPos[1];
                if(net->_netSegs[j]->endPos[1] < minCol)
                    minCol = net->_netSegs[j]->endPos[1];
                if(net->_netSegs[j]->endPos[1] > maxCol)
                    maxCol = net->_netSegs[j]->endPos[1];
            }
            for(unsigned j=minRow;j<=maxRow;++j){
                for(unsigned k=minCol;k<=maxCol;++k){
                    netcongestion += _gridList[j-1][k-1]->get2dCongestion();
                }
            }

            net->_avgCongestion = netcongestion / ((double)((maxRow-minRow+1)*(maxCol-minCol+1)));
            net->_centerRow     = (int)(round(((double)minRow + (double)maxRow)/2.0));
            net->_centerCol     = (int)(round(((double)minCol + (double)maxCol)/2.0));
            //cout << "Net " << i+1 << " " << setprecision(3) <<  _netList[i]->_avgCongestion << " " <<  _netList[i]->_centerRow << " " << _netList[i]->_centerCol << "\n";
            //cout << minRow << " " << maxRow << " " << minCol << " " << maxCol << "\n";
        }
        else{
            net->_avgCongestion = 1;
        }
    }

    //Find the most congested net
    moveNet = _netList[0];
    moveNetCongestion = _netList[0]->_avgCongestion;
    for(unsigned i=1;i<_netList.size();++i){
        if(_netList[i]->_avgCongestion < moveNetCongestion){
            moveNet = _netList[i];
            moveNetCongestion = _netList[i]->_avgCongestion;
        }
    }
    //cout << "Net " << moveNet->_netId << " is moved!\n";

    //Find NewCenter for the most congested net
    double bestCH = -100;
    double newCenterRow = moveNet->_centerRow;
    double newCenterCol = moveNet->_centerCol;
    for(unsigned i=0;i<_netList.size();++i){
        if((_netList[i] != moveNet) && (!_netList[i]->_pinSet.empty())){
            if(Share(moveNet,_netList[i]) > 0){
                if(_netList[i]->_avgCongestion > bestCH){
                    bestCH = _netList[i]->_avgCongestion;
                }
                cout << "Net " << moveNet->_netId << " and " << i+1 << " have share value " << Share(moveNet,_netList[i]) << "\n";
            }
        }
    }
    //cout << "BestCH = " << setprecision(3) << bestCH << "\n";
    for(unsigned i=0;i<_netList.size();++i){
        if((_netList[i] != moveNet) && (!_netList[i]->_pinSet.empty())){
            if((Share(moveNet,_netList[i]) > 0) && (moveNet->_avgCongestion < _netList[i]->_avgCongestion)){
                newCenterRow += Move(moveNet,_netList[i],bestCH).first;
                newCenterCol += Move(moveNet,_netList[i],bestCH).second;
            }
        }
    }
    //cout << "Center is moved from (" << moveNet->_centerRow << "," << moveNet->_centerCol << ") to (" << newCenterRow << "," << newCenterCol << ").\n";

    //Move the associated cells
    int offsetRow = (int)(round(newCenterRow - moveNet->_centerRow));
    int offsetCol = (int)(round(newCenterCol - moveNet->_centerCol));
    unsigned newRow, newCol;
    std::map<unsigned,unsigned>::iterator ite = moveNet->_assoCellInstMap.begin();
    for(unsigned i=0;i<moveNet->_assoCellInstMap.size();++i){
        if(_instList[ite->first-1]->is_movable()){
            change_notifier(_instList[ite->first-1]);
            remove2DBlkDemand(_instList[ite->first-1]);
            remove3DBlkDemand(_instList[ite->first-1]);
            if(_curMovedSet.insert(_instList[ite->first-1]).second == true)
                ++_curMoveCnt;
            if(_instList[ite->first-1]->getPos().first + offsetRow > Ggrid::rEnd)
                newRow = Ggrid::rEnd;
            else if(_instList[ite->first-1]->getPos().first + offsetRow < Ggrid::rBeg)
                newRow = Ggrid::rBeg;
            else
                newRow = _instList[ite->first-1]->getPos().first + offsetRow;
            
            if(_instList[ite->first-1]->getPos().second + offsetCol > Ggrid::cEnd)
                newCol = Ggrid::cEnd;
            else if(_instList[ite->first-1]->getPos().second + offsetCol < Ggrid::cBeg)
                newCol = Ggrid::cBeg;
            else
                newCol = _instList[ite->first-1]->getPos().second + offsetCol;
            
            _instList[ite->first-1] -> move(Pos(newRow,newCol));
            add2DBlkDemand(_instList[ite->first-1]);
            add3DBlkDemand(_instList[ite->first-1]);
            for(unsigned j=0;j<_instList[ite->first-1]->assoNet.size();++j){
                _netList[_instList[ite->first-1]->assoNet[j]-1]->_toRemoveDemand = true;
            }

            cout << "CellInst " << ite->first << " is moved to (" << _instList[ite->first-1]->getPos().first << "," << _instList[ite->first-1]->getPos().second << ").\n";
        }
        ++ite;
    }
    cout << "CurMoveCnt: " << _curMoveCnt << "\n";
    for(unsigned i=0;i<_netList.size();++i){
        if(_netList[i]->_toRemoveDemand == true){
            remove2DDemand(_netList[i]);
            _netList[i]->_toReroute = true;
            //cout << "Net " << i+1 << " need to be rerouted.\n";
            _netList[i]->_toRemoveDemand = false;
        }
    }
}

void RouteMgr::forcedirectedPlace (){
    cout << "Force-directed placement...\n";

    CellInst* moveCell;
    unsigned s=_instList.size()-1;
    for(unsigned i=0; i<_instList.size(); ++i){
        if(_instList[i]->is_movable() && (_instList[i]->_hasmovedbyfd == false)){
            //cout << "CellInst " << i+1 << " on (" << _instList[i]->getPos().first << "," << _instList[i]->getPos().second << ") has 2dcongestion " << setprecision(3) << _instList[i]->getGrid()->get2dCongestion() << "\n";
            moveCell = _instList[i];
            s = i;
            break;
        }
    }
    
    for(unsigned i=s+1; i<_instList.size(); ++i){
        // cout << "Grid addr : " << _instList[i]->getGrid() << " Mgr Grid Addr : " << _gridList[_instList[i]->getPos().first-1][_instList[i]->getPos().second-1] << endl; 
        // cout << "CellInst " << i+1 << " on (" << _instList[i]->getPos().first << "," << _instList[i]->getPos().second << ") has 2dcongestion " << setprecision(3) << _instList[i]->getGrid()->get2dCongestion() << "\n";
        if((_instList[i]->is_movable()) && (_instList[i]->getGrid()->get2dCongestion() < moveCell->getGrid()->get2dCongestion()) && (_instList[i]->_hasmovedbyfd == false))
            moveCell = _instList[i];
    }

    //go through _netList, find out all associated nets and thus associated cells and multiplications, then calculating new pos
    double row_numerator = 0;
    double row_denominator = 0;
    double col_numerator = 0;
    double col_denominator = 0;
    int new_row;
    int new_col;
    change_notifier(moveCell);
    remove2DBlkDemand(moveCell);
    remove3DBlkDemand(moveCell);
    if(_curMovedSet.insert(moveCell).second == true){
        ++_curMoveCnt;
    }
    moveCell->_hasmovedbyfd = true;
    cout << "CellInst " << moveCell->getId() << " is moved!\n";
    cout << "CurMoveCnt: " << _curMoveCnt << "\n";
    for(unsigned i=0; i<moveCell->assoNet.size(); ++i){
        //cout << "Associated net " << _netList[moveCell->assoNet[i]-1]->_netId << "\n";
        int pin_num = _netList[moveCell->assoNet[i]-1]->getPinSet().size() - 1;
        std::set<PinPair>::iterator it = _netList[moveCell->assoNet[i]-1]->getPinSet().begin();
        if(pin_num > 0){
            for(int j=0; j<pin_num+1; ++j){
                if(_instList[(*it).first-1] != moveCell){
                    //cout << _instList[(*it).first-1]->getPos().first << " " << _instList[(*it).first-1]->getPos().second << "\n";
                    //cout << "Pin_num: " << (double)pin_num << "\n";
                    row_numerator += ((double)(_instList[(*it).first-1]->getPos().first))/((double)(pin_num));
                    col_numerator += ((double)(_instList[(*it).first-1]->getPos().second))/((double)(pin_num));
                    row_denominator += 1.0/((double)(pin_num));
                    col_denominator += 1.0/((double)(pin_num));
                }
                ++it;
            }
        }
        remove2DDemand(_netList[moveCell->assoNet[i]-1]);
    }
    new_row = (int)(round((double)(row_numerator) / (double)(row_denominator)));
    new_col = (int)(round((double)(col_numerator) / (double)(col_denominator)));
    if(new_row > (int)Ggrid::rEnd)
        new_row = Ggrid::rEnd;
    else if(new_row < (int)Ggrid::rBeg)
        new_row = Ggrid::rBeg;
    if(new_col > (int)Ggrid::cEnd)
        new_col = Ggrid::cEnd;
    else if(new_col < (int)Ggrid::cBeg)
        new_col = Ggrid::cBeg;
    cout << "Old position: " << moveCell->getPos().first << " " << moveCell->getPos().second << "\n";
    moveCell->move(Pos(new_row,new_col));
    add2DBlkDemand(moveCell);
    add3DBlkDemand(moveCell);
    cout << "New position: " << moveCell->getPos().first << " " << moveCell->getPos().second << "\n";
}

unsigned RouteMgr::Share(Net* a, Net* b){
    unsigned numShareCell = 0;
    std::map<unsigned,unsigned>::iterator ite_a = a->_assoCellInstMap.begin();
    std::map<unsigned,unsigned>::iterator ite_b = b->_assoCellInstMap.begin();
    while(true){
        if(ite_a == a->_assoCellInstMap.end() || ite_b == b->_assoCellInstMap.end())
            break;
        
        if(ite_a->first > ite_b->first)
            ++ite_b;
        else if(ite_a->first < ite_b->first)
            ++ite_a;
        else{
            numShareCell += ite_a->second;
            ++ite_a;
        }
    }
    /*for(unsigned i=0;i<_instList.size();++i){
        if(b->_assoCellInst[i] != 0){
            numShareCell += a->_assoCellInst[i];
        }
    }*/
    return numShareCell;
}

pair<double,double> RouteMgr::Move(Net* a, Net* b, double BestCH){
    //cout << "associated net : " << b->_netId << " ";
    //cout << b->_centerRow << " " << a->_centerRow << " " << b->_avgCongestion << " " << a->_avgCongestion << " " << BestCH << " " << a->_pinSet.size() << " " << Share(a,b) << " ";
    double offsetRow = ((double)b->_centerRow - (double)a->_centerRow) * ((b->_avgCongestion - a->_avgCongestion)/(BestCH - a->_avgCongestion)) * ((double)Share(a,b)/(double)a->_pinSet.size());
    double offsetCol = ((double)b->_centerCol - (double)a->_centerCol) * ((b->_avgCongestion - a->_avgCongestion)/(BestCH - a->_avgCongestion)) * ((double)Share(a,b)/(double)a->_pinSet.size());
    pair<double,double> offset(offsetRow,offsetCol);
    //cout << "offsetrow = " << offsetRow << " offsetcol = " << offsetCol << "\n";
    return offset;
}

bool
RouteMgr::findCand(unsigned min, unsigned max, vector<int>& cands)
{
    /* Find layer candidates for layer assignment */
    for (unsigned i=min; i<=max; i += 2) {
        cands.push_back(i);
        cout << "Candidate: L" << i << "\n";
    }
    if (cands.empty()) {
        return false;
    }
    return true;
}


bool RouteMgr::layerassign(NetList& toLayNet)
{
    cout << "LayerAssign...\n";
    vector<Segment*> toDel;
    unsigned maxLayer = _laySupply.size();

    for (auto& net : toLayNet)
    {
        // Determine target layer
        // - Satisfy minLayCons
        // - Use less layers as possible
        // 1. Find candidates
        // ?? < minLayer < ?? < maxLayer
        // ?? can be curLayer or avgPinLayer or PinLayer
        // 2. Check grid capacity
        // 3. Choose the layer with shortest via
        // 4. Connect the wires
        // TODO: Add via to minLayer
        unsigned segCnt = net->_netSegs.size();
        unsigned layCons = net->_minLayCons;
        
        cout << "\nNet " << net->_netId;
        cout << " MinLayerConstr " << layCons << "\n";

        int minLayer = layCons ? layCons : 1;
        int minH = minLayer + (1 - minLayer%2);
        int minV = minLayer + (minLayer%2);
        int curLayer = 1;
        int targetLayer = minLayer;
        vector<int> candidatesV;
        vector<int> candidatesH;
        // 1. Find candidates
        findCand(minH, maxLayer, candidatesH);
        findCand(minV, maxLayer, candidatesV);
        
        for (unsigned i=0; i<segCnt; ++i)
        {
            Segment* seg = net->_netSegs[i];
            vector<int> candidates;
            
            if (!i) { curLayer = seg->startPos[2]; }
            #ifdef DEBUG
            cout << "Assigning ";
            seg->print();
            cout << " on " << seg->checkDir() << "\n";
            cout << "curLayer " << curLayer << " minLayer " << minLayer
            << " avgPinLayer " << net->_avgPinLayer << "\n";
            #endif

            // Z
            if (seg->checkDir() == 'Z')
            {
                if (!(seg->startPos[2] >= minLayer || seg->endPos[2] >= minLayer)) {
                    if (seg->startPos[2] > seg->endPos[2]) {
                        seg->startPos[2] = seg->endPos[2];
                        seg->endPos[2] = minLayer;
                        curLayer = minLayer;
                    } else {
                        seg->endPos[2] = minLayer;
                        curLayer = minLayer;
                    }
                } else if (seg->endPos[2] == seg->startPos[2]) {
                    if (seg->startPos[2] == curLayer) {
                        seg->print();
                        cout << "Stupid seg... Delete it!\n";
                        //toDel.push_back(seg);
                        //net->_netSegs.erase(net->_netSegs.begin()+i);
                        //curLayer = seg->startPos[2];
                        continue;
                    } else {
                        seg->startPos[2] = curLayer;
                        continue;
                    }
                }
                #ifndef DEBUG
                cout << "Successfully assigned...";
                seg->print();
                cout << "\n";
                cout << "\n";
                #endif
                continue;
            }
            
            // H or V
            int diff = INT16_MAX;
            if (seg->checkDir() == 'H') {
                if (candidatesH.size() == 0) {
                    cout << "No candidate was found!\n";
                    cout << "Do placement again!\n";
                    return false;
                }
                for (auto& j : candidatesH) {
                    diff = ((diff) < (j-curLayer)) ? diff : j-curLayer;
                }
            } else {
                if (candidatesV.size() == 0) {
                    cout << "No candidate was found!\n";
                    cout << "Do placement again!\n";
                    return false;
                }
                for (auto& j : candidatesV) {
                    diff = ((diff) < (j-curLayer)) ? diff : j-curLayer;
                }
            }
            targetLayer = curLayer + diff;
            cout << "targetLayer " << targetLayer << "\n";
            // 2. Check grid capacity
            /*
            if (!check3dOverflow(seg->startPos[0], seg->startPos[1], i)) {
                
            } else {
                return false;
            }
            */
            if (seg->startPos[2] && curLayer != seg->startPos[2]) {
                // Add a Z-seg
                Segment* zSeg = new Segment(seg);
                zSeg->startPos[2] = curLayer;
                zSeg->endPos[0] = seg->startPos[0];
                zSeg->endPos[1] = seg->startPos[1];
                zSeg->endPos[2] = seg->startPos[2];
                net->addSeg(zSeg);
                cout << "Add new Segment" << endl;
                zSeg->print();
                cout << endl;
            }
            
            if (curLayer != targetLayer) {
                // Add a Z-seg
                Segment* zSeg = new Segment(seg);
                zSeg->startPos[2] = curLayer;
                zSeg->endPos[0] = seg->startPos[0];
                zSeg->endPos[1] = seg->startPos[1];
                zSeg->endPos[2] = targetLayer;
                net->addSeg(zSeg);
                cout << "Add new Segment" << endl;
                zSeg->print();
                cout << endl;
                curLayer = targetLayer;
            }
            
            if (seg->endPos[2] && i == segCnt-1) {
                cout << "Last Seg ";
                seg->print();
                cout << "\n";
                if (curLayer != seg->endPos[2]) {
                    // Add a Z-seg
                    Segment* zSeg1 = new Segment(seg);
                    zSeg1->startPos[0] = seg->endPos[0];
                    zSeg1->startPos[1] = seg->endPos[1];
                    zSeg1->startPos[2] = curLayer;
                    net->addSeg(zSeg1);
                    cout << "Add new Segment" << endl;
                    zSeg1->print();
                    cout << endl;
                }
            }
            // Finish layer assignment
            seg->startPos[2] = targetLayer;
            seg->endPos[2] = targetLayer;
            cout << "Successfully assigned...";
            seg->print();
            cout << "\n";
            cout << "\n";
        }
        add3DDemand(net);
        net->printSummary();
    }
    for (auto& seg1 : toDel) {
        cout << "Deleting " << seg1 << "\n";
        delete seg1;
    }
    return true;
}

void
RouteMgr::koova_place()
{
    // heuristicly move the first cell instance
    /*unsigned fst = _instList[0]->getPos().first;
    unsigned sec = _instList[0]->getPos().second;
    _instList[0]->move(Pos((Ggrid::cEnd + fst)/2, (Ggrid::rEnd + sec)/2));
    _curMovedSet.insert(_instList[0]);
    change_notifier(_instList[0]);
    ++_curMoveCnt;*/
}

void
RouteMgr::change_notifier(CellInst* su)
{
    for(auto& m : su->assoNet)
    {
        _netList[m-1]->shouldReroute(true);
    }
}

void
RouteMgr::koova_route()
{
    /*if (_curRouteSegs.empty()) { _curRouteSegs = _initRouteSegs; }
    NetList toRouteNet = NetList();
    for (auto m : _netList)
    {
        if (m->shouldReroute())
        {
            toRouteNet.push_back(m);
            
            m->getPinSet();
            // TODO: Reroute Net m
            m->shouldReroute(false);
            
        }
    }*/
}
