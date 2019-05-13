//
//  Author: Harry Kai-Ho Chan
//  Email: khchanak@cse.ust.hk
//


/*
 *  Implementation of the approximate algorithms proposed in the paper
 *  "Efficient Processing of Spatial Group Keyword Queries"
 *  by Xin Cao et al.
 */

#ifndef __coskq__cao_alg_new__
#define __coskq__cao_alg_new__

#include "cao_alg.h"

obj_set_t* Sum_Appro( query_t* q);

obj_set_t* Cao_Appro2_new( query_t* q, std::unordered_map<KEY_TYPE, KEY_TYPE>* keyfreq_hashmap);

B_KEY_TYPE findMostInfreqKey(psi_t* psi_v, std::unordered_map<KEY_TYPE, KEY_TYPE>* keyfreq_hashmap);

#endif /* defined(__coskq__cao_alg_new__) */
