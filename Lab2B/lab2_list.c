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

static pthread_mutex_t mutex_lock;
static int spin_lock = 0;

static SortedList_t shared_list;
static SortedList_t *list_array;

int opt_yield = 0;

// handle segmentation faults
void segfault_handler(int signal_number)
{
    if (signal_number == SIGSEGV)
    {
        fprintf(stderr, "Caught and received SIGSEGV.\n");/*
        fprintf(stderr, "Error %d: %s\n", errno, (char *)(strerror(errno)));*/
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
    for (int i = *offset; i < numIterations + *offset; i++)
    {
        SortedList_insert(&shared_list, &list_array[i]);
    }

    // check length of the list
    int len = SortedList_length(&shared_list);

    if (len < 0)
    {
        fprintf(stderr, "Corrupted list!\n");
        exit(2);
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
        if (SortedList_delete(SortedList_lookup(&shared_list, (&list_array[i])->key)))
        {
            fprintf(stderr, "Error deleting element!\n");
            exit(2);
        }
    }
}

// Makes and deletes lists, with mutex lock
void listAdd_mutex(int *offset)
{
    //printf("Offset: %d\n", *offset);
    for (int i = *offset; i < numIterations + *offset; i++)
    {
        //printf("Adding #%d: %s\n", i, list_array[i].key);
        //fflush(stdout);
        //mutex lock
        //printf("%s is waiting on mutex lock:  -----------------------\n", list_array[i].key);
        //fflush(stdout);
        pthread_mutex_lock(&mutex_lock);
        //printf("%s received lock.         -----------------------\n", list_array[i].key);
        SortedList_insert(&shared_list, &list_array[i]);
        pthread_mutex_unlock(&mutex_lock);
    }

    // check length of list
    int len;
    pthread_mutex_lock(&mutex_lock);
    len = SortedList_length(&shared_list);
    pthread_mutex_unlock(&mutex_lock);

    if (len < 0)
    {
        fprintf(stderr, "Corrupted list!\n");
        exit(2);
    }

    //delete all added elements

    for (int i = *offset; i < numIterations + *offset; i++)
    {
        //printf("Adding #%d: %s\n", i, list_array[i].key);
        //fflush(stdout);
        //mutex lock
        //printf("%s is waiting on mutex lock:  -----------------------\n", list_array[i].key);
        //fflush(stdout);
        pthread_mutex_lock(&mutex_lock);
        //printf("%s received lock.         -----------------------\n", list_array[i].key);
        if (SortedList_delete(SortedList_lookup(&shared_list, (&list_array[i])->key)))
        {
            fprintf(stderr, "Error deleting element!\n");
            exit(2);
        }
        pthread_mutex_unlock(&mutex_lock);
    }
}

// Makes and deletes lists, with spin lock
void listAdd_spin(int *offset)
{
    for (int i = *offset; i < numIterations + *offset; i++)
    {

        // printf("Adding #%d: %s\n", i, list_array[i].key);
        // fflush(stdout);

        //spin lock
        while (__sync_lock_test_and_set(&spin_lock, 1))
            continue;
        SortedList_insert(&shared_list, &list_array[i]);
        __sync_lock_release(&spin_lock);

    }

    // check length of list
    int len;

    while (__sync_lock_test_and_set(&spin_lock, 1))
        continue;
    len = SortedList_length(&shared_list);
    __sync_lock_release(&spin_lock);

    if (len < 0)
    {
        fprintf(stderr, "Corrupted list!\n");
        exit(2);
    }


    //delete all added elements

    for (int i = *offset; i < numIterations + *offset; i++)
    {
        //printf("Adding #%d: %s\n", i, list_array[i].key);
        //fflush(stdout);
        //mutex lock
        //printf("%s is waiting on mutex lock:  -----------------------\n", list_array[i].key);
        //fflush(stdout);
        while (__sync_lock_test_and_set(&spin_lock, 1))
            continue;        //printf("%s received lock.         -----------------------\n", list_array[i].key);

        if (SortedList_delete(SortedList_lookup(&shared_list, (&list_array[i])->key)))
        {
            fprintf(stderr, "Error deleting element!\n");
            exit(2);
        }
        __sync_lock_release(&spin_lock);
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
        default:
            fprintf(stderr, "Bad argument.\n");
            exit(1);
            break;
        }

    }



    if (pthread_mutex_init(&mutex_lock, NULL) != 0)
    {
        printf("\n mutex init has failed\n");
        exit(1);
    }
    //list to be shared by threads
    SortedList_t *head = &shared_list;
    head->prev = NULL;
    head->next = NULL;
    head->key = NULL;


    // keep track of number of items in list
    size_t size = numThreads * numIterations;
    int thread_handlers[numThreads];

    list_array = malloc(sizeof(SortedList_t) * size);
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
            exit(2);
        }
    }

    // high-resolution end time
    clock_gettime(CLOCK_MONOTONIC, &end);
    long long end_time = end.tv_sec * 1000000000 + end.tv_nsec;

    // check that final length of the list is 0
    if (SortedList_length(&shared_list) != 0)
    {
        fprintf(stderr, "List was not length 0!\n");
        exit(2);
    }

    // test type
    test_type = concatString(test_type, suffix);

    // outputting csv statistics
    long long total_time = end_time - start_time; // total time, nanosec
    long long numOperations = numThreads * numIterations * 3; // total number of operations
    long long avg_time = total_time / numOperations; // average time per op, nanoseconds
    printf("%s,%d,%d,1,%lld,%lld,%lld\n", test_type, numThreads, numIterations, numOperations, total_time, avg_time);

    pthread_mutex_destroy(&mutex_lock);


    free(list_array);
    exit(0);
}

#endif
