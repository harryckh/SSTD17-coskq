//
//  Author: Harry Kai-Ho Chan
//  Email: khchanak@cse.ust.hk
//

#include "maxdotsize.h"
#include "cao_alg_new.h"
#include <iostream>
#include <fstream>

using namespace std;

#define QUERY_SIZE		50
//#define	RATIO_THRESHOLD	1.3

#define QUERY_LOG		"query_log.txt"


IRTree_t		IRTree_v;
coskq_stat_t	stat_v;

int				cost_tag;
///
data_t* data_v;
bool firstTime = true;
unordered_map<KEY_TYPE, KEY_TYPE>* keyfreq_hashmap;

//memory.
float		first_memory_v;
float		first_memory_max;

float		first_tree_memory_v;
float		first_tree_memory_max;

void coskq( int ratio_tag);


//void collect_ratio_stat( );

int main(int argc, char* argv[])
{
    
    coskq( 0);
    
	return 0;
}


/*
 *	The interface for the CoSKQ query.
 *
 *  ///@cost_tag:
 =   3: Without inherent cost
 =   13: With inherent cost
 *	@alg_opt:
 *		=	10: MaxDotSize-Exact;
 *		=	11: MaxDotSize-Appro;
 *		=	2: Long-Appro;
 *		=	3: Cao-Exact;
 *		=	4: Cao-Appro1;
 *      =   52: Cao-Appro2;
 *		=	9: Cao-Appro3;
 *
 *  @ratio_tag:
 *      =   1: compute the approximate ratio
 */
void coskq( int ratio_tag)
{
	int i, j, size_sum;
	B_KEY_TYPE cost, cost_opt, cost_sum, ratio_sum;
	B_KEY_TYPE* ratio;
	coskq_config_t* cfg;
	query_t** q_set;
	range* MBR;
	obj_set_t* S, *S_opt;
	FILE *r_fp;
  
	memset( &stat_v, 0, sizeof( coskq_stat_t));
	
	stat_v.ratio_min = 3;
	stat_v.ratio_max = 1;
    size_sum=0;
	//Read the cofig.
    ///read the config.txt, return coskq_config_t pointer
    printf( "Reading configuration ...\n");
	cfg = read_config_coskq( );

	cfg->prune_opt = 0;
    
	cost_tag = cfg->cost_measure;
    if( cost_tag == 3)
        printf( "The MaxDotSize measurement (without inherent-cost):\n");
    else
        printf( "The MaxDotSize measurement (with inherent-cost):\n");

	//Read the data.
    printf( "Reading data ...\n");
    keyfreq_hashmap = new unordered_map<KEY_TYPE, KEY_TYPE>();
    data_v = read_data_coskq( cfg, keyfreq_hashmap);
//   printf("#obj:%d\n",data_v->obj_n);
    
#ifndef WIN32
	float sys_t, usr_t, usr_t_sum = 0;
	struct rusage IR_tree_sta, IR_tree_end;
    
	GetCurTime( &IR_tree_sta);
#endif
    //Option 1: Build the tree from scratch.
    //Build the IR-tree.
    if( cfg->tree_tag == 0)
    {
        printf( "Building IR-tree ...\n");
        build_IRTree( data_v);
        
        print_and_check_tree( 1, cfg->tree_file);
        //check_IF( );
    }
    else
    {
        //Option 2: Read an existing tree.
        printf( "Reading IR-Tree ...\n");
        read_tree( cfg->tree_file);
    }
    
#ifndef WIN32
	GetCurTime( &IR_tree_end);
	GetTime( &IR_tree_sta, &IR_tree_end, &stat_v.irtree_build_time, &sys_t);
#endif
   //Get the whole range.
	MBR = get_MBR_node( IRTree_v.root, IRTree_v.dim);
    
	//Generate the set of querys.
	printf( "Generating queries ...\n");
	q_set = gen_query_set2( cfg->q_set_size, cfg->q_key_n, MBR, data_v, cfg->low, cfg->high);
	if( q_set == NULL)
	{
		printf( "Query generation failed!\n");
		exit( 0);
	}
    
	//Query.
    printf( "Performing Queries ...\n");
    if (cfg->alg_opt == 10)
        printf( "MaxDotSize-Exact:\n");
    else if (cfg->alg_opt == 11)
        printf( "MaxDotSize-Appro:\n");
    else if( cfg->alg_opt == 1)
        printf( "Long-Exact:\n");
    else if( cfg->alg_opt == 2)
        printf( "Long-Appro:\n");
    else if( cfg->alg_opt == 3)
        printf( "Cao-Exact:\n");
    else if( cfg->alg_opt == 4)
        printf( "Cao-Appro1:\n");
    else if(cfg->alg_opt == 52)
        printf( "Cao-Appro2:\n");
    else if (cfg->alg_opt==9)
        printf( "Cao-Appro3:\n");
    
	ratio = ( double*)malloc( cfg->q_set_size * sizeof( double));
	memset( ratio, 0, cfg->q_set_size * sizeof( double));
	
	ratio_sum = 0;
	cost_sum = 0;
    
	for( i=0; i<cfg->q_set_size; i++)
	{
		printf( "Query #%i ...\n", i+1);
        
#ifndef WIN32
        struct rusage query_sta, query_end;
        
        GetCurTime( &query_sta);
#endif
		S_opt = NULL;
        
        if (cfg->alg_opt == 10) ///MaxDotSize-Exact
        {
            S = MaxDotSize(q_set[i], 1, cost_tag);
        }
        else if (cfg->alg_opt == 11) ///MaxDotSize-Appro
        {
            S = MaxDotSize(q_set[i], 2, cost_tag);
        }
        else if( cfg->alg_opt == 2)///Long-Appro
        {
            S = CostEnum_Appro( q_set[ i]);
        }
        else if( cfg->alg_opt == 3)///Cao-Exact
        {
            S = Cao_Exact( q_set[ i],query_sta);
        }
        else if( cfg->alg_opt == 4)///Cao-Appro1
        {
            S = Cao_Appro1( q_set[ i]);
            if( ratio_tag == 1)
                S_opt = MaxDotSize(q_set[i], 1, cost_tag);
        }
        else if( cfg->alg_opt == 5) ///Cao-Appro2
        {
            S = Cao_Appro2( q_set[ i]);
            if( ratio_tag == 1)
                S_opt = MaxDotSize(q_set[i], 1, cost_tag);
        }
        else if (cfg->alg_opt == 9)///Cao-Appro3
        {
            S = Sum_Appro(q_set[i]);
            if( ratio_tag == 1)
                S_opt = MaxDotSize(q_set[i], 1, cost_tag);
        }
        else if (cfg->alg_opt == 52)  //Cao-Appro-new
        {
            S = Cao_Appro2_new(q_set[i], keyfreq_hashmap);
            if( ratio_tag == 1)
                S_opt = MaxDotSize(q_set[i], 1, cost_tag);
        }
       
#ifndef WIN32
        GetCurTime( &query_end);
        
        GetTime( &query_sta, &query_end, &usr_t, &sys_t);
        usr_t_sum += usr_t;
#endif
    	cost = comp_cost(cost_tag, S, q_set[ i]);
		cost_sum += cost;
        size_sum += S->obj_n;
        if( S_opt != NULL)
		{
          
			//Ratio processing.
			cost_opt = comp_cost(cost_tag, S_opt, q_set[ i]);
            
			ratio[ i] = cost / cost_opt;
			if( ratio[ i] < stat_v.ratio_min)
				stat_v.ratio_min = ( float)ratio[ i];
			if( ratio[ i] > stat_v.ratio_max)
				stat_v.ratio_max = ( float)ratio[ i];
            
			ratio_sum += ( float)ratio[ i];
			stat_v.ratio_aver = ( float)ratio_sum / ( i+1);
            
			//Calculate the deviation.
			for( j=0; j<=i; j++)
				stat_v.ratio_dev += ( float)pow( ( ratio[ j] - stat_v.ratio_aver), 2);
			stat_v.ratio_dev /= i + 1;
			stat_v.ratio_dev = ( float)sqrt( stat_v.ratio_dev);
		}
        
		//Print the accumulated query results.
		if( i == 0)
		{
            if( ( r_fp = fopen( COSKQ_RESULT_FILE, "w")) == NULL)
			{
				fprintf( stderr, "Cannot open the coskq_result file.\n");
				exit( 0);
                
			}
            
		}
		else
		{   if( ( r_fp = fopen( COSKQ_RESULT_FILE, "a")) == NULL)
			{
				fprintf( stderr, "Cannot open the coskq_result file.\n");
				exit( 0);
			}
		}
        
		fprintf( r_fp, "Query #%i:\n", i+1);
		fprintf( r_fp, "Keywords: ");
		print_k_list( q_set[ i]->psi_v->k_head, r_fp);
        ///
        fprintf( r_fp, "Locations: ");
		print_loc( q_set[ i]->loc_v, r_fp);
        ///
        
		//Print the query result.
		print_obj_set( S, r_fp);
		fprintf( r_fp, "Cost: %0.4lf\n", cost);
		printf( "Cost: %0.4lf\n", cost);
#ifndef WIN32
		fprintf( r_fp, "Time: %f\n", usr_t);
		printf( "Time: %f\n", usr_t);
		fprintf( r_fp, "Ratio: %lf\n\n", ratio[ i]);
		printf( "Ratio: %lf\n\n", ratio[ i]);
#endif
        
		fclose( r_fp);
        
		//Print the statistics.
		stat_v.aver_cost = cost_sum / ( i+1);
        stat_v.aver_size = (float)size_sum / (float)( i+1);
        
#ifndef WIN32
		stat_v.q_time = usr_t_sum / ( i+1);
#endif
		print_coskq_stat( cfg, i+1);
		
		release_obj_set( S);
		if( S_opt != NULL)
			release_obj_set( S_opt);
        
    }
    
	//Release the resources.
	for( i=0; i<cfg->q_set_size; i++)
		release_query( q_set[ i]);
	free( q_set);
	free( MBR);
	//printf("Memory balance: %f\n", stat_v.memory_v / cfg->q_set_size);
	free( cfg);
}


