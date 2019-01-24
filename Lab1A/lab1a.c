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
#include <wait.h>

static int verbose = 0;
extern int optind;
// Wastes memory;
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

int main(int argc, char *argv[])
{
    int file_des = 0;
    int curr_fd = 0;
    size_t files_size = 20;
    int *files = malloc(files_size * sizeof(int));
    if (files == NULL)
    {
        fprintf(stderr, "Malloc error on Files array\n" );
        exit(1);
    }

    int err = 0;

    const struct option opts[] =
    {
        {"rdonly", required_argument, 0, 'r'},
        {"wronly", required_argument, 0, 'w'},
        {"command", required_argument, 0, 'c'},
        {"verbose", no_argument, 0, 'v'}
    };

    int option_index = 0;
    int argno = 1;
    long c = 0;

    while (1)
    {
        c = getopt_long(argc, argv, "", opts, &option_index);

        if (c == -1)
            break;

        switch ((char)(c))
        {
        case 'r':
            if (optarg)
            {
                if (verbose)
                {
                    fprintf(stdout, "--rdonly %s\n", optarg);
                }
                curr_fd = open(optarg, O_RDONLY);
                if (curr_fd != -1)
                {

                }
                else
                {
                    fprintf(stderr, ">>>error reading: %s\n", argv[option_index + 1]);



                    err = (1 > err) ? 1 : err;
                }


                if ((size_t)file_des > files_size)
                {
                    files = (int*)realloc(files, files_size + 200 * sizeof(int));
                    files_size += 200;
                }
                files[file_des] = curr_fd;
                file_des++;


                if (files == NULL)
                {
                    fprintf(stderr, "realloc error on Files array\n" );
                    exit(1);
                }

                argno++;

            }
            else
            {
                //no argument too rdonly file
                fprintf(stderr, "No argument to --rdonly option.\n");

                exit(1);
            }
            break;
        case 'w':
            if (optarg)
            {
                if (verbose)
                {
                    // do verbose stuff
                    fprintf(stdout, "--wronly %s\n", optarg);
                }
                //do something here
                curr_fd = open(optarg, O_WRONLY);
                if (curr_fd != -1)
                {

                }
                else
                {
                    fprintf(stderr, ">>>error writing: %s\n", argv[option_index + 1]);


                    err = (1 > err) ? 1 : err;
                }
                if ((size_t)file_des > files_size)
                {
                    files = (int*)realloc(files, files_size + 200 * sizeof(int));
                    files_size += 200;
                }
                files[file_des] = curr_fd;
                file_des++;
                argno++;

            }
            else
            {
                //no argument too rdonly file
                fprintf(stderr, "No argument to --wronly option.\n");

                exit(1);
            }
            break;
        case 'v':
            verbose = 1;
            //argno++;
            break;
        case 'c':
            if (argno < argc - 3)
            {
                argno++;
                int i = (int)(atoi(argv[argno]));
                //printf("%s\n", argv[argno]);
                int o = (int)(atoi(argv[argno + 1]));
                //printf("%s\n", argv[argno+1]);
                int e = (int)(atoi(argv[argno + 2]));
                //printf("%s\n", argv[argno+2]);

                if (i >= file_des || i < 0 || o >= file_des || o < 0 || e >= file_des || e < 0)
                {
                    fprintf(stderr, "Command was given a file descriptor out of the range.\n" );
                    exit(1);
                }

                if (files[i] < 0 || files[o] < 0 || files[e] < 0)
                {
                    fprintf(stderr, "The file descsriptor you tried to reference is invalid. Does the file exist?\n");
                    exit(1);
                }


                argno += 3;
                char *command = "";
                while (argno < argc)
                {
                    if (strstr(argv[argno], "--") == argv[argno])
                        break;
                    command = concatString(command, argv[argno]);
                    command = concatString(command, " ");

                    argno++;
                    optind++;
                }
                argno--;

                if (verbose)
                {
                    fprintf(stdout, "--command %d %d %d %s\n", i, o, e, command);
                }


                char *cmds[4];
                cmds[0] = "/bin/bash";
                cmds[1] = "-c";
                cmds[2] = command;
                cmds[3] = NULL;

                pid_t proc = fork();
                if (proc == -1)
                {
                    // failed to create a subprocess
                    fprintf(stderr, "Failed to create subprocess\n");
                }
                else if (proc == 0)
                {
                    dup2(files[i], 0);
                    close(files[i]);
                    dup2(files[o], 1);
                    close(files[o]);
                    dup2(files[e], 2);
                    close(files[e]);

                    execvp(cmds[0], cmds);
                    perror("failed to execute the command\n");


                }
                else
                {

                }
            }
            else
            {
                fprintf(stderr, "Command requires at least three arguments. \n");
                err = (1 > err) ? 1 : err;
            }
            break;
        default:
            fprintf(stderr, "unrecognized option %s\n", argv[argno]);
            err = 1;
            break;
        }
        argno++;
    }

    while (file_des >= 0)
    {
        close(files[file_des]);
        file_des--;
    }

    free(files);
    exit(err);


}