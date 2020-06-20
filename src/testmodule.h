#ifndef __TESTMODULE_H__
#define __TESTMODULE_H__
#include <vector>
#include "NoximCommunication.h"
#include "NoximTaskGraphEntry.h"

extern "C"{
	bool getMapping(char * taskmapping_input_filename, int dimx, int dimy, std::vector <NoximCommunication> * traffic_table, std::vector<NoximTaskGraphEntry> * task_graph, int task_manager_x, int task_manager_y);
}
#endif
