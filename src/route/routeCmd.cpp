/****************************************************************************
  FileName     [ routeCmd.cpp ]
  PackageName  [ route ]
  Synopsis     [ Define basic route package commands ]
  Author       [ Chien-Kai Ma, Kai-Chun Chang, Yu-Wei Fan ]
  Copyright    [ Copyleft(c) 2020-present NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include <iostream>
#include <iomanip>
#include "routeMgr.h"
#include "routeCmd.h"
#include "util.h"

using namespace std;

extern RouteMgr* routeMgr;
extern int effLimit;

bool
initRouteCmd()
{
   if (!(cmdMgr->regCmd("REad", 2, new RouteReadCmd) &&
         cmdMgr->regCmd("Write", 1, new RouteWriteCmd) &&
         cmdMgr->regCmd("Place", 1, new PlaceCmd) &&
         cmdMgr->regCmd("ROute", 2, new RouteCmd) &&
         cmdMgr->regCmd("Optimize", 1, new OptimizeCmd) &&
         cmdMgr->regCmd("Mgrprint", 1, new MgrPrintCmd) &&
         cmdMgr->regCmd("Layprint", 1, new LayPrintCmd) &&
         cmdMgr->regCmd("Netprint", 1, new NetPrintCmd) &&
         cmdMgr->regCmd("Cellprint", 1, new CellPrintCmd)
      )) {
      cerr << "Registering \"route\" commands fails... exiting" << endl;
      return false;
   }
   return true;
}

enum RouteCmdState
{
   // Order matters! Do not change the order!!
   ROUTEINIT,
   CIRREAD,
   PLACE,
   ROUTE,
   ROUTEOPT,
   // dummy end
   CIRCMDTOT
};

static RouteCmdState curCmd = ROUTEINIT;

//----------------------------------------------------------------------
//    Read <(string fileName)> [-Replace]
//----------------------------------------------------------------------
CmdExecStatus
RouteReadCmd::exec(const string& option)
{
   // check option
   vector<string> options;
   if (!CmdExec::lexOptions(option, options))
      return CMD_EXEC_ERROR;
   if (options.empty())
      return CmdExec::errorOption(CMD_OPT_MISSING, "");

   bool doReplace = false;
   string fileName;
   for (size_t i = 0, n = options.size(); i < n; ++i) {
      if (myStrNCmp("-Replace", options[i], 2) == 0) {
         if (doReplace) return CmdExec::errorOption(CMD_OPT_EXTRA,options[i]);
         doReplace = true;
      }
      else {
         if (fileName.size())
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[i]);
         fileName = options[i];
      }
   }

   if (routeMgr != 0) {
      if (doReplace) {
         cerr << "Note: original circuit is replaced..." << endl;
         curCmd = ROUTEINIT;
         delete routeMgr; routeMgr = 0;
      }
      else {
         cerr << "Error: circuit already exists!!" << endl;
         return CMD_EXEC_ERROR;
      }
   }
   routeMgr = new RouteMgr;

   if (!routeMgr->readCircuit(fileName)) {
      curCmd = ROUTEINIT;
      delete routeMgr; routeMgr = 0;
      return CMD_EXEC_ERROR;
   }

   curCmd = CIRREAD;

   return CMD_EXEC_DONE;
}

void
RouteReadCmd::usage(ostream& os) const
{
   os << "Usage: Read <(string fileName)> [-Replace]" << endl;
}

void
RouteReadCmd::help() const
{
   cout << setw(15) << left << "Read: "
        << "read in routing data and construct the netlist" << endl;
}

//----------------------------------------------------------------------
//    Write [(int gateId)][-Output (string txtFile)][-Demand (string File)]
//----------------------------------------------------------------------
CmdExecStatus
RouteWriteCmd::exec(const string& option)
{
   if (!routeMgr) {
      cerr << "Error: circuit is not yet constructed!!" << endl;
      return CMD_EXEC_ERROR;
   }
   // check option
   vector<string> options;
   CmdExec::lexOptions(option, options);

   if (options.empty()) {
      // TODO
      cout << "Constructing..." << endl;
      routeMgr->writeCircuit(cout);
      return CMD_EXEC_DONE;
   }
   bool hasFile = false;
   bool type = 0;

   ofstream outfile;
   for (size_t i = 0, n = options.size(); i < n; ++i) {
      if (myStrNCmp("-Output", options[i], 2) == 0) {
         type = 0;
         if (hasFile) 
            return CmdExec::errorOption(CMD_OPT_EXTRA, options[i]);
         if (++i == n)
            return CmdExec::errorOption(CMD_OPT_MISSING, options[i-1]);
         outfile.open(options[i].c_str(), ios::out);
         if (!outfile)
            return CmdExec::errorOption(CMD_OPT_FOPEN_FAIL, options[1]);
         hasFile = true;
      }
      else if (myStrNCmp("-Demand", options[i], 2) == 0) {
         type = 1;
         if (hasFile) 
            return CmdExec::errorOption(CMD_OPT_EXTRA, options[i]);
         if (++i == n)
            return CmdExec::errorOption(CMD_OPT_MISSING, options[i-1]);
         outfile.open(options[i].c_str(), ios::out);
         if (!outfile)
            return CmdExec::errorOption(CMD_OPT_FOPEN_FAIL, options[1]);
         hasFile = true;
      }
      else return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[i]);
   }
   assert (hasFile);
   if (type == 0)
      routeMgr->writeCircuit(outfile);
   else
      routeMgr->writeDemand(outfile);

   return CMD_EXEC_DONE;
}

void
RouteWriteCmd::usage(ostream& os) const
{
   os << "Usage: Write [(int gateId)][-Output (string txtFile)][-Demand (string File)]" << endl;
}

void
RouteWriteCmd::help() const
{
   cout << setw(15) << left << "RouteWrite: "
        << "write the netlist to a new text file (.txt)\n";
}

//----------------------------------------------------------------------
//    Place [ -Default | -Net | -Force | -Check | -Summary ]
//----------------------------------------------------------------------
CmdExecStatus
PlaceCmd::exec(const string& option)
{
   if (!routeMgr) {
      cerr << "Error: circuit is not yet constructed!!" << endl;
      return CMD_EXEC_ERROR;
   }

   string token;
   if (!CmdExec::lexSingleOption(option, token))	
      return CMD_EXEC_ERROR;

   if (token.empty() || myStrNCmp("-Default", token, 2) == 0)	
      routeMgr->place();
   else if (myStrNCmp("-Net", token, 2) == 0)	
      routeMgr->netbasedPlace();
   else if (myStrNCmp("-Force", token, 2) == 0)	
      routeMgr->forcedirectedPlace();
   else if (myStrNCmp("-Check", token, 2) == 0)
      // TODO
      cout << "TODO\n";
   else if (myStrNCmp("-Summary", token, 2) == 0) {
      // TODO
      cout << "TODO\n";
      return CMD_EXEC_DONE;
   } else 	
      return CmdExec::errorOption(CMD_OPT_ILLEGAL, token);

   assert(curCmd != ROUTEINIT);
   curCmd = PLACE;

   return CMD_EXEC_DONE;
}

void
PlaceCmd::usage(ostream& os) const
{
   os << "Usage: Place [ -Default | -Net |"
      << " -Force | -Check | -Summary ]" << endl;
}

void
PlaceCmd::help() const
{
   cout << setw(15) << left << "Place: "
        << "perform placements\n";
}

//----------------------------------------------------------------------
//    Route [ -3D | -Check | -Summary ]
//----------------------------------------------------------------------
CmdExecStatus
RouteCmd::exec(const string& option)
{
   if (!routeMgr) {
      cerr << "Error: circuit is not yet constructed!!" << endl;
      return CMD_EXEC_ERROR;
   }

   string token;
   if (!CmdExec::lexSingleOption(option, token))	
      return CMD_EXEC_ERROR;

   if (token.empty() || myStrNCmp("-3D", token, 2) == 0)	
      routeMgr->route();
   else if (myStrNCmp("-Check", token, 2) == 0) {
      // TODO
      cout << "TODO\n";
   } else if (myStrNCmp("-Summary", token, 2) == 0) {
      // TODO
      cout << "TODO\n";
      return CMD_EXEC_DONE;
   } else 	
      return CmdExec::errorOption(CMD_OPT_ILLEGAL, token);

   assert(curCmd != ROUTEINIT);
   curCmd = ROUTE;

   return CMD_EXEC_DONE;
}

void
RouteCmd::usage(ostream& os) const
{
   os << "Usage: Route [ -3D | -Check | -Summary ]" << endl;
}

void
RouteCmd::help() const
{
   cout << setw(15) << left << "Route: "
        << "perform routing\n";
}

//----------------------------------------------------------------------
//    Optimize < -All | -Overflow | -REroute | -2pinreroute | -Evaluate | -RAnk >
//----------------------------------------------------------------------
CmdExecStatus
OptimizeCmd::exec(const string& option)
{
   if (!routeMgr) {
      cerr << "Error: circuit is not yet constructed!!" << endl;
      return CMD_EXEC_ERROR;
   }
   // check option
   //vector<string> options;
   string token;
   if (!CmdExec::lexSingleOption(option, token))	
      return CMD_EXEC_ERROR;

   //CmdExec::lexOptions(option, token);

   if (myStrNCmp("-All", token, 2) == 0) {
      routeMgr->place();
      routeMgr->route();
      routeMgr->replaceBest();
   } else if (myStrNCmp("-REroute", token, 3) == 0) {
      routeMgr->reroute();
   } else if (myStrNCmp("-Overflow", token, 2) == 0) {
      routeMgr->checkOverflow();
   } else if (myStrNCmp("-2PinReroute", token, 2) == 0) {
      routeMgr->reduceOverflow();
   }
   else if (myStrNCmp("-Evaluate", token, 2) == 0)	
      routeMgr->replaceBest();
   else if (myStrNCmp("-RAnk", token, 3) == 0)
      routeMgr->printRank();
   else 	
      return CmdExec::errorOption(CMD_OPT_ILLEGAL, token);

   assert(curCmd != ROUTEINIT);
   curCmd = ROUTEOPT;

   return CMD_EXEC_DONE;
}

void
OptimizeCmd::usage(ostream& os) const
{
   os << "Usage: Optimize < -All | -Overflow | -REroute | -2pinreroute | -Evaluate | -RAnk >" << endl;
}

void
OptimizeCmd::help() const
{
   cout << setw(15) << left << "Optimize: "
        << "perform optimizations\n";
}

//----------------------------------------------------------------------
//    MgrPrint [-Summary | -Netlist | -MC | -Extra | -NOndefaultsupply]
//----------------------------------------------------------------------
CmdExecStatus
MgrPrintCmd::exec(const string& option)
{
   // check option
   string token;
   if (!CmdExec::lexSingleOption(option, token))
      return CMD_EXEC_ERROR;

   if (!routeMgr) {
      cerr << "Error: circuit is not yet constructed!!" << endl;
      return CMD_EXEC_ERROR;
   }
   if (token.empty() || myStrNCmp("-Summary", token, 2) == 0)
      routeMgr->printRouteSummary();
   else if (myStrNCmp("-Input", token, 2) == 0)
      routeMgr->printInputSummary();
   else if (myStrNCmp("-NEtlist", token, 3) == 0)
      routeMgr->printNetlist();
   else if (myStrNCmp("-MC", token, 3) == 0)
      routeMgr->printMCList();
   else if (myStrNCmp("-Extra", token, 2) == 0)
      routeMgr->printExtraDemand();
   else if (myStrNCmp("-NOndefaultsupply", token, 3) == 0)
      routeMgr->printNonDefaultSupply();
   else if (myStrNCmp("-FECpairs", token, 4) == 0)
      cout << "routeMgr->printFECPairs()" << endl;
   else
      return CmdExec::errorOption(CMD_OPT_ILLEGAL, token);

   return CMD_EXEC_DONE;
}

void
MgrPrintCmd::usage(ostream& os) const
{  
   os << "Usage: MgrPrint [ -Summary | -Input |"
      << " -Netlist | -MC | -Extra | -NOndefaultsupply ]\n";
}

void
MgrPrintCmd::help() const
{  
   cout << setw(15) << left << "MgrPrint: " << "print RouteMgr summary\n";
}

//----------------------------------------------------------------------
//    CELLPrint [-Summary | -ALl | -Mc (int idx) | -AssoNet (int idx)]
//----------------------------------------------------------------------
CmdExecStatus
CellPrintCmd::exec(const string& option)
{
   // check option
   vector<string> tokens;
   if (!CmdExec::lexOptions(option, tokens))
      return CMD_EXEC_ERROR;

   if (!routeMgr) {
      cerr << "Error: circuit is not yet constructed!!" << endl;
      return CMD_EXEC_ERROR;
   }

   if (tokens.empty())
      // TODO
      routeMgr->printCellSummary();
   else if (tokens.size() == 1) {
      if (myStrNCmp("-Summary", tokens[0], 2) == 0) {
         // TODO
         routeMgr->printCellSummary();
      } else if (myStrNCmp("-ALl", tokens[0], 3) == 0)
         routeMgr->printCellInst();
      else if (myStrNCmp("-MC", tokens[0], 3) == 0)
         routeMgr->printMCList();
      else if (myStrNCmp("-ASsonet", tokens[0], 3) == 0)
         routeMgr->printAssoNet();
      else {
         int idx;
         if (!myStr2Int(tokens[0], idx)) {
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[0]);
         }
         if (!(routeMgr->printCellInst(idx))) {
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[0]);
         }
      }
   } else if (tokens.size() == 2) {
      int idx;
      if (myStrNCmp("-MC", tokens[0], 3) == 0) {
         if (!myStr2Int(tokens[1], idx)) {
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[1]);
         }
         if (!(routeMgr->printMC(idx))) {
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[1]);
         }
      } else if (myStrNCmp("-ASsonet", tokens[0], 3) == 0) {
         if (!myStr2Int(tokens[1], idx)) {
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[1]);
         }
         if (!(routeMgr->printAssoNet(idx))) {
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[1]);
         }
      }
   } else {
      return CmdExec::errorOption(CMD_OPT_EXTRA, tokens[2]);
   }
   

   return CMD_EXEC_DONE;
}

void
CellPrintCmd::usage(ostream& os) const
{  
   os << "Usage: CELLPrint [-Summary | -ALl | -Mc (int idx) | -AssoNet (int idx)]" << endl;
}

void
CellPrintCmd::help() const
{  
   cout << setw(15) << left << "CELLPrint: " << "print cell\n";
}

//----------------------------------------------------------------------
//    LAYPrint [Pos pos][int layer][-SUMmary | -SUPply | -Demand | -2DSupply | -2DDemand]
//----------------------------------------------------------------------
CmdExecStatus
LayPrintCmd::exec(const string& option)
{
   // check option
   vector<string> tokens;
   if (!CmdExec::lexOptions(option, tokens))
      return CMD_EXEC_ERROR;

   if (!routeMgr) {
      cerr << "Error: circuit is not yet constructed!!" << endl;
      return CMD_EXEC_ERROR;
   }

   if (tokens.empty())
      // TODO
      cout << "routeMgr->printSummary()" << endl;
   else if (tokens.size() == 1) {
      if (myStrNCmp("-Summary", tokens[0], 2) == 0) {
         // TODO
         cout << "routeMgr->printSummary()" << endl;
      } else if (myStrNCmp("-SUPply", tokens[0], 4) == 0)
         routeMgr->printLaySupply();
      else if (myStrNCmp("-Demand", tokens[0], 2) == 0)
         routeMgr->printGridDemand();
      else if (myStrNCmp("-2DSupply", tokens[0], 4) == 0)
         routeMgr->print2DSupply();
      else if (myStrNCmp("-2DDemand", tokens[0], 4) == 0)
         routeMgr->print2DDemand();
      else if (myStrNCmp("-2DCongestion", tokens[0], 4) == 0)
         routeMgr->print2DCongestion();
      else
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[0]);
   } else if (tokens.size() == 2) {
      int idx1, idx2;
      if (myStr2Int(tokens[0], idx1) && myStr2Int(tokens[1], idx2)) {
         routeMgr->getGrid(Pos(idx1, idx2))->printSummary();
      } else {
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[0]);
      }
   } else if (tokens.size() == 3) {
      int idx1, idx2, idx3;
      if (myStr2Int(tokens[0], idx1)
          && myStr2Int(tokens[1], idx2)
          && myStr2Int(tokens[2], idx3)) {
         Ggrid* mygrid = routeMgr->getGrid(Pos(idx1, idx2));
         ((*(mygrid))[idx3])->printSummary();
      } else {
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[0]);
      }
   } else {
      return CmdExec::errorOption(CMD_OPT_EXTRA, tokens[3]);
   }

   return CMD_EXEC_DONE;
}

void
LayPrintCmd::usage(ostream& os) const
{  
   os << "Usage: LAYPrint [Pos pos][int layer][-SUMmary | -SUPply | -Demand | -2DSupply  "
      << "| -2DDemand | -2DCongestion]" << endl;
}

void
LayPrintCmd::help() const
{  
   cout << setw(15) << left << "LAYPrint: " << "print layer\n";
}

//----------------------------------------------------------------------
//    NETPrint [-SUmmary | -SEgment | -Assoinst (int idx) | int idx]
//----------------------------------------------------------------------
CmdExecStatus
NetPrintCmd::exec(const string& option)
{
   // check option
   vector<string> tokens;
   if (!CmdExec::lexOptions(option, tokens))
      return CMD_EXEC_ERROR;

   if (!routeMgr) {
      cerr << "Error: circuit is not yet constructed!!" << endl;
      return CMD_EXEC_ERROR;
   }

   if (tokens.empty())
      // TODO
      cout << "routeMgr->printSummary()" << endl;
   else if (tokens.size() == 1) {
      int idx;
      if (myStrNCmp("-SUmmary", tokens[0], 3) == 0) {
         // TODO
         cout << "routeMgr->printSummary()" << endl;
      } else if (myStrNCmp("-SEgment", tokens[0], 3) == 0)
         routeMgr->printInitSegs();
      else if (myStr2Int(tokens[0], idx))
         routeMgr->printNet(idx);
      else
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[0]);
   } else if (tokens.size() == 2) {
      int idx;
      if (myStrNCmp("-Assoinst", tokens[0], 2) == 0) {
         if (!myStr2Int(tokens[1], idx)) {
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[1]);
         }
         if (!(routeMgr->printAssoInst(idx))) {
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[1]);
         }
      }
      else
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[1]);
   } else {
      return CmdExec::errorOption(CMD_OPT_EXTRA, tokens[2]);
   }
   return CMD_EXEC_DONE;
}

void
NetPrintCmd::usage(ostream& os) const
{  
   os << "Usage: NETPrint [-Summary | -SEgment |"
      << " -Assoinst (int idx) | int idx]" << endl;
}

void
NetPrintCmd::help() const
{  
   cout << setw(15) << left << "NETPrint: " << "print net\n";
}


/*
//----------------------------------------------------------------------
//    CIRGate <<(int gateId)> [<-FANIn | -FANOut><(int level)>]>
//----------------------------------------------------------------------
CmdExecStatus
CirGateCmd::exec(const string& option)
{
   if (!routeMgr) {
      cerr << "Error: circuit has not been read!!" << endl;
      return CMD_EXEC_ERROR;
   }

   // check option
   vector<string> options;
   if (!CmdExec::lexOptions(option, options))
      return CMD_EXEC_ERROR;

   if (options.empty())
      return CmdExec::errorOption(CMD_OPT_MISSING, "");

   int gateId = -1, level = 0;
   bool doFanin = false, doFanout = false;
   CirGate* thisGate = 0;
   for (size_t i = 0, n = options.size(); i < n; ++i) {
      bool checkLevel = false;
      if (myStrNCmp("-FANIn", options[i], 5) == 0) {
         if (doFanin || doFanout)
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[i]);
         doFanin = true;
         checkLevel = true;
      }
      else if (myStrNCmp("-FANOut", options[i], 5) == 0) {
         if (doFanin || doFanout)
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[i]);
         doFanout = true;
         checkLevel = true;
      }
      else if (!thisGate) {
         if (!myStr2Int(options[i], gateId) || gateId < 0)
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[i]);
         thisGate = routeMgr->getGate(gateId);
         if (!thisGate) {
            cerr << "Error: Gate(" << gateId << ") not found!!" << endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
         }
      }
      else if (thisGate)
         return CmdExec::errorOption(CMD_OPT_EXTRA, options[i]);
      else
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[i]);
      if (checkLevel) {
         if (++i == n)
            return CmdExec::errorOption(CMD_OPT_MISSING, options[i-1]);
         if (!myStr2Int(options[i], level) || level < 0)
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[i]);
         checkLevel = true;
      }
   }

   if (!thisGate) {
      cerr << "Error: Gate id is not specified!!" << endl;
      return CmdExec::errorOption(CMD_OPT_MISSING, options.back());
   }

   if (doFanin)
      thisGate->reportFanin(level);
   else if (doFanout)
      thisGate->reportFanout(level);
   else
      thisGate->reportGate();

   return CMD_EXEC_DONE;
}

void
CirGateCmd::usage(ostream& os) const
{
   os << "Usage: CIRGate <<(int gateId)> [<-FANIn | -FANOut><(int level)>]>"
      << endl;
}

void
CirGateCmd::help() const
{
   cout << setw(15) << left << "CIRGate: " << "report a gate\n";
}

*/


/*
//----------------------------------------------------------------------
//    CIRSIMulate <-Random | -File <string patternFile>>
//                [-Output (string logFile)]
//----------------------------------------------------------------------
CmdExecStatus
CirSimCmd::exec(const string& option)
{
   if (!routeMgr) {
      cerr << "Error: circuit is not yet constructed!!" << endl;
      return CMD_EXEC_ERROR;
   }
   // check option
   vector<string> options;
   CmdExec::lexOptions(option, options);

   ifstream patternFile;
   ofstream logFile;
   bool doRandom = false, doFile = false, doLog = false;
   for (size_t i = 0, n = options.size(); i < n; ++i) {
      if (myStrNCmp("-Random", options[i], 2) == 0) {
         if (doRandom || doFile)
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[i]);
         doRandom = true;
      }
      else if (myStrNCmp("-File", options[i], 2) == 0) {
         if (doRandom || doFile)
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[i]);
         if (++i == n)
            return CmdExec::errorOption(CMD_OPT_MISSING, options[i-1]);
         patternFile.open(options[i].c_str(), ios::in);
         if (!patternFile)
            return CmdExec::errorOption(CMD_OPT_FOPEN_FAIL, options[i]);
         doFile = true;
      }
      else if (myStrNCmp("-Output", options[i], 2) == 0) {
         if (doLog)
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[i]);
         if (++i == n)
            return CmdExec::errorOption(CMD_OPT_MISSING, options[i-1]);
         logFile.open(options[i].c_str(), ios::out);
         if (!logFile)
            return CmdExec::errorOption(CMD_OPT_FOPEN_FAIL, options[i]);
         doLog = true;
      }
      else
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[i]);
   }

   if (!doRandom && !doFile)
      return CmdExec::errorOption(CMD_OPT_MISSING, "");

   assert (curCmd != ROUTEINIT);
   if (doLog)
      routeMgr->setSimLog(&logFile);
   else routeMgr->setSimLog(0);

   if (doRandom)
      routeMgr->randomSim();
   else
      routeMgr->fileSim(patternFile);
   routeMgr->setSimLog(0);
   curCmd = CIRSIMULATE;
   
   return CMD_EXEC_DONE;
}

void
CirSimCmd::usage(ostream& os) const
{
   os << "Usage: CIRSIMulate <-Random | -File <string patternFile>>\n"
      << "                   [-Output (string logFile)]" << endl;
}

void
CirSimCmd::help() const
{
   cout << setw(15) << left << "CIRSIMulate: "
        << "perform Boolean logic simulation on the circuit\n";
}

//----------------------------------------------------------------------
//    CIRFraig
//----------------------------------------------------------------------
CmdExecStatus
CirFraigCmd::exec(const string& option)
{
   if (!routeMgr) {
      cerr << "Error: circuit is not yet constructed!!" << endl;
      return CMD_EXEC_ERROR;
   }
   // check option
   vector<string> options;
   CmdExec::lexOptions(option, options);

   if (!options.empty())
      return CmdExec::errorOption(CMD_OPT_EXTRA, options[0]);

   if (curCmd != CIRSIMULATE) {
      cerr << "Error: circuit is not yet simulated!!" << endl;
      return CMD_EXEC_ERROR;
   }
   routeMgr->fraig();
   curCmd = CIRFRAIG;

   return CMD_EXEC_DONE;
}

void
CirFraigCmd::usage(ostream& os) const
{
   os << "Usage: CIRFraig" << endl;
}

void
CirFraigCmd::help() const
{
   cout << setw(15) << left << "CIRFraig: "
        << "perform Boolean logic simulation on the circuit\n";
}
*/


