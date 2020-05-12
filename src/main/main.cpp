/****************************************************************************
  FileName     [ main.cpp ]
  PackageName  [ main ]
  Synopsis     [ Define main() ]
  Author       [ Chien-Kai Ma, Kai-Chun Chang, Yu-Wei Fan ]
  Copyright    [ Copyleft(c) 2020-present NTU, Taiwan  ]
****************************************************************************/

#include "util.h"
#include "cmdParser.h"

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
   cout << "Usage: routeTest [ < inputFile > < outputFile > ]" << endl;
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
      if (!cmdMgr->openDofile(argv[1])) {
         cerr << "Error: cannot open file \"" << argv[1] << "\"!!\n";
         myexit();
      }
      // TODO: generate output file
      string outFileName = argv[2];
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
