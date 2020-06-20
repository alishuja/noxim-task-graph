#ifndef __NOXIMTASKGRAPHENTRY_H__
#define __NOXIMTASKGRAPHENTRY_H__

#include <map>

struct NoximTaskGraphEntry{
	int node;
	std::map <int, int> inputs;
	int time;
	std::map <int, int> outputs;
};

#endif
