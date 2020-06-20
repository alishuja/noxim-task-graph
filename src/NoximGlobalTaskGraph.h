#ifndef __NOXIMGLOBALTASKGRAPH_H__
#define __NOXIMGLOBALTASKGRAPH_H__

#include <iostream>
#include <map>
#include <vector>
#include <utility>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include "NoximMain.h"
#include "NoximTaskGraphEntry.h"

using namespace std;

class NoximGlobalTaskGraph{
	public:
		NoximGlobalTaskGraph();
		bool load(const char *fname);
		NoximTaskGraphEntry * getNodeInfo(int node);
		void setTaskGraph(vector<NoximTaskGraphEntry> task_graph);
		vector <int> getParentNodes();
		vector <int> getLeafNodes();
	private:
		vector <NoximTaskGraphEntry> task_graph;
};

#endif
