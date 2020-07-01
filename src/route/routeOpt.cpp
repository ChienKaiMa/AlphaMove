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
    /*Psuedo code
        1: for net i=1 to n do
        2: Compute CenterY (i) of the bounding box of net i
        3: Compute the average horizontal congestion CH(i)
        4: end for
        5: for net i=1 to n do
        6: NewCenterY (i)=CenterY (i)
        7: if CH(i) < 0 then
        8: BestCH = CH(i)
        9: for each net j where Share(i; j) > 0 do
        10: if BestCH < CH(i) then
        11: BestCH = CH(i)
        12: end if
        13: end for
        14: if BestCH > 0 and BestCH ̸= CH(i) then
        15: Update NewCenterY (i) using equation 5
        16: end if
        17: end if
        18: end for
        19: for cell q=1 to m do
        20: MinY = +∞;MaxY = 0
        21: Let Q be the set of associated nets of cell q
        22: for each connected net i ∈ Q do
        23: OffsetY = CellCoordY (q) − CenterY (i)
        24: CellY (i) = OffsetY + NewCenterY (i)
        25: if MinY > CellY (i) then
        26: MinY = CellY (i)
        27: else if MaxY < CellY (i) then
        28: MaxY = CellY (i)
        29: end if
        30: end for
        31: MidY = (MinY +MaxY )=2
        32: Find i1; i2 ∈ Q, such that CellY (i1) and CellY (i2) are closest to MidY , and CellY (i1) ≤ MidY ≤ CellY (i2)
        33: Move CellCoordY (q) to the closest position in the range [CellY (i1), CellY (i2)]
        34: end for
    */
    }
    else{ //force-directed
        CellInst* moveCell;
        unsigned s;
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
        }
        new_row = (int)(round((double)(row_numerator) / (double)(row_denominator)));
        new_col = (int)(round((double)(col_numerator) / (double)(col_denominator)));
        moveCell->move(Pos(new_row,new_col));
        cout << "New position: " << moveCell->getPos().first << " " << moveCell->getPos().second << "\n";
    }
}

void RouteMgr::layerassign()
{
    cout << "LayerAssign..." << "(Not function-ready!)" << endl;
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
