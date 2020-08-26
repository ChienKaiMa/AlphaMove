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
#include <csignal>

using namespace std;

#define TIME_LIMIT 3600

//----------------------------------------------------------------------
//    Global cmd Manager
//----------------------------------------------------------------------
CmdParser* cmdMgr = new CmdParser("route> ");

extern bool initCommonCmd();
extern bool initRouteCmd();

ofstream outfile;
extern RouteMgr *routeMgr;

static void signal_handler(int signum){
   //cout << "Time limit exceeded!!" << endl;
   cout << "\nOutput file ... \n";
   routeMgr->writeCircuit(outfile);
   myUsage.report(true, true);
   exit(0);
}

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
      #ifdef DEBUG
      std::time_t time = std::time(0);
      cout << "time: " << std::ctime(&time);
      #endif
      
      outfile.open(argv[2], ios::out);
      if (!outfile) {
         cerr << "Output file open fail!" << endl;
         return 1;
      }
      alarm(TIME_LIMIT-100);
      //alarm(60);
      signal(SIGALRM, &signal_handler);
      struct sigaction sigCHandler;
      sigCHandler.sa_handler = signal_handler;
      sigemptyset(&sigCHandler.sa_mask);
      sigCHandler.sa_flags = 0;
      sigaction(SIGINT, &sigCHandler, NULL);
      sigaction(SIGSEGV, &sigCHandler, NULL);
      
      routeMgr = new RouteMgr();
      // TODO: generate output file
      string inputFile = argv[1];
      string outFileName = argv[2];
      routeMgr->readCircuit(inputFile);
      routeMgr->printInputSummary();
      
      routeMgr->reroute();
      routeMgr->reroute();
      if(routeMgr->getCellCnt() > 100){
         routeMgr->precisePnR(0);
         routeMgr->precisePnR(0);
         routeMgr->reroute();
         routeMgr->precisePnR(0);
         routeMgr->precisePnR(0);
      }
      //routeMgr->reroute();
      else{
         routeMgr->mainPnR();
      }
      routeMgr->writeCircuit(outfile);
      
      #ifdef DEBUG
      std::time_t time_2 = std::time(0);
      cout << "time: " << std::ctime(&time_2);
      #endif

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
