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
   if (!(cmdMgr->regCmd("Read", 1, new RouteReadCmd) &&
         cmdMgr->regCmd("Write", 1, new RouteWriteCmd) &&
         cmdMgr->regCmd("OPTimize", 3, new RouteOptCmd) &&
         cmdMgr->regCmd("LAYPrint", 4, new LayPrintCmd) &&
         cmdMgr->regCmd("NETPrint", 4, new NetPrintCmd) &&
         cmdMgr->regCmd("CELLPrint", 5, new CellPrintCmd)
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
   ROUTEOPT,
   CIRSTRASH,
   CIRSIMULATE,
   CIRFRAIG,
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
//    Write [(int gateId)][-Output (string txtFile)]
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
      cout << "routeMgr->writeAag(cout)" << endl;
      return CMD_EXEC_DONE;
   }
   bool hasFile = false;
   /*
   int gateId;
   CirGate *thisGate = NULL;
   ofstream outfile;
   for (size_t i = 0, n = options.size(); i < n; ++i) {
      if (myStrNCmp("-Output", options[i], 2) == 0) {
         if (hasFile) 
            return CmdExec::errorOption(CMD_OPT_EXTRA, options[i]);
         if (++i == n)
            return CmdExec::errorOption(CMD_OPT_MISSING, options[i-1]);
         outfile.open(options[i].c_str(), ios::out);
         if (!outfile)
            return CmdExec::errorOption(CMD_OPT_FOPEN_FAIL, options[1]);
         hasFile = true;
      }
      else if (myStr2Int(options[i], gateId) && gateId >= 0) {
         if (thisGate != NULL)
            return CmdExec::errorOption(CMD_OPT_EXTRA, options[i]);
         thisGate = routeMgr->getGate(gateId);
         if (!thisGate) {
            cerr << "Error: Gate(" << gateId << ") not found!!" << endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[i]);
         }
         if (!thisGate->isAig()) {
             cerr << "Error: Gate(" << gateId << ") is NOT an AIG!!" << endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[i]);
         }
      }
      else return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[i]);
   }

   if (!thisGate) {
      assert (hasFile);
      routeMgr->writeAag(outfile);
   }
   else if (hasFile) routeMgr->writeGate(outfile, thisGate);
   else routeMgr->writeGate(cout, thisGate);
   */

   return CMD_EXEC_DONE;
}

void
RouteWriteCmd::usage(ostream& os) const
{
   os << "Usage: Write [(int gateId)][-Output (string txtFile)]" << endl;
}

void
RouteWriteCmd::help() const
{
   cout << setw(15) << left << "RouteWrite: "
        << "write the netlist to a new text file (.txt)\n";
}

//----------------------------------------------------------------------
//    OPTimize
//----------------------------------------------------------------------
CmdExecStatus
RouteOptCmd::exec(const string& option)
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

   assert(curCmd != ROUTEINIT);
   // routeMgr->optimize();
   curCmd = ROUTEOPT;

   return CMD_EXEC_DONE;
}

void
RouteOptCmd::usage(ostream& os) const
{
   os << "Usage: OPTimize" << endl;
}

void
RouteOptCmd::help() const
{
   cout << setw(15) << left << "OPTimize: "
        << "perform optimizations\n";
}

//----------------------------------------------------------------------
//    CELLPrint [-Summary | -Netlist]
//----------------------------------------------------------------------
CmdExecStatus
CellPrintCmd::exec(const string& option)
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
      cout << "routeMgr->printSummary()" << endl;
   else if (myStrNCmp("-Netlist", token, 2) == 0)
      cout << "routeMgr->printNetlist()" << endl;
   else if (myStrNCmp("-PI", token, 3) == 0)
      cout << "routeMgr->printPIs()" << endl;
   else if (myStrNCmp("-PO", token, 3) == 0)
      cout << "routeMgr->printPOs()" << endl;
   else if (myStrNCmp("-FLoating", token, 3) == 0)
      cout << "routeMgr->printFloatGates()" << endl;
   else if (myStrNCmp("-FECpairs", token, 4) == 0)
      cout << "routeMgr->printFECPairs()" << endl;
   else
      return CmdExec::errorOption(CMD_OPT_ILLEGAL, token);

   return CMD_EXEC_DONE;
}

void
CellPrintCmd::usage(ostream& os) const
{  
   os << "Usage: CELLPrint [-Summary | -Netlist]" << endl;
}

void
CellPrintCmd::help() const
{  
   cout << setw(15) << left << "CELLPrint: " << "print cell\n";
}

//----------------------------------------------------------------------
//    LAYPrint [-Summary | -Netlist | -PI | -PO | -FLoating | -FECpairs]
//----------------------------------------------------------------------
CmdExecStatus
LayPrintCmd::exec(const string& option)
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
      cout << "routeMgr->printSummary()" << endl;
   else if (myStrNCmp("-Netlist", token, 2) == 0)
      cout << "routeMgr->printNetlist()" << endl;
   else if (myStrNCmp("-PI", token, 3) == 0)
      cout << "routeMgr->printPIs()" << endl;
   else if (myStrNCmp("-PO", token, 3) == 0)
      cout << "routeMgr->printPOs()" << endl;
   else if (myStrNCmp("-FLoating", token, 3) == 0)
      cout << "routeMgr->printFloatGates()" << endl;
   else if (myStrNCmp("-FECpairs", token, 4) == 0)
      cout << "routeMgr->printFECPairs()" << endl;
   else
      return CmdExec::errorOption(CMD_OPT_ILLEGAL, token);

   return CMD_EXEC_DONE;
}

void
LayPrintCmd::usage(ostream& os) const
{  
   os << "Usage: LAYPrint [-Summary | -Netlist | -PI | -PO | -FLoating "
      << "| -FECpairs]" << endl;
}

void
LayPrintCmd::help() const
{  
   cout << setw(15) << left << "LAYPrint: " << "print layer\n";
}

//----------------------------------------------------------------------
//    NETPrint [-Summary | -Netlist | -PI | -PO | -FLoating | -FECpairs]
//----------------------------------------------------------------------
CmdExecStatus
NetPrintCmd::exec(const string& option)
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
      cout << "routeMgr->printSummary()" << endl;
   else if (myStrNCmp("-Netlist", token, 2) == 0)
      cout << "routeMgr->printNetlist()" << endl;
   else if (myStrNCmp("-PI", token, 3) == 0)
      cout << "routeMgr->printPIs()" << endl;
   else if (myStrNCmp("-PO", token, 3) == 0)
      cout << "routeMgr->printPOs()" << endl;
   else if (myStrNCmp("-FLoating", token, 3) == 0)
      cout << "routeMgr->printFloatGates()" << endl;
   else if (myStrNCmp("-FECpairs", token, 4) == 0)
      cout << "routeMgr->printFECPairs()" << endl;
   else
      return CmdExec::errorOption(CMD_OPT_ILLEGAL, token);

   return CMD_EXEC_DONE;
}

void
NetPrintCmd::usage(ostream& os) const
{  
   os << "Usage: NETPrint [-Summary | -Netlist | -PI | -PO | -FLoating "
      << "| -FECpairs]" << endl;
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


