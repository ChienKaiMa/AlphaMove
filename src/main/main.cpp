/****************************************************************************
  FileName     [ main.cpp ]
  PackageName  [ main ]
  Synopsis     [ Define main() ]
  Author       [ Chien-Kai Ma, Kai-Chun Chang, Yu-Wei Fan ]
  Copyright    [ Copyleft(c) 2020-present NTU, Taiwan  ]
****************************************************************************/

#include "util.h"
#include "cmdParser.h"
#include "../route/routeMgr.h"
#include <cassert>

using namespace std;

//----------------------------------------------------------------------
//    Global cmd Manager
//----------------------------------------------------------------------
CmdParser* cmdMgr = new CmdParser("route> ");

extern bool initCommonCmd();
extern bool initRouteCmd();

static void
usage()
{
   cout << "Usage: ./cell_move_router [ < inputFile > < outputFile > ]" << endl;
}

static void
myexit()
{
   usage();
   exit(-1);
}

int
main(int argc, char** argv)
{
   myUsage.reset();

   ifstream dof;

   if (argc == 3) {  // < inputFile > < outputFile >
      // TODO: handle input file instead of cmd dofile
      /*
      if (!cmdMgr->openDofile(argv[1])) {
         cerr << "Error: cannot open file \"" << argv[1] << "\"!!\n";
         myexit();
      }
      */
      // TODO: generate output file
      string inputFile = argv[1];
      string outFileName = argv[2];
      RouteMgr *rMgr = new RouteMgr;
      rMgr->readCircuit(inputFile);
      rMgr->printRouteSummary();
      rMgr->initSupply();
      rMgr->place();
      rMgr->route();
      ofstream outfile;
      outfile.open(argv[2], ios::out);
      if (!outfile) {
         cerr << "Output file open fail!" << endl;
         return 1;
      }
      rMgr->writeCircuit(outfile);
      myUsage.report(true, true);
      return 0;
   }
   else if (argc != 1) {
      cerr << "Error: illegal number of argument (" << argc << ")!!\n";
      myexit();
   }

   if (!initCommonCmd() || !initRouteCmd())
      return 1;

   CmdExecStatus status = CMD_EXEC_DONE;
   while (status != CMD_EXEC_QUIT) {  // until "quit" or command error
      status = cmdMgr->execOneCmd();
      cout << endl;  // a blank line between each command
   }

   return 0;
}
