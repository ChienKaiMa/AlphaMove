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
    unsigned maxRow = 0;
    unsigned maxCol = 0;
    /*for(unsigned i=0;i<moveNet->_netSegs.size();++i){
        if(moveNet->_netSegs[i]->startPos[0] < minRow)
            minRow = moveNet->_netSegs[i]->startPos[0];
        else if(moveNet->_netSegs[i]->startPos[0] > maxRow)
            maxRow = moveNet->_netSegs[i]->startPos[0];
        if(moveNet->_netSegs[i]->endPos[0] < minRow)
            minRow = moveNet->_netSegs[i]->endPos[0];
        else if(moveNet->_netSegs[i]->endPos[0] > maxRow)
            maxRow = moveNet->_netSegs[i]->endPos[0];
        
        if(moveNet->_netSegs[i]->startPos[1] < minCol)
            minCol = moveNet->_netSegs[i]->startPos[1];
        else if(moveNet->_netSegs[i]->startPos[1] > maxCol)
            maxCol = moveNet->_netSegs[i]->startPos[1];
        if(moveNet->_netSegs[i]->endPos[1] < minCol)
            minCol = moveNet->_netSegs[i]->endPos[1];
        else if(moveNet->_netSegs[i]->endPos[1] > maxCol)
            maxCol = moveNet->_netSegs[i]->endPos[1];
    }
    for(unsigned i=minRow;i<=maxRow;++i){
        for(unsigned j=minCol;j<=minCol;++j){
            moveNetCongestion += _gridList[i-1][j-1]->get2dCongestion();
        }
    }
    moveNetCongestion = moveNetCongestion / ((double)((maxRow-minRow+1)*(maxCol-minCol+1)));*/

    //Store the net congestions and bouding boxes
    for(unsigned i=0;i<_netList.size();++i){
        Net* net = _netList[i];
        double netcongestion = 0;
        if(!(net->_netSegs.empty())){
            minRow = Ggrid::rEnd;
            minCol = Ggrid::cEnd;
            maxRow = 0;
            maxCol = 0;
            for(unsigned j=0;j<net->_netSegs.size();++j){
                if(net->_netSegs[j]->startPos[0] < minRow)
                    minRow = net->_netSegs[j]->startPos[0];
                else if(net->_netSegs[j]->startPos[0] > maxRow)
                    maxRow = net->_netSegs[j]->startPos[0];
                if(net->_netSegs[j]->endPos[0] < minRow)
                    minRow = net->_netSegs[j]->endPos[0];
                else if(net->_netSegs[j]->endPos[0] > maxRow)
                    maxRow = net->_netSegs[j]->endPos[0];

                if(net->_netSegs[j]->startPos[1] < minCol)
                    minCol = net->_netSegs[j]->startPos[1];
                else if(net->_netSegs[j]->startPos[1] > maxCol)
                    maxCol = net->_netSegs[j]->startPos[1];
                if(net->_netSegs[j]->endPos[1] < minCol)
                    minCol = net->_netSegs[j]->endPos[1];
                else if(net->_netSegs[j]->endPos[1] > maxCol)
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
    for(unsigned i=0;i<_netList.size();++i){
        if((_netList[i] != moveNet) && (!_netList[i]->_pinSet.empty())){
            if((Share(moveNet,_netList[i]) > 0) && (moveNet->_avgCongestion < _netList[i]->_avgCongestion)){
                newCenterRow += Move(moveNet,_netList[i],bestCH).first;
                newCenterCol += Move(moveNet,_netList[i],bestCH).second;
            }
        }
    }

    //Move the associated cells

}

void RouteMgr::forcedirectedPlace (){
    cout << "Force-directed placement...\n";

    CellInst* moveCell;
    unsigned s=_instList.size()-1;
    for(unsigned i=0; i<_instList.size(); ++i){
        if(_instList[i]->is_movable() && (_instList[i]->_hasmovedbyfd == false)){
            cout << "CellInst " << i+1 << " on (" << _instList[i]->getPos().first << "," << _instList[i]->getPos().second << ") has 2dcongestion " << setprecision(3) << _instList[i]->getGrid()->get2dCongestion() << "\n";
            moveCell = _instList[i];
            s = i;
            break;
        }
    }
    
    for(unsigned i=s+1; i<_instList.size(); ++i){
        // cout << "Grid addr : " << _instList[i]->getGrid() << " Mgr Grid Addr : " << _gridList[_instList[i]->getPos().first-1][_instList[i]->getPos().second-1] << endl; 
        cout << "CellInst " << i+1 << " on (" << _instList[i]->getPos().first << "," << _instList[i]->getPos().second << ") has 2dcongestion " << setprecision(3) << _instList[i]->getGrid()->get2dCongestion() << "\n";
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
    if(_movedSet.insert(moveCell).second == true){
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
    double offsetRow = (double)(b->_centerRow - a->_centerRow) * ((b->_avgCongestion - a->_avgCongestion)/(BestCH - a->_avgCongestion)) * ((double)Share(a,b)/a->_pinSet.size());
    double offsetCol = (double)(b->_centerCol - a->_centerCol) * ((b->_avgCongestion - a->_avgCongestion)/(BestCH - a->_avgCongestion)) * ((double)Share(a,b)/a->_pinSet.size());
    pair<double,double> offset(offsetRow,offsetCol);
    return offset;
}

void RouteMgr::layerassign(NetList& toLayNet)
{
    cout << "LayerAssign..." << endl;
    for (auto& net : toLayNet)
    {
        cout << "MinLayerConstr " << net->_minLayCons << endl;
        for (auto& seg : net->_netSegs)
        {
            cout << seg->checkDir() << endl;
            if (seg->checkDir() == 'H') {

            } else if (seg->checkDir() == 'V') {
                if (net->_minLayCons) {

                }
            } else { // Dir = 'Z'
                if (seg->startPos[2] == seg->endPos[2]) {
                    // delete seg;
                }
            }
            //net->addSeg();
        }
    }
}

void
RouteMgr::koova_place()
{
    // heuristicly move the first cell instance
    unsigned fst = _instList[0]->getPos().first;
    unsigned sec = _instList[0]->getPos().second;
    _instList[0]->move(Pos((Ggrid::cEnd + fst)/2, (Ggrid::rEnd + sec)/2));
    _movedSet.insert(_instList[0]);
    change_notifier(_instList[0]);
    ++_curMoveCnt;
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
    if (_curRouteSegs.empty()) { _curRouteSegs = initRouteSegs; }
    NetList toRouteNet = NetList();
    for (auto m : _netList)
    {
        if (m->shouldReroute())
        {
            toRouteNet.push_back(m);
            /*
            m->getPinSet();
            // TODO: Reroute Net m
            m->shouldReroute(false);
            */
        }
    }
}
