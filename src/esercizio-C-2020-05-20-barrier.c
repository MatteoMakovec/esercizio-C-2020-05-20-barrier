#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>


#define N 5

pthread_barrier_t thread_barrier;
sem_t mutex;

int number_of_threads = N;
int count;

#define CHECK_ERR(a,msg) {if ((a) == -1) { perror((msg)); exit(EXIT_FAILURE); } }

void * thread_function(void * arg) {
	int s;

	printf("rendezvous\n");

	if (sem_wait(&mutex) == -1) {
		perror("sem_wait");
		exit(EXIT_FAILURE);
	}

	count++;

	if (sem_post(&mutex) == -1) {
		perror("sem_post");
		exit(EXIT_FAILURE);
	}

	s = pthread_barrier_wait(&thread_barrier);

	printf("critical point\n");

	return NULL;
}


int main() {
	int s;
	pthread_t threads[N];

	s = sem_init(&mutex, 0, 1);

	s = pthread_barrier_init(&thread_barrier, NULL, N);
	CHECK_ERR(s,"pthread_barrier_init")

	for (int i=0; i < number_of_threads; i++) {
		s = pthread_create(&threads[i], NULL, thread_function, NULL);

		if (s != 0) {
			perror("pthread_create");
			exit(EXIT_FAILURE);
		}
	}

	for (int i=0; i < number_of_threads; i++) {
		s = pthread_join(threads[i], NULL);

		if (s != 0) {
			perror("pthread_join");
			exit(EXIT_FAILURE);
		}

	}

	s = pthread_barrier_destroy(&thread_barrier);
	CHECK_ERR(s,"pthread_barrier_destroy")

	printf("\ncount: %d", count);

	return 0;
}
