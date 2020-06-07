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
#include "routeMgr.h"
#include "../util/util.h"

using namespace std;

// TODO: Implement member functions for class RouteMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
RouteMgr* routeMgr = 0;
/*
enum RouteParseError {
  EXTRA_SPACE,
  MISSING_SPACE,
  ILLEGAL_WSPACE,
  ILLEGAL_NUM,
  ILLEGAL_IDENTIFIER,
  ILLEGAL_SYMBOL_TYPE,
  ILLEGAL_SYMBOL_NAME,
  MISSING_NUM,
  MISSING_IDENTIFIER,
  MISSING_NEWLINE,
  MISSING_DEF,
  CANNOT_INVERTED,
  MAX_LIT_ID,
  REDEF_GATE,
  REDEF_SYMBOLIC_NAME,
  REDEF_CONST,
  NUM_TOO_SMALL,
  NUM_TOO_BIG,

  DUMMY_END
};

/**************************************/
/*   Static variables and functions   */
/**************************************/
/*
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;

static bool
parseError(RouteParseError err)
{
  switch (err) {
    case EXTRA_SPACE:
        cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
            << ": Extra space character is detected!!" << endl;
        break;
    case MISSING_SPACE:
        cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
            << ": Missing space character!!" << endl;
        break;
    case ILLEGAL_WSPACE: // for non-space white space character
        cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
            << ": Illegal white space char(" << errInt
            << ") is detected!!" << endl;
        break;
    case ILLEGAL_NUM:
        cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
            << errMsg << "!!" << endl;
        break;
    case ILLEGAL_IDENTIFIER:
        cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
            << errMsg << "\"!!" << endl;
        break;
    case ILLEGAL_SYMBOL_TYPE:
        cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
            << ": Illegal symbol type (" << errMsg << ")!!" << endl;
        break;
    case ILLEGAL_SYMBOL_NAME:
        cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
            << ": Symbolic name contains un-printable char(" << errInt
            << ")!!" << endl;
        break;
    case MISSING_NUM:
        cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
            << ": Missing " << errMsg << "!!" << endl;
        break;
    case MISSING_IDENTIFIER:
        cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
            << errMsg << "\"!!" << endl;
        break;
    case MISSING_NEWLINE:
        cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
            << ": A new line is expected here!!" << endl;
        break;
    case MISSING_DEF:
        cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
            << " definition!!" << endl;
        break;
    case CANNOT_INVERTED:
        cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
            << ": " << errMsg << " " << errInt << "(" << errInt/2
            << ") cannot be inverted!!" << endl;
        break;
    case MAX_LIT_ID:
        cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
            << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
            << endl;
        break;
    case REDEF_GATE:
        cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
            << "\" is redefined, previously defined as "
            << "errGate->getTypeStr()" << " in line " << "errGate->getLineNo()"
            << "!!" << endl;
        break;
    case REDEF_SYMBOLIC_NAME:
        cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
            << errMsg << errInt << "\" is redefined!!" << endl;
        break;
    case REDEF_CONST:
        cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
            << ": Cannot redefine const (" << errInt << ")!!" << endl;
        break;
    case NUM_TOO_SMALL:
        cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
            << " is too small (" << errInt << ")!!" << endl;
        break;
    case NUM_TOO_BIG:
        cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
            << " is too big (" << errInt << ")!!" << endl;
        break;
    default: break;
  }
  return false;
}
*/
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
    string buffer;
    int supply; // To save defaultSupply
    // LayerList myLayers;
    uint tmpCnt; // tempCount
    uint tmpCnt1;
    uint tmpCnt2;
    uint tmpCnt3;
    bool tempQ;

    unsigned bndCoord[4];
    unsigned gridCoord[3];
    ifs >> buffer; // MaxCellMove
    ifs >> maxMoveCnt; 
    ifs >> buffer; // GGridBoundaryIdx
    for(unsigned i=0; i<4; ++i)
        ifs >> bndCoord[i];
    Ggrid::setBoundary(bndCoord[0], bndCoord[1], bndCoord[2], bndCoord[3]);
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
        // myLayers.push_back(lay);
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

    // Initial cell and net placement
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
        Ggrid* cool = new Ggrid(Pos(tmpCnt1, tmpCnt2));
        CellInst* love = new CellInst(i+1, cool, this->_mcList[mcNum-1], tempQ);
        _instList.push_back(love);
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
            brook = new Net(layCons);
        } else {
            brook = new Net(0); // [Important] NoCstr
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
            brook->addPin(Pos(inst, mstrPin));
        }
        _netList.push_back(brook);
    }

    // Initial routing data
    ifs >> buffer; // NumRoutes
    ifs >> tmpCnt; // routeSegmentCount
    for(unsigned i=0; i<tmpCnt; ++i)
    {
        for(unsigned i=0; i<3; ++i)
            ifs >> gridCoord[i];
            // TODO: save to starting point
        for(unsigned i=0; i<3; ++i)
            ifs >> gridCoord[i];
            // TODO: save to ending point
        ifs >> buffer; // netName
    }

    return true;
}

void
RouteMgr::printRouteSummary()
{
    cout << "MaxCellMove" << setw(8) << maxMoveCnt << endl;
    // TODO
    cout << "CurrentMove" << setw(8) << 0 << endl;
    cout << "Boundary" << setw(5) << Ggrid::xMin << " " << Ggrid::yMin << " " << Ggrid::xMax
    << " " << Ggrid::yMax << endl;
    cout << "Total layer" << setw(8) << _laySupply.size() << endl;
    cout << "Total MC" << setw(11) << _mcList.size() << endl;
    // TODO: Support more information
    // cout << "CellInst" << setw(11) << numCellInst << endl;
    // cout << "OrigNet" << setw(12) << numNet << endl;
    // cout << "CurrNet" << setw(12) << numNet << endl;
    // cout << "OrigRoute" << setw(10) << numRoute << endl;    
    // cout << "CurrRoute" << setw(10) << numRoute << endl;
}

void
RouteMgr::printNetlist()
{
    for(unsigned i=0; i<_netList.size(); ++i)
    {
        cout << endl;
        cout << "MinLayerConstr " << _netList[i]->getMinLayCons() << endl;
        _netList[i]->printPinSet();
    }
}

void
RouteMgr::printMCList()
{
    for(unsigned i=0; i<_mcList.size(); ++i)
    {
        cout << endl;
        _mcList[i]->printMC();
    }
}

void
RouteMgr::printLaySupply()
{
    for(unsigned i=0; i<_laySupply.size(); ++i)
    {
        cout << _laySupply[i] << endl;
    }
}

void
RouteMgr::printExtraDemand()
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
RouteMgr::printNonDefaultSupply()
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
RouteMgr::printCellInst()
{
    for(unsigned i=0; i<_instList.size(); ++i)
    {
        cout << endl;
        _instList[i]->printCell();
    }
}

void
RouteMgr::initSupply()
{
    /*Psuedo Code
    total_default_supply = 0;
    for i=1 to _layerSupply.length
        total_default_supply += _layerSupply[i];
    for i=1 to gridList.length
        gridList[i]._2dSupply = total_default_supply;
        for i=1 to _layerSupply.length
            use unorder_map to add/sub nondefault supply
    */
}

void
RouteMgr::add2DDemand(Net* net) //Intialize、after each route
{
    /*Psuedo Code
    unsigned available_layer = _layerSupply.length - net->_minLayCons;
    double constraint = _layerSupply.length/available_layer;

    go through net, for every passing grid g, call g.updateDemand(constraint)
    */
}

void 
RouteMgr::remove2DDemand(Net* net) //before each route
{
    /*Psuedo Code
    unsigned available_layer = _layerSupply.length - net->_minLayCons;
    double constraint = _layerSupply.length/available_layer;

    go through net, for every passing grid g, call g.updateDemand(-constraint)
    */
}
