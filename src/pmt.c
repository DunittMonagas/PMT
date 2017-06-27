

#include <unistd.h>

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <pmt.h>
#include <queue.h>

typedef struct thread{

   	pmtID id;
   	PMT_STATUS status;
	mctx_t *ctx;
   	void *mctx_arg;
	int priority;
	void (*mctx_func)(void*);
	void *sk_addr;
	int block_type;
	time_t lastRun;
	int timeout;

}thread_t;


static int numThread= 0;
static int sigstksz = 16384;
static queue_t *threadQueue= NULL;
static bool threadExecution= false;
static thread_t *currentThread= NULL;
static thread_t *threadPool[MAX_THREAD];
static bool (*scheduler)(void*, void*)= NULL;

/*
static thread_t* getThread(){

	int i;
	for(i= 0; i<MAX_THREAD; ++i)
		if(!threadQueue[i].status)
			return &threadQueue[i];

	return NULL;
}
*/

bool priorities(void *lhs, void *rhs){

	//printf("lhs %d\n", ((thread_t*)lhs)->priority);
	//printf("rhs %d\n", ((thread_t*)rhs)->priority);

	
	return ((thread_t*)lhs)->priority < ((thread_t*)rhs)->priority;

}

bool prioritiesAging(void *lhs, void *rhs){

	//printf("lhs %d\n", ((thread_t*)lhs)->priority);
	//printf("rhs %d\n", ((thread_t*)rhs)->priority);

	double diff= difftime(((thread_t*)lhs)->lastRun, ((thread_t*)rhs)->lastRun);

	printf("%f\n", diff);

	if(diff < 0.0)
		return false;
	else
		return true;
	//return ((thread_t*)lhs)->priority < ((thread_t*)rhs)->priority;

}



void mctx_create(mctx_t *mctx, void (*sf_addr)(void *), void *sf_arg, void *sk_addr, size_t sk_size){

	struct sigaction sa;
	struct sigaction osa;
	struct sigaltstack ss;
	struct sigaltstack oss;
	sigset_t osigs;
	sigset_t sigs;

	/* Step 1: 
	Preserve the current signal mask and block an 
	arbitrary worker signal (we use SIGUSR1, but any
	signal can be used for this – even an already used
	one). This worker signal is later temporarily 
	required for the trampoline step. */
	printf("INICIO PASO 1\n");
	sigemptyset(&sigs);
	sigaddset(&sigs, SIGUSR1);
	sigprocmask(SIG_BLOCK, &sigs, &osigs);
	printf("FIN PASO 1\n");

	/* Step 2: 
	Preserve a possibly existing signal action for the
	worker signal and configure a trampoline function
	as the new temporary signal action. The signal 
	delivery is configured to occur on an alternate signal
	stack (see next step). */
	printf("INICIO PASO 2\n");
	memset((void *)&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = mctx_create_trampoline;
	sa.sa_flags = SA_ONSTACK;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGUSR1, &sa, &osa);
	printf("FIN PASO 2\n");

	/* Step 3: 
	Preserve a possibly active alternate signal stack
	and configure the memory chunk starting at
	sk addr as the new temporary alternate signal
	stack of length sk size. */
	printf("INICIO PASO 3\n");
	ss.ss_sp = sk_addr;
	ss.ss_size = sk_size;
	ss.ss_flags = 0;
	sigaltstack(&ss, &oss);
	printf("FIN PASO 3\n");

	/* Step 4: 
	Save parameters for the trampoline step (mctx,
	sf addr, sf arg, etc.) in global variables, send the
	current process the worker signal, temporarily unblock 
	it and this way allow it to be delivered on the
	signal stack in order to transfer execution control
	to the trampoline function. */
	printf("INICIO PASO 4\n");
	mctx_creat = mctx;
	mctx_creat_func = sf_addr;
	mctx_creat_arg = sf_arg;
	mctx_creat_sigs = osigs;
	mctx_called = false;
	kill(getpid(), SIGUSR1);
	sigfillset(&sigs);
	sigdelset(&sigs, SIGUSR1);
	while (!mctx_called)
		sigsuspend(&sigs);
	printf("FIN PASO 4\n");

	/* Step 6: 
	Restore the preserved alternate signal stack, 
	preserved signal action and preserved signal mask for
	worker signal. This way an existing application
	configuration for the worker signal is restored. */
	printf("INICIO PASO 6\n");
	sigaltstack(NULL, &ss);
	ss.ss_flags = SS_DISABLE;
	sigaltstack(&ss, NULL);
	if (!(oss.ss_flags & SS_DISABLE))
		sigaltstack(&oss, NULL);
	sigaction(SIGUSR1, &osa, NULL);
	sigprocmask(SIG_SETMASK, &osigs, NULL);
	printf("FIN PASO 6\n");

	/* Step 7: 
	Save the current machine context of
	mctx create. This allows us to return to this
	point after the next trampoline step.

	Step 8:
	Restore the previously saved machine context of
	the trampoline function (mctx) to again transfer 
	execution control onto the alternate stack, but this
	time without(!) signal handler scope. */
	printf("INICIO PASO 7 Y 8\n");
	mctx_switch(&mctx_caller, mctx);
	printf("FIN PASO 7 Y 8\n");

	/* Step 14: 
	Return to the calling application. */
	printf("INICIO PASO 14\n");

	return;

}

void mctx_create_trampoline(int sig){

	/* Step 5: 
	After the trampoline function asynchronously entered, 
	save its machine context in the mctx structure 
	and immediately return from it to terminate
	the signal handler scope. */
	printf("INICIO PASO 5\n");
	if (mctx_save(mctx_creat) == 0){
		mctx_called = true;
		return;
	}
	printf("FIN PASO 5\n");

	/* Step 9: 
	After reaching the trampoline function (mctx)
	again, immediately bootstrap into a clean stack
	frame by just calling a second function. */
	printf("INICIO PASO 9\n");
	mctx_create_boot();
	printf("FIN PASO 9\n");

}

void mctx_create_boot(){

	void (*mctx_start_func)(void *);
	void *mctx_start_arg;

	/* Step 10: 
	Set the new signal mask to be the same as
	the original signal mask which was active when
	mctx create was called. This is required because 
	in the first trampoline step we usually had at
	least the worker signal blocked. */
	printf("INICIO PASO 10\n");
	sigprocmask(SIG_SETMASK, &mctx_creat_sigs, NULL);
	printf("FIN PASO 10\n");

	/* Step 11: 
	Load the passed startup information (sf addr,
	sf arg) from mctx create into local (stack-based) 
	variables. This is important because their
	values have to be preserved in machine context 
	dependent memory until the created machine context
	is the first time restored by the application. */
	printf("INICIO PASO 11\n");
	mctx_start_func = mctx_creat_func;
	mctx_start_arg = mctx_creat_arg;
	printf("FIN PASO 11\n");

	/* Step 12:
	Save the current machine context for later 
	restoring by the calling application.

	Step 13: 
	Restore the previously saved machine context of
	mctx create to transfer execution control back
	to it. */
	printf("INICIO PASO 12\n");
	printf("INICIO PASO 13\n");
	mctx_switch(mctx_creat, &mctx_caller);
	printf("FIN PASO 12\n");
	printf("FIN PASO 13\n");

	/* The thread ‘‘magically’’ starts... */
	printf("INICIO FUNCIÓN\n");
	mctx_start_func(mctx_start_arg);
	printf("FIN FUNCIÓN\n");

	currentThread->status= PMT_FINISHED;

	printf("************************************************\n");
	mctx_restore(&mctx_caller);

	/* NOTREACHED */
	//abort();
	//pmtDestroyThread();

}

int pmtInitialize(){

	numThread= 0;
	currentThread= NULL;
	threadExecution= false;
	threadQueue= queueAlloc(NULL);

	int i;
	for(i= 0; i<MAX_THREAD; ++i)
		threadPool[i]= NULL;

/*
	int i;
	for(i= 0; i<MAX_THREAD; ++i){

		//threadQueue[i].ctx= NULL;
   		threadQueue[i].id= 0;
   		threadQueue[i].status= PMT_INVALID;
   		threadQueue[i].mctx_arg= NULL;
		threadQueue[i].priority= -1;
		threadQueue[i].mctx_func= NULL;
		threadQueue[i].sk_addr= NULL;
		threadQueue[i].block_type= -1;
		threadQueue[i].time= 0;
		threadQueue[i].timeout= 0;

	}
*/
	return PMT_OK;

}

int pmtTerminate(){

	thread_t *thr;
	while(!queueEmpty(threadQueue)){

		thr= queueFront(threadQueue);
		queuePop(threadQueue);

		if(thr->sk_addr)
			free(thr->sk_addr);

		if(thr->ctx)
			free(thr->ctx);

		thr->ctx= NULL;
		thr->mctx_func= NULL;
		thr->mctx_arg= NULL;
		thr->sk_addr= NULL;

		printf("ID %d, %d\n", thr->id, thr->priority);

		free(thr);

	}

	//Eliminar cada hilo por separado.
	queueFree(threadQueue);

	int i;
	for(i= 0; i<MAX_THREAD; ++i)
		threadPool[i]= NULL;

/*
	int i;
	for(i= 0; i<MAX_THREAD; ++i){

		//threadQueue[i].ctx= NULL;
   		threadQueue[i].id= 0;
   		threadQueue[i].status= PMT_INVALID;
   		threadQueue[i].mctx_arg= NULL;
		threadQueue[i].priority= -1;
		threadQueue[i].mctx_func= NULL;
		threadQueue[i].block_type= -1;
		threadQueue[i].time= 0;
		threadQueue[i].timeout= 0;

		if(threadQueue[i].sk_addr)
			free(threadQueue[i].sk_addr);

		threadQueue[i].sk_addr= NULL;

	}
*/
	return PMT_OK;
}

static int pmtDestroyThread(thread_t *thr){

	if(thr == NULL)
		return PMT_INVALID_THREAD;

	if(thr->sk_addr)
		free(thr->sk_addr);

	if(thr->ctx)
		free(thr->ctx);

	thr->ctx= NULL;
	thr->mctx_func= NULL;
	thr->mctx_arg= NULL;
	thr->sk_addr= NULL;

	int i;
	for(i= 0; i<MAX_THREAD; ++i)
		if(threadPool[i] != NULL && threadPool[i]->id == thr->id)
			threadPool[i]= NULL;

	free(thr);
	thr= NULL;

	return PMT_OK;

}

int pmtCreateThread(pmtID *id, void (*func)(void*), void* arg){

	if(numThread == MAX_THREAD)
		return PMT_THREAD_LIMIT_EXCEEDED;

	thread_t *thr = malloc(sizeof(thread_t));
	//thread_t *thr= getThread();

	thr->id= numThread++;
	thr->status= PMT_INVALID;
	//printf("%d\n", thr->id);
    thr->ctx = malloc(sizeof(mctx_t));
    thr->priority= 1;
    thr->mctx_func= func;
    thr->mctx_arg= arg;
    thr->sk_addr= malloc(sigstksz);
	thr->block_type= -1;
	thr->timeout= -1;

	int i;
	for(i= 0; i<MAX_THREAD; ++i)
		if(threadPool[i] == NULL){
			printf("LIBRE\n");
			threadPool[i]= thr;
			break;
		}

	//DUDA ¿mctx_create(..., thr->sk_addr + sigstksz, sigstksz) ó mctx_create(..., thr->sk_addr, sigstksz)?

	//mctx_create(thr->ctx, thr->mctx_func, thr->mctx_arg, thr->sk_addr + sigstksz, sigstksz);
	mctx_create(thr->ctx, thr->mctx_func, thr->mctx_arg, thr->sk_addr, sigstksz);

	time(&(thr->lastRun));
	thr->status= PMT_READY;

	queuePushBack(threadQueue, thr);

	(*id)= thr->id;
	printf("SALIO\n");

	return PMT_OK;
}

void pmtYield(){

	if(threadExecution){

		if(mctx_save(currentThread->ctx)){

		}else{
			//time(&(currentThread->lastRun));
			mctx_restore(&mctx_caller);
		}

	}

}

int pmtRunThread(){

	//if(id >= MAX_THREAD)
	//	return PMT_THREAD_LIMIT_EXCEEDED;

	//if(!threadQueue[id].status)
	//	return PMT_INVALID_THREAD;

	//thread_t *thr;
	while(!queueEmpty(threadQueue)){

		currentThread= (thread_t*)queueFront(threadQueue);

		if(currentThread->status == PMT_READY){

			queuePop(threadQueue);
			threadExecution= true;
			mctx_switch(&mctx_caller, currentThread->ctx);
			threadExecution= false;
			time(&(currentThread->lastRun));
			//currentThread->status= PMT_FINISHED;

		}

		if(currentThread->status == PMT_READY){

			queuePushBack(threadQueue, currentThread);

		}else{

			pmtDestroyThread(currentThread);

		}
		
	}

	return PMT_OK;

}

int pmtSetupThread(pmtID id, int priority){

	int i;
	thread_t *thr= NULL;

	for(i= 0; i<MAX_THREAD; ++i)
		if(threadPool[i] != NULL && threadPool[i]->id == id){
			printf("ENCONTRADO\n");
			thr= threadPool[i];
			break;
		}

	thr->priority= priority;
	
	//INICIO DEL CÓDIGO VERGONZOSO
	queueFree(threadQueue);
	threadQueue= queueAlloc(scheduler);

	for(i= 0; i<MAX_THREAD; ++i)
		if(threadPool[i] != NULL){
			//printf("ENCONTRADO\n");
			queuePushBack(threadQueue, threadPool[i]);
			
		}
	//FIN DEL CÓDIGO VERGONZOSO

	return PMT_OK;
}

int pmtSetupScheduler(PMT_SCHEDULER newScheduler){

	if(newScheduler == PMT_FIFO)
		scheduler= NULL;
		//queueSetupComparisonFunction(threadQueue, NULL);
	else if(newScheduler == PMT_ROUND_ROBIN)
		scheduler= NULL;
		//queueSetupComparisonFunction(threadQueue, NULL);
	else if(newScheduler == PMT_PRIORIY)
		scheduler= priorities;
		//queueSetupComparisonFunction(threadQueue, priorities);
	else if(newScheduler == PMT_PRIORIY_AGING)
		scheduler= prioritiesAging;
		//queueSetupComparisonFunction(threadQueue, prioritiesAging);

	queueSetupComparisonFunction(threadQueue, scheduler);

	return PMT_OK;
}
