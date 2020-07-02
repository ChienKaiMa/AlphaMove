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

void RouteMgr::mainPnR()
{
    while(true){
        this->place();
        if( _curMoveCnt > _maxMoveCnt ){
            cout << "Maximum cell-movements!!" << endl;
            cout << "P&R terminates..." << endl;
            return;
        }
        bool canRoute = this->route();
        if(canRoute){
            unsigned newWL = evaluateWireLen();// evaluate total wirelength
            if( newWL<_bestTotalWL){
                _bestTotalWL = newWL;
            }
        }
        this->_placeStrategy = !canRoute;
    }
}



void RouteMgr::place()
{
    cout << "Place..." << endl;
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

    //Store the net congestions and bouding boxes
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
    for(unsigned i=0;i<moveNet->_assoCellInst.size();++i){
        if(moveNet->_assoCellInst[i] > 0){
            if(_instList[i]->is_movable()){
                change_notifier(_instList[i]);
                if(_curMovedSet.insert(_instList[i]).second == true)
                    ++_curMoveCnt;
                if(_instList[i]->getPos().first + offsetRow > Ggrid::rEnd)
                    newRow = Ggrid::rEnd;
                else if(_instList[i]->getPos().first + offsetRow < Ggrid::rBeg)
                    newRow = Ggrid::rBeg;
                else
                    newRow = _instList[i]->getPos().first + offsetRow;
                
                if(_instList[i]->getPos().second + offsetCol > Ggrid::cEnd)
                    newCol = Ggrid::cEnd;
                else if(_instList[i]->getPos().second + offsetCol < Ggrid::cBeg)
                    newCol = Ggrid::cBeg;
                else
                    newCol = _instList[i]->getPos().second + offsetCol;
                
                _instList[i] -> move(Pos(newRow,newCol));
                for(unsigned j=0;j<_instList[i]->assoNet.size();++j){
                    _netList[_instList[i]->assoNet[j]-1]->_toRemoveDemand = true;
                }

                cout << "CellInst " << i+1 << " is moved to (" << _instList[i]->getPos().first << "," << _instList[i]->getPos().second << ").\n";
            }
        }
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
    moveCell->move(Pos(new_row,new_col));
    cout << "New position: " << moveCell->getPos().first << " " << moveCell->getPos().second << "\n";
}

unsigned RouteMgr::Share(Net* a, Net* b){
    unsigned numShareCell = 0;
    for(unsigned i=0;i<_instList.size();++i){
        if(b->_assoCellInst[i] != 0){
            numShareCell += a->_assoCellInst[i];
        }
    }
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

bool RouteMgr::layerassign(NetList& toLayNet)
{
    cout << "LayerAssign..." << endl;
    vector<Segment*> toDel;
    for (auto& net : toLayNet)
    {
        unsigned curLayer, targetLayer;
        unsigned segCnt = net->_netSegs.size();
        unsigned layCons = net->_minLayCons;
        cout << "MinLayerConstr " << layCons << endl;
        if (layCons) {
            for (unsigned i=0; i<segCnt; ++i)
            {
                Segment*& seg = net->_netSegs[i];
                cout << seg->checkDir() << endl;
                Segment save;
                if (!i) {
                    curLayer = seg->startPos[2];
                } else if (i == (segCnt-1)) {
                    Segment save = Segment(*seg);
                }

                if (seg->checkDir() == 'H') {
                    if (curLayer >= seg->startPos[2]) {
                        targetLayer = seg->startPos[2] + (1 - seg->startPos[2]%2);
                        if (layCons > targetLayer) {
                            targetLayer = layCons + (1 - layCons%2);
                        }
                    } else {
                        targetLayer = seg->startPos[2] - (1 - seg->startPos[2]%2);
                        if (layCons > targetLayer) {
                            targetLayer = layCons + (1 - layCons%2);
                        }
                    }
                } else if (seg->checkDir() == 'V') {
                    if (curLayer >= seg->startPos[2]) {
                        targetLayer = seg->startPos[2] + (seg->startPos[2]%2);
                        if (layCons > targetLayer) {
                            targetLayer = layCons + (layCons%2);
                        }
                    } else {
                        targetLayer = seg->startPos[2] - (seg->startPos[2]%2);
                        if (layCons > targetLayer) {
                            targetLayer = layCons + (layCons%2);
                        }
                    }
                }    

                if (seg->checkDir() == 'Z') {
                    if (seg->startPos[2] == seg->endPos[2]) {
                        if (seg->startPos[2] < layCons) {
                            // Move to the minRoutingLayer
                            seg->endPos[2] = layCons;
                        } else {
                            toDel.push_back(seg);
                        }
                    } else if (!(seg->startPos[2] > layCons || seg->endPos[2] > layCons)) {
                        // Add a Z-seg
                        Segment* zSeg = new Segment(*seg);
                        zSeg->endPos[0] = seg->startPos[0];
                        zSeg->endPos[1] = seg->startPos[1];
                        zSeg->endPos[2] = layCons;
                        net->addSeg(zSeg);
                        cout << "Add new Segment" << endl;
                        zSeg->print();
                    }
                } else { // H or V
                    if (curLayer != seg->startPos[2]) {
                        // Add a Z-seg
                        Segment* zSeg = new Segment(*seg);
                        zSeg->endPos[0] = seg->startPos[0];
                        zSeg->endPos[1] = seg->startPos[1];
                        zSeg->endPos[2] = curLayer;
                        net->addSeg(zSeg);
                        cout << "Add new Segment" << endl;
                        zSeg->print();
                        cout << endl;
                        // Finish layer assignment
                        seg->startPos[2] = targetLayer;
                        seg->endPos[2] = targetLayer;
                        curLayer = targetLayer;
                    } else if (curLayer != targetLayer) {
                        // Add a Z-seg
                        Segment* zSeg = new Segment(*seg);
                        zSeg->endPos[0] = seg->startPos[0];
                        zSeg->endPos[1] = seg->startPos[1];
                        zSeg->endPos[2] = targetLayer;
                        net->addSeg(zSeg);
                        cout << "Add new Segment" << endl;
                        zSeg->print();
                        cout << endl;
                        // Finish layer assignment
                        seg->startPos[2] = targetLayer;
                        seg->endPos[2] = targetLayer;
                        curLayer = targetLayer;
                    }
                    
                    if (i == segCnt-1) {
                        cout << seg->endPos[2] << endl;
                        if (curLayer != seg->endPos[2]) {
                            // Add a Z-seg
                            Segment* zSeg = new Segment(save);
                            
                            cout << curLayer << endl;
                            cout << seg->endPos[2] << endl;

                            zSeg->startPos[0] = seg->endPos[0];
                            zSeg->startPos[1] = seg->endPos[1];
                            zSeg->startPos[2] = curLayer;
                            net->addSeg(zSeg);
                            cout << "Add new Segment" << endl;
                            zSeg->print();
                        }
                    }
                }
            }
        } else {
            cout << "segCnt = " << segCnt << endl;
            for (unsigned i=0; i<segCnt; ++i)
            {
                Segment*& seg = net->_netSegs[i];
                cout << seg->checkDir() << endl;
                Segment save;

                if (!i) {
                    curLayer = seg->startPos[2];
                } else if (i == (segCnt-1)) {
                    Segment save = Segment(*seg);
                }
                if (seg->checkDir() == 'H') {
                    if (curLayer >= seg->startPos[2]) {
                        targetLayer = seg->startPos[2] + (1 - seg->startPos[2]%2);
                    } else {
                        targetLayer = seg->startPos[2] - (1 - seg->startPos[2]%2);
                    }
                } else if (seg->checkDir() == 'V') {
                    if (curLayer >= seg->startPos[2]) {
                        targetLayer = seg->startPos[2] + (seg->startPos[2]%2);
                    } else {
                        targetLayer = seg->startPos[2] - (seg->startPos[2]%2);
                    }
                }
                
                if (seg->checkDir() == 'Z') {
                    if (seg->startPos[2] == seg->endPos[2]) {
                        toDel.push_back(seg);
                    }
                } else { // H or V
                    if (curLayer != seg->startPos[2]) {
                        // Add a Z-seg
                        Segment* zSeg = new Segment(*seg);
                        cout << "DEBUG" << seg->startPos[2] << endl;
                        zSeg->endPos[0] = seg->startPos[0];
                        zSeg->endPos[1] = seg->startPos[1];
                        zSeg->endPos[2] = curLayer;
                        net->addSeg(zSeg);
                        cout << "Add new Segment" << endl;
                        zSeg->print();
                        cout << endl;
                        // Finish layer assignment
                        seg->startPos[2] = targetLayer;
                        seg->endPos[2] = targetLayer;
                        curLayer = targetLayer;
                    } else if (curLayer != targetLayer) {
                        // Add a Z-seg
                        Segment* zSeg = new Segment(*seg);
                        zSeg->endPos[0] = seg->startPos[0];
                        zSeg->endPos[1] = seg->startPos[1];
                        zSeg->endPos[2] = targetLayer;
                        net->addSeg(zSeg);
                        cout << "Add new Segment" << endl;
                        zSeg->print();
                        cout << endl;
                        // Finish layer assignment
                        seg->startPos[2] = targetLayer;
                        seg->endPos[2] = targetLayer;
                        curLayer = targetLayer;
                    }

                    if (i == segCnt-1) {
                        cout << seg->endPos[2] << endl;
                        if (curLayer != seg->endPos[2]) {
                            // Add a Z-seg
                            Segment* zSeg = new Segment(save);
                            
                            cout << curLayer << endl;
                            cout << seg->endPos[2] << endl;

                            zSeg->startPos[0] = seg->endPos[0];
                            zSeg->startPos[1] = seg->endPos[1];
                            zSeg->startPos[2] = curLayer;
                            net->addSeg(zSeg);
                            cout << "Add new Segment" << endl;
                            zSeg->print();
                        }
                    }
                }
            }
        }
        
    }
    for (auto& seg : toDel) {
        cout << "Deleting " << seg << endl;
        delete seg;
        cout << "Still have " << toDel.size() << " item(s) to delete." << endl;
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
