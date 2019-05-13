//
//  Author: Harry Kai-Ho Chan
//  Email: khchanak@cse.ust.hk
//


#include "cao_alg_new.h"


/*
 *	The implementation of the "Cao-Appro3" algorithm.
 *  CCG+15
 *
 */
obj_set_t* Sum_Appro( query_t* q)
{
    int i, rear, top;
    B_KEY_TYPE costV;
    b_heap_t* U;
    obj_set_t* V;
    void* e,* e2;
    node_t* node_v,* node_v2;
    obj_t* obj_v,* obj_v2;
    psi_t* mSet, * pSet;
    
    U = alloc_b_heap( INI_HEAP_SIZE);
    
    rear = 1;
    U->obj_arr[ rear].element = ( void*)IRTree_v.root;
    U->obj_arr[ rear].e_tag = 1;
    U->obj_arr[ rear].key = 0;
    
    b_h_insert( U, rear++);
    
    V = alloc_obj_set();
    costV = 0;
    
    mSet = alloc_psi();
    copy_k_list(mSet->k_head, q->psi_v->k_head);
    mSet->key_n = q->psi_v->key_n;
    
    pSet = alloc_psi();
    copy_k_list(pSet->k_head, q->psi_v->k_head);
    pSet->key_n = q->psi_v->key_n;
    
    //Best-first search process.
    while(mSet->key_n!=0 && !b_h_is_empty( U))
    {
        top = b_h_get_top( U);
        e = U->obj_arr[ top].element;
        
        B_KEY_TYPE key =U->obj_arr[ top].key;
        U->obj_arr[top].key =-1; //**
        
        if( U->obj_arr[ top].e_tag == 2){
            costV = costV +key;
            
            
            //e is an obj_t*.
            obj_v = ( obj_t*)e;
            
            add_obj_set_entry(obj_v, V);
            
            //pSet <- mSet
            release_psi(pSet);
            pSet = alloc_psi();
            copy_k_list(pSet->k_head, mSet->k_head);
            pSet->key_n = mSet->key_n;
            
            //mSet <- mSet - o.\psi
            psi_exclusion(mSet, obj_v);
            
            //for each element in the heap
            //  U->rear != rear
            for(int i=1;i<=U->rear;i++){
                int k = U->h_arr[i];
                
                e2 =  U->obj_arr[k].element;
                
                if( U->obj_arr[k].e_tag == 2){//obj
                    obj_v2 = ( obj_t*)e2;
                    
                    int cnt = is_relevant_obj(obj_v2, mSet);//cnt= number of intersection of keyword in obj_v2 and mSet
                    if(cnt==0)
                        U->obj_arr[k].key = -1;
                    else
                        U->obj_arr[k].key = U->obj_arr[k].key * ((double)is_relevant_obj(obj_v2, pSet)) / ((double)cnt);
                    
                    
                }else{//node
                    node_v2 = (node_t*)e2;
                    
                    int cnt = is_relevant_node(node_v2, mSet);
                    
                    if(cnt==0)
                        U->obj_arr[k].key = -1;
                    else
                        U->obj_arr[k].key = U->obj_arr[k].key *    ((double)is_relevant_node(node_v2, pSet)) / ((double)cnt);
                    
                }
            }//end for
            b_h_restruct_heap(U, rear);
        }
        else
        {
            //e is an node_t*.
            node_v = ( node_t*)e;
            for( i=0; i<node_v->num; i++)
            {
                if( node_v->level == 0)
                {
                    //node_v is a leaf-node.
                    obj_t* child = (obj_t*)node_v->child[i];
                    
                    int cnt = is_relevant_obj(child, mSet);
                    
                    if (cnt > 0){
                        loc_t* loc_v = get_obj_loc(child);
                        
                        U->obj_arr[ rear].element = node_v->child[ i];
                        U->obj_arr[ rear].e_tag = 2;
                        U->obj_arr[ rear].key = calc_dist_loc(loc_v, q->loc_v)/((double)cnt);
                        
                        //Enqueue.
                        b_h_insert( U, rear++);
                        release_loc(loc_v);
                    }
                }
                else
                {
                    //node_v is an inner-node.
                    node_t* child = (node_t*)node_v->child[i];
                    
                    int cnt = is_relevant_node(child, mSet);
                    
                    if (cnt > 0){
                        U->obj_arr[ rear].element = node_v->child[ i];
                        U->obj_arr[ rear].e_tag = 1;
                        U->obj_arr[ rear].key = calc_minDist_node(child, q->loc_v)/((double)cnt);
                        //Enqueue.
                        b_h_insert( U, rear++);
                        
                    }
                }
            }//for
        }//else
    }//while
    
    release_b_heap( U);
    release_psi(mSet);
    release_psi(pSet);
    
    
    return V;
}


/*
 *	The implementation of the "Cao-Appro2" algorithm.
 *
 */
obj_set_t* Cao_Appro2_new( query_t* q, std::unordered_map<KEY_TYPE, KEY_TYPE>* keyfreq_hashmap)
{
	int i, rear, top;
	KEY_TYPE c_key;
	B_KEY_TYPE costV, costV_1, dist;
	b_heap_t* U;
	obj_set_t* V, *V_1, *V_tmp;
	void* e;
	node_t* node_v;
	obj_t* obj_v;
	bst_node_t* bst_node_v;
	BIT_TYPE p_list;
	loc_t* loc_v;
	query_t* q_new;
    
	//printf( "Cao-Appro2:\n");
    
	U = alloc_b_heap( INI_HEAP_SIZE);
    
	rear = 1;
	U->obj_arr[ rear].element = ( void*)IRTree_v.root;
	U->obj_arr[ rear].e_tag = 1;
	U->obj_arr[ rear].key = calc_minDist_node( IRTree_v.root, q->loc_v);
    
	b_h_insert( U, rear++);
	
	V = Cao_Appro1( q);
    
	/*s*/
	stat_v.n_k ++;
	/*s*/
    
	if( V == NULL)
	{
		release_b_heap( U);
		return NULL;
	}
    
	costV = comp_cost( cost_tag, V, q);
    
    //new :find the most infrequent keyword
    c_key = findMostInfreqKey(q->psi_v, keyfreq_hashmap);
    
	//Best-first search process.
	while( !b_h_is_empty( U))
	{
		top = b_h_get_top( U);
		e = U->obj_arr[ top].element;
        
		/*t/
         if( e == NULL)
         printf( "");
         /*t*/
        
		if( U->obj_arr[ top].e_tag == 1)
		{
			//e is an node_t*.
			node_v = ( node_t*)e;
            
			//Check the distance.
			dist = calc_minDist_node( node_v, q->loc_v);
            
            //new: pruning
            if (dist > costV){
                break;
            }
            
			bst_node_v = bst_search( node_v->bst_v, c_key);
			if( bst_node_v == NULL)
				continue;
            
			p_list = bst_node_v->p_list;
			for( i=0; i<node_v->num; i++)
			{
				//Check the c_key keyword.
				if( !get_k_bit( p_list, i))
					continue;
   
				U->obj_arr[ rear].element = node_v->child[ i];
				if( node_v ->level > 0)
				{
					//node_v is an inner-node.
					U->obj_arr[ rear].e_tag = 1;
					U->obj_arr[ rear].key = calc_minDist( node_v->MBRs[ i], q->loc_v);
				}
				else
				{
					//node_v is a leaf-node.
					U->obj_arr[ rear].e_tag = 2;
					U->obj_arr[ rear].key = calc_minDist( node_v->MBRs[ i], q->loc_v);
				}
                
				//Enqueue.
				b_h_insert( U, rear++);
			}//
		}
		else
		{
			//e is an obj_t*.
			obj_v = ( obj_t*)e;
			loc_v = get_obj_loc( obj_v);
            
            //new: pruning
            dist = calc_dist_loc(q->loc_v, loc_v);
            if (dist > costV){
                break;
            }
            
			//Construct a new query instance.
			q_new = alloc_query( );
			q_new->loc_v = loc_v;
            
			q_new->psi_v = alloc_psi( );
			copy_k_list( q_new->psi_v->k_head, q->psi_v->k_head); //obj_v->k_head);
            
			//Solve the new query.
			V_1 = Cao_Appro1( q_new);
			//add_obj_set_entry( obj_v, V_1);
            
			if( V_1 == NULL)
			{
				release_query( q_new);
				continue;
			}
            
			costV_1 = comp_cost(cost_tag, V_1, q);
            
			if( costV_1 < costV)
			{
				costV = costV_1;
				V_tmp = V;
				V = V_1;
				
				release_obj_set( V_tmp);
			}
			else
				release_obj_set( V_1);
            
			release_query( q_new);
			
			/*s*/
			stat_v.n_k ++;
			/*s*/
		}//else
	}//while
    
	release_b_heap( U);
    
	return V;
}

B_KEY_TYPE findMostInfreqKey(psi_t* psi_v, std::unordered_map<KEY_TYPE, KEY_TYPE>* keyfreq_hashmap){
  
    B_KEY_TYPE mostInfreqKey = -1;
    B_KEY_TYPE count=INFINITY;
    k_node_t* k_node_v = psi_v->k_head->next;
    std::unordered_map< KEY_TYPE, KEY_TYPE>::const_iterator got;
    
    while ( k_node_v !=NULL){
        got = keyfreq_hashmap->find(k_node_v->key);
        
        if (got == keyfreq_hashmap->end())
            exit(-1);//not found means error and exit
        
        //find min
        if ( count > got->second )
        {
            mostInfreqKey = got->first;
            count = got->second;
        }
        
        k_node_v = k_node_v->next;
    }
    
   // printf("key:%f \t cnt:%f\n",mostInfreqKey, count);
    
    return mostInfreqKey;

}









