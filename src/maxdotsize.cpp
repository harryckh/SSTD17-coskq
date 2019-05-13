//
//  Author: Harry Kai-Ho Chan
//  Email: khchanak@cse.ust.hk
//

#include "maxdotsize.h"

/*
 *	The implementation of the greedy process of the approximate algorithm in the paper.
 *
 *  @cost_tag :
 *      = 3: WorstCase
 *      = 13: InherentCost
 */

obj_set_t* MaxDotSize_ConstructGreedyFeasibleSet( obj_t* o, query_t* q, int cost_tag, obj_set_t* O_t)
{
    /// printf("object id:%d\n",o->id);
	obj_node_t* obj_node_v,* obj_node_picked;
    obj_set_t* S,* obj_set_v;
    k_node_t *k_node_v;
    
    int num_inter;
    double ratio,ratio_max;
    psi_t* psi_v;
    int map_size =0;
    obj_set_t** map; //2D array storing the objects having X uncovered keywords
    
	//Obtain the "un-covered" keywords by S.
	k_node_v = key_exclusion( q->psi_v->k_head, o->k_head);
    psi_v = const_psi(k_node_v);
    
    if(psi_v->key_n==0){
        release_psi(psi_v);
        S = alloc_obj_set();
        //Include object o in S.
        add_obj_set_entry( o, S);
        
        return S;
    }
    
    S = alloc_obj_set();
    //Include object o in S.
    add_obj_set_entry( o, S);
    
    obj_set_v =(O_t);
    
    if(cost_tag==3){
        //initialize the 2D array
        map_size =psi_v->key_n;
        map = new obj_set_t*[map_size]; // -1 inside the array index
        for(int i=0;i<map_size;i++)
            map[i] = alloc_obj_set();
        
        //iterate each object and allocate to the corresponding set in the array
        obj_node_v = obj_set_v->head->next;
        while(obj_node_v != NULL)
        {
            num_inter = is_relevant_obj(obj_node_v->obj_v, psi_v);
            if(num_inter >0)
                add_obj_set_entry(obj_node_v->obj_v, map[num_inter-1]);
            obj_node_v=obj_node_v->next;
        }
    }
    
    //find the object which has the greatest number of uncovered keywords
    //while there are keyword not covered yet
    while( psi_v->key_n > 0 )
	{
        obj_node_picked = NULL;
        obj_node_v = obj_set_v->head->next;
        
        if(cost_tag==3)//MaxDotSize
        {
            for( int i=psi_v->key_n; i>0 ; i--){
                //we are checking obj_node_prev->next for easier removing object
                obj_node_t* obj_node_prev = map[i-1]->head;
                obj_node_t* obj_node_temp;
                while( obj_node_prev->next !=NULL){
                    num_inter = is_relevant_obj(obj_node_prev->next->obj_v, psi_v);
                    //printf("o:%d\t %d\n",obj_node_prev->next->obj_v->id, num_inter);
                    
                    if(num_inter == 0){
                        //remove this obj from the current list
                        obj_node_temp=obj_node_prev->next;
                        obj_node_prev->next = obj_node_prev->next->next;
                        map[i-1]->obj_n --;
                        free(obj_node_temp);
                        continue;
                        
                    }
                    
                    if( num_inter == i ){
                        //this object can be add to S'
                        obj_node_picked=obj_node_prev->next;
                        goto J;
                    }
                    else{                        //remove this obj from the current list
                        obj_node_temp=obj_node_prev->next;
                        obj_node_prev->next = obj_node_prev->next->next;
                        map[i-1]->obj_n --;
                        // then put it in the correct list
                        add_obj_set_entry(obj_node_temp->obj_v, map[num_inter-1]);
                        free(obj_node_temp);
                        
                       // print_obj_set(map[num_inter-1], stdout);
                    }
                    //obj_node_prev = obj_node_prev->next;
                }
            }
        }
        else if (cost_tag==13)//Inherent-Cost aware
        {
            ratio_max = 0.0;
            while(obj_node_v != NULL){
                ratio =((double)is_relevant_obj(obj_node_v->obj_v, psi_v))
                        / ((double)obj_node_v->obj_v->inherent);
                if(ratio > ratio_max){
                    ratio_max = ratio;
                    obj_node_picked = obj_node_v;
                }
                obj_node_v = obj_node_v->next;
            }
        }
        else{
            // !(cost_tag == 3 or cost_tag == 13)
            exit(-1);
        }
        
    J:  ///if no object is found, we cannot covers all keywords
        if( obj_node_picked == NULL)
        {
            release_obj_set(S);
            S = NULL;
            break;
        }
        
        //update the uncovered keywords
        //S=S \cup o'
        // \psi = \psi - o'\psi
        add_obj_set_entry( obj_node_picked->obj_v, S);
        psi_exclusion(psi_v, obj_node_picked->obj_v);
        
 	}///end while
    
    if( psi_v->key_n != 0){
        release_obj_set(S);
        S = NULL;
    }
    
    //Release the memory.
    release_psi(psi_v);
    
    if(cost_tag==3){
    for(int i=0;i<map_size;i++)
         release_obj_set(map[i]);
    delete [] map;
    }
    
    
	return S;
}


/*
 *  The implementaiton of Algotihm 1 - MaxDotSize-E (and MaxDotSize-A) in the paper.
 *
 *  s_tag = 1: Exact
 *  s_tag = 2: Approximate
 *  
 *  cost_tag = 3: MaxDotSize
 *  cost_tag = 13: Inherent-cost aware
 *
 */

obj_set_t* MaxDotSize( query_t* q, int s_tag, int cost_tag)
{
    
    int top;
    B_KEY_TYPE LB, UB, cost_c, cost, dist;
    obj_set_t* S_c;				//the current best solution.
    obj_set_t* S;               //the newly constructed feasible set.
    obj_set_t* R;				//region R.
    disk_t* disk_u, *disk_l;	//the outer and inner disks.
    obj_t* o;
    loc_t* loc_v;
    obj_node_t* obj_node_v;
    obj_set_t * O_t;
    
    
    //Compute the LB and UB of cost(S*, q).c_1;
    S_c = comp_bounds(cost_tag,  q, LB, UB);
    ///LB and UB are updated in comp_bounds function
    cost_c = UB;
    if( S_c == NULL)
        return NULL;
  
    //Initialize region R.
    //An alternative implementation is possible here.
    //Direct range query with the range be a "ring".
    disk_u = alloc_disk( IRTree_v.dim);
    disk_l = alloc_disk( IRTree_v.dim);
    
    //disk_u & disk_l is centered at loc_v with r=UB & LB
    set_disk( disk_u, q->loc_v, UB);
    set_disk( disk_l, q->loc_v, LB);
    
    R = range_query_sorted_dist( disk_u, q, q->loc_v); //set of query distance owners
    O_t = range_query( disk_l, q); //maintain the objects in D(q,d(o,q)) dynamically
    
    //exclude the objects in region_u that are inside disk_l
    obj_exclusion_disk( R, disk_l);
    
    /*s/
    printf( "#cands: %i\n", R->obj_n);
    /*s*/
    /*t/
    print_obj_set( R, stdout);
    /*t*/
    
    bool firstObj = true;
    //Pre-checking.
    if( R->obj_n == 0)
        goto E;
    
    obj_node_v = R->head->next;
    while( obj_node_v !=NULL)
    //-while there exist unprocessed relevant object o in R(S)
    {
        
        o = obj_node_v->obj_v;
        
        loc_v = get_obj_loc( o);
        dist = calc_dist_loc( loc_v, q->loc_v);
        release_loc( loc_v);
        
        //lemma 2
        //r_max
        if( dist > cost_c)
            break;

        //lemma 3
        if ((!firstObj) && is_relevant_obj(o, q->psi_v) == 1)
        {
            firstObj = false;
            obj_node_v = obj_node_v->next;
            continue;
        }
        
        
        
        /*s*/
        stat_v.n_1_sum ++;
        /*s*/
        
        //The "ConstructFeasibleSet" procedure.
        if(s_tag==1){ //Exact Algorithm
            //Pre-checking (for the boundary case that |S| = 1).
            S = alloc_obj_set( );
            add_obj_set_entry( o, S);
            //---
            if(is_covered_obj_set( S, q))
            {
                cost = comp_cost(cost_tag, S, q);
                if( cost < cost_c)
                {
                    release_obj_set( S_c);
                    S_c = S;
                    cost_c = cost;
                }
                else
                    release_obj_set( S);
            }else{
                release_obj_set( S);
                
                //---Update UB by using Appro. alg---
                S = MaxDotSize_ConstructGreedyFeasibleSet( o, q, cost_tag, O_t);
                add_obj_set_entry(o, O_t);
                if( !S){
                    obj_node_v = obj_node_v->next;
                    continue;
                }
                cost = comp_cost( cost_tag, S, q);
                if( cost <= cost_c)
                {
                    release_obj_set( S_c);
                    S_c = S;
                    cost_c = cost;
                }
                else
                    release_obj_set( S);
                //----------------------------------------------------*/
                FindBestFeasibleSet(cost_tag, o, q, S_c, cost_c);
            }
        }else if(s_tag==2){ //Approximate Algorithm
            S = MaxDotSize_ConstructGreedyFeasibleSet( o, q, cost_tag, O_t);
        
            add_obj_set_entry(o, O_t);
            if( !S){
                obj_node_v = obj_node_v->next;
                continue;
            }
            cost = comp_cost( cost_tag, S, q);
            
            if( cost <= cost_c)
            {
                /// printf("cost:%0.2lf, cost_c:%0.2lf",cost,cost_c);
                release_obj_set( S_c);
                S_c =  S;
                S = NULL;
                cost_c = cost;
             }
            else
                release_obj_set( S);
            
        }
        
        obj_node_v = obj_node_v->next;
    }//while
    //Release the resource.
//    release_b_heap( R_heap);
E:
    release_obj_set(O_t);
    
    remove_identical_obj( S_c);
    release_disk( disk_u);
    release_disk( disk_l);
    release_obj_set( R);
    
    return S_c;
}

/*
 * The implementation of Algorithm 2 - findBestFeasibleSet in the paper.
 *
 */

void FindBestFeasibleSet(int cost_tag, obj_t* o, query_t* q, obj_set_t*& S_c, B_KEY_TYPE& cost_c)
{
    //Disks.
	B_KEY_TYPE radius_1;
	loc_t* loc_v1;
	disk_t* disk_v1;
	obj_set_t* O_t;
	psi_t* psi_v;
    k_node_t* k_head_1;
    
    k_head_1 = key_exclusion( q->psi_v->k_head, o->k_head);
	psi_v = const_psi( k_head_1);
    
    //Construct the disks.
	//1. Disk(q, d(o, q)).
	loc_v1 = get_obj_loc( o);
	radius_1 = calc_dist_loc( loc_v1, q->loc_v);
	disk_v1 = const_disk( q->loc_v, radius_1);
	//Range query on disk_v1.
	O_t = range_query( disk_v1, q);
    
	release_loc( loc_v1);
	release_disk( disk_v1);
	
	//Check whether O_t covers the keywords in the query.
	if( !FeasibilityCheck( O_t, psi_v))
		goto E;
    
  	/*s*/
	stat_v.O_t_size_sum += O_t->obj_n;
	stat_v.psi_n_sum += psi_v->key_n;
    /*s*/
	
    //Find a sub-set of O_t which covers psi_v.
    EnumerateSubset(cost_tag, O_t, psi_v, o, q, S_c, cost_c);
    
E:
	//Release the resources.
	release_psi( psi_v);
//	release_obj_set( O_t);
    
	return ;
}

void EnumerateSubset(int cost_tag, obj_set_t* O_t, psi_t* psi, obj_t* o, query_t* q, obj_set_t*& S_c, B_KEY_TYPE& cost_c){
    
    bst_t* IF_v;
	obj_set_t* S_0;
    B_KEY_TYPE x=0.0, addCostLB=0.0 ;
   
    //Pruning in Section 4.1.2 of the paper
    if (cost_tag == 3) {
        removeObjDominated(cost_tag, q, O_t, psi);
    }

	//Construct an IF structure.
    IF_v = const_IF( O_t, psi);
    
	//Initialize the S_0.
	S_0 = alloc_obj_set( );
    add_obj_set_entry( o, S_0);
    
    if(cost_tag == 3)//MaxDotSize
    {
        x = cost_c / calc_minDist(o->MBR, q->loc_v);
    }
    else if (cost_tag==13) //Inherent-cost aware
    {
        x = cost_c / calc_minDist(o->MBR, q->loc_v) ;
        addCostLB = o->inherent;
    }
    
	//Invoke the sub-procedure "recursively".
	EnumerateSubset_sub( cost_tag, IF_v, psi, S_0, o, q, x, addCostLB, S_c, cost_c);
    
	//Release the resources.
	bst_release( IF_v);
	release_obj_set( S_0);
    
	return;
}

/*
 *	The sub-procedure of function "EnumerateSubset".
 *  value of x depends on the cost_tag
 *      cost_tag==3 (MaxDotSize):
 *                                      x = max num of obj
 *      cost_tag==13 (Inherent-cost aware):
 *                                      x = max. sum of inherent cost
 *  addCostLB = the current sum of inherent cost of S_0
 */
void EnumerateSubset_sub(int cost_tag, bst_t* IF_v, psi_t* psi, obj_set_t* S_0, obj_t* o, query_t* q, B_KEY_TYPE x, B_KEY_TYPE addCostLB, obj_set_t*& S_c,  B_KEY_TYPE& cost_c)
{
    
	obj_t* obj_v;
	bst_node_t* bst_node_v;
	obj_node_t* obj_node_v;
	bst_node_list_t* bst_node_list_v;
    B_KEY_TYPE cost_new =0;
    B_KEY_TYPE addCostLB_new=0.0;
    
	if( IF_v->node_n == 0){
        return;
    }
    
	bst_node_v = IF_v->root;
	obj_node_v = bst_node_v->p_list_obj->head->next;
    
	while( obj_node_v != NULL)
	{
        //Pick an object.
		obj_v = obj_node_v->obj_v;
        
       	//Update the IF_v.
		bst_node_list_v = update_IF_obj( IF_v, obj_v);
        
       
        addCostLB_new = addCostLB + ((double)obj_v->inherent);
        
		//Update the S_0.
		//obj_v is added at the first place of S_0.
		add_obj_set_entry( obj_v, S_0);
        
        //Recursive call
        if (addCostLB_new < x)
        {
            EnumerateSubset_sub( cost_tag, IF_v, psi, S_0, o, q, x, addCostLB_new, S_c, cost_c);
        }
        
		//Checking.
        if(IF_v->node_n == 0)
        {
            //update current best
          
            cost_new = comp_cost(cost_tag, S_0,q);
            if( cost_new < cost_c)
            {
                release_obj_set( S_c);
                S_c = copy_obj_set(S_0) ;
                cost_c = cost_new;
            }
        }
        
        //Restore the S_0.
		remove_obj_set_entry( S_0);
        
		//Restore the IF_v.
		restore_IF_bst_node_list( IF_v, bst_node_list_v);
		release_bst_node_list( bst_node_list_v);
        
		//Try the next object candidate.
		obj_node_v = obj_node_v->next;
	}
    return ;
}



/* check objects one by one: remove objects that are being dominated by other object */
void removeObjDominated( int cost_tag, query_t* q, obj_set_t* O_t, psi_t* psi_v){
    
    obj_node_t* obj_node_v,* obj_node_v2, * obj_node_temp;
    bool remove;
    
   
    obj_node_v = O_t->head;
    // checking obj_node_v->next
    while (obj_node_v->next != NULL)
    {
        remove = false;
        obj_node_v2 =  O_t->head->next;
        while (obj_node_v2 != NULL)
        {
            if ( obj_node_v->next->obj_v == obj_node_v2->obj_v){
                obj_node_v2 = obj_node_v2->next;
                continue;
            }
            
            if(cost_tag==10)//distance constraint
            {
                if(obj_node_v2->numOfInt==0){
                    obj_node_v2 = obj_node_v2->next;
                    continue;
                }
                    
            }
            
            if(isDominated(obj_node_v->next->obj_v, obj_node_v2->obj_v, psi_v))
            {
                //remove obj_node_v->next, i.e. outer loop obj
                obj_node_temp = obj_node_v->next;
                obj_node_v->next = obj_node_v->next->next;
                free(obj_node_temp);
                remove = true;
                O_t->obj_n--;
                break;
            }
                obj_node_v2 = obj_node_v2->next;
            
        }
        if(!remove)
            obj_node_v=obj_node_v->next;
    }
    
}

/* check whether obj_v is dominated by obj_v2 */
bool isDominated(obj_t* obj_v, obj_t* obj_v2, psi_t* psi_v){
    
    k_node_t* k_node_v;
    B_KEY_TYPE key;
    
    k_node_v = psi_v->k_head->next;
    
    //for each keyword in  psi_v
    while( k_node_v != NULL){
        key = k_node_v->key;
        //if there exist a keyword that is covered by obj_v but not obj_v2
        //obj_v is not dominated by obj_v2
        if( has_key_obj(obj_v, key) && !has_key_obj(obj_v2, key))
            return false;
        k_node_v=k_node_v->next;
    }
    return true;
}


/*
 *	 range query.
 *
 *	DFS: recursive implementation.
 * modified from range_query and range_query_sub, this version does not need the disk
 * all related objs are retrived
 */
obj_set_t* range_query(query_t* q)
{
	obj_set_t* obj_set_v;
    
	obj_set_v = alloc_obj_set( );
    retrieve_sub_tree(  IRTree_v.root, obj_set_v, q);
    
	return obj_set_v;
}

/*
 *	Release the binary search tree T.
 * modified from bst_release
 */
void release_IF( bst_t* T)
{
	if( T != NULL)
	{
		if( T->root != NULL)
			release_IF_sub( T->root);
        
        free( T);
        
		/*s*/
		stat_v.memory_v -= sizeof( bst_t);
		/*s*/
	}
}

void release_IF_sub( bst_node_t* x)
{
	if( x->left != NULL)
		bst_release_sub( x->left);
	if( x->right != NULL)
		bst_release_sub( x->right);
    
    release_obj_set(x->p_list_obj);
	free( x);
    
	/*s*/
	stat_v.memory_v -= sizeof( bst_node_t);
	/*s*/
}

//=================================================================================


/*
 *	Exclude the keywords that occur in @obj_v from @psi_v1.
 *
 *	The excluded keywords are returned in @psi_excluded.
 *
 */
psi_t* psi_exclusion( psi_t* psi_v1, obj_t* obj_v)
{
    
    psi_t* psi_excluded = alloc_psi();
    k_node_t* k_node_excluded = psi_excluded->k_head; //pointer always point to the end of link list
    
    int tag;
    k_node_t* k_node_prev, *k_node_v1, *k_node_v2;
    
    k_node_prev = psi_v1->k_head;
    k_node_v1 = psi_v1->k_head->next;
    while( k_node_v1 != NULL)
    {
        tag = 0;
        k_node_v2 = obj_v->k_head->next;
        while( k_node_v2 != NULL)
        {
            if( k_node_v2->key == k_node_v1->key)
            {
                tag = 1;
                break;
            }
            
            k_node_v2 = k_node_v2->next;
        }
        if( tag==1){
            //The current keyword should be deleted from psi_v1
            
            //pointed moved to new end
            add_keyword_entry(k_node_excluded, k_node_v1->key);
            psi_excluded->key_n++;
            
            k_node_prev->next = k_node_v1->next;
            k_node_v1->next=NULL;
            free(k_node_v1);
            psi_v1->key_n--;
            
            k_node_v1=k_node_prev->next;
        }else{
            k_node_v1 = k_node_v1->next;
            k_node_prev=k_node_prev->next;
        }
    }
    
    return psi_excluded;
}


/*
 *	Construct a psi_t structure based on a list of keywords @k_head.
 *  Deep copy performed.
 */
void psi_insert( psi_t* psi_v,k_node_t* k_head)
{
    k_node_t* k_node_v;
    
    k_node_v = k_head->next;
    while( k_node_v != NULL)
    {
        add_keyword_entry( psi_v->k_head, k_node_v->key);
        psi_v->key_n ++;
        
        k_node_v = k_node_v->next;
    }
    
    return ;
    
}

/*
 *	Find the keywords that occur in both @k_head2 and @k_head1.
 */
psi_t* key_intersection( k_node_t* k_head1, k_node_t* k_head2)
{
    k_node_t *k_node_v1, *k_node_v2;
    psi_t* psi_v;
    
    psi_v= alloc_psi();
    k_node_t* k_node_v=psi_v->k_head;
    
    k_node_v1 = k_head1->next;
    while( k_node_v1 != NULL)
    {
        //printf("k_node_v1->key: %.0lf \n",k_node_v1->key);
        
        k_node_v2 = k_head2->next;
        while( k_node_v2 != NULL)
        {
            //  printf("k_node_v2->key: %.0lf \n",k_node_v2->key);
            
            if( k_node_v2->key == k_node_v1->key)
            {
                add_keyword_entry( k_node_v, k_node_v2->key);
                psi_v->key_n ++;
                break;
            }
            k_node_v2 = k_node_v2->next;
        }
        k_node_v1 = k_node_v1->next;
    }
    
    return psi_v;
}


//==================================

/*
 *	Check whether a node @node_v is "relevant" or not.
 */
int is_relevant_node(node_t* node_v, psi_t* psi_v)
{
    int cnt=0;
    k_node_t* k_node_v;
    
    k_node_v = psi_v->k_head->next;
    while( k_node_v != NULL)
    {
        if (has_key_node( node_v, k_node_v->key))
            cnt++;
        
        k_node_v = k_node_v->next;
    }
    
    return cnt;
}


/*
 *	Check whether an object @obj_v is "relevant" to the query @q.
 *	That is, whether @obj_v contains a keyword in the query @q.
 */
int is_relevant_obj( obj_t* obj_v, psi_t* psi_v)
{
    int cnt=0;
    k_node_t* k_node_v;
    
    k_node_v = psi_v->k_head->next;
    while( k_node_v != NULL)
    {
        if( has_key_obj( obj_v, k_node_v->key))
            cnt++;
        
        k_node_v = k_node_v->next;
    }
    
    return cnt;
}


/*
 *	Check whether a node @node_v is "relevant" or not.
 */
int is_relevant_node(node_t* node_v, obj_t* obj_v)
{
    int cnt=0;
    k_node_t* k_node_v;
    
    k_node_v = obj_v->k_head->next;
    while( k_node_v != NULL)
    {
        if (has_key_node( node_v, k_node_v->key))
            cnt++;
        
        k_node_v = k_node_v->next;
    }
    
    return cnt;
}

/*
 *	return the list of keywords exist in both node_v and psi_v
 */
psi_t* node_intersection(node_t* node_v, psi_t* psi_v)
{
    psi_t* psi_rtn;
    k_node_t* k_node_v;
    B_KEY_TYPE key;
    
    psi_rtn = alloc_psi();
    k_node_v = psi_v->k_head->next;
    while( k_node_v != NULL)
    {
        key = k_node_v->key;
        if (has_key_node( node_v, key))
            add_psi_entry(psi_rtn, key);
        
        k_node_v = k_node_v->next;
    }
    
    return psi_rtn;
}


/*
 *	Count the number of keywords that occur in both @k_head2 and @k_head1.
 *
 */
int number_intersection( k_node_t* k_head1, k_node_t* k_head2)
{
    int count=0;
    k_node_t *k_node_v1, *k_node_v2;
    KEY_TYPE key;
    
    k_node_v1 = k_head1->next;
    while( k_node_v1 != NULL)
    {
        key = k_node_v1->key;
        k_node_v2 = k_head2->next;
        while( k_node_v2 != NULL)
        {
            if( k_node_v2->key == key)
            {
                count++;
                break;
            }
            k_node_v2 = k_node_v2->next;
        }
        k_node_v1 = k_node_v1->next;
    }
    
    return count;
}

/*
 *	Retrieve all the objects that contain keyword @key.
 *  IR-tree is used
 */
obj_set_t* retrieve_obj_key( KEY_TYPE key)
{
    obj_set_t* obj_set_v;
    b_heap_t* U;
    int rear_u, top_u, i;
    void* e;
    node_t* node_v;
    bst_node_t* bst_node_v;
    BIT_TYPE p_list;
    obj_t* obj_v;
    
    
    obj_set_v = alloc_obj_set( );
    
    U = alloc_b_heap( INI_HEAP_SIZE);
    
    rear_u = 1;
    U->obj_arr[ rear_u].element = ( void*)IRTree_v.root;
    U->obj_arr[ rear_u].e_tag = 1;
    U->obj_arr[ rear_u].key = 0;
    
    b_h_insert( U, rear_u++);
    while( !b_h_is_empty( U))
    {
        top_u = b_h_get_top( U);
        e = U->obj_arr[ top_u].element;
        
        if( U->obj_arr[ top_u].e_tag == 1)
        {
            //e is an node_t*.
            node_v = ( node_t*)e;
            
            
            bst_node_v = bst_search( node_v->bst_v, key);
            if( bst_node_v == NULL)
                continue;
            
            p_list = bst_node_v->p_list;
            for( i=0; i<node_v->num; i++)
            {
                //Check the c_key keyword.
                if( !get_k_bit( p_list, i))
                    continue;
                
                
                U->obj_arr[ rear_u].element = node_v->child[ i];
                if( node_v ->level > 0)
                {
                    //node_v is an inner-node.
                    U->obj_arr[ rear_u].e_tag = 1;
                    U->obj_arr[ rear_u].key = 0;
                }
                else
                {
                    //node_v is a leaf-node.
                    U->obj_arr[ rear_u].e_tag = 2;
                    U->obj_arr[ rear_u].key = 0;
                }
                
                //Enqueue.
                b_h_insert( U, rear_u++);
            }//
        }
        else
        {
            //e is an obj_t*.
            obj_v = ( obj_t*)e;
            add_obj_set_entry(obj_v, obj_set_v);
        }
        
    }
    
    release_b_heap( U);
    
    
    return obj_set_v;
}

//===========================================================================================
/*
 *	Circle range query.
 *
 *	DFS: recursive implementation.
 *
 *  1. objects are sorted by distance to loc_v, closest to farthest
 *  2. obj->dist storing the distance
 */
obj_set_t* range_query_sorted_dist( disk_t* disk_v, query_t* q, loc_t* loc_v)
{
    obj_set_t* obj_set_v;
    
    obj_set_v = alloc_obj_set( );
    range_query_sorted_dist_sub( IRTree_v.root, disk_v, obj_set_v, q, loc_v);
    
    return obj_set_v;
}


/*
 *	Range query on the sub-tree rooted at @node_v.
 *	@disk_v indicates the range which is a circle.
 *
 *	The results are stored in @obj_set_v.
 */
void range_query_sorted_dist_sub( node_t* node_v, disk_t* disk_v, obj_set_t* &obj_set_v, query_t* q, loc_t* loc_v)
{
    int i;
    BIT_TYPE p_list;
    range* MBR;
    
    if( node_v->parent == NULL)
        MBR = get_MBR_node( node_v, IRTree_v.dim);
    else
        MBR = node_v->parent->MBRs[ node_v->loc];
    
    //No overlapping.
    if( !is_overlap( MBR, disk_v))
        return;
    
    //Enclosed entrely.
    if( is_enclosed( MBR, disk_v))
    {
        retrieve_sub_tree_sorted_dist( node_v, obj_set_v, q, loc_v);
        if( node_v->parent == NULL)
        {
            free( MBR);
            
            /*s*/
            stat_v.memory_v -= IRTree_v.dim * sizeof( range);
            /*s*/
        }
        
        return;
    }
    
    //The remaining cases.
    if( node_v->level == 0)//node_v is a leaf-node.
    {
        ///for each object inside the leaf node
        for( i=0; i<node_v->num; i++)
        {
            ///if inside the disk and relevant, then add into obj_set_v
            if( is_enclosed( ( ( obj_t*)( node_v->child[ i]))->MBR, disk_v) &&
               is_relevant_obj( ( obj_t*)( node_v->child[ i]), q))
            {
                add_obj_set_entry_sorted( ( obj_t*)( node_v->child[ i]), obj_set_v, calc_minDist((( obj_t*)( node_v->child[ i]))->MBR, loc_v));
            }
        }
    }
    else//node_v is an inner-node.
    {
        ///retrieve a list of relevant childean
        p_list = is_relevant_node( node_v, q);
        
        ///recursive call for each child in the list
        for( i=0; i<node_v->num; i++)
        {
            if( get_k_bit( p_list, i))
                range_query_sorted_dist_sub( ( node_t*)( node_v->child[ i]), disk_v, obj_set_v, q, loc_v);
        }
    }
    
    if( node_v->parent == NULL)
    {
        free( MBR);
        /*s*/
        stat_v.memory_v -= IRTree_v.dim * sizeof( range);
        /*s*/
    }
}

/*
 *	Retrieve all the objects located at the sub-tree rooted at @node_v.
 *	The retrieved objects are stored in obj_set_v.
 */
void retrieve_sub_tree_sorted_dist( node_t* node_v, obj_set_t* &obj_set_v, query_t* q, loc_t* loc_v)
{
    int i;
    BIT_TYPE p_list;
    
    if( node_v->level == 0)
    {
        //node_v is a leaf-node.
        //Retrieve all its objects.
        for( i=0; i<node_v->num; i++)
        {
            if( is_relevant_obj( ( obj_t*)( node_v->child[ i]), q))
                add_obj_set_entry_sorted( ( obj_t*)( node_v->child[ i]), obj_set_v, calc_minDist((( obj_t*)( node_v->child[ i]))->MBR, loc_v));
        }
    }
    else
    {
        //node_v is an inner-node.
        //Invoke the function recursively.
        p_list = is_relevant_node( node_v, q);
        for( i=0; i<node_v->num; i++)
        {
            if( get_k_bit( p_list, i))
                retrieve_sub_tree_sorted_dist( ( node_t*)( node_v->child[ i]), obj_set_v, q, loc_v);
        }
    }
}


//===========================================================================================
//===========================================================================================
//===========================================================================================

/*
 *	Add an object entry @obj_v to @obj_set_v.
 *  the linked list is sorted by dist, from smallest to largest
 */
void add_obj_set_entry_sorted( obj_t* obj_v, obj_set_t* obj_set_v, B_KEY_TYPE dist)
{
    obj_node_t* obj_node_v, * obj_node_new;
    
    obj_node_v = obj_set_v->head;
    while (obj_node_v->next != NULL){
        if ( dist < obj_node_v->next->dist )
            break;
        obj_node_v = obj_node_v->next;
    }
    
    //obj_node_v pointing to the node before insert position
    //-----
    obj_node_new = ( obj_node_t*)malloc( sizeof( obj_node_t));
    memset( obj_node_new, 0, sizeof( obj_node_t));
    
    /*s*/
    stat_v.memory_v += sizeof( obj_node_t);
    if( stat_v.memory_v > stat_v.memory_max)
        stat_v.memory_max = stat_v.memory_v;
    /*s*/
    
    obj_node_new->obj_v = obj_v;
    obj_node_new->dist = dist;
    
    obj_node_new->next = obj_node_v->next;
    obj_node_v->next = obj_node_new;
    
    obj_set_v->obj_n++;
    //-----
    
    
}

