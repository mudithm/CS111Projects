/*
NAME: Mudith Mallajosyula
EMAIL: mudithm@g.ucla.edu
ID: ---------
*/


#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static int segfault_flag = 0;
static int catch_flag = 0;
extern int errno;

// causes a segmentation fault by attempting to
// dereference a null pointer
void segFaultFunction()
{
    char * p = NULL;
    *p = 'a';
}

// signal handle
void signal_handler(int signal_number)
{
    fprintf(stderr, "%d caught\n", signal_number);
    exit(4);
}

int main(int argc, char *argv[])
{
    // strings to hold input and output filenames
    char * input = NULL;
    char * output = NULL;

    // holds file descsriptor for input and output
    int file_descriptor;

    // options for the command
    const struct option opts[] =
    {
        {"input", required_argument, 0, 'i'},
        {"output", required_argument, 0, 'o'},
        {"segfault", no_argument, 0, 's'},
        {"catch", no_argument, 0, 'c'},
        {"dump-core", no_argument, 0, 'd'}
    };

    // holds the current index of the opts[] array for getopt_long
    int option_index = 0;

    // holds the return value of getopt_long, which will either be a character, 0, or -1
    long c = 0;
    while (1)
    {
        // parse the options passed to the program, if any
        c = getopt_long(argc, argv, "abc:", opts, &option_index);

        // if c==-1, there are no more options to be processed
        if (c == -1)
            break;

        // handles the options that don't require arguments
        switch ((char)(c))
        {
        case 'i':
            if (optarg)
                input = optarg;
            else
            {
                fprintf(stderr, "Unable to open input file\n");
                exit(2);
            }
            break;
        case 'o':
            if (optarg)
                output = optarg;
            else
            {
                fprintf(stderr, "Unable to open output file\n");
                exit(3);
            }
            break;
        case 'd':
            // resets the catch flag if --dump-core is passed
            catch_flag = 0;
            break;
        case 'c':
            catch_flag = 1;
            break;
        case 's':
            //if (catch_flag)
            //   catch_flag = 0;
            segfault_flag = 1;
            break;
        default:
            fprintf(stderr, "Unrecognized argument.\n");
            exit(1);
            break;
        }

    }

    if (segfault_flag)
    {
        // if catch flag is toggled, register the SIGSEGV and log an error
        if (catch_flag)
            signal(SIGSEGV, segfault_handler);
        segFaultFunction();

        // in case, somehow, no fault occurs
        fprintf(stderr, "Dumped Core.\n");
        exit(139);
    }

    if (input != NULL)
    {
        file_descriptor = open(input, O_RDONLY);
        if (file_descriptor >= 0)
        {
            close (0);
            dup(file_descriptor);
            close(file_descriptor);
        }
        else
        {
            fprintf(stderr, "Unable to open input file.\n");
            exit(2);
        }
    }

    if (output != NULL)
    {
        file_descriptor = creat(output, 0666);
        if (file_descriptor >= 0)
        {
            close(1);
            dup(file_descriptor);
            close(file_descriptor);
        } else
        {
            fprintf(stderr, "Unable to open output file.\n");
            exit(3);
        }
    }

    // creates a 128 byte buffer to hold characters
    char buffer[128];
    while (read(0, buffer, 1) > 0)
    {
        write(1, buffer, 1);
    }


    exit(0);

}