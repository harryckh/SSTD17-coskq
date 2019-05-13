//
//  Author: Harry Kai-Ho Chan
//  Email: khchanak@cse.ust.hk
//

#ifndef MAXDOTSIZE_H
#define MAXDOTSIZE_H


#include "cao_alg.h"

obj_set_t* MaxDotSize_ConstructGreedyFeasibleSet(  obj_t* o, query_t* q, int cost_tag, obj_set_t* O_t);

obj_set_t* MaxDotSize( query_t* q,int s_tag, int cost_tag);

void FindBestFeasibleSet(int cost_tag, obj_t* o, query_t* q, obj_set_t*& S_c, B_KEY_TYPE& cost_c);
void EnumerateSubset( int cost_tag, obj_set_t* O_t, psi_t* psi, obj_t* o, query_t* q, obj_set_t*& S_a, B_KEY_TYPE& cost_c);

void EnumerateSubset_sub( int cost_tag, bst_t* IF_v, psi_t* psi, obj_set_t* S_0, obj_t* o, query_t* q,B_KEY_TYPE d, B_KEY_TYPE doq ,  obj_set_t*& S_a,  B_KEY_TYPE& cost_c);

//pruning

void removeObjDominated( int cost_tag, query_t* q, obj_set_t* O_t, psi_t* psi_v);

bool isDominated(obj_t* obj_v, obj_t* obj_v2, psi_t* psi_v);

//Some useful functions

obj_set_t* range_query(query_t* q);

void release_IF( bst_t* T);

void release_IF_sub( bst_node_t* x);

//-----

void psi_insert( psi_t* psi_v,k_node_t* k_head);

psi_t* key_intersection( k_node_t* k_head1, k_node_t* k_head2);

psi_t* psi_exclusion( psi_t* psi_v1, obj_t* obj_v);

int is_relevant_node(node_t* node_v, psi_t* psi_v);

int is_relevant_obj( obj_t* obj_v, psi_t* psi_v);

int is_relevant_node(node_t* node_v, obj_t* obj_v);

psi_t* node_intersection(node_t* node_v, psi_t* psi_v);

int number_intersection( k_node_t* k_head1, k_node_t* k_head2);

obj_set_t* retrieve_obj_key( KEY_TYPE key);

obj_set_t* range_query_sorted_dist( disk_t* disk_v, query_t* q, loc_t* loc_v);

void range_query_sorted_dist_sub( node_t* node_v, disk_t* disk_v, obj_set_t* &obj_set_v, query_t* q, loc_t* loc_v);

void retrieve_sub_tree_sorted_dist( node_t* node_v, obj_set_t* &obj_set_v, query_t* q, loc_t* loc_v);

void add_obj_set_entry_sorted( obj_t* obj_v, obj_set_t* obj_set_v, B_KEY_TYPE dist);



#endif
