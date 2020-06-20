/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2010 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the implementation of the processing element
 */

#include "NoximProcessingElement.h"

int NoximProcessingElement::randInt(int min, int max)
{
	return min +
		(int) ((double) (max - min + 1) * rand() / (RAND_MAX + 1.0));
}

void NoximProcessingElement::taskMappingProcess(){
	if (reset.read()){
		if(NoximGlobalParams::taskmapping_mode){
			if (local_id==0){
				vector <NoximCommunication> temp_comm;
				vector <NoximTaskGraphEntry> temp_tg;
				NoximGlobalParams::traffic_distribution = TRAFFIC_TABLE_BASED;
				NoximGlobalParams::taskgraph_mode = true;
				void *mod_handle;
				typedef bool (* getMapping_t)(char *, int, int, vector<NoximCommunication> *, vector<NoximTaskGraphEntry> *, int, int);
				char * mod_error;
				mod_handle = dlopen(NoximGlobalParams::taskmapping_module_filename, RTLD_LAZY);
				if (!mod_handle){
					cerr<<dlerror()<<endl;
					assert(0);
				}
				getMapping_t getMapping = (getMapping_t)(dlsym(mod_handle, "getMapping"));
				if ((mod_error = dlerror()) != NULL){
					cerr<<mod_error<<endl;
					assert(0);
				}
				(*getMapping)(NoximGlobalParams::taskmapping_input_filename, NoximGlobalParams::mesh_dim_x, NoximGlobalParams::mesh_dim_y, &temp_comm, &temp_tg, 0, 0);
				traffic_table->setTrafficTable(temp_comm);
				task_graph->setTaskGraph(temp_tg);
				cout<<temp_tg.size()<<endl;
				dlclose(mod_handle);
			}
		}
	}
}
void NoximProcessingElement::rxProcess()
{
	if (reset.read()) {
		ack_rx.write(0);
		current_level_rx = 0;
		packet_received.clear();
	} else {
		if (req_rx.read() == 1 - current_level_rx) {
			NoximFlit flit_tmp = flit_rx.read();
			if (NoximGlobalParams::verbose_mode > VERBOSE_OFF) {
				cout << sc_simulation_time() << ": ProcessingElement[" <<
					local_id << "] RECEIVING " << flit_tmp;
			}
			if (NoximGlobalParams::taskgraph_mode && flit_tmp.dst_id==local_id && flit_tmp.flit_type==FLIT_TYPE_TAIL){
				NoximTaskGraphEntry * temp_taskgraph_entry= task_graph->getNodeInfo(local_id);
				if (temp_taskgraph_entry!=NULL){
					packet_received[flit_tmp.src_id]++;
					//calculating average time
					packet_received_average_time[flit_tmp.src_id]=((packet_received_average_time[flit_tmp.src_id]*packet_received_num[flit_tmp.src_id])+(sc_simulation_time()-flit_tmp.timestamp))/(++packet_received_num[flit_tmp.src_id]);
					if (NoximGlobalParams::verbose_mode > VERBOSE_OFF){
						cout<<"Total packets received from node "<<flit_tmp.src_id<<": "<<packet_received[flit_tmp.src_id]<<endl;
						cout<<"Approximate average time of "<<packet_received_num[flit_tmp.src_id]<<" packets received from node "<<flit_tmp.src_id<<": "<<packet_received_average_time[flit_tmp.src_id]<<endl<<endl;
					}
				}
			}
			if (NoximGlobalParams::verbose_mode>VERBOSE_OFF) cout<<endl;
			current_level_rx = 1 - current_level_rx;	// Negate the old value for Alternating Bit Protocol (ABP)
		}
		ack_rx.write(current_level_rx);
	}
}
void NoximProcessingElement::tgProcess()
{
	if(reset.read()){
		processing_time_counter=0;
		processing_time_set=false;
	}
	else{
		if (NoximGlobalParams::taskgraph_mode){
		//	vector<int> leaf_nodes = task_graph->getLeafNodes();
		//	cout<<"Leaf nodes: ";
		//	for(vector<int>::iterator it =leaf_nodes.begin();it!=leaf_nodes.end(); it++){
		//		cout<<*it<<" ";
		//	}
		//	cout<<endl;
			NoximTaskGraphEntry * temp_tg = task_graph->getNodeInfo(local_id);
			if (temp_tg != NULL){
				if (processing_time_set==true){
					if (processing_time_counter>1) processing_time_counter--;
					else{
						cout<<sc_simulation_time()<<": ProcessingElement["<<local_id<<"] "<<temp_tg->time<<" time units consumed"<<endl;
						processing_time_set=false;

						for (map<int, int>::iterator it=temp_tg->outputs.begin(); it!=temp_tg->outputs.end(); it++){
							cout<<sc_simulation_time()<<": ProcessingElement["<<local_id<<"] Pushing "<<it->second<<" packets into packet queue to node "<<it->first<<endl;
							for(int i =0;i<it->second;i++){
								taskgraph_output.push(it->first);
							}
						}
						if (first_packet_process_time==-1)
							first_packet_process_time = sc_time_stamp().to_double();
					}
				}
				else{
					bool req = true;
					for (map<int, int>::iterator it= temp_tg->inputs.begin(); it!=temp_tg->inputs.end();it++){
						if (packet_received[it->first] < it->second){
							req = false;
							break;
						}
					}
					if (req == true){
						for(map<int, int>::iterator it = temp_tg->inputs.begin(); it != temp_tg->inputs.end();it++){
							cout<<sc_simulation_time()<<": ProcessingElement["<<local_id<<"] "<<it->second<<" out of "<<packet_received[it->first]<<" from node "<<it->first<<" are being consumed at node "<<local_id<<endl;
							packet_received[it->first]-=it->second;

						}
						if (temp_tg->time > 0){
							cout<<sc_simulation_time()<<": ProcessingElement["<<local_id<<"] "<<temp_tg->time<<" time units to be consumed"<<endl;
							processing_time_counter=temp_tg->time;
							processing_time_set=true;
						}
					}
				}

			}
		}
	}

}
void NoximProcessingElement::txProcess()
{
	if (reset.read()) {
		req_tx.write(0);
		current_level_tx = 0;
		transmittedAtPreviousCycle = false;
	} else {
		NoximPacket packet;

		if (canShot(packet)) {
			packet_queue.push(packet);
			transmittedAtPreviousCycle = true;
		} else
			transmittedAtPreviousCycle = false;


		if (ack_tx.read() == current_level_tx) {
			if (!packet_queue.empty()) {
				NoximFlit flit = nextFlit();	// Generate a new flit
				if (NoximGlobalParams::verbose_mode > VERBOSE_OFF) {
					cout << sc_time_stamp().to_double() /
						1000 << ": ProcessingElement[" << local_id <<
						"] SENDING " << flit << endl;
				}
				flit_tx->write(flit);	// Send the generated flit
				current_level_tx = 1 - current_level_tx;	// Negate the old value for Alternating Bit Protocol (ABP)
				req_tx.write(current_level_tx);
				if (NoximGlobalParams::taskgraph_mode && first_packet_transmit_time==-1){
					first_packet_transmit_time = sc_time_stamp().to_double();
				}

			}
		}
	}
}

NoximFlit NoximProcessingElement::nextFlit()
{
	NoximFlit flit;
	NoximPacket packet = packet_queue.front();

	flit.src_id = packet.src_id;
	flit.dst_id = packet.dst_id;
	flit.timestamp = packet.timestamp;
	flit.sequence_no = packet.size - packet.flit_left;
	flit.hop_no = 0;
	//  flit.payload     = DEFAULT_PAYLOAD;

	if (packet.size == packet.flit_left)
		flit.flit_type = FLIT_TYPE_HEAD;
	else if (packet.flit_left == 1)
		flit.flit_type = FLIT_TYPE_TAIL;
	else
		flit.flit_type = FLIT_TYPE_BODY;

	packet_queue.front().flit_left--;
	if (packet_queue.front().flit_left == 0)
		packet_queue.pop();

	return flit;
}

bool NoximProcessingElement::canShot(NoximPacket & packet)
{
	bool shot;
	double threshold;

	if (NoximGlobalParams::traffic_distribution != TRAFFIC_TABLE_BASED) {
		if (!transmittedAtPreviousCycle)
			threshold = NoximGlobalParams::packet_injection_rate;
		else
			threshold = NoximGlobalParams::probability_of_retransmission;

		shot = (((double) rand()) / RAND_MAX < threshold);
		if (shot) {
			switch (NoximGlobalParams::traffic_distribution) {
				case TRAFFIC_RANDOM:
					packet = trafficRandom();
					break;

				case TRAFFIC_TRANSPOSE1:
					packet = trafficTranspose1();
					break;

				case TRAFFIC_TRANSPOSE2:
					packet = trafficTranspose2();
					break;

				case TRAFFIC_BIT_REVERSAL:
					packet = trafficBitReversal();
					break;

				case TRAFFIC_SHUFFLE:
					packet = trafficShuffle();
					break;

				case TRAFFIC_BUTTERFLY:
					packet = trafficButterfly();
					break;

				default:
					assert(false);
			}
		}
	} else {			// Table based communication traffic
		if (NoximGlobalParams::taskgraph_mode && task_graph->getNodeInfo(local_id)!=NULL){
			if (taskgraph_output.size()==0){
				return false;
			}
			threshold = NoximGlobalParams::packet_injection_rate;
			shot = (((double) rand()) / RAND_MAX < threshold);
			if (shot){
				packet = trafficTaskgraph();
			}

		}
		else{
			if (NoximGlobalParams::taskmapping_mode){
				if (traffic_table->occurrencesAsSource(local_id)==0)
					return false;
			}
			else
				if (never_transmit)
					return false;

			double now = sc_time_stamp().to_double() / 1000;
			bool use_pir = (transmittedAtPreviousCycle == false);
			vector < pair < int, double > > dst_prob;
			double threshold =
				traffic_table->getCumulativePirPor(local_id, (int) now,
						use_pir, dst_prob);

			double prob = (double) rand() / RAND_MAX;
			shot = (prob < threshold);
			if (shot) {
				for (unsigned int i = 0; i < dst_prob.size(); i++) {
					if (prob < dst_prob[i].second) {
						packet.make(local_id, dst_prob[i].first, now,
								getRandomSize());
						break;
					}
				}
			}
		}
	}

	return shot;
}

NoximPacket NoximProcessingElement::trafficRandom()
{
	NoximPacket p;
	p.src_id = local_id;
	double rnd = rand() / (double) RAND_MAX;
	double range_start = 0.0;

	//cout << "\n " << sc_time_stamp().to_double()/1000 << " PE " << local_id << " rnd = " << rnd << endl;

	int max_id =
		(NoximGlobalParams::mesh_dim_x * NoximGlobalParams::mesh_dim_y) -
		1;

	// Random destination distribution
	do {
		p.dst_id = randInt(0, max_id);

		// check for hotspot destination
		for (uint i = 0; i < NoximGlobalParams::hotspots.size(); i++) {
			//cout << sc_time_stamp().to_double()/1000 << " PE " << local_id << " Checking node " << NoximGlobalParams::hotspots[i].first << " with P = " << NoximGlobalParams::hotspots[i].second << endl;

			if (rnd >= range_start
					&& rnd <
					range_start + NoximGlobalParams::hotspots[i].second) {
				if (local_id != NoximGlobalParams::hotspots[i].first) {
					//cout << sc_time_stamp().to_double()/1000 << " PE " << local_id <<" That is ! " << endl;
					p.dst_id = NoximGlobalParams::hotspots[i].first;
				}
				break;
			} else
				range_start += NoximGlobalParams::hotspots[i].second;	// try next
		}
	} while (p.dst_id == p.src_id);

	p.timestamp = sc_time_stamp().to_double() / 1000;
	p.size = p.flit_left = getRandomSize();

	return p;
}

NoximPacket NoximProcessingElement::trafficTranspose1()
{
	NoximPacket p;
	p.src_id = local_id;
	NoximCoord src, dst;

	// Transpose 1 destination distribution
	src.x = id2Coord(p.src_id).x;
	src.y = id2Coord(p.src_id).y;
	dst.x = NoximGlobalParams::mesh_dim_x - 1 - src.y;
	dst.y = NoximGlobalParams::mesh_dim_y - 1 - src.x;
	fixRanges(src, dst);
	p.dst_id = coord2Id(dst);

	p.timestamp = sc_time_stamp().to_double() / 1000;
	p.size = p.flit_left = getRandomSize();

	return p;
}

NoximPacket NoximProcessingElement::trafficTranspose2()
{
	NoximPacket p;
	p.src_id = local_id;
	NoximCoord src, dst;

	// Transpose 2 destination distribution
	src.x = id2Coord(p.src_id).x;
	src.y = id2Coord(p.src_id).y;
	dst.x = src.y;
	dst.y = src.x;
	fixRanges(src, dst);
	p.dst_id = coord2Id(dst);

	p.timestamp = sc_time_stamp().to_double() / 1000;
	p.size = p.flit_left = getRandomSize();

	return p;
}

void NoximProcessingElement::setBit(int &x, int w, int v)
{
	int mask = 1 << w;

	if (v == 1)
		x = x | mask;
	else if (v == 0)
		x = x & ~mask;
	else
		assert(false);
}

int NoximProcessingElement::getBit(int x, int w)
{
	return (x >> w) & 1;
}

inline double NoximProcessingElement::log2ceil(double x)
{
	return ceil(log(x) / log(2.0));
}

NoximPacket NoximProcessingElement::trafficBitReversal()
{

	int nbits =
		(int)
		log2ceil((double)
				(NoximGlobalParams::mesh_dim_x *
				 NoximGlobalParams::mesh_dim_y));
	int dnode = 0;
	for (int i = 0; i < nbits; i++)
		setBit(dnode, i, getBit(local_id, nbits - i - 1));

	NoximPacket p;
	p.src_id = local_id;
	p.dst_id = dnode;

	p.timestamp = sc_time_stamp().to_double() / 1000;
	p.size = p.flit_left = getRandomSize();

	return p;
}

NoximPacket NoximProcessingElement::trafficShuffle()
{

	int nbits =
		(int)
		log2ceil((double)
				(NoximGlobalParams::mesh_dim_x *
				 NoximGlobalParams::mesh_dim_y));
	int dnode = 0;
	for (int i = 0; i < nbits - 1; i++)
		setBit(dnode, i + 1, getBit(local_id, i));
	setBit(dnode, 0, getBit(local_id, nbits - 1));

	NoximPacket p;
	p.src_id = local_id;
	p.dst_id = dnode;

	p.timestamp = sc_time_stamp().to_double() / 1000;
	p.size = p.flit_left = getRandomSize();

	return p;
}

NoximPacket NoximProcessingElement::trafficButterfly()
{

	int nbits =
		(int)
		log2ceil((double)
				(NoximGlobalParams::mesh_dim_x *
				 NoximGlobalParams::mesh_dim_y));
	int dnode = 0;
	for (int i = 1; i < nbits - 1; i++)
		setBit(dnode, i, getBit(local_id, i));
	setBit(dnode, 0, getBit(local_id, nbits - 1));
	setBit(dnode, nbits - 1, getBit(local_id, 0));

	NoximPacket p;
	p.src_id = local_id;
	p.dst_id = dnode;

	p.timestamp = sc_time_stamp().to_double() / 1000;
	p.size = p.flit_left = getRandomSize();

	return p;
}

NoximPacket NoximProcessingElement::trafficTaskgraph()
{
	NoximPacket p;
	p.src_id = local_id;
	p.dst_id = taskgraph_output.front();
	taskgraph_output.pop();
	p.timestamp = sc_time_stamp().to_double() / 1000;
	p.size = p.flit_left = getRandomSize();

	return p;
}

void NoximProcessingElement::fixRanges(const NoximCoord src,
		NoximCoord & dst)
{
	// Fix ranges
	if (dst.x < 0)
		dst.x = 0;
	if (dst.y < 0)
		dst.y = 0;
	if (dst.x >= NoximGlobalParams::mesh_dim_x)
		dst.x = NoximGlobalParams::mesh_dim_x - 1;
	if (dst.y >= NoximGlobalParams::mesh_dim_y)
		dst.y = NoximGlobalParams::mesh_dim_y - 1;
}

int NoximProcessingElement::getRandomSize()
{
	return randInt(NoximGlobalParams::min_packet_size,
			NoximGlobalParams::max_packet_size);
}
