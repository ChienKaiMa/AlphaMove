routeMgr.o: routeMgr.cpp routeMgr.h routeNet.h routeDef.h ../util/util.h \
 ../util/rnGen.h ../util/myUsage.h
routeCmd.o: routeCmd.cpp routeMgr.h routeNet.h routeDef.h routeCmd.h \
 ../../include/cmdParser.h ../../include/cmdCharDef.h \
 ../../include/util.h ../../include/rnGen.h ../../include/myUsage.h
routeOpt.o: routeOpt.cpp routeMgr.h routeNet.h routeDef.h \
 ../../include/util.h ../../include/rnGen.h ../../include/myUsage.h
routeNet.o: routeNet.cpp routeNet.h routeDef.h routeMgr.h
