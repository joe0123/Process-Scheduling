#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define FIFO	0
#define RR	1
#define SJF	2
#define PSJF	3

#define	P_CPU	0
#define C_CPU	1

#define TQ 500

typedef struct Process{
	char name[32];
	int ready_time;
	int exec_time;
	int pid;
	int start;
	int end;
}Process;

int match_policy(char* policy){
	if(strcmp(policy, "FIFO") == 0)
		return FIFO;
	else if(strcmp(policy, "RR") == 0)
		return RR;
	else if(strcmp(policy, "SJF") == 0)
		return SJF;
	else if(strcmp(policy, "PSJF") == 0)
		return PSJF;
	else
		return -1;
}


int cmp_FIFO(const void* a, const void* b){
	return ((Process *)a)->ready_time - ((Process *)b)->ready_time;
}

int cmp_end(const void* a, const void* b){
	return ((Process *)a)->end - ((Process *)b)->end;
}

static inline int swap_queue(int* queue, int queue_num){
	int tmp[queue_num];
	for(int i = 0; i < queue_num; i++)
		tmp[i] = queue[i];
	queue[queue_num - 1] = queue[0];
	for(int i = 1; i < queue_num; i++)
		queue[i - 1] = tmp[i];
}

static inline int decide_proc(int policy, int N, Process* procs, int last_id, int* rr, int* rr_queue, int rr_num){
	int curr_id = last_id;	// default: last runner is the next runner

	// (curr_id == -1 || (curr_id != -1 && procs[curr_id].pid == -1)) means our curr_id is now absent.
	switch(policy){
		case FIFO:
			if(curr_id == -1 || (curr_id != -1 && procs[curr_id].pid == -1)){
				int i = curr_id + 1;
				curr_id = -1;
				for(; i < N; i++)
					// Find the nearest one behind curr_id thanks to qsort
					if(procs[i].pid != -1){
						curr_id = i;
						break;
					}
			}
			break;
		case RR:
			if(curr_id == -1 || (curr_id != -1 && procs[curr_id].pid == -1) || *rr == 0){
				if(rr_num <= 0)
					curr_id = -1;
				else{
					if(curr_id != -1 && procs[curr_id].pid != -1 && *rr == 0)
						swap_queue(rr_queue, rr_num);
					curr_id = rr_queue[0];
				}
				*rr = TQ;
			}
			break;
		case SJF:
			if(curr_id == -1 || (curr_id != -1 && procs[curr_id].pid == -1)){
				curr_id = -1;
				for(int i = 0; i < N; i++){
					if(procs[i].pid == -1)
						continue;
					// Find one with shortest exec_time in the whole array
					if(curr_id == -1 || (curr_id != -1 && procs[i].exec_time < procs[curr_id].exec_time))
						curr_id = i;
				}
			}
			break;
		case PSJF:
			if(curr_id == -1 || (curr_id != -1 && procs[curr_id].pid == -1))
				curr_id = -1;
			// Find one with shortest exec_time in the whole array regardless of curr_id
			for(int i = 0; i < N; i++){
				if(procs[i].pid == -1)
					continue;
				if(curr_id == -1 || (curr_id != -1 && procs[i].exec_time < procs[curr_id].exec_time))
					curr_id = i;
			}
			break;
	}
	return curr_id;
}


int scheduling(int policy, int N, Process *procs){
/* Sort the processes, key1=ready_time, key2=exec_time */
	qsort(procs, N, sizeof(Process), cmp_FIFO);

/* Start */
	int last_id = -1;
	int curr_time = 0;
	int rr = TQ;
	int rr_queue[N];
	int rr_num = 0;
	int done_count = 0;
	while(1){
	/* Fork the process who's ready */
		for(int i = last_id + 1; i < N; i++)
			if(procs[i].ready_time == curr_time){
				procs[i].pid = 1;
				if(policy == RR){
					rr_queue[rr_num] = i;
					rr_num++;
				}
			}
	/* Determine who's next */
		int curr_id = decide_proc(policy, N, procs, last_id, &rr, rr_queue, rr_num);
#ifdef DEBUG_CURR
		printf("time = %d, curr_proc = %s\n", curr_time, procs[curr_id].name);
		fflush(stdout);
#endif
	
	/* Context Switch */
		if(curr_id != last_id)
			if(curr_id != -1 && procs[curr_id].pid != -1 && procs[curr_id].start == -1)
				procs[curr_id].start = curr_time;
	/* Run time */
		curr_time += 1;
		if(curr_id != -1){
			procs[curr_id].exec_time--;
			rr--;
		}

	/* Wait for done process if exists */
		if(curr_id != -1 && procs[curr_id].exec_time <= 0){
			procs[curr_id].pid = -1;
			procs[curr_id].end = curr_time;
			if(policy == RR){
				swap_queue(rr_queue, rr_num);
				rr_num--;
			}
#ifdef DEBUG_DONE
			printf("Process %s is done at %d\n", procs[curr_id].name, curr_time);
			fflush(stdout);
#endif
			done_count += 1;
		/* End of scheduling */
			if(done_count == N)
				break;
		}
		if(curr_id != -1)
			last_id = curr_id;
	}
}

int main(){
	char tmp[10];
	scanf("%s", tmp);
	int policy = match_policy(tmp);
	assert(policy != -1);
	
	int N;
	scanf("%d", &N);
	
	Process *procs = (Process *)malloc(N * sizeof(Process));
	for(int i = 0; i < N; i++){
		scanf("%s%d%d", procs[i].name, &procs[i].ready_time, &procs[i].exec_time);
		procs[i].pid = -1;
		procs[i].start = -1;
	}

	scheduling(policy, N, procs);
	qsort(procs, N, sizeof(Process), cmp_end);
	for(int i = 0; i < N; i++)
		printf("%s %d %d\n", procs[i].name, procs[i].start, procs[i].end);
	fflush(stdout);
}
