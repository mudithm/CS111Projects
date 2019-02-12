/*
NAME: Mudith Mallajosyula
EMAIL: mudithm@g.ucla.edu
ID: 404937201
*/


#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include <string.h>

#include <time.h>
#include <pthread.h>

// long counter for testing
static long long counter = 0;
static char sync_val = ' ';

static pthread_mutex_t mutex_lock;
static int spin_lock = 0;


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

int opt_yield = 0;
void add(long long *pointer, long long value) {
    long long sum = *pointer + value;
    if (opt_yield)
        sched_yield();
    *pointer = sum;
}

// Adds and subtracts 1, without protection
void addAndSubtract(void *numIterations)
{
    int *ct = (int *)numIterations;
    for (int i = 0; i < *ct; i++)
    {
        add(&counter, 1);
        add(&counter, -1);
    }
}

// Adds and subtracts 1, without mutex locks
void addAndSubtract_mutex(void *numIterations)
{
    int *ct = (int *)numIterations;
    for (int i = 0; i < *ct; i++)
    {
        pthread_mutex_lock(&mutex_lock);
        add(&counter, 1);
        add(&counter, -1);
        pthread_mutex_unlock(&mutex_lock);
    }
}

// Adds and subtracts 1, without spin lock
void addAndSubtract_spin(void *numIterations)
{
    int *ct = (int *)numIterations;
    for (int i = 0; i < *ct; i++)
    {
        while (__sync_lock_test_and_set(&spin_lock, 1))
            continue;
        add(&counter, 1);
        add(&counter, -1);
        __sync_lock_release(&spin_lock);

    }
}

// Adds and subtracts 1, without compare-and-swap
void addAndSubtract_compare(void *numIterations)
{
    int *ct = (int *)numIterations;
    long long prevCount, currCount;
    for (int i = 0; i < *ct; i++)
    {
        while (1)
        {
        	prevCount = counter;
        	currCount = prevCount + 1;
        	if (__sync_val_compare_and_swap(&counter, prevCount, currCount) == prevCount)
        		break;
        }
         while (1)
        {
        	prevCount = counter;
        	currCount = prevCount - 1;
        	if (__sync_val_compare_and_swap(&counter, prevCount, currCount) == prevCount)
        		break;
        }
    }
}


int main(int argc, char *argv[])
{

    const struct option opts[] =
    {
        {"threads", required_argument, 0, 't'},
        {"iterations", required_argument, 0, 'i'},
        {"yield", no_argument, 0, 'y'},
        {"sync", required_argument, 0, 's'}
    };

    // holds the current index of the opts[] array for getopt_long
    int option_index = 0;

    // Degault number of threads and iterations is 1
    int numThreads = 1;
    int numIterations = 1;

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
            opt_yield = 1;
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
                } else if (strcmp(optarg, "c") == 0)
                {   // compare-and-swap
                    sync_val = 'c';
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



    // Default test type is add-none
    char* test_type = "add";

    if (opt_yield)
        test_type = "add-yield";

    char* suffix;


    // high-resolution start time
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    long long start_time = start.tv_sec * 1000000000 + start.tv_nsec;

    // Initialize array of threads
    pthread_t threads[numThreads];

    // for thread errors
    int thread_err;

    void * addFunction;


    // decide which add function to use
    if (sync_val != ' ')
    {
        if (sync_val == 'm')
        {
            addFunction = &addAndSubtract_mutex;
            suffix = "-m";
        }
        else if (sync_val == 's')
        {
            addFunction = &addAndSubtract_spin;
            suffix = "-s";
        }
        else
        {
            addFunction = &addAndSubtract_compare;
            suffix = "-c";
        }
    }
    else 
    {
        addFunction = &addAndSubtract;
        suffix = "-none";
    }

    // create n threads
    for (int i = 0; i < numThreads; i++)
    {
        thread_err = pthread_create(&threads[i], NULL, addFunction, (void *)&numIterations);
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

    // test type
    test_type = concatString(test_type, suffix);

    // outputting csv statistics
    long long total_time = end_time - start_time; // total time, nanosec
    long long numOperations = numThreads * numIterations * 2; // total number of operations
    long long avg_time = total_time / numOperations; // average time per op, nanoseconds
    printf("%s,%d,%d,%lld,%lld,%lld,%lld\n", test_type, numThreads, numIterations, numOperations, total_time, avg_time, counter);

    exit(0);
}