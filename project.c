#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

//Memory address
typedef unsigned long long int mem_addr_t;

typedef struct cache_line {
    int valid;
    mem_addr_t tag;
    int nru;
} cache_line_t;

typedef cache_line_t* cache_set_t;
typedef cache_set_t* cache_t;
cache_t cache;
int hit_count=0;
int miss_count=0;


void initCache(int* config)
{
    int i,j;

    cache = (cache_set_t*) malloc(sizeof(cache_set_t*)*config[2]);
    for (i=0; i<config[2]; i++)
	{
        cache[i]=(cache_line_t*) malloc(sizeof(cache_line_t)*config[3]);
        for (j=0; j<config[3]; j++)
		{
            cache[i][j].valid = 0;
            cache[i][j].tag = 0;
            cache[i][j].nru = 1;
            //printf("!!!%d!!!",cache[i][j].nru);
        }
    }
    /* Computes set index mask */
    //set_index_mask = (mem_addr_t) (pow(2, s) - 1);
}

void freeCache(int*config)
{	
	int i=0;
    for (;i<config[2];i++)
        free(cache[i]);
    free(cache);
}


char output[64][1000000];


void access_the_cache(char* addr,int* index_scheme,int* config,int num_index_scheme)
{
	char change_line[]="\n";
	char space[]=" ";
	char miss[]=" miss\n";
	char hit[]=" hit\n";
	
	char addr_arr[config[0]];
	strcpy(addr_arr,addr);
	//calculate the index and tag
	int	index_offset=(int)log(config[2])+1;
	int index=0;
	mem_addr_t tag=0;
	int i=index_scheme[1];
	int p=0;
	for(;i>=index_scheme[0];i--)
	{	
		index+=(int)(addr_arr[i]-'0')*pow(2,p++);
	}
	index=index%config[2];
	//printf("Index: %d  ",index);
	
	p=0;
	for(i=config[0]-index_offset;i>=0;i--)
	{
		//printf("%d",config[0]);
		if(i<index_scheme[0] || i>index_scheme[1])
		{
			tag+=(mem_addr_t)(addr_arr[i]-'0')*pow(2,p++);
		}	
	}
	//printf("Tag: %d ",tag);
	

	//start to access
	int which_block_to_pick;
	int hit_flag=0;
	for (i=0;i<config[3];i++)
    {	
        if (cache[index][i].valid && (cache[index][i].tag == tag))
        {
            ++hit_count;
            cache[index][i].nru=0;
            hit_flag = 1;
            //printf("Hit!\n");
            strcat(output[num_index_scheme],hit);
        }
    }
    if (!hit_flag)
    {   
		int if_all_zero=1;
		
        //printf("Miss!\n");
        strcat(output[num_index_scheme],miss);
        for (i=0;i<config[3];i++)
    	{
    		if (cache[index][i].nru==1)
    		{
    			which_block_to_pick=i;
    			if_all_zero=0;
    			break;
			}
    	}
    	if (if_all_zero)
    	{
    		for (i=0;i<config[3];i++)
    		{
	    		cache[index][i].nru=1;
			}
			which_block_to_pick=0;
    	}	
        ++miss_count;
        cache[index][which_block_to_pick].valid = 1;
        cache[index][which_block_to_pick].tag = tag;
        cache[index][which_block_to_pick].nru = 0;
    }
	return;
}

void cache_simulator(int* config,char** references,int num_of_references, char* output_file)
{	
	int i;
	int index_scheme[2];
	int	block_offset=(int)log(config[1])+1;
	int	index_offset=(int)log(config[2])+1;
	int the_least_miss_count=99999;
	int who_to_output;
	index_scheme[0]=config[0]-block_offset-index_offset;
	index_scheme[1]=config[0]-block_offset-1;
	
	char a[]="Offset bit count: ";
	char b[]="Indexing bit count: ";
	char c[]="Indexing bits: ";
	char d[]=".benchmark testcase1\n";
	char e[]=".end\n\n";
	char change_line[]="\n";
	char space[]=" ";
	
	char Offset_bit_count[5];
	sprintf(Offset_bit_count,"%d",block_offset);
	char Indexing_bit_count[5];
	sprintf(Indexing_bit_count,"%d",index_offset);

	char Indexing_bit1[5];
	int real_index_bit1=config[0]-index_scheme[0]-1;
	sprintf(Indexing_bit1,"%d",real_index_bit1);
	
	char Indexing_bit2[5];
	int real_index_bit2=config[0]-index_scheme[1]-1;
	sprintf(Indexing_bit2,"%d",real_index_bit2);
	char Indexing_bit_now[5];
	int index_loop=real_index_bit1;
	
	
	int num_index_scheme=0;
	//Try different indexing scheme
	while(index_scheme[0]>=0)
	{	
		strcat(output[num_index_scheme],a);
		strcat(output[num_index_scheme],Offset_bit_count);
		strcat(output[num_index_scheme],change_line);
		strcat(output[num_index_scheme],b);
		strcat(output[num_index_scheme],Indexing_bit_count);
		strcat(output[num_index_scheme],change_line);
		strcat(output[num_index_scheme],c);
		
		//printf("%d",index_scheme[0]);
		
		index_loop = real_index_bit1;
		for(;index_loop>=real_index_bit2;index_loop--)
		{
			sprintf(Indexing_bit_now,"%d",index_loop);
			//printf("Index: %s\n",Indexing_bit_now);
			strcat(output[num_index_scheme],Indexing_bit_now);
			strcat(output[num_index_scheme],space);
		}
//			strcat(output[num_index_scheme],Indexing_bit1);
//			strcat(output[num_index_scheme],space);
//			strcat(output[num_index_scheme],Indexing_bit2);

		
		strcat(output[num_index_scheme],change_line);
		strcat(output[num_index_scheme],change_line);
		strcat(output[num_index_scheme],d);
		
		miss_count=0;
		hit_count=0;
		//build up cache
		initCache(config);
		//Do num_of_references times to fulfill the request sequences
		for(i=0;i<num_of_references;i++)
		{
			strtok(references[i+1], "\n");
			strcat(output[num_index_scheme],references[i+1]);
			
			access_the_cache(references[i+1],index_scheme,config,num_index_scheme);
		}
		strcat(output[num_index_scheme],e);
		
		//decide who to output from output
		if (the_least_miss_count>miss_count)
		{
			the_least_miss_count=miss_count;
			who_to_output=num_index_scheme;
		}	
		
		char f[]="Total cache miss count: ";
		char ms_cnt[5];
		sprintf(ms_cnt,"%d",miss_count);
		strcat(output[num_index_scheme],f);
		strcat(output[num_index_scheme],ms_cnt);

		index_scheme[0]-=1;
		index_scheme[1]-=1;
		num_index_scheme++;
		
		sprintf(Indexing_bit1,"%d",++real_index_bit1);
		sprintf(Indexing_bit2,"%d",++real_index_bit2);
		//free cache
		freeCache(config);
	}
	strcat(output[who_to_output],change_line);
	//printf("%s",output[who_to_output]);
	
	
	FILE *fp;
	fp = fopen(output_file, "w");
	fputs(output[who_to_output], fp);
	fclose(fp);

	return;
}

int main(int argc, char *argv[]) {
	int i = 0;
	/* 
	//Read the arguments from terminal
	printf("We have %d arguments:\n", argc);
	
	for (; i < argc; ++i)
	{
		printf("[%d] %s\n", i, argv[i]);
	}
	*/
	char *input_file1=argv[1];
  	char *input_file2=argv[2];
  	char *output_file=argv[3]; 
	/*
	char *input_file1="cache1.org";
  	char *input_file2="reference1.lst";
  	char *output_file="index.rpt";
  	*/
	//get the config info
	i=0;
	int config[4];//pass to cache simulator
	char buffer[100];
	FILE *fp1 = fopen(input_file1, "r");                 
	while (fgets(buffer, sizeof(buffer), fp1)) {
		char *word = strrchr(buffer, ' ') + 1;
		config[i++]=atoi(word);
		//printf("%d",config[i-1]);
	}
	fclose(fp1);
	
	
	//get the sequences info
	char *input_sequences[80000];//pass to cache simulator
	memset(input_sequences, 0, 80000 * sizeof(char *));
	i=0;
	FILE *fp2 = fopen(input_file2, "r");                
	while (fgets(buffer, sizeof(buffer), fp2)) 
	{
		input_sequences[i] = (char*)malloc(strlen(buffer) + 1);
		strcpy(input_sequences[i], buffer);
		//Print out what's been loaded in
		//printf("%s",input_sequences[i]);
		i++;
	}
	int number_of_sequences=i-3;//pass to cache simulator
	fclose(fp2);
	/*---------------------------------------------------------------------------------------*/
	//declare a dynamic 2D char array for output
	/*
	char **output = (char**)malloc(sizeof(char*)*60);
	int q = 0;
	for(; q < 60; q++) {
	    output[i] = (char*)malloc(100000*sizeof(char));
	}
	*/
	
	char a[]="Address bits: ";
	char b[]="Block size: ";
	char c[]="Cache sets: ";
	char d[]="Associativity: ";
	
	char Address_bits[5];
	sprintf(Address_bits,"%d",config[0]);
	char Block_size[5];
	sprintf(Block_size,"%d",config[1]);
	char Cache_sets[5];
	sprintf(Cache_sets,"%d",config[2]);
	char Associativity[5];
	sprintf(Associativity,"%d",config[3]);
	char change_line[]="\n";
	
	for(i=0;i<64;i++)
	{
		strcat(output[i],a);
		strcat(output[i],Address_bits);
		strcat(output[i],change_line);
		strcat(output[i],b);
		strcat(output[i],Block_size);
		strcat(output[i],change_line);
		strcat(output[i],c);
		strcat(output[i],Cache_sets);
		strcat(output[i],change_line);
		strcat(output[i],d);
		strcat(output[i],Associativity);
		strcat(output[i],change_line);
		strcat(output[i],change_line);
	}
	cache_simulator(config, input_sequences, number_of_sequences, output_file);

	/*---------------------------------------------------------------------------------------*/
	/*
	for(i = 0; i < 60; i++) {
    	free(output[i]);
	}
	free(output);
	*/
	
	return 0;
}
