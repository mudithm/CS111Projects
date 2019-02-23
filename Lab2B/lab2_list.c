/*
NAME: Mudith Mallajosyula
EMAIL: mudithm@g.ucla.edu
ID: 404937201
*/

#ifndef SORTEDLIST_INCLUDED
#define SORTEDLIST_INCLUDED


#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include "SortedList.h"

// long counter for testing
static char sync_val = ' ';
static char *yield_val = "";

static int numIterations = 1;
static int numSublists = 1;

//static pthread_mutex_t mutex_lock;
//static int spin_lock = 0;

static long long protected_wait_time = 0;


int opt_yield = 0;

// Struct to contain the sublists
typedef struct
{
    SortedList_t list_head;
    pthread_mutex_t mutex_lock;
    int spin_lock;
} Sublist_t;

static SortedList_t *list_array;
static Sublist_t *sublist_array;

// modified djb2 hash for hahsing the keys into the new sublists
unsigned long hash(const char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

// prints contents of the list, for debugging purposes
void print2List(SortedList_t *list)
{
    if (list == NULL)
    {
        printf("invalid list\n");
	free(list_array);
	free(sublist_array);
	exit(2);
    }
    printf("\n==================================\n");
    printf("============>printing list<========\n");
    printf("==================================\n\n");
    fflush(stdout);


    SortedListElement_t *iter = list->next;
    while (iter != NULL && iter->key != NULL)
    {
        printf("%s ", iter->key);
        iter = iter->next;
    }
    printf("\n");

    printf("\n==================================\n");
    printf("========<done printing list>======\n");
    printf("==================================\n\n");
    fflush(stdout);
}


// handle segmentation faults
void segfault_handler(int signal_number)
{
    if (signal_number == SIGSEGV)
    {
        fprintf(stderr, "Caught and received SIGSEGV.\n");/*
        fprintf(stderr, "Error %d: %s\n", errno, (char *)(strerror(errno)));*/
	free(sublist_array);
	free(list_array);
	exit(2);
    }
}

// concatenates strings
char *concatString(char *str1, char *str2)
{
    size_t outlen = strlen(str1) + strlen (str2) + 1;

    char *str = malloc(outlen);
    str[0] = '\0';
    strcat(str, str1);
    strcat(str, str2);
    //str1 = str;
    return str;
}

// Makes and deletes list, without protection
void listAdd(int *offset)
{
    Sublist_t *list;
    SortedList_t *head;
    for (int i = *offset; i < numIterations + *offset; i++)
    {
        list = &(sublist_array[hash(list_array[i].key) % numSublists]);
        head = &(list->list_head);
        SortedList_insert(head, &list_array[i]);
    }

    int len = 0;
    // check length of the lists
    for (int i = 0; i < numSublists; i++)
    {
        int temp_len = SortedList_length(head);

        if (temp_len < 0)
        {
            fprintf(stderr, "Corrupted list!\n");
	    free(sublist_array);
	    free(list_array);
	    exit(2);
        }
        len += temp_len;
    }


    //delete all added elements

    for (int i = *offset; i < numIterations + *offset; i++)
    {
        //printf("Adding #%d: %s\n", i, list_array[i].key);
        //fflush(stdout);
        //mutex lock
        //printf("%s is waiting on mutex lock:  -----------------------\n", list_array[i].key);
        //fflush(stdout);
        //printf("%s received lock.         -----------------------\n", list_array[i].key);
        list = &(sublist_array[hash(list_array[i].key) % numSublists]);
        head = &(list->list_head);
        if (SortedList_delete(SortedList_lookup(head, (&list_array[i])->key)))
        {
            fprintf(stderr, "Error deleting element!\n");
	    free(sublist_array);
	    free(list_array);
	    exit(2);
        }
    }
}

// Makes and deletes lists, with mutex lock
void listAdd_mutex(int *offset)
{
    struct timespec mutex_start, mutex_end;
    long long mutex_start_time, mutex_end_time;
    //printf("Offset: %d\n", *offset);
    Sublist_t *list;
    SortedList_t *head;

    for (int i = *offset; i < numIterations + *offset; i++)
    {
        //printf("Adding #%d: %s\n", i, list_array[i].key);
        //fflush(stdout);
        //mutex lock
        //printf("%s is waiting on mutex lock:  -----------------------\n", list_array[i].key);
        //fflush(stdout);
        list = &(sublist_array[hash(list_array[i].key) % numSublists]);
        head = &(list->list_head);

        clock_gettime(CLOCK_MONOTONIC, &mutex_start);
        mutex_start_time = mutex_start.tv_sec * 1000000000 + mutex_start.tv_nsec;

        pthread_mutex_lock(&(list->mutex_lock));

        clock_gettime(CLOCK_MONOTONIC, &mutex_end);
        mutex_end_time = mutex_end.tv_sec * 1000000000 + mutex_end.tv_nsec;
        protected_wait_time += (mutex_end_time - mutex_start_time);
        //printf("%s received lock.         -----------------------\n", list_array[i].key);
        SortedList_insert(head, &list_array[i]);
        pthread_mutex_unlock(&(list->mutex_lock));
    }

    // check length of list
    int len = 0;

    for (int i = 0; i < numSublists; i++)
    {
        list = &(sublist_array[i]);
        head = &(list->list_head);

        clock_gettime(CLOCK_MONOTONIC, &mutex_start);
        mutex_start_time = mutex_start.tv_sec * 1000000000 + mutex_start.tv_nsec;

        pthread_mutex_lock(&(list->mutex_lock));

        clock_gettime(CLOCK_MONOTONIC, &mutex_end);
        mutex_end_time = mutex_end.tv_sec * 1000000000 + mutex_end.tv_nsec;
        protected_wait_time += (mutex_end_time - mutex_start_time);

        int temp_len = SortedList_length(head);


        pthread_mutex_unlock(&(list->mutex_lock));

        if (temp_len < 0)
        {
            fprintf(stderr, "Corrupted list!\n");
	    free(sublist_array);
	    free(list_array);
	    exit(2);
        }

        len += temp_len;
    }


    //delete all added elements

    for (int i = *offset; i < numIterations + *offset; i++)
    {
        //printf("Adding #%d: %s\n", i, list_array[i].key);
        //fflush(stdout);
        //mutex lock
        //printf("%s is waiting on mutex lock:  -----------------------\n", list_array[i].key);
        //fflush(stdout);
        list = &(sublist_array[hash(list_array[i].key) % numSublists]);
        head = &(list->list_head);

        clock_gettime(CLOCK_MONOTONIC, &mutex_start);
        mutex_start_time = mutex_start.tv_sec * 1000000000 + mutex_start.tv_nsec;

        pthread_mutex_lock(&(list->mutex_lock));

        clock_gettime(CLOCK_MONOTONIC, &mutex_end);
        mutex_end_time = mutex_end.tv_sec * 1000000000 + mutex_end.tv_nsec;
        protected_wait_time += (mutex_end_time - mutex_start_time);

        //printf("%s received lock.         -----------------------\n", list_array[i].key);
        if (SortedList_delete(SortedList_lookup(head, (&list_array[i])->key)))
        {
            fprintf(stderr, "Error deleting element!\n");
	    free(sublist_array);
	    free(list_array);
	    exit(2);
        }
        pthread_mutex_unlock(&(list->mutex_lock));
    }
}

// Makes and deletes lists, with spin lock
void listAdd_spin(int *offset)
{
    struct timespec spin_start, spin_end;
    long long spin_start_time, spin_end_time;

    Sublist_t *list;
    SortedList_t *head;

    for (int i = *offset; i < numIterations + *offset; i++)
    {
        list = &(sublist_array[hash(list_array[i].key) % numSublists]);
        head = &(list->list_head);

        //printf("Adding #%d: %s to sublist #%ld\n", i, list_array[i].key, hash(list_array[i].key) % numSublists);
        //fflush(stdout);

        //spin lock
        clock_gettime(CLOCK_MONOTONIC, &spin_start);
        spin_start_time = spin_start.tv_sec * 1000000000 + spin_start.tv_nsec;

        while (__sync_lock_test_and_set(&(list->spin_lock), 1))
            continue;

        clock_gettime(CLOCK_MONOTONIC, &spin_end);
        spin_end_time = spin_end.tv_sec * 1000000000 + spin_end.tv_nsec;
        protected_wait_time += (spin_end_time - spin_start_time);


        SortedList_insert(head, &list_array[i]);
        //print2List(head);



        __sync_lock_release(&(list->spin_lock));

    }

    // check length of list
    int len = 0;

    for (int i = 0; i < numSublists; i++)
    {
        list = &(sublist_array[i]);
        head = &(list->list_head);

        clock_gettime(CLOCK_MONOTONIC, &spin_start);
        spin_start_time = spin_start.tv_sec * 1000000000 + spin_start.tv_nsec;

        while (__sync_lock_test_and_set(&(list->spin_lock), 1))
            continue;

        clock_gettime(CLOCK_MONOTONIC, &spin_end);
        spin_end_time = spin_end.tv_sec * 1000000000 + spin_end.tv_nsec;
        protected_wait_time += (spin_end_time - spin_start_time);

        int temp_len = SortedList_length(head);

        __sync_lock_release(&(list->spin_lock));

        if (temp_len < 0)
        {
            fprintf(stderr, "Corrupted list!\n");
	    free(list_array);
	    free(sublist_array);
	    exit(2);
        }

        len += temp_len;
    }



    //delete all added elements

    for (int i = *offset; i < numIterations + *offset; i++)
    {
        //printf("removing #%d: %s\n", i, list_array[i].key);
        //fflush(stdout);
        //mutex lock
        //printf("%s is waiting on mutex lock:  -----------------------\n", list_array[i].key);
        //fflush(stdout);

        list = &(sublist_array[hash(list_array[i].key) % numSublists]);
        head = &(list->list_head);

        clock_gettime(CLOCK_MONOTONIC, &spin_start);
        spin_start_time = spin_start.tv_sec * 1000000000 + spin_start.tv_nsec;
        while (__sync_lock_test_and_set(&(list->spin_lock), 1))
            continue;
        clock_gettime(CLOCK_MONOTONIC, &spin_end);
        spin_end_time = spin_end.tv_sec * 1000000000 + spin_end.tv_nsec;
        protected_wait_time += (spin_end_time - spin_start_time);

        //printf("%s received lock.         -----------------------\n", list_array[i].key);
        // print2List(&(list.list_head));

        if (SortedList_delete(SortedList_lookup(head, list_array[i].key)))
        {
            fprintf(stderr, "Error deleting element %s!\n", list_array[i].key);
            free(list_array);
	    free(sublist_array);
	    exit(2);
        }
        __sync_lock_release(&(list->spin_lock));


    }




}

int main(int argc, char *argv[])
{

    const struct option opts[] =
    {
        {"threads", required_argument, 0, 't'},
        {"iterations", required_argument, 0, 'i'},
        {"yield", required_argument, 0, 'y'},
        {"sync", required_argument, 0, 's'},
        {"lists", required_argument, 0, 'l'},
        {NULL, 0, NULL, 0}
    };

    // holds the current index of the opts[] array for getopt_long
    int option_index = 0;

    // Degault number of threads and iterations is 1
    int numThreads = 1;

    // holds the return value of getopt_long, which will either be a character, 0, or -1
    long c = 0;
    while (1)
    {
        // parse the options passed to the program, if any
        c = getopt_long(argc, argv, "", opts, &option_index);

        // if c==-1, there are no more options to be processed
        if (c == -1)
            break;

        // handles the options that don't require arguments
        switch ((char)(c))
        {
        case 't':
            if (optarg) {
                numThreads = atoi(optarg);
                if (numThreads < 1)
                {
                    fprintf(stderr, "Invalid number of threads: %s\n", optarg);
                    exit(1);
                }
            }
            else
            {
                fprintf(stderr, "No argument passed to --threads\n");
                exit(1);
            }
            break;
        case 'i':
            if (optarg) {
                numIterations = atoi(optarg);
                if (numIterations < 1)
                {
                    fprintf(stderr, "Invalid number of iterations: %s\n", optarg);
                    exit(1);
                }
            }
            else
            {
                fprintf(stderr, "No argument passed to --iterations\n");
                exit(1);
            }
            break;
        case 'y':
            if (optarg)
            {
                for (size_t i = 0; i < strlen(optarg); i++)
                {
                    if (optarg[i] == 'i')
                    {   // insert yield
                        opt_yield |= INSERT_YIELD;
                    } else if (optarg[i] == 'd')
                    {   // delete yield
                        opt_yield |= DELETE_YIELD;
                    } else if (optarg[i] == 'l')
                    {   // lookup yield
                        opt_yield |= LOOKUP_YIELD;
                    } else
                    {
                        fprintf(stderr, "Invalid yield option %s\n", optarg);
                        exit(1);
                    }
                }

                if (opt_yield & INSERT_YIELD)
                    yield_val = concatString(yield_val, "i");
                if (opt_yield & DELETE_YIELD)
                    yield_val = concatString(yield_val, "d");
                if (opt_yield & LOOKUP_YIELD)
                    yield_val = concatString(yield_val, "l");
            }
            break;
        case 's':
            if (optarg)
            {
                if (strcmp(optarg, "m") == 0)
                {   // Mutex lock
                    sync_val = 'm';
                } else if (strcmp(optarg, "s") == 0)
                {   // spin lock
                    sync_val = 's';
                } else
                {
                    fprintf(stderr, "Invalid sync option %s\n", optarg);
                    exit(1);
                }
            }
            break;
        case 'l':
            if (optarg) {
                numSublists = atoi(optarg);
                if (numSublists < 1)
                {
                    fprintf(stderr, "Invalid number of sublists: %s\n", optarg);
                    exit(1);
                }
//                printf("Sublists: %s\n", optarg);
            }
            else
            {
                fprintf(stderr, "No argument passed to --lists\n");
                exit(1);
            }
            break;
        default:
            fprintf(stderr, "Bad argument.\n");
            exit(1);
            break;
        }

    }

    // keep track of number of items in list
    size_t size = numThreads * numIterations;
    int thread_handlers[numThreads];

    // array for SortedListElements and Sublists
    sublist_array = malloc(sizeof(Sublist_t) * numSublists);
    list_array = malloc(sizeof(SortedList_t) * size);

    // initializing mutex locks and sublists.
    for (int i = 0; i < numSublists; i++)
    {
        Sublist_t *sub = &sublist_array[i];

        if (sync_val == 'm' && pthread_mutex_init(&(sub->mutex_lock), NULL) != 0)
        {
            fprintf(stderr, "\n mutex init for sublist %d has failed\n", i);
	    free(list_array);
	    free(sublist_array);
	    exit(2);
        }

        if (sync_val == 's')
        {
            sub->spin_lock = 0;
        }

    }



    // seed random number generator
    srand (time(NULL));



    // generate random keys
    for (int i = 0, tnum = 0; i < (int)size; i++)
    {
        if (i % numIterations == 0) {
            thread_handlers[tnum] = i;
            tnum++;
        }
        //random 3 digit key
        char *keys = malloc(4);
        keys[0] = (char) random() % 26 + 'a'; // generates letter
        keys[1] = (char) random() % 26 + 'a'; // generates letter
        keys[2] = (char) random() % 26 + 'a'; // generates letter
        keys[3] = 0;
        //printf("location %d: %s\n", i, keys);
        list_array[i].key = keys;
        list_array[i].prev = NULL;
        list_array[i].next = NULL;
    }


    /* for (int i = 0; i < size; i++)
     {
         printf("Key %d: %s\n", i, list_array[i].key);
     }*/


    /*    for (int i = 0; i < numThreads; i++)
        {
            printf("---%d\n", thread_handlers[i]);
        }*/

    // Default test type is add-none
    char* test_type = "list-";

    // if opt_yield is set, change test type string
    if (opt_yield)
        test_type = concatString(test_type, yield_val);
    else
        test_type = concatString(test_type, "none");

    char* suffix;
    void *listFunction;

// decide which add function to use
    if (sync_val != ' ')
    {
        if (sync_val == 'm')
        {
            listFunction = &listAdd_mutex;
            suffix = "-m";
        }
        else if (sync_val == 's')
        {
            listFunction = &listAdd_spin;
            suffix = "-s";
        }
    }
    else
    {
        listFunction = &listAdd;
        suffix = "-none";
    }

    // Signal segfault handler
    signal(SIGSEGV, segfault_handler);


    // high-resolution start time
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    long long start_time = start.tv_sec * 1000000000 + start.tv_nsec;

    // Initialize array of threads
    pthread_t threads[numThreads];

    // for thread errors
    int thread_err = 0;


    // create n threads
    for (int i = 0; i < numThreads; i++)
    {
        thread_err = pthread_create(&threads[i], NULL, listFunction, &(thread_handlers[i]));
        if (thread_err)
        {
            fprintf(stderr, "Error creating threads\n");
	    free(list_array);
	    free(sublist_array);
	    exit(2);
        }
    }

    // join threads after execution
    for (int j = 0; j < numThreads; j++)
    {
        thread_err = pthread_join(threads[j], NULL);
        if (thread_err)
        {
            fprintf(stderr, "Error joining threads\n");
	    free(list_array);
	    free(sublist_array);
	    exit(2);
        }
    }

    // high-resolution end time
    clock_gettime(CLOCK_MONOTONIC, &end);
    long long end_time = end.tv_sec * 1000000000 + end.tv_nsec;

    int len = 0;
    SortedList_t *lst;
    // check that final length of the lists is 0
    for (int i = 0; i < numSublists; i++)
    {
        lst = &(sublist_array[i].list_head);
        int temp_len = SortedList_length(lst);
        //print2List(&(sublist_array[i].list_head));
        if (temp_len != 0)
        {
            fprintf(stderr, "options: %s Sublist %d was not length 0!\n", test_type, i);
	    free(list_array);
	    free(sublist_array);
	    exit(2);
        }
    }
    if (len != 0)
    {
        fprintf(stderr, "List was not length 0!\n");
	free(list_array);
	free(sublist_array);
	exit(2);
    }

    // test type
    test_type = concatString(test_type, suffix);

    // outputting csv statistics
    long long total_time = end_time - start_time; // total time, nanosec
    long long numOperations = numThreads * numIterations * 3; // total number of operations
    long long avg_time = total_time / numOperations; // average time per op, nanoseconds
    long long avg_wait_time = protected_wait_time / ((numIterations * 3 + 1) * numThreads); // avg wait time per sync
    printf("%s,%d,%d,%d,%lld,%lld,%lld,%lld\n", test_type, numThreads, numIterations, numSublists, numOperations, total_time, avg_time, avg_wait_time);

    free(list_array);
    free(sublist_array);
    exit(0);
}

#endif
