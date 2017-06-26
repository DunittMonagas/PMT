
#ifndef __PMT_H__
#define __PMT_H__

#include <signal.h>
#include <setjmp.h>

/* machine context data structure */
typedef struct mctx_st{
	jmp_buf jb;
}mctx_t;

typedef unsigned int pmtID;

#define PMT_OK 0
#define	PMT_THREAD_LIMIT_EXCEEDED 1
#define PMT_INVALID_THREAD 2

typedef enum STATUS{

	PMT_INVALID= 0,
	PMT_READY,
	PMT_FINISHED

}PMT_STATUS;


#define MAX_THREAD 10

/* save machine context */
#define mctx_save(mctx) \
	setjmp((mctx)->jb)

/* restore machine context */
#define mctx_restore(mctx) \
	longjmp((mctx)->jb, 1)

/* switch machine context */
#define mctx_switch(mctx_old,mctx_new) \
	if(setjmp((mctx_old)->jb) == 0) \
		longjmp((mctx_new)->jb, 1)

static mctx_t mctx_caller;
static sig_atomic_t mctx_called;

static mctx_t *mctx_creat;
static void (*mctx_creat_func)(void *);
static void *mctx_creat_arg;
static sigset_t mctx_creat_sigs;


static void mctx_create(mctx_t *mctx, void (*sf_addr)(void *), void *sf_arg, void *sk_addr, size_t sk_size);
static void mctx_create_trampoline(int sig);
static void mctx_create_boot();

int pmtInitialize();
int pmtTerminate();
int pmtCreateThread(pmtID *id, void (*func)(void*), void* arg);
void pmtYield();
int pmtRunThread();
int pmtSetupThread(pmtID id);
int pmtSetupScheduler(pmtID id);

#endif