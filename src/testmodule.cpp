#include "testmodule.h"
bool getMapping(char *taskmapping_input_filename, int dimx, int dimy, std::vector<NoximCommunication> * traffic_table, std::vector<NoximTaskGraphEntry> * task_graph, int task_manager_x, int task_manager_y){
	//Adding random nodes
	if (!traffic_table)
		traffic_table = new std::vector<NoximCommunication>;
	if(!task_graph)
		task_graph = new std::vector<NoximTaskGraphEntry>;

	NoximCommunication temp_comm;
	NoximTaskGraphEntry temp_tg;

	temp_tg.node=1;
	temp_tg.inputs.clear();
	temp_tg.inputs[4]=3;
	temp_tg.inputs[5]=2;
	temp_tg.time=3;
	temp_tg.outputs.clear();
	temp_tg.outputs[2]=1;
	task_graph->push_back(temp_tg);
	
	temp_comm.src=4;
	temp_comm.dst=1;
	temp_comm.pir=1.5;
	temp_comm.por=1.5;
	temp_comm.t_on=0;
	temp_comm.t_off=1000;
	temp_comm.t_period=1010;
	traffic_table->push_back(temp_comm);

	temp_comm.src=5;
	traffic_table->push_back(temp_comm);

	return true;
}
