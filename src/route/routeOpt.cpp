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
        if( getCurMoveCnt() > _maxMoveCnt ){
            cout << "Maximum cell-movements!!" << endl;
            cout << "P&R terminates..." << endl;
            return;
        }
        RouteExecStatus canRoute = this->route();
        _netRank->update();
        _netRank->showTopTen();
        cout << "End of Routing..." << endl;
        if(canRoute == ROUTE_EXEC_DONE){
            unsigned newWL = evaluateWireLen();// evaluate total wirelength
            if( newWL<_bestTotalWL){
                _bestTotalWL = newWL;
                storeBestResult();
            }
        }
        this->_placeStrategy = (canRoute == ROUTE_EXEC_DONE) ? FORCE_DIRECTED : CONGESTION_BASED;
        printRouteSummary();
        myUsage.report(true, false);
    }
}



void RouteMgr::place()
{
    cout << "Place...\n";
    if(_placeStrategy == CONGESTION_BASED){ //Congestion-based
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
            //remove from original cellInstList
            for(unsigned j=0;j<_instList[ite->first-1]->getGrid()->cellInstList.size();++j){
                if(_instList[ite->first-1]->getGrid()->cellInstList[j] == _instList[ite->first-1]){
                    _instList[ite->first-1]->getGrid()->cellInstList.erase(_instList[ite->first-1]->getGrid()->cellInstList.begin() + j);
                    break;
                }
            }
            //remove same gGrid demand
            for(unsigned j=0;j<_instList[ite->first-1]->getGrid()->cellInstList.size();++j){
                if(_instList[ite->first-1]->getGrid()->cellInstList[j] != _instList[ite->first-1]){
                    remove2DNeighborDemand(_instList[ite->first-1],_instList[ite->first-1]->getGrid()->cellInstList[j],0);
                    remove3DNeighborDemand(_instList[ite->first-1],_instList[ite->first-1]->getGrid()->cellInstList[j],0);
                }
            }
            //remove adjHGrid demand
            if(_instList[ite->first-1]->getPos().second == Ggrid::cBeg){
                unsigned next_col = _instList[ite->first-1]->getPos().second + 1;
                for(unsigned j=0;j<_gridList[_instList[ite->first-1]->getPos().first-1][next_col-1]->cellInstList.size();++j){
                    remove2DNeighborDemand(_instList[ite->first-1], _gridList[_instList[ite->first-1]->getPos().first-1][next_col-1]->cellInstList[j], 1);
                    remove2DNeighborDemand(_gridList[_instList[ite->first-1]->getPos().first-1][next_col-1]->cellInstList[j], _instList[ite->first-1], 1);
                    remove3DNeighborDemand(_instList[ite->first-1], _gridList[_instList[ite->first-1]->getPos().first-1][next_col-1]->cellInstList[j], 1);
                    remove3DNeighborDemand(_gridList[_instList[ite->first-1]->getPos().first-1][next_col-1]->cellInstList[j], _instList[ite->first-1], 1);
                }
            }
            else if(_instList[ite->first-1]->getPos().second == Ggrid::cEnd){
                unsigned prev_col = _instList[ite->first-1]->getPos().second - 1;
                for(unsigned j=0;j<_gridList[_instList[ite->first-1]->getPos().first-1][prev_col-1]->cellInstList.size();++j){
                    remove2DNeighborDemand(_instList[ite->first-1], _gridList[_instList[ite->first-1]->getPos().first-1][prev_col-1]->cellInstList[j], 1);
                    remove2DNeighborDemand(_gridList[_instList[ite->first-1]->getPos().first-1][prev_col-1]->cellInstList[j], _instList[ite->first-1], 1);
                    remove3DNeighborDemand(_instList[ite->first-1], _gridList[_instList[ite->first-1]->getPos().first-1][prev_col-1]->cellInstList[j], 1);
                    remove3DNeighborDemand(_gridList[_instList[ite->first-1]->getPos().first-1][prev_col-1]->cellInstList[j], _instList[ite->first-1], 1);
                }
            }
            else{
                unsigned next_col = _instList[ite->first-1]->getPos().second + 1;
                unsigned prev_col = _instList[ite->first-1]->getPos().second - 1;
                for(unsigned j=0;j<_gridList[_instList[ite->first-1]->getPos().first-1][next_col-1]->cellInstList.size();++j){
                    remove2DNeighborDemand(_instList[ite->first-1], _gridList[_instList[ite->first-1]->getPos().first-1][next_col-1]->cellInstList[j], 1);
                    remove2DNeighborDemand(_gridList[_instList[ite->first-1]->getPos().first-1][next_col-1]->cellInstList[j], _instList[ite->first-1], 1);
                    remove3DNeighborDemand(_instList[ite->first-1], _gridList[_instList[ite->first-1]->getPos().first-1][next_col-1]->cellInstList[j], 1);
                    remove3DNeighborDemand(_gridList[_instList[ite->first-1]->getPos().first-1][next_col-1]->cellInstList[j], _instList[ite->first-1], 1);
                }
                for(unsigned j=0;j<_gridList[_instList[ite->first-1]->getPos().first-1][prev_col-1]->cellInstList.size();++j){
                    remove2DNeighborDemand(_instList[ite->first-1], _gridList[_instList[ite->first-1]->getPos().first-1][prev_col-1]->cellInstList[j], 1);
                    remove2DNeighborDemand(_gridList[_instList[ite->first-1]->getPos().first-1][prev_col-1]->cellInstList[j], _instList[ite->first-1], 1);
                    remove3DNeighborDemand(_instList[ite->first-1], _gridList[_instList[ite->first-1]->getPos().first-1][prev_col-1]->cellInstList[j], 1);
                    remove3DNeighborDemand(_gridList[_instList[ite->first-1]->getPos().first-1][prev_col-1]->cellInstList[j], _instList[ite->first-1], 1);
                }
            }

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
            
            if (Pos(newRow,newCol) != _instList[ite->first-1]->getInitPos()) {
                _curMovedSet.insert(_instList[ite->first-1]);
            } else {
                _curMovedSet.erase(_instList[ite->first-1]);
            }
            _instList[ite->first-1] -> move(Pos(newRow,newCol));
            add2DBlkDemand(_instList[ite->first-1]);
            add3DBlkDemand(_instList[ite->first-1]);
            //add same gGrid demand
            for(unsigned j=0;j<_instList[ite->first-1]->getGrid()->cellInstList.size();++j){
                if(_instList[ite->first-1]->getGrid()->cellInstList[j] != _instList[ite->first-1]){
                    add2DNeighborDemand(_instList[ite->first-1],_instList[ite->first-1]->getGrid()->cellInstList[j],0);
                    add3DNeighborDemand(_instList[ite->first-1],_instList[ite->first-1]->getGrid()->cellInstList[j],0);
                }
            }
            //add adjHGrid demand
            if(_instList[ite->first-1]->getPos().second == Ggrid::cBeg){
                unsigned next_col = _instList[ite->first-1]->getPos().second + 1;
                for(unsigned j=0;j<_gridList[_instList[ite->first-1]->getPos().first-1][next_col-1]->cellInstList.size();++j){
                    add2DNeighborDemand(_instList[ite->first-1], _gridList[_instList[ite->first-1]->getPos().first-1][next_col-1]->cellInstList[j], 1);
                    add2DNeighborDemand(_gridList[_instList[ite->first-1]->getPos().first-1][next_col-1]->cellInstList[j], _instList[ite->first-1], 1);
                    add3DNeighborDemand(_instList[ite->first-1], _gridList[_instList[ite->first-1]->getPos().first-1][next_col-1]->cellInstList[j], 1);
                    add3DNeighborDemand(_gridList[_instList[ite->first-1]->getPos().first-1][next_col-1]->cellInstList[j], _instList[ite->first-1], 1);
                }
            }
            else if(_instList[ite->first-1]->getPos().second == Ggrid::cEnd){
                unsigned prev_col = _instList[ite->first-1]->getPos().second - 1;
                for(unsigned j=0;j<_gridList[_instList[ite->first-1]->getPos().first-1][prev_col-1]->cellInstList.size();++j){
                    add2DNeighborDemand(_instList[ite->first-1], _gridList[_instList[ite->first-1]->getPos().first-1][prev_col-1]->cellInstList[j], 1);
                    add2DNeighborDemand(_gridList[_instList[ite->first-1]->getPos().first-1][prev_col-1]->cellInstList[j], _instList[ite->first-1], 1);
                    add3DNeighborDemand(_instList[ite->first-1], _gridList[_instList[ite->first-1]->getPos().first-1][prev_col-1]->cellInstList[j], 1);
                    add3DNeighborDemand(_gridList[_instList[ite->first-1]->getPos().first-1][prev_col-1]->cellInstList[j], _instList[ite->first-1], 1);
                }
            }
            else{
                unsigned next_col = _instList[ite->first-1]->getPos().second + 1;
                unsigned prev_col = _instList[ite->first-1]->getPos().second - 1;
                for(unsigned j=0;j<_gridList[_instList[ite->first-1]->getPos().first-1][next_col-1]->cellInstList.size();++j){
                    add2DNeighborDemand(_instList[ite->first-1], _gridList[_instList[ite->first-1]->getPos().first-1][next_col-1]->cellInstList[j], 1);
                    add2DNeighborDemand(_gridList[_instList[ite->first-1]->getPos().first-1][next_col-1]->cellInstList[j], _instList[ite->first-1], 1);
                    add3DNeighborDemand(_instList[ite->first-1], _gridList[_instList[ite->first-1]->getPos().first-1][next_col-1]->cellInstList[j], 1);
                    add3DNeighborDemand(_gridList[_instList[ite->first-1]->getPos().first-1][next_col-1]->cellInstList[j], _instList[ite->first-1], 1);
                }
                for(unsigned j=0;j<_gridList[_instList[ite->first-1]->getPos().first-1][prev_col-1]->cellInstList.size();++j){
                    add2DNeighborDemand(_instList[ite->first-1], _gridList[_instList[ite->first-1]->getPos().first-1][prev_col-1]->cellInstList[j], 1);
                    add2DNeighborDemand(_gridList[_instList[ite->first-1]->getPos().first-1][prev_col-1]->cellInstList[j], _instList[ite->first-1], 1);
                    add3DNeighborDemand(_instList[ite->first-1], _gridList[_instList[ite->first-1]->getPos().first-1][prev_col-1]->cellInstList[j], 1);
                    add3DNeighborDemand(_gridList[_instList[ite->first-1]->getPos().first-1][prev_col-1]->cellInstList[j], _instList[ite->first-1], 1);
                }
            }
            //add to new cellInstList
            _instList[ite->first-1]->getGrid()->cellInstList.push_back(_instList[ite->first-1]);

            for(unsigned j=0;j<_instList[ite->first-1]->assoNet.size();++j){
                _netList[_instList[ite->first-1]->assoNet[j]-1]->_toRemoveDemand = true;
            }

            cout << "CellInst " << ite->first << " is moved to (" << _instList[ite->first-1]->getPos().first << "," << _instList[ite->first-1]->getPos().second << ").\n";
        }
        ++ite;
    }
    cout << "CurMoveCnt: " << getCurMoveCnt() << "\n";
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
        bool min_lay_constraint = false;
        for(unsigned j=0;j<_instList[i]->assoNet.size();++j){
            if(_netList[_instList[i]->assoNet[j]-1]->getMinLayCons() == _laySupply.size()){
                min_lay_constraint = true;
                cout << "Instance " << i+1 << " is in min_lay_constraint net " << _instList[i]->assoNet[j] << "!\n";
                break;
            }
        }
        if(_instList[i]->is_movable() && (_instList[i]->_hasmovedbyfd == false) /*&& (min_lay_constraint == false)*/){
            //cout << "CellInst " << i+1 << " on (" << _instList[i]->getPos().first << "," << _instList[i]->getPos().second << ") has 2dcongestion " << setprecision(3) << _instList[i]->getGrid()->get2dCongestion() << "\n";
            moveCell = _instList[i];
            s = i;
            break;
        }
    }
    
    if(s<_instList.size()-1){
        for(unsigned i=s+1; i<_instList.size(); ++i){
            // cout << "Grid addr : " << _instList[i]->getGrid() << " Mgr Grid Addr : " << _gridList[_instList[i]->getPos().first-1][_instList[i]->getPos().second-1] << endl; 
            // cout << "CellInst " << i+1 << " on (" << _instList[i]->getPos().first << "," << _instList[i]->getPos().second << ") has 2dcongestion " << setprecision(3) << _instList[i]->getGrid()->get2dCongestion() << "\n";
            bool min_lay_constraint = false;
            for(unsigned j=0;j<_instList[i]->assoNet.size();++j){
                if(_netList[_instList[i]->assoNet[j]-1]->getMinLayCons() == _laySupply.size()){
                    min_lay_constraint = true;
                    cout << "Instance " << i+1 << " is in min_lay_constraint net " << _instList[i]->assoNet[j] << "!\n";
                    break;
                }
            }
            if((_instList[i]->is_movable()) && (_instList[i]->getGrid()->get2dCongestion() < moveCell->getGrid()->get2dCongestion()) && (_instList[i]->_hasmovedbyfd == false) /*&& (min_lay_constraint == false)*/)
                moveCell = _instList[i];
        }
    }
    else if(_instList[s]->is_movable() && (_instList[s]->_hasmovedbyfd == false)){
        moveCell = _instList[s];
    }
    else{
        netbasedPlace();
        return;
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
    //remove from original cellInstList
    for(unsigned j=0;j<moveCell->getGrid()->cellInstList.size();++j){
        if(moveCell->getGrid()->cellInstList[j] == moveCell){
            moveCell->getGrid()->cellInstList.erase(moveCell->getGrid()->cellInstList.begin() + j);
            break;
        }
    }
    //remove same gGrid demand
    for(unsigned j=0;j<moveCell->getGrid()->cellInstList.size();++j){
        if(moveCell->getGrid()->cellInstList[j] != moveCell){
            remove2DNeighborDemand(moveCell,moveCell->getGrid()->cellInstList[j],0);
            remove3DNeighborDemand(moveCell,moveCell->getGrid()->cellInstList[j],0);
        }
    }
    //remove adjHGrid demand
    if(moveCell->getPos().second == Ggrid::cBeg){
        unsigned next_col = moveCell->getPos().second + 1;
        for(unsigned j=0;j<_gridList[moveCell->getPos().first-1][next_col-1]->cellInstList.size();++j){
            remove2DNeighborDemand(moveCell, _gridList[moveCell->getPos().first-1][next_col-1]->cellInstList[j], 1);
            remove2DNeighborDemand(_gridList[moveCell->getPos().first-1][next_col-1]->cellInstList[j], moveCell, 1);
            remove3DNeighborDemand(moveCell, _gridList[moveCell->getPos().first-1][next_col-1]->cellInstList[j], 1);
            remove3DNeighborDemand(_gridList[moveCell->getPos().first-1][next_col-1]->cellInstList[j], moveCell, 1);
        }
    }
    else if(moveCell->getPos().second == Ggrid::cEnd){
        unsigned prev_col = moveCell->getPos().second - 1;
        for(unsigned j=0;j<_gridList[moveCell->getPos().first-1][prev_col-1]->cellInstList.size();++j){
            remove2DNeighborDemand(moveCell, _gridList[moveCell->getPos().first-1][prev_col-1]->cellInstList[j], 1);
            remove2DNeighborDemand(_gridList[moveCell->getPos().first-1][prev_col-1]->cellInstList[j], moveCell, 1);
            remove3DNeighborDemand(moveCell, _gridList[moveCell->getPos().first-1][prev_col-1]->cellInstList[j], 1);
            remove3DNeighborDemand(_gridList[moveCell->getPos().first-1][prev_col-1]->cellInstList[j], moveCell, 1);
        }
    }
    else{
        unsigned next_col = moveCell->getPos().second + 1;
        unsigned prev_col = moveCell->getPos().second - 1;
        for(unsigned j=0;j<_gridList[moveCell->getPos().first-1][next_col-1]->cellInstList.size();++j){
            remove2DNeighborDemand(moveCell, _gridList[moveCell->getPos().first-1][next_col-1]->cellInstList[j], 1);
            remove2DNeighborDemand(_gridList[moveCell->getPos().first-1][next_col-1]->cellInstList[j], moveCell, 1);
            remove3DNeighborDemand(moveCell, _gridList[moveCell->getPos().first-1][next_col-1]->cellInstList[j], 1);
            remove3DNeighborDemand(_gridList[moveCell->getPos().first-1][next_col-1]->cellInstList[j], moveCell, 1);
        }
        for(unsigned j=0;j<_gridList[moveCell->getPos().first-1][prev_col-1]->cellInstList.size();++j){
            remove2DNeighborDemand(moveCell, _gridList[moveCell->getPos().first-1][prev_col-1]->cellInstList[j], 1);
            remove2DNeighborDemand(_gridList[moveCell->getPos().first-1][prev_col-1]->cellInstList[j], moveCell, 1);
            remove3DNeighborDemand(moveCell, _gridList[moveCell->getPos().first-1][prev_col-1]->cellInstList[j], 1);
            remove3DNeighborDemand(_gridList[moveCell->getPos().first-1][prev_col-1]->cellInstList[j], moveCell, 1);
        }
    }
    
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
    
    // <Koova edited>
    if (Pos(new_row,new_col) != moveCell->getInitPos()) {
        _curMovedSet.insert(moveCell);
    } else {
        _curMovedSet.erase(moveCell);
    }
    
    moveCell->_hasmovedbyfd = true;
    cout << "CellInst " << moveCell->getId() << " is moved!\n";
    cout << "CurMoveCnt: " << getCurMoveCnt() << "\n";
    // </Koova edited>

    cout << "Old position: " << moveCell->getPos().first << " " << moveCell->getPos().second << "\n";
    moveCell->move(Pos(new_row,new_col));
    add2DBlkDemand(moveCell);
    add3DBlkDemand(moveCell);
    //remove same gGrid demand
    for(unsigned j=0;j<moveCell->getGrid()->cellInstList.size();++j){
        if(moveCell->getGrid()->cellInstList[j] != moveCell){
            add2DNeighborDemand(moveCell,moveCell->getGrid()->cellInstList[j],0);
            add3DNeighborDemand(moveCell,moveCell->getGrid()->cellInstList[j],0);
        }
    }
    //remove adjHGrid demand
    if(moveCell->getPos().second == Ggrid::cBeg){
        unsigned next_col = moveCell->getPos().second + 1;
        for(unsigned j=0;j<_gridList[moveCell->getPos().first-1][next_col-1]->cellInstList.size();++j){
            add2DNeighborDemand(moveCell, _gridList[moveCell->getPos().first-1][next_col-1]->cellInstList[j], 1);
            add2DNeighborDemand(_gridList[moveCell->getPos().first-1][next_col-1]->cellInstList[j], moveCell, 1);
            add3DNeighborDemand(moveCell, _gridList[moveCell->getPos().first-1][next_col-1]->cellInstList[j], 1);
            add3DNeighborDemand(_gridList[moveCell->getPos().first-1][next_col-1]->cellInstList[j], moveCell, 1);
        }
    }
    else if(moveCell->getPos().second == Ggrid::cEnd){
        unsigned prev_col = moveCell->getPos().second - 1;
        for(unsigned j=0;j<_gridList[moveCell->getPos().first-1][prev_col-1]->cellInstList.size();++j){
            add2DNeighborDemand(moveCell, _gridList[moveCell->getPos().first-1][prev_col-1]->cellInstList[j], 1);
            add2DNeighborDemand(_gridList[moveCell->getPos().first-1][prev_col-1]->cellInstList[j], moveCell, 1);
            add3DNeighborDemand(moveCell, _gridList[moveCell->getPos().first-1][prev_col-1]->cellInstList[j], 1);
            add3DNeighborDemand(_gridList[moveCell->getPos().first-1][prev_col-1]->cellInstList[j], moveCell, 1);
        }
    }
    else{
        unsigned next_col = moveCell->getPos().second + 1;
        unsigned prev_col = moveCell->getPos().second - 1;
        for(unsigned j=0;j<_gridList[moveCell->getPos().first-1][next_col-1]->cellInstList.size();++j){
            add2DNeighborDemand(moveCell, _gridList[moveCell->getPos().first-1][next_col-1]->cellInstList[j], 1);
            add2DNeighborDemand(_gridList[moveCell->getPos().first-1][next_col-1]->cellInstList[j], moveCell, 1);
            add3DNeighborDemand(moveCell, _gridList[moveCell->getPos().first-1][next_col-1]->cellInstList[j], 1);
            add3DNeighborDemand(_gridList[moveCell->getPos().first-1][next_col-1]->cellInstList[j], moveCell, 1);
        }
        for(unsigned j=0;j<_gridList[moveCell->getPos().first-1][prev_col-1]->cellInstList.size();++j){
            add2DNeighborDemand(moveCell, _gridList[moveCell->getPos().first-1][prev_col-1]->cellInstList[j], 1);
            add2DNeighborDemand(_gridList[moveCell->getPos().first-1][prev_col-1]->cellInstList[j], moveCell, 1);
            add3DNeighborDemand(moveCell, _gridList[moveCell->getPos().first-1][prev_col-1]->cellInstList[j], 1);
            add3DNeighborDemand(_gridList[moveCell->getPos().first-1][prev_col-1]->cellInstList[j], moveCell, 1);
        }
    }
    //add to new cellInstList
    moveCell->getGrid()->cellInstList.push_back(moveCell);

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

RouteExecStatus
RouteMgr::errorOption(RouteExecError rError)
{
    switch (rError) {
        case ROUTE_OVERFLOW:
            cerr << "Overflow is inevitable!\n";
            cerr << "Try net-based placement!\n";
            return ROUTE_EXEC_ERROR;
        case ROUTE_DIR_ILLEGAL:
            cerr << "Wrong direction segment is inevitable!\n";
            cerr << "Do placement again!\n";
            return ROUTE_EXEC_ERROR;
        default:
            cerr << "Unknown error!\n";
            return ROUTE_EXEC_ERROR;
    }
}

RouteExecStatus
RouteMgr::layerassign(Net* net)
{
    cout << "\nLayerAssign...\n";
    vector<Segment*> toDel;
    //unsigned maxLayer = _laySupply.size();
    RouteExecStatus myStatus = ROUTE_EXEC_DONE;
    RouteExecError myError = ROUTE_EXEC_ERROR_TOT;

        // 1. Find candidates
        int layCons = net->_minLayCons;
        int minLayer = layCons ? layCons : 1;

        int curLayer = 1;
        int targetLayer = minLayer;
        vector<int> candidatesV;
        vector<int> candidatesH;
        net->findHCand(candidatesH);
        net->findVCand(candidatesV);
        set<Layer*> myAlpha;

        unsigned segCnt = net->_netSegs.size();
        for (unsigned i=0; i<segCnt; ++i)
        {
            Segment* seg = net->_netSegs[i];
            vector<int> candidates;
            
            if (!i) { curLayer = seg->startPos[2]; }

            // Z
            if (seg->checkDir() == DIR_Z)
            {
                if (!(seg->startPos[2] >= unsigned(minLayer) || seg->endPos[2] >= unsigned(minLayer))) {
                    if (seg->startPos[2] > seg->endPos[2]) {
                        seg->startPos[2] = seg->endPos[2];
                        seg->endPos[2] = minLayer;
                        curLayer = minLayer;
                    } else {
                        seg->endPos[2] = minLayer;
                        curLayer = minLayer;
                    }
                } else if (seg->endPos[2] == seg->startPos[2]) {
                    if (seg->startPos[2] == unsigned(curLayer)) {
                        seg->print();
                        #ifdef DEBUG
                        cout << " Stupid seg... Delete it!\n";
                        #endif
                        //toDel.push_back(seg);
                        //net->_netSegs.erase(net->_netSegs.begin()+i);
                        //curLayer = seg->startPos[2];
                    } else {
                        seg->startPos[2] = curLayer;
                    }
                }
                set<Layer*> newZGrids = seg->newGrid(net, myAlpha);
                for (auto g : newZGrids) { g->addDemand(1); }
                if (seg->checkOverflow()) {
                    myStatus = ROUTE_EXEC_ERROR;
                    myError = ROUTE_OVERFLOW;
                    errorOption(myError);
                }
                //for (auto g : newZGrids) { g->removeDemand(1); }
                for (auto g : newZGrids) { myAlpha.insert(g); /*g->addDemand(1);*/ }
                #ifdef DEBUG
                cout << "Successfully assigned...";
                seg->print();
                cout << "\n";
                cout << "\n";
                #endif
                continue;
            }
            
            // H or V
            // Try to select a layer with no overflow for the H or V segment
            // If overflow is inevitable, choose the layer closest to curLayer
            // Connect to the net with Z-segments
            if (seg->startPos[2] && unsigned(curLayer) != seg->startPos[2]) {
                // Add a Z-seg
                Segment* zSeg = new Segment(seg);
                zSeg->startPos[2] = curLayer;
                zSeg->endPos[0] = seg->startPos[0];
                zSeg->endPos[1] = seg->startPos[1];
                zSeg->endPos[2] = seg->startPos[2];
                net->addSeg(zSeg);
                #ifdef DEBUG
                cout << "Add new Segment ";
                zSeg->print();
                cout << endl;
                #endif
                /*
                for (unsigned j=zSeg->startPos[2]; j<=zSeg->endPos[2]; ++j) {
                    if (this->check3dOverflow(zSeg->startPos[0], zSeg->startPos[1], j) == GRID_FULL_CAP) {
                        myStatus = ROUTE_EXEC_ERROR;
                        cout << "Should pass it\n";
                    }
                }
                */
                set<Layer*> newZGrids = zSeg->newGrid(net, myAlpha);
                for (auto g : newZGrids) { g->addDemand(1); }
                if (zSeg->checkOverflow()) {
                    myStatus = ROUTE_EXEC_ERROR;
                    myError = ROUTE_OVERFLOW;
                    errorOption(myError);
                }
                //for (auto g : newZGrids) { g->removeDemand(1); }
                for (auto g : newZGrids) { myAlpha.insert(g); /*g->addDemand(1);*/ }
                curLayer = (unsigned(curLayer) < seg->startPos[2]) ? curLayer : seg->startPos[2];
            }

            int diff = INT16_MAX;
            if (seg->checkDir() == DIR_H)
            {
                if (candidatesH.size() == 0) {
                    myStatus = ROUTE_EXEC_ERROR;
                    myError = ROUTE_DIR_ILLEGAL;
                    errorOption(myError);
                    targetLayer = 1;
                    diff = 0;
                } else {
                    for (auto& j : candidatesH) {
                        Segment newSeg = Segment(*seg);
                        newSeg.assignLayer(j);
                        Segment newZSeg = Segment(*seg);
                        newZSeg.startPos[2] = curLayer;
                        newZSeg.endPos[0] = seg->startPos[0];
                        newZSeg.endPos[1] = seg->startPos[1];
                        newZSeg.endPos[2] = targetLayer;
                        set<Layer*> newGrids = newSeg.newGrid(net, myAlpha);
                        set<Layer*> newZGrids = newZSeg.newGrid(net, myAlpha);
                        for (auto zg : newZGrids) { newGrids.insert(zg); }
                        for (auto g : newGrids) { g->addDemand(1); }
                        if (!newSeg.checkOverflow() && !newZSeg.checkOverflow()) {
                            diff = ((diff) < (j-curLayer)) ? diff : j-curLayer;
                        }
                        for (auto g : newGrids) { g->removeDemand(1); }
                    }
                    if (diff == INT16_MAX) {
                        myStatus = ROUTE_EXEC_ERROR;
                        myError = ROUTE_OVERFLOW;
                        errorOption(myError);
                        for (auto& j : candidatesH) {
                            diff = ((diff) < (j-curLayer)) ? diff : j-curLayer;
                        }
                    }
                }
            } else {
                if (candidatesV.size() == 0) {
                    myStatus = ROUTE_EXEC_ERROR;
                    myError = ROUTE_DIR_ILLEGAL;
                    errorOption(myError);
                    targetLayer = 1;
                    diff = 0;
                } else {
                    for (auto& j : candidatesV) {
                        Segment newSeg = Segment(*seg);
                        newSeg.assignLayer(j);
                        Segment newZSeg = Segment(*seg);
                        newZSeg.startPos[2] = curLayer;
                        newZSeg.endPos[0] = seg->startPos[0];
                        newZSeg.endPos[1] = seg->startPos[1];
                        newZSeg.endPos[2] = targetLayer;
                        set<Layer*> newGrids = newSeg.newGrid(net, myAlpha);
                        set<Layer*> newZGrids = newZSeg.newGrid(net, myAlpha);
                        for (auto zg : newZGrids) { newGrids.insert(zg); }
                        for (auto g : newGrids) { g->addDemand(1); }
                        if (!newSeg.checkOverflow() && !newZSeg.checkOverflow()) {
                            diff = ((diff) < (j-curLayer)) ? diff : j-curLayer;
                        }
                        for (auto g : newGrids) { g->removeDemand(1); }
                    }
                    if (diff == INT16_MAX) {
                        myStatus = ROUTE_EXEC_ERROR;
                        myError = ROUTE_OVERFLOW;
                        errorOption(myError);
                        for (auto& j : candidatesV) {
                            diff = ((diff) < (j-curLayer)) ? diff : j-curLayer;
                        }
                    }
                }
            }
            targetLayer = curLayer + diff;

            
            
            if (curLayer != targetLayer) {
                // Add a Z-seg
                Segment* zSeg = new Segment(seg);
                zSeg->startPos[2] = curLayer;
                zSeg->endPos[0] = seg->startPos[0];
                zSeg->endPos[1] = seg->startPos[1];
                zSeg->endPos[2] = targetLayer;
                net->addSeg(zSeg);
                #ifdef DEBUG
                cout << "Add new Segment ";
                zSeg->print();
                cout << endl;
                #endif
                curLayer = targetLayer;
                /*
                for (unsigned j=zSeg->startPos[2]; j<=zSeg->endPos[2]; ++j) {
                    if (this->check3dOverflow(zSeg->startPos[0], zSeg->startPos[1], j) == GRID_FULL_CAP) {
                        myStatus = ROUTE_EXEC_ERROR;
                        cout << "Should pass it\n";
                    }
                }*/
                set<Layer*> newZGrids = zSeg->newGrid(net, myAlpha);
                for (auto g : newZGrids) { myAlpha.insert(g); g->addDemand(1); }
                if (zSeg->checkOverflow()) {
                    myStatus = ROUTE_EXEC_ERROR;
                    myError = ROUTE_OVERFLOW;
                    errorOption(myError);
                }
            }
            
            if (seg->endPos[2] && i == segCnt-1) {
                #ifdef DEBUG
                cout << "Last Seg ";
                seg->print();
                cout << "\n";
                #endif
                if (unsigned(curLayer) != seg->endPos[2]) {
                    // Add a Z-seg
                    Segment* zSeg1 = new Segment(seg);
                    zSeg1->startPos[0] = seg->endPos[0];
                    zSeg1->startPos[1] = seg->endPos[1];
                    zSeg1->startPos[2] = curLayer;
                    net->addSeg(zSeg1);
                    #ifdef DEBUG
                    cout << "Add new Segment ";
                    zSeg1->print();
                    cout << endl;
                    #endif
                    /*
                    for (unsigned j=zSeg1->startPos[2]; j<=zSeg1->endPos[2]; ++j) {
                        if (this->check3dOverflow(zSeg1->startPos[0], zSeg1->startPos[1], j) == GRID_FULL_CAP) {
                            myStatus = ROUTE_EXEC_ERROR;
                            cout << "Should pass it\n";
                        }
                    }
                    */
                    set<Layer*> newZGrids = zSeg1->newGrid(net, myAlpha);
                    for (auto g : newZGrids) { myAlpha.insert(g); g->addDemand(1); }
                    if (zSeg1->checkOverflow()) {
                        myStatus = ROUTE_EXEC_ERROR;
                        myError = ROUTE_OVERFLOW;
                        errorOption(myError);
                    }
                }
            }
            // Finish layer assignment
            seg->startPos[2] = targetLayer;
            seg->endPos[2] = targetLayer;
            if (seg->checkOverflow()) {
                myStatus = ROUTE_EXEC_ERROR;
                myError = ROUTE_OVERFLOW;
                errorOption(myError);
            }
            set<Layer*> decision = seg->newGrid(net, myAlpha);
            for (auto g : decision) {
                myAlpha.insert(g);
                g->addDemand(1); }
            #ifdef DEBUG
            cout << "Successfully assigned...";
            seg->print();
            cout << "\n";
            cout << "\n";
            #endif
        }

        // Final check for the net
        if (net->checkOverflow()) {
            myStatus = ROUTE_EXEC_ERROR;
            myError = ROUTE_OVERFLOW;
            //net->shouldReroute(true);
        }
    assert(myStatus != ROUTE_EXEC_TOT);
    if (myStatus == ROUTE_EXEC_ERROR) {
        return errorOption(myError);
    } else { return ROUTE_EXEC_DONE; }
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
