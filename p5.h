// ******************************************************************************************************************
//  CPU Scheduler
//  Copyright(C) 2018  James LoForti
//  Contact Info: jamesloforti@gmail.com
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.If not, see<https://www.gnu.org/licenses/>.
//									     ____.           .____             _____  _______   
//									    |    |           |    |    ____   /  |  | \   _  \  
//									    |    |   ______  |    |   /  _ \ /   |  |_/  /_\  \ 
//									/\__|    |  /_____/  |    |__(  <_> )    ^   /\  \_/   \
//									\________|           |_______ \____/\____   |  \_____  /
//									                             \/          |__|        \/ 
//
// ******************************************************************************************************************
//

#define MAX_PROCESSES 25
#define QUANTUM 100
#define MAX_TIME 100000

typedef struct process
{
	int pid;
	int arrivalTime;
	int burstTime;
	int flag;
	int startTime;
	int latestStartTime;
	int endTime;
	int responseTime;
	int waitTime;
	int beginWaiting;
	int turnTime;
	int nextArriving;
	int remainingQuantum;
}Process;

typedef struct processShell
{
	struct process* core;
	struct processShell* next;
}ProcessShell;

typedef struct processq
{
	int size;
	struct processShell* front;
	struct processShell* back;
}ProcessQ;

typedef struct node
{
	struct node* next;
	struct process* data;
}Node;

typedef struct list
{
	struct node* first;
	struct node* last;
	int count;
}List;

//MISC
void read_raw_data();
void init_all();
void calc_times_and_print(char* algorithmType);
void print(char* algorithmType, double responseTime, double turnTime, double waitTime);

//FIRST COME, FIRST SERVE
void fcfs();
void next_process();
void update();

//SHORTEST JOB FIRST
void sjf();
void next_process_sjf();
void update_sjf();
void sort_by_burst_sjf();
void sort_by_arrival_sjf();

//SHORTEST REMAINING TIME FIRST
void srtf();
int next_event();
void add_arrivals();
void fork_srtf();
void running_to_waiting(); // shared with rr
void waiting_to_running();
void runner_complete(); // shared with rr
int process_interrupt();
int query_next_arrival(); // shared with rr
void force_start(); // shared with rr
void update_srtf();
void send_to_waiting(Process* process); // shared with all
void sort_list_burst();

//ROUND ROBIN
void rr();
int next_event_rr();
void add_arrivals_rr();
void fork_rr();
int process_interrupt_rr();
int get_remaining_burst();
int get_relative_start();
void waiting_to_running_rr();

//LIST
void list_constructor(List* self);
void list_param_constructor(List* self, Node* _first, Node* _last, int _count);
void list_destructor(List* self);
void push_front(List* self, Node* newNode);
Node* pop_front(List* self);
void push_back(List* self, Node* newNode);
Node* pop_back(List* self);
int duplicate(List* self, Node* newNode);

//NODE
void node_constructor(Node* self);
void node_param_constructor(Node* self, Process* _data);
void node_destructor(Node* self);
Node* get_next(Node* self);
Process* get_data(Node* self);
void set_next(Node* self, Node* node);
void set_data(Node* self, Process* process);