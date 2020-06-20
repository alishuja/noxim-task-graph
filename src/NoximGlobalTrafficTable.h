/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2010 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the definition of the global traffic table
 */

#ifndef __NOXIMGLOBALTRAFFIC_TABLE_H__
#define __NOXIMGLOBALTRAFFIC_TABLE_H__

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "NoximMain.h"
#include "NoximCommunication.h"
using namespace std;

class NoximGlobalTrafficTable {

  public:

    NoximGlobalTrafficTable();

    // Load traffic table from file. Returns true if ok, false otherwise
    bool load(const char *fname);

    // Returns the cumulative pir por along with a vector of pairs. The
    // first component of the pair is the destination. The second
    // component is the cumulative shotting probability.
    double getCumulativePirPor(const int src_id,
			       const int ccycle,
			       const bool pir_not_por,
			       vector < pair < int, double > > &dst_prob);

    // Returns the number of occurrences of soruce src_id in the traffic
    // table
    int occurrencesAsSource(const int src_id);

    // Insert entry manually
    void insert(const int src, const int dst, const float pir = 1.5, const float por=1.5, const int t_on=-1, const int t_off=-1, const int t_period = 0);
    void setTrafficTable(vector < NoximCommunication > traffic_table);
  private:

     vector < NoximCommunication > traffic_table;
};

#endif
