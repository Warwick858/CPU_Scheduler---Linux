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

//Project Purpose:
//To emulate a CPU scheduler, using the following algorithms:
//First come first serve, shortest job first, shortest remaining time first, and round robin

#include <stdlib.h> 
#include <stdio.h> // needed for printf()
#include <string.h> // needed for memcpy()
#include "p5.h"

//Declare globals
int rawData[MAX_PROCESSES * 2]; // 25 processes * 2 values each
Process processes[MAX_PROCESSES];
Process pendingSJF[MAX_PROCESSES];
List waitingList;
int running;
int clock;
int numProcesses;
int processesRemaining;
int countSJF;

int main(int argc, char* argv[])
{
	//Print opening seperator, name
	printf("\n*********************************************** "
			"\nName: James LoForti \n\n");

	//Read in the raw data and create an array of processes
	read_raw_data();

	//Initialize globals
	init_all();

	//Copy array of processes so original is uneffected
	Process processesCopy[MAX_PROCESSES];
	memcpy(&processesCopy, &processes, sizeof(processes));

	//Exercise all 4 algorithms
	fcfs();

	init_all();
	memcpy(&processes, &processesCopy, sizeof(processes));

	sjf();

	init_all();
	memcpy(&processes, &processesCopy, sizeof(processes));

	srtf();

	init_all();
	memcpy(&processes, &processesCopy, sizeof(processes));

	rr();

	//Print closing seperator
	printf("\n*********************************************** \n");

	getchar();

	return 0;
} // end main()

void read_raw_data()
{
	//Function vars:
	int rawCount = 0;
	int count = 0;

	//Read process arrival times & burst times from stdin
	int num;
	while (fscanf(stdin, "%d", &num) == 1)
	{
		//Save the data to an array
		rawData[rawCount++] = num;
	} // end while

	//Save the total number of processes
	numProcesses = (rawCount / 2);

	//For every set of raw data
	int i;
	for (i = 0; i < numProcesses; i++)
	{
		//Even = Arrival / Odd = Burst
		processes[i].arrivalTime = rawData[i * 2];
		processes[i].burstTime = rawData[(i * 2) + 1];
		processes[i].pid = i;

		//If this isn't the last process
		if ((i * 2) + 2 < rawCount)
		{
			//Save next processes arrival time
			processes[i].nextArriving = rawData[(i * 2) + 2];
		}
	} // end for
} // end function read_raw_data

//***************************************************************************FIRST COME FIRST SERVE

void fcfs()
{
	//Initialize runner
	running = processes[0].pid;
	processes[0].startTime = processes[0].arrivalTime;
	clock = processes[0].startTime;

	//While processes remain to be executed
	while (processesRemaining)
	{
		next_process();
		update();
	}

	//Calculate avg times and print to console
	calc_times_and_print("First Come, First Serve");
} // end function fcfs()

void next_process()
{
	//For every process (except for the 1st)
	int i;
	for (i = 1; i < numProcesses; i++)
	{
		//If the process isn't dead
		if (processes[i].flag != -1)
		{
			//If the process has arrived
			if (processes[i].arrivalTime <= clock)
			{
				//Add it to the list
				send_to_waiting(&processes[i]);

				//Remove process and decrement count
				processes[i].flag = -1;
				processesRemaining--;
			}
		} // end for
	} // end for
} // end function next_event()

void update()
{
	//If there's processes waiting
	if (waitingList.count)
	{
		//Save the count since we'll loose it later
		int waitingCount = waitingList.count;

		//For every process waiting
		int i;
		for (i = 0; i < waitingCount; i++)
		{
			//Save the next process
			Node* next = pop_front(&waitingList);

			//Save as runner
			running = next->data->pid;
			processes[running].startTime = clock;

			//If the now running process had to wait
			if (processes[running].startTime > processes[running].arrivalTime)
			{
				//Calc and add wait time to total process wait time
				int currWait = (processes[running].startTime - processes[running].arrivalTime);
				processes[running].waitTime += currWait;
			}

			//Set clock to next event (current running process's end time)
			clock = (processes[running].startTime + processes[running].burstTime);
			processes[running].endTime = clock; // finish the current running process
		} // end for
	} // end if

	//Set clock to next event (current running process's end time)
	clock = (processes[running].startTime + processes[running].burstTime);
	processes[running].endTime = clock; // finish the current running process

	//If the next process hasn't arrived
	if (processes[running + 1].arrivalTime > clock)
	{
		//Set the clock next process start time
		clock = processes[running + 1].arrivalTime; 
	}
} // end function update()

//***************************************************************************SHORTEST JOB FIRST

void sjf()
{
	//Initialize runner
	running = processes[0].pid;
	processes[0].flag = -1;
	processes[0].startTime = processes[0].arrivalTime;
	clock = processes[0].startTime;
	
	//While processes remain to be executed
	while (processesRemaining)
	{
		next_process_sjf();
		update_sjf();
	}

	//Calculate avg times and print to console
	calc_times_and_print("Shortest Job First");
} // end function sjf()

void next_process_sjf()
{
	//For every element in the sjfPending array
	int i;
	for (i = 0; i < numProcesses; i++)
	{
		//Clear it out
		memset(&pendingSJF[i], 0, sizeof(Process));
	}
	countSJF = 0;

	//For every process (except for the 1st)
	for (i = 1; i < numProcesses; i++)
	{
		//If this process has not already ended
		if (processes[i].flag != -1)
		{
			//If this process has arrived
			if (processes[i].arrivalTime <= clock)
			{
				//Add potential next process to array
				pendingSJF[countSJF++] = processes[i];
			} // end if
		} // end for
	} // end for
} // end function next_event_sjf()

void update_sjf()
{
	//If there's a process that's ready
	if (countSJF > 0)
	{
		//Sort processes that have arrived by burst time
		sort_by_burst_sjf();

		//Save next process as running, set its start time, mark as run, decrement count
		running = pendingSJF[0].pid;
		processes[running].startTime = clock;
		processes[running].flag = -1;
		processesRemaining--;

		//If the now running process had to wait
		if (processes[running].startTime > processes[running].arrivalTime)
		{
			//Calc and add wait time to total process wait time
			int currWait = (processes[running].startTime - processes[running].arrivalTime);
			processes[running].waitTime += currWait;
		} // end if

		//Set clock to next event (current running process's end time)
		clock = (processes[running].startTime + processes[running].burstTime);
		processes[running].endTime = clock; // finish the current running process

	} // end if
	else // no processes ready
	{
		//Set clock to next event (current running process's end time)
		clock = (processes[running].startTime + processes[running].burstTime);
		processes[running].endTime = clock; // finish the current running process
	} // end else

	//If no processes have arrived
	if (countSJF == 0 && running != 0)
	{
		//Find the next arriving process
		sort_by_arrival_sjf();

		int i;
		for (i = 0; i < numProcesses; i++)
		{
			//If that process hasn't already run
			if (processes[i].flag != -1)
			{
				//Set the clock to the that processes start time
				clock = processes[i].arrivalTime;
				break;
			}
		} // end for
	} // end if
} // end function update_sjf()

void sort_by_burst_sjf()
{
	//for-loop for outer values
	int j;
	for (j = 0; j < countSJF; j++)
	{
		//for-loop for all inner values
		int i;
		for (i = 0; i < countSJF; i++)
		{
			//Skip comparison test if uninitialized values are encountered
			if (pendingSJF[i].burstTime != 0 && pendingSJF[i + 1].burstTime != 0)
			{
				//If the next burst is lower than the current burst
				if (pendingSJF[i].burstTime > pendingSJF[i + 1].burstTime)
				{
					//Swap the processes
					Process temp = pendingSJF[i];
					pendingSJF[i] = pendingSJF[i + 1];
					pendingSJF[i + 1] = temp;
				}
			} // end if
		} // end for
	} // end for
} // end function sort_by_burst_sjf()

void sort_by_arrival_sjf()
{
	//for-loop for outer values
	int j;
	for (j = 0; j < countSJF; j++)
	{
		//for-loop for all inner values
		int i;
		for (i = 0; i < countSJF; i++)
		{
			//Skip comparison test if uninitialized values are encountered
			if (processes[i].arrivalTime != 0 && processes[i + 1].arrivalTime != 0)
			{
				//If the next burst is lower than the current burst
				if (processes[i].arrivalTime > processes[i + 1].arrivalTime)
				{
					//Swap the processes
					Process temp = processes[i];
					processes[i] = processes[i + 1];
					processes[i + 1] = temp;
				}
			} // end if
		} // end for
	} // end for
} // end function sort_by_arrival_sjf()

//***************************************************************************SHORTEST REMAINING TIME FIRST

void srtf()
{
	//Initialize runner
	running = processes[0].pid;
	processes[0].startTime = processes[0].arrivalTime;
	clock = processes[0].startTime;

	//While processes remain to be executed
	while (processesRemaining)
	{
		//If next event is an arrival
		if (next_event()) // sets clock
		{
			add_arrivals();
		}

		//Update wait times
		update_srtf();

		//If runner could change
		if (waitingList.first != NULL)
		{
			fork_srtf(); // switches runner, begins waiting
		}
		else // end runner
		{
			runner_complete();
			force_start();
		}
	} // end while

	//Close out the last process
	clock += processes[running].burstTime;
	processes[running].endTime = clock;

	//Calculate avg times and print to console
	calc_times_and_print("Shortest Remaining Time First");
} // end function srtf()

int next_event()
{
	//Get next arrival time
	int nextArriving = query_next_arrival();

	//If last process already arrived
	if (!nextArriving)
	{
		//Force next event to always be runner's end time
		nextArriving = MAX_TIME;
	}

	//If runner is alive and will end before new arrival
	if (!processes[running].flag && ((clock + processes[running].burstTime) < nextArriving))
	{
		//Set clock to runner's end time
		clock += processes[running].burstTime;
		return 0;
	}
	else // new arrival occurs earlier
	{
		//Set clock to next process's arrival time
		clock = nextArriving;
		return 1;
	}
} // end function next_event()

void add_arrivals()
{
	//For every process
	int i;
	for (i = 0; i < numProcesses; i++)
	{
		//If this process isn't the one currently running
		if (running != i)
		{
			//If this process has not already ended
			if (processes[i].flag != -1)
			{
				//If this process has arrived
				if (processes[i].arrivalTime <= clock)
				{
					//Add it to the waiting list
					send_to_waiting(&processes[i]);
				} // end if
			} // end for
		} // end if
	} // end for

	//Sort the waiting list by process burst time
	sort_list_burst();
} // end function add_arrivals()

void fork_srtf()
{
	//If the runner has to wait
	if (process_interrupt())
	{
		running_to_waiting();
		waiting_to_running();
	}

	//For every process waiting
	Node* next = waitingList.first;
	while (next != NULL)
	{
		//Start their wait time
		next->data->beginWaiting = clock;
		next = next->next;
	}
} // end function fork_srtf()

void running_to_waiting()
{
	//If runner is NOT finished
	if (!processes[running].flag)
	{
		//Save runner's wait time and add runner to the waiting list
		processes[running].beginWaiting = clock;
		send_to_waiting(&processes[running]);
	}
} // end function running_to_waiting()

void waiting_to_running()
{
	//Save new runner, set its start time, and remove it from the list
	running = waitingList.first->data->pid;

	//If runner hasn't already started
	if (!processes[running].startTime)
	{
		processes[running].startTime = clock;
	}
	else // process has been waiting
	{
		processes[running].latestStartTime = clock;
	}
	
	pop_front(&waitingList);
} // end function waiting_to_running()

int process_interrupt()
{
	int currBurst = 0;
	int remainingBurst = 0;

	//If runner never had to wait, use startTime
	if (!processes[running].latestStartTime)
	{
		//For the amount of burst time the runner has completed
		currBurst = (clock - processes[running].startTime);
	}
	else // runner waited, use latestStartTime
	{
		//For the amount of burst time the runner has completed
		currBurst = (clock - processes[running].latestStartTime);
	}

	//Subtract it from runner's total burst time
	remainingBurst = (processes[running].burstTime - currBurst);

	//If the runner is NOT finished
	if (remainingBurst)
	{
		//Save runner's remaining burst time
		processes[running].burstTime = remainingBurst;

		//If the runner needs less time than the new comer
		if (remainingBurst < waitingList.first->data->burstTime)
		{
			//Runner continues to run
			processes[running].latestStartTime = clock;
			return 0;
		}
		else // new comer needs less time
		{
			//New process takes its place
			return 1;
		}
	} // end if
	else // runner is done
	{
		runner_complete();
		return 1;
	}
} // end function process_interrupt()

void runner_complete()
{
	//End running process
	processes[running].endTime = clock;
	processes[running].flag = -1;
	processesRemaining--;
} // end function runner_complete()

int query_next_arrival()
{
	//For every process after the runner
	int i;
	for (i = (processes[running].pid + 1); i < numProcesses; i++)
	{
		//If the processes isn't dead
		if (!processes[i].flag && !processes[i].beginWaiting)
		{
			return processes[i].arrivalTime;
		}
	} // end for

	return 0;
} // end function quer_next_arrival()

void force_start()
{
	//For every process after the runner
	int i;
	for (i = (processes[running].pid + 1); i < numProcesses; i++)
	{
		//If the processes isn't dead
		if (!processes[i].flag)
		{
			running = processes[i].pid;
			processes[running].startTime = processes[running].arrivalTime;
			clock = processes[running].startTime;
			return;
		}
	} // end for
} // end function force_start()

void update_srtf()
{
	//Sum up wait times for all processes
	int i;
	for (i = 0; i < numProcesses; i++)
	{
		if (processes[i].flag != -1)
		{
			if (processes[i].beginWaiting != 0)
			{
				processes[i].waitTime += (clock - processes[i].beginWaiting);
				processes[i].beginWaiting = 0;
			}
		} // end if
	} // end for
} // end function update_srtf()

void send_to_waiting(Process* process)
{
	Node* node = malloc(sizeof(Node));
	node_param_constructor(node, process);
	push_back(&waitingList, node);
} // end function send_to_waiting()

void sort_list_burst()
{
	int killSwitch;
	Node* rPtr = NULL;
	Node* lPtr = NULL;

	//If list is empty
	if (waitingList.count)
	{
		do
		{
			killSwitch = 0;
			rPtr = waitingList.first;

			while (rPtr->next != lPtr)
			{
				//If bigger comes before smaller
				if (rPtr->data->burstTime > rPtr->next->data->burstTime)
				{
					//Swap
					Process* temp = rPtr->data;
					rPtr->data = rPtr->next->data;
					rPtr->next->data = temp;

					killSwitch = 1;
				} // end if

				rPtr = rPtr->next;
			} // end while

			lPtr = rPtr;
		} while (killSwitch);
	} // end if
} // end function sort_list_burst()

//***************************************************************************ROUND ROBIN

void rr()
{
	//Initialize runner
	running = processes[0].pid;
	processes[0].startTime = processes[0].arrivalTime;
	clock = processes[0].startTime;

	//While processes remain to be executed
	while (processesRemaining)
	{
		//If next event is an arrival
		if (next_event_rr()) // sets clock
		{
			add_arrivals_rr();
		}

		//Update wait times
		update_srtf();

		//If runner could change
		if (waitingList.first != NULL)
		{
			fork_rr(); // switches runner, begins waiting
		}
		else if (query_next_arrival() > (get_relative_start() + processes[running].burstTime))
		{
			//runner_complete();
			if (processes[running].flag == -1)
			{
				force_start();
			}
		} // end else-if
	} // end while

	//Close out the last process
	clock += processes[running].burstTime;
	processes[running].endTime = clock;

	//Calculate avg times and print to console
	calc_times_and_print("Round Robin (w/ quantum 100)");
} // end function rr()

int next_event_rr()
{
	//Get next arrival time
	int nextArriving = query_next_arrival();

	//If last process already arrived
	if (!nextArriving)
	{
		//Force next event to always be runner's end time
		nextArriving = MAX_TIME;
	}

	//If runner will end before quantum
	int temp = processes[running].remainingQuantum; // for readability
	if (processes[running].burstTime <= temp)
	{
		//But an arrival will occur before that
		if (nextArriving < (clock + processes[running].burstTime))
		{
			//Save runner's remaining quantum and jump to next arrival
			processes[running].remainingQuantum = (temp - (nextArriving - clock));
			clock = nextArriving;
			return 1;
		}
		else // end runner
		{
			//Set clock to runner's end time
			clock += processes[running].burstTime;
			runner_complete();
			return 0;
		}
	} // end if
	else if (nextArriving <= (clock + temp))
	{
		//Save runner's remaining quantum and jump to next arrival
		processes[running].remainingQuantum = (temp - (nextArriving - clock));
		clock = nextArriving;
		return 1;
	}

	//Runner's burst exceeds quantum
	clock += temp;
	processes[running].remainingQuantum = QUANTUM; 
	processes[running].burstTime = get_remaining_burst();
	processes[running].latestStartTime = clock;
	
	return 0;
} // end function next_event_rr()

void add_arrivals_rr()
{
	//For every process
	int i;
	for (i = 0; i < numProcesses; i++)
	{
		//If this process isn't the one currently running
		if (running != i)
		{
			//If this process has not already ended
			if (processes[i].flag != -1)
			{
				//If this process has arrived
				if (processes[i].arrivalTime <= clock)
				{
					//Add it to the waiting list
					send_to_waiting(&processes[i]);
				} // end if
			} // end for
		} // end if
	} // end for
} // end function add_arrivals_rr()

void fork_rr()
{
	//If the runner has to wait
	if (process_interrupt_rr())
	{
		running_to_waiting();
		waiting_to_running_rr();
	}

	//For every process waiting
	Node* next = waitingList.first;
	while (next != NULL)
	{
		//Start their wait time
		next->data->beginWaiting = clock;
		next = next->next;
	}
} // end function fork_rr()

int process_interrupt_rr()
{
	int remainingBurst = get_remaining_burst();

	//If the runner is NOT finished
	if (remainingBurst)
	{
		//Save runner's remaining burst time
		processes[running].burstTime = remainingBurst;

		//If runner has quantum left, but is not about to start a full new one
		if (processes[running].remainingQuantum != 0 && processes[running].remainingQuantum != QUANTUM)
		{
			//Runner continues to run
			processes[running].latestStartTime = clock;
			return 0;
		}
		else // runner's time is up
		{
			//New process takes its place
			return 1;
		}
	} // end if
	else // runner is done
	{
		if (processes[running].flag != -1)
		{
			runner_complete();
		}
		return 1;
	}
} // end function process_interrupt_rr()

int get_remaining_burst()
{
	int currBurst = 0;
	int remainingBurst = 0;

	//If runner never had to wait, use startTime
	if (!processes[running].latestStartTime)
	{
		//For the amount of burst time the runner has completed
		currBurst = (clock - processes[running].startTime);
	}
	else // runner waited, use latestStartTime
	{
		//For the amount of burst time the runner has completed
		currBurst = (clock - processes[running].latestStartTime);
	}

	//Subtract it from runner's total burst time
	remainingBurst = (processes[running].burstTime - currBurst);

	return remainingBurst;
} // end function get_remaining_burst()

int get_relative_start()
{
	if (processes[running].latestStartTime)
	{
		return processes[running].latestStartTime;
	}
	else
	{
		return processes[running].startTime;
	}
} // end function get_relative_start()

void waiting_to_running_rr()
{
	//Save new runner
	running = waitingList.first->data->pid;

	//If runner hasn't already started
	if (!processes[running].startTime)
	{
		processes[running].startTime = clock;
	}
	else // process has been waiting
	{
		processes[running].latestStartTime = clock;
	}

	pop_front(&waitingList);
} // end function waiting_to_running_rr()

//***************************************************************************OTHER

void init_all()
{
	//Initialize globals
	processesRemaining = (numProcesses - 1);
	running = 0;
	clock = 0;

	//Free memory
	list_destructor(&waitingList);

	//Set quantum value for each process struct
	int i;
	for (i = 0; i < numProcesses; i++)
	{
		processes[i].remainingQuantum = QUANTUM;
	}
} // end function init_all()

void calc_times_and_print(char* algorithmType)
{
	//Declare locals
	double sumResponseTime = 0;
	double sumTurnTime = 0;
	double sumWaitTime = 0;

	//For every process
	int i;
	for (i = 0; i < numProcesses; i++)
	{
		//Add up their response, turnaround, and wait times
		sumResponseTime += (processes[i].startTime - processes[i].arrivalTime);
		sumTurnTime += (processes[i].endTime - processes[i].arrivalTime);
		sumWaitTime += processes[i].waitTime;

		// processes[i].responseTime = (processes[i].startTime - processes[i].arrivalTime);
		// processes[i].turnTime = (processes[i].endTime - processes[i].arrivalTime);

		// printf("Process: %d\n"
		// 	"\tArrival Time: %d\n"
		// 	"\tStart Time: %d\n"
		// 	"\tEnd Time: %d\n"
		// 	"\tResponse Time: %d\n"
		// 	"\tTurn Time: %d\n"
		// 	"\tWait Time: %d\n"
		// 	, i, processes[i].arrivalTime, 
		// 	processes[i].startTime, 
		// 	processes[i].endTime, 
		// 	processes[i].responseTime, 
		// 	processes[i].turnTime, 
		// 	processes[i].waitTime);
	} // end for

	  //Calculate avg times
	double avgResponseTime = (sumResponseTime / numProcesses);
	double avgTurnTime = (sumTurnTime / numProcesses);
	double avgWaitTime = (sumWaitTime / numProcesses);

	//Print result to console
	print(algorithmType, avgResponseTime, avgTurnTime, avgWaitTime);
} // end function calc_times_and_print()

void print(char* algorithmType, double responseTime, double turnTime, double waitTime)
{
	//Print the description for the algorithm used
	printf("\n%s:\n", algorithmType);

	//Print avg times
	printf("\tAVG Response Time: %.2f\n"
			"\tAVG Turnaround Time: %.2f\n"
			"\tAVG Wait Time: %.2f\n",
			responseTime, turnTime, waitTime);
} // end function print()

//***************************************************************************LIST
void list_constructor(List* self)
{
	self->first = NULL;
	self->last = NULL;
	self->count = 0;
} // end function constructor

void list_param_constructor(List* self, Node* _first, Node* _last, int _count)
{
	self->first = _first;
	self->last = _last;
	self->count = _count;
} // end function parameterized constructor

void list_destructor(List* self)
{
	//Save first as Node p
	Node* p = self->first;

	//While p is not null
	while (p != NULL)
	{
		//Get the next node of p and save as Node pnext
		Node* pnext = get_next(p);

		//Remove p from list
		free(p);
		p = pnext;

	} // end while
} // end destructor

void push_front(List* self, Node* newNode)
{
	//If list is empty
	if (self->count == 0)
	{
		self->first = newNode;
		self->last = newNode;
	}
	else  //If list is not empty
	{
		//Push the previously first node into the next position of the newNode
		set_next(newNode, self->first);

		//Set newNode as first
		self->first = newNode;
	}

	//Increment counter
	self->count++;
} // end function push_front()

Node* pop_front(List* self)
{
	//If list is not empty
	if (self->count != 0)
	{
		//Save the first node
		Node* saveFirst = self->first;

		//Set the next node as first
		self->first = get_next(self->first);

		//If first was the last node
		if (self->first == NULL)
		{
			//Set last to NULL
			self->last = NULL;
		}

		//Decrement the count
		self->count--;

		return saveFirst;
	}
	else  //If list is empty
	{
		return NULL;
	}
} // end function pop_front()

void push_back(List* self, Node* newNode)
{
	//If list is empty
	if (self->count == 0)
	{
		self->first = newNode;
		self->last = newNode;
	}
	else  // list is NOT empty
	{
		if (!duplicate(self, newNode))
		{
			//Push the newNode into the next position of the previously last node
			set_next(self->last, newNode);

			//Set newNode as last
			self->last = newNode;
		}
		else // DUPLICATE
			return;
	} // end else

	//Increment counter
	self->count++;
} // end function push_back()

Node* pop_back(List* self)
{
	//If the list is not empty
	if (self->count != 0)
	{
		//Take last and save as new pointer
		Node* temp = self->last;
		Node* n = self->first;

		//While the next pointer of each node is not equal to last
		while (get_next(n) != self->last)
		{
			//Get the next node and save as n
			n = get_next(n);
		}
		//Set new last to null
		set_next(n, NULL);

		//Set last to new last
		self->last = n;

		//Decrement the count
		self->count--;

		return temp;
	}
	else  //If list is empty
	{
		return NULL;
	}
} // end function pop_back()

int duplicate(List* self, Node* newNode)
{
	Node* current = self->first;

	while (current != NULL)
	{
		//If a match isn't made
		if (current->data->pid != newNode->data->pid)
		{
			current = current->next;
		}
		else // match found
		{
			return 1;
		}
	} // end while

	return 0;
} // end function duplicate()

//***************************************************************************NODE

void node_constructor(Node* self)
{
	self->next = NULL;
	self->data = NULL;
} // end constructor

void node_param_constructor(Node* self, Process* _data)
{
	self->data = _data;
	self->next = NULL;
} // end parameterized constructor

void node_destructor(Node* self)
{
	//Check to ensure pointer has not already been deleted
	if (self->data != NULL)
	{
		free(self->data);
		self->data = NULL;
	}
} // end destructor

Node* get_next(Node* self)
{
	return self->next;
} // end function get_next()

Process* get_data(Node* self)
{
	return self->data;
} // end function get_data()

void set_next(Node* self, Node* node)
{
	self->next = node;
} // end function set_next()

void set_data(Node* self, Process* process)
{
	self->data = process;
} // end function set_data()