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

int maxD(char * file, char * buffer){

	// sem_t * sem = sem_open(SEM_NAME, O_CREAT, 0644, 1);

	const int SIZE = 4096;
	const char * name = "MEM";
	const char * name2 = "MEM2";
	int shm_fd, shm_fd2;
	void * ptr;
	void * ptr2;

	shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
	shm_fd2 = shm_open(name2, O_CREAT | O_RDWR, 0666);

	ftruncate(shm_fd, SIZE);
	ftruncate(shm_fd2, SIZE);

	ptr = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);
	ptr2 = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd2, 0);

	FILE * ptr_file;
	char line[1000];

	ptr_file = fopen(file, "r");

	if (!ptr_file){
		return -1;
	}

	int * array = (int*)malloc(sizeof(int));
	int i = 1;


	while(fgets(line, 1000, ptr_file)){

		if (line[strlen(line) - 1] != '\n'){
			// im stupid
			//line[strlen(line)] == '\0';
		}
		else {
			line[strlen(line)-1] = '\0';
		}

		int value = atoi(line);

		array = (int*)realloc(array, i*sizeof(int));
		array[i-1] = value;
		i++;
	}

	fclose(ptr_file);
	// each child can create at most 2 children, so the structure grows like a binary tree

	i = i-1;

	// printf("There are %d elements\n", i);

	int iterations = 6;
	int num = i / iterations;

	if (i <= iterations){
		int m = max(array, i);
		printf("The maximum is %d\n", m);
		return m;
	}

	// printf("num is %d\n", num);

	int rem = i % iterations;
	if (rem != 0){
		iterations++;
	}

	// printf("%d iterations\n", iterations);
	// printf("%d remaining\n", rem);

	int j;
	int last = 0;
	int index = 0;

	sprintf(ptr2, "%d", array[0]);

	while (1){

		int status;
		// int current = atoi((char*)ptr);

		if (2*index + 1 > iterations ){
			// printf("%d\n", index);
			if (index >= iterations){
				// extraneous node, do nothing
				sleep(1);
				exit(0);
			}


			int * subArray;
			int subMax;
			last = index * num;

			if (index == iterations - 1 && rem != 0){
				subArray = (int*)malloc(rem*sizeof(int));
				memcpy(&subArray[0], &array[last], rem * sizeof(int));
				subMax = max(subArray, rem);
				// printf("I go from %d to %d, which is %d to %d\n", last, last + rem - 1, array[last], array[last + rem - 1]);
				// printf("MAX HERE IS %d\n", subMax);
			}
			else {
				subArray = (int*)malloc(num*sizeof(int));
				memcpy(&subArray[0], &array[last], num * sizeof(int));
				subMax = max(subArray, num);
				// printf("I go from %d to %d, which is %d to %d\n", last, last + num - 1, array[last], array[last+num - 1]);
				// printf("MAX HERE IS %d\n", subMax);
			}

			//sem_wait(sem);
			int current = atoi((char*)ptr2);
			// printf("------ %d or %d -----\n", current, subMax);

			if (current < subMax){
				sprintf(ptr2, "%d", subMax);
			}

			//sem_post(sem);
			if (index == iterations - 1){
				sleep(1);
				int max = atoi((char*)ptr2);
				printf("The maximum is %d\n", max);
				//sem_close(sem);
				return max;
			}
			sleep(1);
			exit(0);
		}
		// sprintf(ptr, "%d", current + 2);

		pid_t pid2;
		pid_t pid1;

		pid1 = fork();

		if (pid1 < 0){
			printf("Forking error...\n");

		}
		else if (pid1 == 0){
			// child, aka left node
			index = 2*index + 1;
			sprintf(buffer, "%sHi, my PID is %d, and my parent PID is %d\n", buffer, getpid(), getppid());
		}
		else {  // parent process
			pid2 = fork();

			if (pid2 < 0){
				printf("Forking error...\n");
			}
			else if (pid2 == 0){
				// child, aka right node
				index = 2*index + 2;
				sprintf(buffer, "%sHi, my PID is %d, and my parent PID is %d\n", buffer, getpid(), getppid());
			}
			else {


				// printf("%d\n", index);
				// parent process, aka current node
				// do some work, and then break

				int * subArray;
				int subMax;
				last = index * num;

				if (index == iterations - 1 && rem != 0){
					subArray = (int*)malloc(rem*sizeof(int));
					memcpy(&subArray[0], &array[last], rem * sizeof(int));
					subMax = max(subArray, rem);
					// printf("I go from %d to %d, which is %d to %d\n", last, last + rem - 1, array[last], array[last + rem - 1]);
					// printf("MAX HERE IS %d\n", subMax);
				}
				else {
					subArray = (int*)malloc(num*sizeof(int));
					memcpy(&subArray[0], &array[last], num * sizeof(int));
					subMax = max(subArray, num);
					// printf("I go from %d to %d, which is %d to %d\n", last, last + num - 1, array[last], array[last+num - 1]);
					// printf("MAX HERE IS %d\n", subMax);
				}

				//sem_wait(sem);

				int current = atoi((char*)ptr2);
				// printf("------ %d or %d -----\n", current, subMax);

				if (current < subMax){
					sprintf(ptr2, "%d", subMax);
				}

				if (index == 0){
					int k;
					for (k = 1; k < i - 1; k++){
						wait(NULL);
						// printf("Reaped %d\n", k);
					}
					break;
				}
				sleep(1);
				exit(0);
			}
		}
	}

	if (index == 0){
		exit(0);
	}
	exit(-1);


}

int minD(char * file, char * buffer){

	// sem_t * sem = sem_open(SEM_NAME, O_CREAT, 0644, 1);

	const int SIZE = 4096;
	const char * name = "MEM";
	const char * name2 = "MEM2";
	int shm_fd, shm_fd2;
	void * ptr;
	void * ptr2;

	shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
	shm_fd2 = shm_open(name2, O_CREAT | O_RDWR, 0666);

	ftruncate(shm_fd, SIZE);
	ftruncate(shm_fd2, SIZE);

	ptr = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);
	ptr2 = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd2, 0);

	FILE * ptr_file;
	char line[1000];

	ptr_file = fopen(file, "r");

	if (!ptr_file){
		return -1;
	}

	int * array = (int*)malloc(sizeof(int));
	int i = 1;


	while(fgets(line, 1000, ptr_file)){

		if (line[strlen(line) - 1] != '\n'){
			// im stupid
			//line[strlen(line)] == '\0';
		}
		else {
			line[strlen(line)-1] = '\0';
		}

		int value = atoi(line);

		array = (int*)realloc(array, i*sizeof(int));
		array[i-1] = value;
		i++;
	}

	fclose(ptr_file);
	// each child can create at most 2 children, so the structure grows like a binary tree

	i = i-1;

	// printf("There are %d elements\n", i);

	int iterations = 6;
	int num = i / iterations;

	if (i <= iterations){
		int m = min(array, i);
		printf("The minimum is %d\n", m);
		return m;
	}

	// printf("num is %d\n", num);

	int rem = i % iterations;
	if (rem != 0){
		iterations++;
	}

	// printf("%d iterations\n", iterations);
	// printf("%d remaining\n", rem);

	int j;
	int last = 0;
	int index = 0;

	sprintf(ptr2, "%d", array[0]);

	while (1){

		int status;
		// int current = atoi((char*)ptr);

		if (2*index + 1 > iterations ){
			// printf("%d\n", index);
			if (index >= iterations){
				// extraneous node, do nothing
				sleep(1);
				exit(0);
			}

			int * subArray;
			int subMin;
			last = index * num;

			if (index == iterations - 1 && rem != 0){
				subArray = (int*)malloc(rem*sizeof(int));
				memcpy(&subArray[0], &array[last], rem * sizeof(int));
				subMin = min(subArray, rem);
				// printf("I go from %d to %d, which is %d to %d\n", last, last + rem - 1, array[last], array[last + rem - 1]);
				// printf("MAX HERE IS %d\n", subMax);
			}
			else {
				subArray = (int*)malloc(num*sizeof(int));
				memcpy(&subArray[0], &array[last], num * sizeof(int));
				subMin = min(subArray, num);
				// printf("I go from %d to %d, which is %d to %d\n", last, last + num - 1, array[last], array[last+num - 1]);
				// printf("MAX HERE IS %d\n", subMax);
			}

			//sem_wait(sem);
			int current = atoi((char*)ptr2);
			// printf("------ %d or %d -----\n", current, subMax);

			if (current > subMin){
				sprintf(ptr2, "%d", subMin);
			}

			//sem_post(sem);
			if (index == iterations - 1){
				sleep(1);
				int min = atoi((char*)ptr2);
				printf("The minimum is %d\n", min);
				//sem_close(sem);
				return min;
			}
			sleep(1);
			exit(0);
		}
		// sprintf(ptr, "%d", current + 2);

		pid_t pid2;
		pid_t pid1;

		pid1 = fork();

		if (pid1 < 0){
			printf("Forking error...\n");

		}
		else if (pid1 == 0){
			// child, aka left node
			index = 2*index + 1;
			sprintf(buffer, "%sHi, my PID is %d, and my parent PID is %d\n", buffer, getpid(), getppid());
		}
		else {  // parent process
			pid2 = fork();

			if (pid2 < 0){
				printf("Forking error...\n");
			}
			else if (pid2 == 0){
				// child, aka right node
				index = 2*index + 2;
				sprintf(buffer, "%sHi, my PID is %d, and my parent PID is %d\n", buffer, getpid(), getppid());
			}
			else {

				// printf("%d\n", index);
				// parent process, aka current node
				// do some work, and then break

				int * subArray;
				int subMin;
				last = index * num;

				if (index == iterations - 1 && rem != 0){
					subArray = (int*)malloc(rem*sizeof(int));
					memcpy(&subArray[0], &array[last], rem * sizeof(int));
					subMin = min(subArray, rem);
					// printf("I go from %d to %d, which is %d to %d\n", last, last + rem - 1, array[last], array[last + rem - 1]);
					// printf("MAX HERE IS %d\n", subMax);
				}
				else {
					subArray = (int*)malloc(num*sizeof(int));
					memcpy(&subArray[0], &array[last], num * sizeof(int));
					subMin = min(subArray, num);
					// printf("I go from %d to %d, which is %d to %d\n", last, last + num - 1, array[last], array[last+num - 1]);
					// printf("MAX HERE IS %d\n", subMax);
				}

				//sem_wait(sem);

				int current = atoi((char*)ptr2);
				// printf("------ %d or %d -----\n", current, subMax);

				if (current > subMin){
					sprintf(ptr2, "%d", subMin);
				}

				if (index == 0){
					int k;
					for (k = 1; k < i - 1; k++){
						wait(NULL);
						// printf("Reaped %d\n", k);
					}
					break;
				}
				sleep(1);
				exit(0);
			}
		}
	}

	if (index == 0){
		exit(0);
	}
	exit(-1);
}

int sumD(char * file, char * buffer){

	// sem_t * sem = sem_open(SEM_NAME, O_CREAT, 0644, 1);

	const int SIZE = 4096;
	const char * name = "MEM";
	const char * name2 = "MEM2";
	int shm_fd, shm_fd2;

	void * ptr2;

	shm_fd2 = shm_open(name2, O_CREAT | O_RDWR, 0666);

	ftruncate(shm_fd2, SIZE);

	ptr2 = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd2, 0);

	FILE * ptr_file;
	char line[1000];

	ptr_file = fopen(file, "r");

	if (!ptr_file){
		return -1;
	}

	int * array = (int*)malloc(sizeof(int));
	int i = 1;


	while(fgets(line, 1000, ptr_file)){

		if (line[strlen(line) - 1] != '\n'){
			// im stupid
			//line[strlen(line)] == '\0';
		}
		else {
			line[strlen(line)-1] = '\0';
		}

		int value = atoi(line);

		array = (int*)realloc(array, i*sizeof(int));
		array[i-1] = value;
		i++;
	}

	fclose(ptr_file);
	// each child can create at most 2 children, so the structure grows like a binary tree

	i = i-1;

	// printf("There are %d elements\n", i);

	int iterations = 6;
	int num = i / iterations;

	if (i <= iterations){
		int m = sum(array, i);
		printf("The sum is %d\n", m);
		return m;
	}

	// printf("num is %d\n", num);

	int rem = i % iterations;
	if (rem != 0){
		iterations++;
	}

	// printf("%d iterations\n", iterations);
	// printf("%d remaining\n", rem);

	int j;
	int last = 0;
	int index = 0;

	sprintf(ptr2, "%d", 0);

	while (1){

		int status;
		// int current = atoi((char*)ptr);

		if (2*index + 1 > iterations ){
			// printf("%d\n", index);
			if (index >= iterations){
				// extraneous node, do nothing
				sleep(1);
				exit(0);
			}

			int * subArray;
			int subSum;
			last = index * num;

			if (index == iterations - 1 && rem != 0){
				subArray = (int*)malloc(rem*sizeof(int));
				memcpy(&subArray[0], &array[last], rem * sizeof(int));
				subSum = sum(subArray, rem);
				// printf("I go from %d to %d, which is %d to %d\n", last, last + rem - 1, array[last], array[last + rem - 1]);
				// printf("SUM HERE IS %d\n", subSum);
			}
			else {
				subArray = (int*)malloc(num*sizeof(int));
				memcpy(&subArray[0], &array[last], num * sizeof(int));
				subSum = sum(subArray, num);
				// printf("I go from %d to %d, which is %d to %d\n", last, last + num - 1, array[last], array[last+num - 1]);
				// printf("SUM HERE IS %d\n", subSum);
			}

			// sem_wait(sem);
			int current = atoi((char*)ptr2);
			// printf("------ %d or %d -----\n", current, subMax);

			sprintf(ptr2, "%d", subSum + current);

			// sem_post(sem);

			if (index == iterations - 1){
				sleep(1);
				/*
				int k;
				for (k = 1; k < iterations; k++){
					wait(NULL);
					printf("Oi\n");
					// printf("Reaped %d\n", k);
				}
				 */
				int sum = atoi((char*)ptr2);
				printf("The sum is %d\n", sum);
				// sem_close(sem);
				return sum;
			}
			sleep(1);
			exit(0);
		}
		// sprintf(ptr, "%d", current + 2);

		pid_t pid2;
		pid_t pid1;

		pid1 = fork();

		if (pid1 < 0){
			printf("Forking error...\n");

		}
		else if (pid1 == 0){
			// child, aka left node
			index = 2*index + 1;
			sprintf(buffer, "%sHi, my PID is %d, and my parent PID is %d\n", buffer, getpid(), getppid());
		}
		else {  // parent process
			pid2 = fork();

			if (pid2 < 0){
				printf("Forking error...\n");
			}
			else if (pid2 == 0){
				// child, aka right node
				index = 2*index + 2;
				sprintf(buffer, "%sHi, my PID is %d, and my parent PID is %d\n", buffer, getpid(), getppid());
			}
			else {

				// printf("%d\n", index);
				// parent process, aka current node
				// do some work, and then break

				int * subArray;
				int subSum;
				last = index * num;

				if (index == iterations - 1 && rem != 0){
					subArray = (int*)malloc(rem*sizeof(int));
					memcpy(&subArray[0], &array[last], rem * sizeof(int));
					subSum = sum(subArray, rem);
					// printf("I go from %d to %d, which is %d to %d\n", last, last + rem - 1, array[last], array[last + rem - 1]);
					// printf("SUM HERE IS %d\n", subSum);
				}
				else {
					subArray = (int*)malloc(num*sizeof(int));
					memcpy(&subArray[0], &array[last], num * sizeof(int));
					subSum = sum(subArray, num);
					// printf("I go from %d to %d, which is %d to %d\n", last, last + num - 1, array[last], array[last+num - 1]);
					// printf("SUM HERE IS %d\n", subSum);
				}

				// sem_wait(sem);

				int current = atoi((char*)ptr2);
				// printf("------ %d or %d -----\n", current, subMax);

				sprintf(ptr2, "%d", subSum + current);

				// sem_post(sem);

				sleep(1);
				exit(0);
			}
		}
	}

	if (index == 0){
		exit(0);
	}
	exit(-1);
}

int D(char * file){

	int shm_fd;
	char * buffer;

	shm_fd = shm_open("TEMP", O_CREAT | O_RDWR, 0666);
	ftruncate(shm_fd, 10000);
	buffer = mmap(0, 10000, PROT_WRITE, MAP_SHARED, shm_fd, 0);

	FILE * output = fopen("resultD.txt", "w");

	sprintf(buffer, "");

	int max_D = maxD(file, buffer);

	sprintf(buffer, "%s\nMax = %d\n\n", buffer, max_D);

	int min_D = minD(file, buffer);

	sprintf(buffer, "%s\nMin = %d\n\n", buffer, min_D);

	int sum_D = sumD(file, buffer);

	sprintf(buffer, "%s\nSum = %d\n\n", buffer, sum_D);

	fprintf(output, buffer);

	fclose(output);

	return 0;

}

