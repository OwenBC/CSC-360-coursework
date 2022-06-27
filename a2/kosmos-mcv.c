/*
 * kosmos-mcv.c (mutexes & condition variables)
 *
 * For UVic CSC 360, Summer 2022
 *
 * Here is some code from which to start.
 *
 * PLEASE FOLLOW THE INSTRUCTIONS REGARDING WHERE YOU ARE PERMITTED
 * TO ADD OR CHANGE THIS CODE. Read from line 133 onwards for
 * this information.
 */

#include <assert.h>
#include <pthread.h>
#include <sched.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "logging.h"


/* Random # below threshold indicates H; otherwise C. */
#define ATOM_THRESHOLD 0.55
#define DEFAULT_NUM_ATOMS 40

#define MAX_ATOM_NAME_LEN 10
#define MAX_KOSMOS_SECONDS 5

/* Global / shared variables */
int  cNum = 0, hNum = 0;
long numAtoms;


/* Function prototypes */
void kosmos_init(void);
void *c_ready(void *);
void *h_ready(void *);
void make_radical(int, int, int, char *);
void wait_to_terminate(int);


/* Needed to pass legit copy of an integer argument to a pthread */
int *dupInt( int i )
{
	int *pi = (int *)malloc(sizeof(int));
	assert( pi != NULL);
	*pi = i;
	return pi;
}


int main(int argc, char *argv[])
{
	long seed;
	numAtoms = DEFAULT_NUM_ATOMS;
	pthread_t **atom;
	int i;
	int status;

	if ( argc < 2 ) {
		fprintf(stderr, "usage: %s <seed> [<num atoms>]\n", argv[0]);
		exit(1);
	}

	if ( argc >= 2) {
		seed = atoi(argv[1]);
	}

	if (argc == 3) {
		numAtoms = atoi(argv[2]);
		if (numAtoms < 0) {
			fprintf(stderr, "%ld is not a valid number of atoms\n",
				numAtoms);
			exit(1);
		}
	}

    kosmos_log_init();
	kosmos_init();

	srand(seed);
	atom = (pthread_t **)malloc(numAtoms * sizeof(pthread_t *));
	assert (atom != NULL);
	for (i = 0; i < numAtoms; i++) {
		atom[i] = (pthread_t *)malloc(sizeof(pthread_t));
		if ( (double)rand()/(double)RAND_MAX < ATOM_THRESHOLD ) {
			hNum++;
			status = pthread_create (
					atom[i], NULL, h_ready,
					(void *)dupInt(hNum)
				);
		} else {
			cNum++;
			status = pthread_create (
					atom[i], NULL, c_ready,
					(void *)dupInt(cNum)
				);
		}
		if (status != 0) {
			fprintf(stderr, "Error creating atom thread\n");
			exit(1);
		}
	}

    /* Determining the maximum number of ethynyl radicals is fairly
     * straightforward -- it will be the minimum of the number of
     * hNum and cNum/2.
     */

    int max_radicals = (hNum < cNum/2 ? hNum : (int)(cNum/2));
#ifdef VERBOSE
    printf("Maximum # of radicals expected: %d\n", max_radicals);
#endif

    wait_to_terminate(max_radicals);
}


/*
* Now the tricky bit begins....  All the atoms are allowed
* to go their own way, but how does the Kosmos ethynyl-radical
* problem terminate? There is a non-zero probability that
* some atoms will not become part of a radical; that is,
* many atoms may be blocked on some condition variable of
* our own devising. How do we ensure the program ends when
* (a) all possible radicals have been created and (b) all
* remaining atoms are blocked (i.e., not on the ready queue)?
*/


/*
 * ^^^^^^^
 * DO NOT MODIFY CODE ABOVE THIS POINT.
 *
 *************************************
 *************************************
 *
 * ALL STUDENT WORK MUST APPEAR BELOW.
 * vvvvvvvv
 */


/* 
 * DECLARE / DEFINE NEEDED VARIABLES IMMEDIATELY BELOW.
 */
int radicals;
int num_free_c;
int num_free_h;

int combining_c;
int combining_h;

pthread_mutex_t count_mutex;
pthread_mutex_t combining_mutex;
pthread_mutex_t staging_area;
pthread_cond_t wait_h;
pthread_cond_t wait_c;
pthread_cond_t conf_h;
pthread_cond_t conf_c;


/*
 * FUNCTIONS YOU MAY/MUST MODIFY.
 */

void kosmos_init() {
    num_free_c = 0;
    num_free_h = 0;
    combining_c = 0;
    combining_h = 0;
    
}


void *h_ready( void *arg )
{
	int id = *((int *)arg);
    char name[MAX_ATOM_NAME_LEN];

    sprintf(name, "h%03d", id);

#ifdef VERBOSE
	printf("%s now exists\n", name);
#endif
    
    pthread_mutex_lock(&count_mutex);
    if (num_free_c > 1) {
        num_free_c -= 2;
        pthread_mutex_lock(&staging_area);
        pthread_mutex_unlock(&count_mutex);
        
        pthread_mutex_lock(&combining_mutex);
        combining_c = -1;
        pthread_mutex_unlock(&combining_mutex);
        
        pthread_cond_signal(&wait_c);
        
        pthread_mutex_lock(&combining_mutex);
        while(combining_c == -1) {pthread_cond_wait(&conf_c, &combining_mutex);}
        int c1 = combining_c;
        combining_c = -1;
        pthread_mutex_unlock(&combining_mutex);
        
        pthread_cond_signal(&wait_c);
        
        pthread_mutex_lock(&combining_mutex);
        while(combining_c == -1) {pthread_cond_wait(&conf_c, &combining_mutex);}
        int c2 = combining_c;
        pthread_mutex_unlock(&combining_mutex);
        
        #ifdef VERBOSE
            fprintf(stdout, "A ethynyl radical was made: c%03d  c%03d  h%03d\n",
                c1, c2, id);
        #endif
        
        kosmos_log_add_entry(++radicals, c1, c2, id, name);
        pthread_mutex_unlock(&staging_area);
    } else {
        num_free_h++;
        pthread_mutex_unlock(&count_mutex);
        pthread_mutex_lock(&combining_mutex);
        while (combining_h != -1) {pthread_cond_wait(&wait_h, &combining_mutex);}
        combining_h = id;
        pthread_mutex_unlock(&combining_mutex);
        pthread_cond_signal(&conf_h);        
    }

	return NULL;
}


void *c_ready( void *arg )
{
	int id = *((int *)arg);
    char name[MAX_ATOM_NAME_LEN];

    sprintf(name, "c%03d", id);

#ifdef VERBOSE
	printf("%s now exists\n", name);
#endif

    pthread_mutex_lock(&count_mutex);
    if (num_free_c > 0 && num_free_h > 0) {
        num_free_c--;
        num_free_h--;
        pthread_mutex_lock(&staging_area);
        pthread_mutex_unlock(&count_mutex);
        
        pthread_mutex_lock(&combining_mutex);
        combining_c = -1;
        combining_h = -1;
        pthread_mutex_unlock(&combining_mutex);
        
        pthread_cond_signal(&wait_h);
        pthread_cond_signal(&wait_c);        
        
        pthread_mutex_lock(&combining_mutex);
        while(combining_h == -1) {pthread_cond_wait(&conf_h, &combining_mutex);}
        int h = combining_h;
        while(combining_c == -1) {pthread_cond_wait(&conf_c, &combining_mutex);}
        int c1 = combining_c;
        pthread_mutex_unlock(&combining_mutex);
        
        #ifdef VERBOSE
            fprintf(stdout, "A ethynyl radical was made: c%03d  c%03d  h%03d\n",
                c1, id, h);
        #endif
        
        kosmos_log_add_entry(++radicals, c1, id, h, name);
        pthread_mutex_unlock(&staging_area);
    } else {
        num_free_c++;        
        pthread_mutex_unlock(&count_mutex);
        pthread_mutex_lock(&combining_mutex);
        while (combining_c != -1) {pthread_cond_wait(&wait_c, &combining_mutex);}
        combining_c = id;
        pthread_mutex_unlock(&combining_mutex);
        pthread_cond_signal(&conf_c);
    }
    
	return NULL;
}


void wait_to_terminate(int expected_num_radicals) {
    while (radicals != expected_num_radicals){}
    pthread_mutex_lock(&staging_area);
    // sleep(MAX_KOSMOS_SECONDS);
    kosmos_log_dump();
    
    exit(0);
}
