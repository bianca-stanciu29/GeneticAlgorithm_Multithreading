#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "genetic_algorithm.h"
int main(int argc, char *argv[]) {
	// array with all the objects that can be placed in the sack
	sack_object *objects = NULL;

	// number of objects
	int object_count = 0;

	// maximum weight that can be carried in the sack
	int sack_capacity = 0;

	// number of generations
	int generations_count = 0;

	//numarul de threaduri dat ca argument 
	int num_threads = atoi(argv[3]);
	//vectorul de threaduri
	pthread_t tid[num_threads];
	//alocare memorie vectori de threaduri
	argThread* args = (argThread*) calloc(num_threads, sizeof(argThread));

	//citirea datelor de intrare ale programului
	if (!read_input(&objects, &object_count, &sack_capacity, &generations_count, argc, argv)) {
			return 0;
	}
	//initializare bariera
	pthread_barrier_t barrier;
	pthread_barrier_init(&barrier, NULL, num_threads);
	//alocarea spatiu pentru vectorii de current_generation, next_generation si new_vector
	individual *current_generation = (individual*) calloc(object_count, sizeof(individual));
	individual *next_generation = (individual*) calloc(object_count, sizeof(individual));
	individual *new_Vector = (individual*) calloc(object_count, sizeof(individual));
	//alocare spatiu pentru vectorii de cromozomi
	for (int i = 0; i < object_count; i++) {
		current_generation[i].chromosomes = (int*) calloc(object_count, sizeof(int));
		next_generation[i].chromosomes = (int*) calloc(object_count, sizeof(int));
		new_Vector[i].chromosomes = (int*) calloc(object_count, sizeof(int));
	}
	//parcurgere threaduri
	for (int i = 0; i < num_threads; i++) {
		//atribuire id
		args[i].id = i;
		//atribuire numar threaduri
		args[i].numThreads = num_threads;
		//atribuire date pentru fiecare individ
		args[i].objects = objects;
		args[i].object_count = object_count;
		args[i].sack_capacity = sack_capacity;
		args[i].generations_count = generations_count;
		//atribuire vectori
		args[i].current_generation = &current_generation;
		args[i].next_generation = &next_generation; 
		args[i].new_Vector = &new_Vector;
		//atribuire bariera
		args[i].barrier = &barrier;
		//creare threaduri
		pthread_create(&tid[i], NULL, run_genetic_algorithm, &args[i]);
	}
	
	for (int i = 0; i < num_threads; i++) {
		pthread_join(tid[i], NULL);
	}
	//distrugerea barierei dupa folosirea acesteia
	pthread_barrier_destroy(&barrier);
	free(objects);

	return 0;
}
