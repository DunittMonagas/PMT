
#include <pmt.h>
#include <stdio.h>
#include <stdlib.h>

void thread1(void *data){

	int i;
	for(i= 0; i<5; ++i){

		printf("Hey, I'm thread #1: %d\n", i);
		pmtYield();
		
	}

}

void fibonacchi(void *data){

	int i;
	int fib[2]= { 0, 1 };
	
	/*sleep( 2 ); */
	printf("fibonacchi(0) = 0\nfibonnachi(1) = 1\n");
	for(i= 2; i<15; ++i){

		int nextFib= fib[0] + fib[1];
		printf("fibonacchi(%d) = %d\n", i, nextFib);
		fib[0]= fib[1];
		fib[1]= nextFib;
		pmtYield();

	}

}

void squares(void *data){

	int i;
	
	/*sleep( 5 ); */
	for (i= 0; i<10; ++ i){

		printf("%d*%d = %d\n", i, i, i*i );
		pmtYield();

	}

}

int main(){

	pmtID id1, id2, id3;

	char str1[]="Thread 1";
	char str2[]="Thread 2";
	char str3[]="Thread 3";


	pmtInitialize();

	pmtCreateThread(&id1, &thread1, (void*)str1);
	printf("Hilo %d creado.\n\n", id1);

	pmtCreateThread(&id2, &fibonacchi, (void*)str2);
	printf("Hilo %d creado.\n\n", id2);

	pmtCreateThread(&id3, &squares, (void*)str3);
	printf("Hilo %d creado.\n\n", id3);

	/*
	printf("ID: %d\n", id1);
	printf("ID: %d\n", id2);
	printf("ID: %d\n", id3);
	*/

	printf("Ejecución del hilo %d\n\n", id1);
	pmtRunThread();
	printf("TERMINÓ %d\n", id1);
/*
	printf("Ejecución del hilo %d\n\n", id2);
	pmtRunThread(id2);
	printf("TERMINÓ %d\n", id2);

	printf("Ejecución del hilo %d\n\n", id3);
	pmtRunThread(id3);
	printf("TERMINÓ %d\n", id3);
*/

	pmtTerminate();

	return 0;
}