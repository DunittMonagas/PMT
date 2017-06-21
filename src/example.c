
#include <pmt.h>
#include <stdio.h>
#include <stdlib.h>

void function(void *data){

	char *str= (char*)data;
	printf("Este es el hilo\n");
	printf("Estos son los datos: %s\n", str);
	
}


int main(){

	pmtId id1, id2, id3, id4;

	char str1[]="Sample string 1";
	char str2[]="Sample string 2";
	char str3[]="Sample string 3";
	char str4[]="Sample string 4";


	pmtInitialize();

	pmtCreateThread(&id1, &function, (void*)str1);
	printf("Hilo %d creado.\n\n", id1);

	pmtCreateThread(&id2, &function, (void*)str2);
	printf("Hilo %d creado.\n\n", id2);

	pmtCreateThread(&id3, &function, (void*)str3);
	printf("Hilo %d creado.\n\n", id3);

	pmtCreateThread(&id4, &function, (void*)str4);
	printf("Hilo %d creado.\n\n", id4);

	/*
	printf("ID: %d\n", id1);
	printf("ID: %d\n", id2);
	printf("ID: %d\n", id3);
	printf("ID: %d\n", id4);
	*/

	printf("Ejecución del hilo %d\n\n", id1);
	pmtRunThread(id1);
/*
	printf("Ejecución del hilo %d\n\n", id2);
	pmtRunThread(id2);

	printf("Ejecución del hilo %d\n\n", id3);
	pmtRunThread(id3);

	printf("Ejecución del hilo %d\n\n", id4);
	pmtRunThread(id4);
	pmtTerminate();
*/

	return 0;
}