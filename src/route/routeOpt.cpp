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
    cout << "Place..." << "(Not function-ready!)" << endl;
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
        CellInst* moveCell = _instList[0];
        for(unsigned i=1;i<_instList.size();++i){
            if(_instList[i]->getGrid()->get2dCongestion() < moveCell->getGrid()->get2dCongestion())
                moveCell = _instList[i];
        }
        //go through _netList, find out all associated nets and thus associated cells and multiplications, then calculating new pos
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
    _instList[0]->move(Pos((Ggrid::yMax + fst)/2, (Ggrid::xMax + sec)/2));
    _movedList.push_back(_instList[0]);
    change_notifier(_instList[0]);
    ++curMoveCnt;
}

void
RouteMgr::change_notifier(CellInst* su)
{
    for(int i=0; i<su->assoNet.size(); ++i)
    {
        _netList[su->assoNet[i]-1]->shouldReroute(true);
    }
}

void
RouteMgr::koova_route()
{
    if (curRouteSegs.empty()) { curRouteSegs = initRouteSegs; }
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
