/****************************************************************************
  FileName     [ routeCmd.h ]
  PackageName  [ route ]
  Synopsis     [ Define basic route package commands ]
  Author       [ Chien-Kai Ma, Kai-Chun Chang, Yu-Wei Fan ]
  Copyright    [ Copyleft(c) 2020-present NTU, Taiwan ]
****************************************************************************/

#ifndef ROUTE_CMD_H
#define ROUTE_CMD_H

#include "cmdParser.h"

CmdClass(RouteReadCmd);
CmdClass(RouteWriteCmd);
CmdClass(RouteOptCmd);
CmdClass(MgrPrintCmd);
CmdClass(CellPrintCmd);
CmdClass(LayPrintCmd);
CmdClass(NetPrintCmd);

#endif // ROUTE_CMD_H
