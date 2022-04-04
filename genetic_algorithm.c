#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "genetic_algorithm.h"
#include <math.h>


int read_input(sack_object **objects, int *object_count, int *sack_capacity, int *generations_count, int argc, char *argv[])
{
	FILE *fp;

	if (argc < 3) {
		fprintf(stderr, "Usage:\n\t./tema1 in_file generations_count\n");
		return 0;
	}

	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		return 0;
	}

	if (fscanf(fp, "%d %d", object_count, sack_capacity) < 2) {
		fclose(fp);
		return 0;
	}

	if (*object_count % 10) {
		fclose(fp);
		return 0;
	}

	sack_object *tmp_objects = (sack_object *) calloc(*object_count, sizeof(sack_object));

	for (int i = 0; i < *object_count; ++i) {
		if (fscanf(fp, "%d %d", &tmp_objects[i].profit, &tmp_objects[i].weight) < 2) {
			free(objects);
			fclose(fp);
			return 0;
		}
	}

	fclose(fp);

	*generations_count = (int) strtol(argv[2], NULL, 10);
	
	if (*generations_count == 0) {
		free(tmp_objects);

		return 0;
	}

	*objects = tmp_objects;

	return 1;
}

void print_objects(const sack_object *objects, int object_count)
{
	for (int i = 0; i < object_count; ++i) {
		printf("%d %d\n", objects[i].weight, objects[i].profit);
	}
}

void print_generation(const individual *generation, int limit)
{
	for (int i = 0; i < limit; ++i) {
		for (int j = 0; j < generation[i].chromosome_length; ++j) {
			printf("%d ", generation[i].chromosomes[j]);
		}

		printf("\n%d - %d\n", i, generation[i].fitness);
	}
}

void print_best_fitness(const individual *generation)
{
	printf("%d\n", generation[0].fitness);
}


void compute_fitness_function(const sack_object *objects, individual *generation, int object_count, int sack_capacity, int id, int numThread)
{
	int weight;
	int profit;
	int threadID = id;
	int N = object_count;
	int start = threadID * ceil((double) N) / numThread;
	int end = fmin((threadID + 1)* (double) N / numThread , N);

	for (int i = start; i < end; ++i) {
		weight = 0;
		profit = 0;

		for (int j = 0; j < generation[i].chromosome_length; ++j) {
			if (generation[i].chromosomes[j]) {
				weight += objects[j].weight;
				profit += objects[j].profit;
			}
		}

		generation[i].fitness = (weight <= sack_capacity) ? profit : 0;
	}
}

int cmpfunc(const void *a, const void *b)
{
	int i;
	individual *first = (individual *) a;
	individual *second = (individual *) b;

	int res = second->fitness - first->fitness; // decreasing by fitness
	if (res == 0) {
		int first_count = 0, second_count = 0;

		for (i = 0; i < first->chromosome_length && i < second->chromosome_length; ++i) {
			first_count += first->chromosomes[i];
			second_count += second->chromosomes[i];
		}

		res = first_count - second_count; // increasing by number of objects in the sack
		if (res == 0) {
			return second->index - first->index;
		}
	}

	return res;
}

void mutate_bit_string_1(const individual *ind, int generation_index)
{
	int i, mutation_size;
	int step = 1 + generation_index % (ind->chromosome_length - 2);

	if (ind->index % 2 == 0) {
		// for even-indexed individuals, mutate the first 40% chromosomes by a given step
		mutation_size = ind->chromosome_length * 4 / 10;
		for (i = 0; i < mutation_size; i += step) {
			ind->chromosomes[i] = 1 - ind->chromosomes[i];
		}
	} else {
		// for even-indexed individuals, mutate the last 80% chromosomes by a given step
		mutation_size = ind->chromosome_length * 8 / 10;
		for (i = ind->chromosome_length - mutation_size; i < ind->chromosome_length; i += step) {
			ind->chromosomes[i] = 1 - ind->chromosomes[i];
		}
	}
}

void mutate_bit_string_2(const individual *ind, int generation_index)
{
	int step = 1 + generation_index % (ind->chromosome_length - 2);

	// mutate all chromosomes by a given step
	for (int i = 0; i < ind->chromosome_length; i += step) {
		ind->chromosomes[i] = 1 - ind->chromosomes[i];
	}
}

void crossover(individual *parent1, individual *child1, int generation_index)
{
	individual *parent2 = parent1 + 1;
	individual *child2 = child1 + 1;
	int count = 1 + generation_index % parent1->chromosome_length;

	memcpy(child1->chromosomes, parent1->chromosomes, count * sizeof(int));
	memcpy(child1->chromosomes + count, parent2->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));

	memcpy(child2->chromosomes, parent2->chromosomes, count * sizeof(int));
	memcpy(child2->chromosomes + count, parent1->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));
}

void copy_individual(const individual *from, const individual *to)
{
	memcpy(to->chromosomes, from->chromosomes, from->chromosome_length * sizeof(int));
}

void free_generation(individual *generation)
{
	int i;

	for (i = 0; i < generation->chromosome_length; ++i) {
		free(generation[i].chromosomes);
		generation[i].chromosomes = NULL;
		generation[i].fitness = 0;
	}
}
void merge(individual *source, int start, int mid, int end, individual *destination) {
	int x = start;
	int y = mid;
	int i;

	for (i = start; i < end; i++) {
		if (end == y || (x < mid && source[x].fitness > source[y].fitness)) {
			destination[i] = source[x];
			x++;
		} else {
			destination[i] = source[y];
			y++;
		}
	}
}

void mergeSort(individual *v, int id, int object_count, int numThreads,individual *vNew, pthread_barrier_t *barrier) {

	int threadID = id;
	int N = object_count;
	int P = numThreads;

	for (int i = 1; i < N; i = i * 2) {
		int numberOfOps = N / (i * 2);
		int start = threadID * numberOfOps / P * 2 * i;
		int end = fmin((threadID + 1) * numberOfOps / P * 2 * i, N);

		for (int j = start; j < end; j += 2 * i) {
			merge(v, j, j + i, j + 2 * i, vNew);
		}
		pthread_barrier_wait(barrier);
		if (threadID == 0) {
			memcpy(v, vNew, object_count*sizeof(individual));
		}
		pthread_barrier_wait(barrier);
	}
}

void *run_genetic_algorithm(void *arg){
	argThread *args = (argThread*) arg;
	//extragerea argumentelor trimise functiei de threaduri
	//id-ul fiecarui thread
	int threadID = args->id;
	//numarul de elemente
	int N = args->object_count;
	//numarul de threaduri
	int numThread = args->numThreads;

	//calculare start si end pentru paralelizare
	int start = threadID * ceil((double) N) / numThread;
	int end = fmin((threadID + 1)* (double) N / numThread , N);

 	individual *tmp = NULL;
	
	for (int i = start; i < end; i++) {
		(*(args->current_generation))[i].fitness = 0;	
		(*(args->current_generation))[i].chromosomes[i] = 1;
		(*(args->current_generation))[i].index = i;
		(*(args->current_generation))[i].chromosome_length = args->object_count;
		(*(args->next_generation))[i].fitness = 0;
		(*(args->next_generation))[i].index = i;
		(*(args->next_generation))[i].chromosome_length = args->object_count;
	}
	
	pthread_barrier_wait(args->barrier);

	int count, cursor;
	// iterate for each generation
	for (int k = 0; k < args->generations_count; ++k) {
		cursor = 0;
		// compute fitness and sort by it
		//functie paralelizata	
		compute_fitness_function(args->objects, (*(args->current_generation)), args->object_count, args->sack_capacity, args->id, args->numThreads);
		pthread_barrier_wait(args->barrier);
		//sortarea indivizi in functie de fitness
		mergeSort((*(args->current_generation)), threadID, N, numThread,  (*(args->new_Vector)), args->barrier);	
		pthread_barrier_wait(args->barrier);
		//pentru un singur thread se vor executa secvential restul operatiilor
		if (args->id == 0) {	
			count = args->object_count * 3 / 10;
			for (int i = 0; i < count; ++i) {
				copy_individual((*(args->current_generation)) + i, (*(args->next_generation)) + i);
			}

			cursor = count;	
			// mutate first 20% children with the first version of bit string mutation
			count = args->object_count * 2 / 10;

			for (int i = 0; i < count; ++i) {
				copy_individual((*(args->current_generation)) + i, (*(args->next_generation)) + cursor + i);
			

				mutate_bit_string_1((*(args->next_generation)) + cursor + i, k);
			
			}
			cursor += count;
			// mutate next 20% children with the second version of bit string mutation
			count = args->object_count * 2 / 10;
		

			for (int i = 0; i < count; ++i) {
				copy_individual((*(args->current_generation)) + i + count, (*(args->next_generation)) + cursor + i);
			
				mutate_bit_string_2((*(args->next_generation)) + cursor + i, k);
			
			}
		
			cursor += count;
			// crossover first 30% parents with one-point crossover
			// (if there is an odd number of parents, the last one is kept as such)
			count = args->object_count * 3 / 10;

			if (count % 2 == 1) {
				copy_individual((*(args->current_generation)) + args->object_count - 1, (*(args->next_generation)) + cursor + count - 1);
				count--;
			}
		
			for (int i = 0; i < count; i += 2) {
				crossover((*(args->current_generation)) + i, (*(args->next_generation)) + cursor + i, k);
			
			}
		
			// switch to new generation
			tmp = (*(args->current_generation));
			(*(args->current_generation)) = (*(args->next_generation));
			(*(args->next_generation)) = tmp;
	

		
			for (int i = 0; i < count; ++i) {
				(*(args->current_generation))[i].index = i;
			}
		
			if (k % 5 == 0) {
				print_best_fitness((*(args->current_generation)));
			}
		}
		pthread_barrier_wait(args->barrier);
	}
	//a treia generatie
	pthread_barrier_wait(args->barrier);
	//se calculatea noul fitness
	compute_fitness_function(args->objects, (*(args->current_generation)), args->object_count, args->sack_capacity, args->id, args->numThreads);
	//se asteapta toate threadurile
	pthread_barrier_wait(args->barrier);
	//se sorteaza indivizii in functie de fitness
	mergeSort((*(args->current_generation)), threadID, N, numThread,  (*(args->new_Vector)), args->barrier);
	pthread_barrier_wait(args->barrier);
	//pentru un thread se va afisa rezultatul
	if (args->id == 0)
		print_best_fitness((*(args->current_generation)));

	// free resources for old generation
	// free_generation((*(args->current_generation)));
	// free_generation((*(args->next_generation)));

	return NULL;
}