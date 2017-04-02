/*
 * program.c
 *
 *  Created on: Feb 21, 2017
 *      Author: RyanMini
 */

// #include "parta.c"
// #include "partb.c"
// #include "partc.c"
// #include "partd.c"

#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <pthread.h>

#define SEM_NAME "/sem"

int max(int * array, int size){

	int max = array[0];
	int i;
	for (i = 0; i < size; i++){
		//printf("%d, ", array[i]);
		if (array[i] > max){
			max = array[i];
		}
	}
	//printf("\n");
	return max;
}

int min(int * array, int size){

	int min = array[0];
	int i;
	for (i = 0; i < size; i++){
		if (array[i] < min){
			min = array[i];
		}
	}
	return min;
}

int sum(int * array, int size){

	int sum = 0;
	int i;
	for (i = 0; i < size; i++){
		sum += array[i];
	}
	return sum;

}

int main(void){

	struct timeval t0, t1, t2, t3, t4;
	time_t curtime;

	gettimeofday(&t3, 0);

	D("test3.txt");
	printf("\n");

	gettimeofday(&t4, 0);
	long elapsedD = ((t4.tv_sec-t3.tv_sec)*1000000 + t4.tv_usec-t3.tv_usec)/1000 - 3000;

	printf("\n");

	if (elapsedD == 0){
		printf("D took < 1 millisecond\n");
	}
	else {
		printf("D took %ld milliseconds\n", elapsedD);
	}






	return 0;


}
