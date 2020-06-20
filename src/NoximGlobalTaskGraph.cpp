#include "NoximGlobalTaskGraph.h"

NoximGlobalTaskGraph::NoximGlobalTaskGraph(){

}

bool NoximGlobalTaskGraph::load(const char *fname){
	ifstream fin(fname, ios::in);
	if(!fin)
		return false;
	task_graph.clear();
	while(!fin.eof()){
		char line[512];
		fin.getline(line, sizeof(line)-1);
		if (line[0]!='\0'){
			char * token = NULL;
			int node, time, num_inputs, num_outputs, temp_input_1, temp_input_2;

			map <int, int> inputs, outputs;
			NoximTaskGraphEntry tg;

			//Reading node number
			token=strtok(line, " ");
			if(sscanf(token, "%d", &node)==0)
				return false;
			cout<<"node on current line: "<<node<<endl;
			tg.node=node;

			//Reading number of inputs
			token=strtok(NULL, " ");
			if (token==NULL)
				return false;
			if (sscanf(token, "%d", &num_inputs)==0)
				return false;
			cout<<"num_inputs on current line: "<<num_inputs<<endl;

			//Reading inputs
			for(int i=0; i<num_inputs; i++){
				//Reading first input
				token=strtok(NULL, " ");
				if (token==NULL)
					return false;
				if(sscanf(token, "%d", &temp_input_1)==0)
					return false;
				cout<<"temp_input_1 on current line: "<<temp_input_1<<endl;

				//Reading second input
				token=strtok(NULL, " ");
				if (token==NULL)
					return false;
				if(sscanf(token, "%d", &temp_input_2)==0)
					return false;
				cout<<"temp_input_2 on current line: "<<temp_input_2<<endl;

				inputs[temp_input_1]=temp_input_2;
			}
			tg.inputs=inputs;

			//Reading time in cycles (if defined)
			token=strtok(NULL, " ");
			if (token==NULL)
				return false; //Some processing time must be defined
			if (sscanf(token, "%d", &time)==0)
				return false;
			cout<<"time on current line: "<<time<<endl;

			//Reading number of outputs
			token=strtok(NULL, " ");
			if (token!=NULL){ //this means that output part is defined			
				if (sscanf(token, "%d", &num_outputs)==0)
					return false;
				cout<<"num_outputs on current line: "<<num_outputs<<endl;
				//Reading outputs
				for(int i=0; i<num_outputs; i++){
					//Reading first output
					token=strtok(NULL, " ");
					if (token==NULL)
						return false;
					if(sscanf(token, "%d", &temp_input_1)==0)
						return false;
					cout<<"temp_input_1 on current line: "<<temp_input_1<<endl;

					//Reading second input
					token=strtok(NULL, " ");
					if (token==NULL)
						return false;
					if(sscanf(token, "%d", &temp_input_2)==0)
						return false;
					cout<<"temp_input_2 on current line: "<<temp_input_2<<endl;

					outputs[temp_input_1]=temp_input_2;
				}

			}
			else{
				cout<<"No output information on current line"<<endl;
				//	time=0;
				outputs.clear();
			}
			tg.time=time;
			tg.outputs=outputs;

			//Inserting into vector
			task_graph.push_back(tg);
		}
	}
	fin.close();
	return true;
}

NoximTaskGraphEntry * NoximGlobalTaskGraph::getNodeInfo(int node){
	NoximTaskGraphEntry * temp = NULL;
	for(vector<NoximTaskGraphEntry>::iterator it=task_graph.begin(); it!=task_graph.end(); it++){
		if ((*it).node==node){
			temp= new NoximTaskGraphEntry;
			temp->node=node;
			temp->inputs=(*it).inputs;
			temp->time=(*it).time;
			temp->outputs=(*it).outputs;
			break;
		}
	}
	return temp;
}
void NoximGlobalTaskGraph::setTaskGraph(vector<NoximTaskGraphEntry> task_graph){
	this->task_graph = task_graph;
}

vector <int> NoximGlobalTaskGraph::getParentNodes(){
	vector <int> parent_nodes;
	for (vector<NoximTaskGraphEntry>::iterator it_task_graph=task_graph.begin(); it_task_graph!=task_graph.end(); it_task_graph++){
		for(map <int, int>::iterator it_inputs=it_task_graph->inputs.begin(); it_inputs != it_task_graph->inputs.end(); it_inputs++){
			bool exists=false;
			for (vector<NoximTaskGraphEntry>::iterator it_task_graph_2 = task_graph.begin(); it_task_graph_2!=task_graph.end(); it_task_graph_2++){
				if (it_inputs->first==it_task_graph_2->node){
					exists=true;
					break;
				}
			}
			if (exists==false){
				bool check=false;
				for(vector<int>::iterator it=parent_nodes.begin();it!=parent_nodes.end();it++){
					if(*it==it_inputs->first){
						check=true;
						break;
					}
				}
				if(check==false)
					parent_nodes.push_back(it_inputs->first);
			}
		}
	}
	return parent_nodes;
}
vector <int> NoximGlobalTaskGraph::getLeafNodes(){
	vector<int> leaf_nodes;
	for (vector<NoximTaskGraphEntry>::iterator it_task_graph=task_graph.begin(); it_task_graph!=task_graph.end(); it_task_graph++){
		if(it_task_graph->outputs.size()==0)
			leaf_nodes.push_back(it_task_graph->node);
	}
	return leaf_nodes;
}
