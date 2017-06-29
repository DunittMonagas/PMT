
#include <pmt.h>
#include <stdio.h>
#include <unistd.h>



void thread1(void *data){

	int i;
	for(i= 0; i<5; ++i){

		printf("Hey, I'm thread #1: %d\n", i);
		//printf("CEDER\n");
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
		//printf("CEDER\n");
		//pmtYield();

	}

}

void squares(void *data){

	int i;
	for (i= 0; i<10; ++ i){

		printf("%d*%d = %d\n", i, i, i*i );
		//printf("CEDER\n");
		//pmtYield();

	}

}


int main(){


	pmtID id1, id2, id3;

	char str1[]="THREAD 1";
	char str2[]="THREAD 2";
	char str3[]="THREAD 3";

	pmtInitialize();

	pmtSetupScheduler(PMT_SETUP_SCHEDULER, PMT_PRIORIY);

	pmtCreateThread(&id1, &thread1, (void*)str1);
	printf("Hilo %d creado.\n", id1);

	pmtCreateThread(&id2, &fibonacchi, (void*)str2);
	printf("Hilo %d creado.\n", id2);

	pmtCreateThread(&id3, &squares, (void*)str3);
	printf("Hilo %d creado.\n", id3);

	printf("ESTABLECIENDO PRIORIDADES\n");	
	pmtSetupThread(id1, 2);
	pmtSetupThread(id2, 12);
	pmtSetupThread(id3, 5);

	printf("EJECUCIÓN\n");
	pmtRunThread();
	printf("TERMINÓ\n");

	pmtTerminate();

	return 0;
}