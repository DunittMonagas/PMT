

#include <unistd.h>

#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <pmt.h>
#include <queue.h>



typedef enum STATUS{

	PMT_INVALID= 0,
	PMT_READY,
	PMT_FINISHED

}PMT_STATUS;


typedef struct thread{

   	pmtID id;
   	PMT_STATUS status;
	mctx_t *ctx;
   	void *mctx_arg;
	int priority;
	void (*mctx_func)(void*);
	sigset_t mctx_sigs;
	void *sk_addr;
	int block_type;
	time_t lastRun;

}thread_t;



static int numThread= 0;
static int sigstksz = 16384;

static queue_t *threadQueue= NULL;
static thread_t *threadPool[MAX_THREAD];
static bool (*scheduler)(void*, void*)= NULL;

static unsigned quantum= 3;
static bool threadExecution= false;
static thread_t *currentThread= NULL;
static bool roundRobin= false;



bool priorities(void *lhs, void *rhs){	
	return ((thread_t*)lhs)->priority < ((thread_t*)rhs)->priority;
}

bool prioritiesAging(void *lhs, void *rhs){

	double diff= difftime(((thread_t*)lhs)->lastRun, ((thread_t*)rhs)->lastRun);

	if(diff < 0.0)
		return false;
	else
		return true;

}


void alarmHandler(int sig){

	if(threadExecution){

		currentThread->block_type= 1;

		sigset_t block;
		sigemptyset(&block);
		sigaddset(&block, SIGALRM);
		sigprocmask(SIG_BLOCK, &block, NULL);

		mctx_switch(currentThread->ctx, &mctx_caller);

		sigemptyset(&block);
		sigaddset(&block, SIGALRM);
		sigprocmask(SIG_UNBLOCK, &block, NULL);

		currentThread->block_type= 0;
		
		alarm(quantum);

	}

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
	sigemptyset(&sigs);
	sigaddset(&sigs, SIGUSR1);
	sigprocmask(SIG_BLOCK, &sigs, &osigs);

	/* Step 2: 
	Preserve a possibly existing signal action for the
	worker signal and configure a trampoline function
	as the new temporary signal action. The signal 
	delivery is configured to occur on an alternate signal
	stack (see next step). */
	memset((void *)&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = mctx_create_trampoline;
	sa.sa_flags = SA_ONSTACK;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGUSR1, &sa, &osa);

	/* Step 3: 
	Preserve a possibly active alternate signal stack
	and configure the memory chunk starting at
	sk addr as the new temporary alternate signal
	stack of length sk size. */
	ss.ss_sp = sk_addr;
	ss.ss_size = sk_size;
	ss.ss_flags = 0;
	sigaltstack(&ss, &oss);

	/* Step 4: 
	Save parameters for the trampoline step (mctx,
	sf addr, sf arg, etc.) in global variables, send the
	current process the worker signal, temporarily unblock 
	it and this way allow it to be delivered on the
	signal stack in order to transfer execution control
	to the trampoline function. */
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

	/* Step 6: 
	Restore the preserved alternate signal stack, 
	preserved signal action and preserved signal mask for
	worker signal. This way an existing application
	configuration for the worker signal is restored. */
	sigaltstack(NULL, &ss);
	ss.ss_flags = SS_DISABLE;
	sigaltstack(&ss, NULL);
	if (!(oss.ss_flags & SS_DISABLE))
		sigaltstack(&oss, NULL);
	sigaction(SIGUSR1, &osa, NULL);
	sigprocmask(SIG_SETMASK, &osigs, NULL);


	/* Step 7: 
	Save the current machine context of
	mctx create. This allows us to return to this
	point after the next trampoline step.

	Step 8:
	Restore the previously saved machine context of
	the trampoline function (mctx) to again transfer 
	execution control onto the alternate stack, but this
	time without(!) signal handler scope. */
	mctx_switch(&mctx_caller, mctx);


	/* Step 14: 
	Return to the calling application. */
	return;

}

void mctx_create_trampoline(int sig){

	/* Step 5: 
	After the trampoline function asynchronously entered, 
	save its machine context in the mctx structure 
	and immediately return from it to terminate
	the signal handler scope. */
	if (mctx_save(mctx_creat) == 0){
		mctx_called = true;
		return;
	}


	/* Step 9: 
	After reaching the trampoline function (mctx)
	again, immediately bootstrap into a clean stack
	frame by just calling a second function. */
	mctx_create_boot();

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
	sigprocmask(SIG_SETMASK, &mctx_creat_sigs, NULL);

	/* Step 11: 
	Load the passed startup information (sf addr,
	sf arg) from mctx create into local (stack-based) 
	variables. This is important because their
	values have to be preserved in machine context 
	dependent memory until the created machine context
	is the first time restored by the application. */
	mctx_start_func = mctx_creat_func;
	mctx_start_arg = mctx_creat_arg;

	/* Step 12:
	Save the current machine context for later 
	restoring by the calling application.

	Step 13: 
	Restore the previously saved machine context of
	mctx create to transfer execution control back
	to it. */
	mctx_switch(mctx_creat, &mctx_caller);
	

	if(roundRobin){

		sigset_t unlock;
		sigemptyset(&unlock);
		sigaddset(&unlock, SIGALRM);
		sigprocmask(SIG_UNBLOCK, &unlock, NULL);	

	}

	/* The thread ‘‘magically’’ starts... */
	mctx_start_func(mctx_start_arg);

	currentThread->status= PMT_FINISHED;

	mctx_restore(&mctx_caller);

	/* NOTREACHED */
	//abort();

}

int pmtInitialize(){

	numThread= 0;
	scheduler= NULL;
	roundRobin= false;
	currentThread= NULL;
	unsigned quantum= 3;
	threadExecution= false;
	threadQueue= queueAlloc(NULL);

	int i;
	for(i= 0; i<MAX_THREAD; ++i)
		threadPool[i]= NULL;

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

		free(thr);

	}

	//Eliminar cada hilo por separado.
	queueFree(threadQueue);

	int i;
	for(i= 0; i<MAX_THREAD; ++i)
		threadPool[i]= NULL;

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

	thr->id= numThread++;
	thr->status= PMT_INVALID;
    thr->ctx = malloc(sizeof(mctx_t));
    thr->priority= 1;
    thr->mctx_func= func;
    thr->mctx_arg= arg;
    thr->sk_addr= malloc(sigstksz);
	thr->block_type= 0;

	int i;
	for(i= 0; i<MAX_THREAD; ++i)
		if(threadPool[i] == NULL){
			threadPool[i]= thr;
			break;
		}

	mctx_create(thr->ctx, thr->mctx_func, thr->mctx_arg, thr->sk_addr, sigstksz);

	time(&(thr->lastRun));
	thr->status= PMT_READY;

	queuePushBack(threadQueue, thr);

	(*id)= thr->id;

	return PMT_OK;

}


void pmtYield(){

	if(roundRobin){
		alarm(0);
	}

	if(threadExecution){

		currentThread->block_type= 1;

		if(mctx_save(currentThread->ctx)){

			sigset_t block;
			sigemptyset(&block);
			sigaddset(&block, SIGALRM);
			sigprocmask(SIG_UNBLOCK, &block, NULL);

			currentThread->block_type= 0;

			if(roundRobin){
				alarm(quantum);
			}
			
		}else{

			sigset_t block;
			sigemptyset(&block);
			sigaddset(&block, SIGALRM);
			sigprocmask(SIG_BLOCK, &block, NULL);

			mctx_restore(&mctx_caller);

		}

	}

}

int pmtRunThread(){

	while(!queueEmpty(threadQueue)){

		currentThread= (thread_t*)queueFront(threadQueue);

		if(currentThread->status == PMT_READY){

			queuePop(threadQueue);
			threadExecution= true;

			if(roundRobin && currentThread->block_type == 0){
				alarm(quantum);
			}

			mctx_switch(&mctx_caller, currentThread->ctx);
			threadExecution= false;

			time(&(currentThread->lastRun));

			if(roundRobin){
				alarm(0);
			}

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
			thr= threadPool[i];
			break;
		}

	thr->priority= priority;
	
	//INICIO DEL CÓDIGO VERGONZOSO
	queueFree(threadQueue);
	threadQueue= queueAlloc(scheduler);

	for(i= 0; i<MAX_THREAD; ++i)
		if(threadPool[i] != NULL){
			queuePushBack(threadQueue, threadPool[i]);
			
		}
	//FIN DEL CÓDIGO VERGONZOSO

	return PMT_OK;
}


int pmtSetupScheduler(PMT_OPTION option, int parameter){

	if(option == PMT_SETUP_QUANTUM){

		roundRobin= true;
		quantum= (unsigned)parameter;
		signal(SIGALRM, alarmHandler);

	}else if(option == PMT_SETUP_SCHEDULER){

		if(parameter == PMT_FIFO)
			scheduler= NULL;

		else if(parameter == PMT_ROUND_ROBIN)
			scheduler= NULL;

		else if(parameter == PMT_PRIORIY)
			scheduler= priorities;

		else if(parameter == PMT_PRIORIY_AGING)
			scheduler= prioritiesAging;

		queueSetupComparisonFunction(threadQueue, scheduler);

	}

	return PMT_OK;

}
