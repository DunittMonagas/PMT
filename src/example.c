
#include <pmt.h>
#include <stdio.h>
#include <stdlib.h>


#include <queue.h>
#include <time.h>
#include <unistd.h>


typedef struct DATOS{
	
	int id;
	time_t lastRun;

}DATOS;

bool compLess(void *lhs, void *rhs){

	//printf("lhs %d\n", ((thread_t*)lhs)->priority);
	//printf("rhs %d\n", ((thread_t*)rhs)->priority);

	
	return (*((int*)lhs) < *((int*)rhs));

}

bool compGreater(void *lhs, void *rhs){

	//printf("lhs %d\n", ((thread_t*)lhs)->priority);
	//printf("rhs %d\n", ((thread_t*)rhs)->priority);

	
	return (*((int*)lhs) > *((int*)rhs));

}


bool env(void *lhs, void *rhs){

	//printf("lhs %d\n", ((thread_t*)lhs)->priority);
	//printf("rhs %d\n", ((thread_t*)rhs)->priority);

	double diff= difftime(((DATOS*)lhs)->lastRun, ((DATOS*)rhs)->lastRun);

	printf("%f\n", diff);

	if(diff < 0.0)
		return false;
	else
		return true;
	//return ((thread_t*)lhs)->priority < ((thread_t*)rhs)->priority;

}

void thread1(void *data){

	int i;

	for(i= 0; i<5; ++i){

		printf("Hey, I'm thread #1: %d\n", i);
		printf("DUERME %s\n", (char*)data);
		sleep(4);
		printf("DESPIERTA %s\n", (char*)data);
		//pmtYield();
		
	}

}

void fibonacchi(void *data){

	int i;
	int fib[2]= { 0, 1 };
	
	printf("fibonacchi(0) = 0\nfibonnachi(1) = 1\n");
	for(i= 2; i<15; ++i){

		int nextFib= fib[0] + fib[1];
		printf("fibonacchi(%d) = %d\n", i, nextFib);
		fib[0]= fib[1];
		fib[1]= nextFib;
		printf("DUERME %s\n", (char*)data);
		sleep(4);
		printf("DESPIERTA %s\n", (char*)data);
		//pmtYield();

	}

}

void squares(void *data){

	int i;
	
	for (i= 0; i<10; ++ i){

		printf("%d*%d = %d\n", i, i, i*i );
		printf("DUERME %s\n", (char*)data);
		sleep(4);
		printf("DESPIERTA %s\n", (char*)data);
		//pmtYield();

	}

}

/*
bool compLess(void *lhs, void *rhs){
	return (*((int*)lhs) < *((int*)rhs));
}

bool compGreater(void *lhs, void *rhs){
	return (*((int*)lhs) > *((int*)rhs));
}
*/

/* number of times the handle will run: */
volatile int breakflag = 3;

void ALARMhandler(int sig){

	signal(SIGALRM, SIG_IGN);          /* ignore this signal       */
	printf("Hello\n");
	signal(SIGALRM, ALARMhandler);     /* reinstall the handler    */
	//signal(SIGALRM, SIG_IGN);          /* default signal handling       */
	alarm(2);

}

void handle(int sig){

	printf("Hello\n");
	--breakflag;
	alarm(3);

}


int main(){

/*	
	signal(SIGALRM, ALARMhandler);
	alarm(2);                     //set alarm clock
  	while (1);
	printf("All done");
*/	
/*
	signal(SIGALRM, handle);
    alarm(1);
    while(breakflag) { 
    	//sleep(1); 
    }
    printf("done\n");

	//sleep(5);
/*
	DATOS *tmp;
	queue_t *queue= queueAlloc(&env);

	sleep(1);

	tmp= (DATOS*)malloc(sizeof(DATOS));
	tmp->id= 1;
	time(&(tmp->lastRun));
	queuePushBack(queue, (void*)tmp);

	sleep(2);

	tmp= (DATOS*)malloc(sizeof(DATOS));
	tmp->id= 2;
	time(&(tmp->lastRun));
	queuePushBack(queue, (void*)tmp);

	sleep(1);
	tmp= (DATOS*)malloc(sizeof(DATOS));
	tmp->id= 51;
	time(&(tmp->lastRun));
	queuePushBack(queue, (void*)tmp);

	sleep(1);
	tmp= (DATOS*)malloc(sizeof(DATOS));
	tmp->id= 14;
	time(&(tmp->lastRun));
	queuePushBack(queue, (void*)tmp);

	sleep(3);
	tmp= (DATOS*)malloc(sizeof(DATOS));
	tmp->id= 11;
	time(&(tmp->lastRun));
	queuePushBack(queue, (void*)tmp);

	sleep(2);
	tmp= (DATOS*)malloc(sizeof(DATOS));
	tmp->id= 99;
	time(&(tmp->lastRun));
	queuePushBack(queue, (void*)tmp);

	sleep(5);
	tmp= (DATOS*)queueFront(queue);
	queuePop(queue);
	time(&(tmp->lastRun));
	queuePushBack(queue, (void*)tmp);

	while(queueSize(queue)){

		tmp= (DATOS*)queueFront(queue);
		printf("%d ", tmp->id);
		queuePop(queue);

	}

	printf("\n");
*/
/*
	queueSetupComparisonFunction(queue, &compGreater);

	tmp= (DATOS*)malloc(sizeof(DATOS));
	tmp->id= 1;
	time(&(tmp->lastRun));
	queuePushBack(queue, (void*)tmp);

	tmp= (DATOS*)malloc(sizeof(DATOS));
	tmp->id= 2;
	time(&(tmp->lastRun));
	queuePushBack(queue, (void*)tmp);

	tmp= (DATOS*)malloc(sizeof(DATOS));
	tmp->id= 51;
	time(&(tmp->lastRun));
	queuePushBack(queue, (void*)tmp);

	tmp= (DATOS*)malloc(sizeof(DATOS));
	tmp->id= 14;
	time(&(tmp->lastRun));
	queuePushBack(queue, (void*)tmp);

	tmp= (DATOS*)malloc(sizeof(DATOS));
	tmp->id= 11;
	time(&(tmp->lastRun));
	queuePushBack(queue, (void*)tmp);

	tmp= (DATOS*)malloc(sizeof(DATOS));
	tmp->id= 99;
	time(&(tmp->lastRun));
	queuePushBack(queue, (void*)tmp);

	while(queueSize(queue)){

		tmp= (DATOS*)queueFront(queue);
		printf("%d ", tmp->id);
		queuePop(queue);

	}

	printf("\n");
	queueFree(queue);

*/


	pmtID id1, id2, id3;

	char str1[]="Thread 1";
	char str2[]="Thread 2";
	char str3[]="Thread 3";


	pmtInitialize();

	pmtSetupScheduler(PMT_SETUP_QUANTUM, 2);
	//pmtSetupScheduler(PMT_PRIORIY_AGING);
	//pmtSetupScheduler(PMT_PRIORIY);

	pmtCreateThread(&id1, &thread1, (void*)str1);
	printf("Hilo %d creado.\n\n", id1);
	//sleep(2);

	//pmtCreateThread(&id2, &fibonacchi, (void*)str2);
	printf("Hilo %d creado.\n\n", id2);
	//sleep(4);

	//pmtCreateThread(&id3, &squares, (void*)str3);
	printf("Hilo %d creado.\n\n", id3);
	//sleep(1);

	/*
	printf("Estableciendo prioridades\n");	
	pmtSetupThread(id1, 2);
	pmtSetupThread(id2, 12);
	pmtSetupThread(id3, 5);
	*/

	printf("Ejecución del hilo %d\n\n", id1);
	pmtRunThread();
	printf("TERMINÓ %d\n", id1);

	pmtTerminate();

	return 0;
}