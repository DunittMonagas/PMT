

#include <signal.h>
#include <setjmp.h>

/* machine context data structure */
typedef struct mctx_st{
	jmp_buf jb;
}mctx_t;

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


void mctx_create(mctx_t *mctx, void (*sf_addr)(void *), void *sf_arg, void *sk_addr, size_t sk_size);
void mctx_create_trampoline(int sig);
void mctx_create_boot();

int pmtInit();
int pmtYield();
int pmtRun();
int pmtConfigThread();
int pmtConfigScheduler();