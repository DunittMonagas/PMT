
#include "pmt.h"



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

	/* The thread ‘‘magically’’ starts... */
	mctx_start_func(mctx_start_arg);

	/* NOTREACHED */
	abort();

}
