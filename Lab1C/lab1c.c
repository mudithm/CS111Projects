/*
NAME: Mudith Mallajosyula
EMAIL: mudithm@g.ucla.edu
ID: 404937201
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
#include <ctype.h>
#include <sys/time.h>
#include <sys/resource.h>

static int verbose = 0;
static int oflag = 0;
static int wait_flag = 0;
static int err = 0;
static int profile_flag = 0;
extern int optind;
pid_t proc = 0;

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

// causes a segmentation fault by attempting to
// dereference a null pointer
void segFaultFunction()
{
    char * p = NULL;
    *p = 'a';
}

void signal_handler(int signal_number)
{
    fprintf(stderr, "%d caught\n", signal_number);
    err = (signal_number > err) ? signal_number : err;
    exit(err);
}

int isNumber(char *str)
{
    int len = strlen(str);
    int i = 0;
    while (i < len)
    {
        if (! isdigit(str[i]))
            return 0;
        i++;
    }
    return 1;
}

int main(int argc, char *argv[])
{
    int file_des = 0;
    int curr_fd = 0;
    size_t files_size = 20;
    int *files = malloc(files_size * sizeof(int));
    int *pipes = malloc(files_size * sizeof(int));
    if (files == NULL)
    {
        fprintf(stderr, "Malloc error on Files array\n" );
        err = (1 > err) ? 1 : err;
        exit(err);
    }

    if (pipes == NULL)
    {
        fprintf(stderr, "Malloc error on pipes array\n" );
        err = (1 > err) ? 1 : err;
        exit(err);
    }

    int wait_invoked = 0, comms = 0;
    for (int z = argc - 1; z > 0; z--)
    {
        if (strcmp(argv[z], "--wait") == 0)
            wait_invoked = 1;
        if (wait_invoked && strcmp(argv[z], "--command") == 0)
        {
            comms++;
        }
    }

        struct rusage waitbuf;

    const struct option opts[] =
    {
        {"rdonly", required_argument, 0, 'r'},  //rdonly:       r x
        {"wronly", required_argument, 0, 'w'},  //wronly:       w x
        {"rdwr", required_argument, 0, 'd'},    //rdwr:         d x

        {"command", required_argument, 0, 'c'}, //command:      c x
        {"wait", no_argument, 0, 'b'},          //wait:         b


        {"close", no_argument, 0, 'e'},         //close:        e
        {"verbose", no_argument, 0, 'v'},       //verbose:      v
        {"profile", no_argument, 0, 'f'},//profile:   f
        {"abort", no_argument, 0, 'g'},         //abort:        g x
        {"catch", no_argument, 0, 'h'},         //catch:        h x
        {"ignore", no_argument, 0, 'j'},        //ignore:       j x
        {"default", no_argument, 0, 'k'},       //default:      k x
        {"pause", no_argument, 0, 'm'},         //pause:        m x



        {"append", no_argument, 0, 'a'},        //append:       a x
        {"cloexec", no_argument, 0, 'l'},       //cloexec:      l x
        {"creat", no_argument, 0, 't'},         //creat:        t x
        {"directory", no_argument, 0, 'i'},     //directory:    i x
        {"dsync", no_argument, 0, 'n'},         //dsync:        n x
        {"excl", no_argument, 0, 'x'},          //excl:         x x
        {"nofollow", no_argument, 0, 'o'},      //nofollow:     o x
        {"rsync", no_argument, 0, 's'},         //rsync:        s x
        {"sync", no_argument, 0, 'y'},          //sync:         y x
        {"trunc", no_argument, 0, 'u'},         //trunc:        u x

        {"pipe", no_argument, 0, 'p'},          //pipe:         p x

    };

    int option_index = 0;
    int argno = 1;
    long c = 0;

    int i = 0;
    int numCommands = 0;
    char *commands[argc];
    pid_t subprocesses[argc];


    while (1)
    {
        c = getopt_long(argc, argv, "", opts, &option_index);

        if (c == -1)
            break;

        switch ((char)(c))
        {
        case 'r':
        {   struct rusage buf;
            if (optarg)
            {
                if (verbose)
                {
                    fprintf(stdout, "--rdonly %s\n", optarg);
                }
                curr_fd = open(optarg, O_RDONLY | oflag, 0644);
                if (curr_fd != -1)
                {

                }
                else
                {
                    fprintf(stderr, ">>>error reading: %s\n", optarg);
                    oflag = 0;
                    err = (1 > err) ? 1 : err;
                }


                if ((size_t)file_des > files_size)
                {
                    files = (int*)realloc(files, files_size + 200 * sizeof(int));
                    pipes = (int*)realloc(pipes, files_size + 200 * sizeof(int));
                    files_size += 200;
                }
                files[file_des] = curr_fd;
                pipes[file_des] = 0;
                file_des++;


                if (files == NULL)
                {
                    fprintf(stderr, "realloc error on Files array\n" );
                    err = (1 > err) ? 1 : err;
                    exit(err);
                }

                if (pipes == NULL)
                {
                    fprintf(stderr, "Malloc error on pipes array\n" );
                    err = (1 > err) ? 1 : err;
                    exit(err);
                }
                argno++;

            }
            else
            {
                //no argument too rdonly file
                fprintf(stderr, "No argument to --rdonly option.\n");

                err = (1 > err) ? 1 : err;
                exit(err);
            }
            oflag = 0;
            if (profile_flag)
            {
                getrusage(RUSAGE_SELF, &buf);
                printf("Time Elapsed for Option --rdonly:   User: %lfs  System CPU: %lfs Total time: %lfs\n", buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0), buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0), buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0) + buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0));
                fflush(stdout);
            }
        }
        break;
        case 'd':
        {   struct rusage buf;
            if (optarg)
            {
                if (verbose)
                {
                    fprintf(stdout, "--rdwr %s\n", optarg);
                }
                curr_fd = open(optarg, O_RDWR | oflag, 0644);
                if (curr_fd != -1)
                {

                }
                else
                {
                    fprintf(stderr, ">>>error reading: %s\n", optarg);
                    oflag = 0;
                    err = (1 > err) ? 1 : err;
                }


                if ((size_t)file_des > files_size)
                {
                    files = (int*)realloc(files, files_size + 200 * sizeof(int));
                    pipes = (int*)realloc(pipes, files_size + 200 * sizeof(int));

                    files_size += 200;
                }
                files[file_des] = curr_fd;
                pipes[file_des] = 0;

                file_des++;


                if (files == NULL)
                {
                    fprintf(stderr, "realloc error on Files array\n" );
                    exit(1);
                }
                if (pipes == NULL)
                {
                    fprintf(stderr, "Malloc error on pipes array\n" );
                    exit(1);
                }
                argno++;
                if (profile_flag)
                {
                    getrusage(RUSAGE_SELF, &buf);
                    printf("Time Elapsed for Option --rdwr:   User: %lfs  System CPU: %lfs Total time: %lfs\n", buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0), buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0), buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0) + buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0));
                    fflush(stdout);
                }
            }
            else
            {
                //no argument too rdonly file
                fprintf(stderr, "No argument to --rdwr option.\n");
                err = (1 > err) ? 1 : err;
                exit(err);
            }

            oflag = 0;
        }
        break;
        case 'w':
        {   struct rusage buf;
            if (optarg)
            {
                if (verbose)
                {
                    // do verbose stuff
                    fprintf(stdout, "--wronly %s\n", optarg);
                }
                //do something here
                curr_fd = open(optarg, O_WRONLY | oflag, 0644);
                //fprintf(stderr, "lllll%d\n", oflag);
                if (curr_fd != -1)
                {

                }
                else
                {
                    fprintf(stderr, ">>>error writing: %s\n", argv[option_index + 1]);
                    oflag = 0;


                    err = (1 > err) ? 1 : err;
                }
                if ((size_t)file_des > files_size)
                {
                    files = (int*)realloc(files, files_size + 200 * sizeof(int));
                    pipes = (int*)realloc(pipes, files_size + 200 * sizeof(int));
                    files_size += 200;
                }
                if (files == NULL)
                {
                    fprintf(stderr, "Malloc error on files array\n" );
                    err = (1 > err) ? 1 : err;
                    exit(err);
                }
                if (pipes == NULL)
                {
                    fprintf(stderr, "Malloc error on pipes array\n" );
                    err = (1 > err) ? 1 : err;
                    exit(err);
                }
                files[file_des] = curr_fd;
                pipes[file_des] = 0;

                file_des++;
                argno++;

                if (profile_flag)
                {
                    getrusage(RUSAGE_SELF, &buf);
                    printf("Time Elapsed for Option --wronly:   User: %lfs  System CPU: %lfs Total time: %lfs\n", buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0), buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0), buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0) + buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0));
                    fflush(stdout);
                }
            }
            else
            {
                //no argument too rdonly file
                fprintf(stderr, "No argument to --wronly option.\n");

                err = (1 > err) ? 1 : err;
                exit(err);
            }
            oflag = 0;
        }
        break;
        case 'v':
        {   struct rusage buf;
            if (verbose)
                printf("--verbose\n");
            verbose = 1;
            if (profile_flag)
            {
                getrusage(RUSAGE_SELF, &buf);
                printf("Time Elapsed for Option --verbose:   User: %lfs  System CPU: %lfs Total time: %lfs\n", buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0), buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0), buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0) + buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0));
                fflush(stdout);
            }
            //argno++;
            break;
        }
        case 'c':
        {   struct rusage buf;
            if (argno < argc - 3)
            {
                argno++;
                int i = (int)(atoi(argv[argno]));
                //printf("%s\n", argv[argno]);
                int o = (int)(atoi(argv[argno + 1]));
                //printf("%s\n", argv[argno+1]);
                int e = (int)(atoi(argv[argno + 2]));
                //printf("%s\n", argv[argno+2]);

                argno += 3;
                optind += 2;
                char *command = "";
                while (argno < argc)
                {
                    if (strstr(argv[argno], "--") == argv[argno])
                        break;
                    command = concatString(command, argv[argno]);
                    command = concatString(command, " ");

                    // printf(">>>>>Command:%s\n", command);

                    argno++;
                    optind++;
                }

                argno--;
                optind--;

                if (i >= file_des || i < 0 || o >= file_des || o < 0 || e >= file_des || e < 0)
                {
                    fprintf(stderr, "Command was given a file descriptor out of the range.\n" );
                    err = (1 > err) ? 1 : err;
                    break;
                }

                if (files[i] < 0 || files[o] < 0 || files[e] < 0)
                {
                    fprintf(stderr, "The file descsriptor you tried to reference is invalid. Is the file open?\n");
                    err = (1 > err) ? 1 : err;
                    break;
                }


                //printf(">>Argno: %d Arg: %s Optind: %d Optarg: %s\n", argno, argv[argno], optind, argv[optind]);


                if (verbose)
                {
                    fprintf(stdout, "--command %d %d %d %s\n", i, o, e, command);
                }

                if (strcmp(command, "") == 0 || strcmp(command, " ") == 0)
                {
                    fprintf(stderr, "--command called without a shell command argument.\n");
                    err = (1 > err) ? 1 : err;
                    break;
                }

                char *cmds[4];
                char* tmp = strstr(command, "bash -c");
                if (tmp != NULL)
                {
                    tmp = command + 7;
                    cmds[0] = "/bin/bash";
                    cmds[1] = "-c";
                    cmds[2] = tmp;
                    cmds[3] = NULL;

                } else
                {
                    cmds[0] = "/bin/bash";
                    cmds[1] = "-c";
                    cmds[2] = command;
                    cmds[3] = NULL;
                }




                proc = fork();
                subprocesses[numCommands] = proc;

                if (proc == -1)
                {
                    // failed to create a subprocess
                    fprintf(stderr, "Failed to create subprocess\n");
                    err = (err > 1) ? err : 1;
                    break;
                }
                else if (proc == 0) // Child process
                {
                    //close(STDIN_FILENO);
                    dup2(files[i], 0);

                    //close(STDOUT_FILENO);
                    dup2(files[o], 1);

                    //close(STDERR_FILENO);
                    dup2(files[e], 2);


                    if (pipes[i] > 0)
                    {
                        if (i + 1 < file_des && pipes[i + 1] > 0)
                        {
                            close(files[i + 1]);
                        }
                    }
                    if (pipes[o] > 0)
                    {
                        if (o - 1 >= 0 && pipes[0 - 1] > 0)
                        {
                            close(files[o - 1]);
                        }
                    }



                    execvp(cmds[0], cmds);
                    perror("failed to execute the command\n");
                }
                else // Parent process
                {
                    if (profile_flag)
                    {
                        getrusage(RUSAGE_SELF, &buf);
                        printf("Time Elapsed for Option --command:   User: %lfs  System CPU: %lfs Total time: %lfs\n", buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0), buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0), buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0) + buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0));
                        fflush(stdout);
                    }

                    commands[numCommands] = command;
                    numCommands++;
                    if (wait_invoked && numCommands < comms) {
                        ;
                    } else {
                        if (pipes[i] > 0)
                        {
                            pipes[i] = -1;
                            close(files[i]);
                        }
                        if (pipes[o] > 0)
                        {
                            pipes[o] = -1;
                            close(files[o]);
                        }

                    }





                }
            }
            else
            {
                fprintf(stderr, "Command requires at least three arguments. \n");
                fflush(stderr);
                err = (1 > err) ? 1 : err;
            }
            optind = argno + 1;
        }
        break;
        case 'l':
        {   struct rusage buf;
            oflag |= O_CLOEXEC;
            if (verbose)
                fprintf(stdout, "--cloexec\n");
            if (profile_flag)
            {
                getrusage(RUSAGE_SELF, &buf);
                printf("Time Elapsed for Option --cloexec:   User: %lfs  System CPU: %lfs Total time: %lfs\n", buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0), buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0), buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0) + buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0));
                fflush(stdout);
            }
            //argno++;
        }
        break;
        case 'a':
        {   struct rusage buf;
            oflag |= O_APPEND;
            if (verbose)
                fprintf(stdout, "--append\n");
            if (profile_flag)
            {
                getrusage(RUSAGE_SELF, &buf);
                printf("Time Elapsed for Option --append:   User: %lfs  System CPU: %lfs Total time: %lfs\n", buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0), buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0), buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0) + buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0));
                fflush(stdout);
            }
            //argno++;
        } break;
        case 't':
        {   struct rusage buf;
            oflag |= O_CREAT;
            if (verbose)
                fprintf(stdout, "--creat\n");
            if (profile_flag)
            {
                getrusage(RUSAGE_SELF, &buf);
                printf("Time Elapsed for Option --creat:   User: %lfs  System CPU: %lfs Total time: %lfs\n", buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0), buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0), buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0) + buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0));
                fflush(stdout);
            }
            //argno++;
        } break;
        case 'i':
        {   struct rusage buf;
            oflag |= O_DIRECTORY;
            if (verbose)
                fprintf(stdout, "--directory\n");
            if (profile_flag)
            {
                getrusage(RUSAGE_SELF, &buf);
                printf("Time Elapsed for Option --directory:   User: %lfs  System CPU: %lfs Total time: %lfs\n", buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0), buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0), buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0) + buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0));
                fflush(stdout);
            }
            //argno++;
        } break;
        case 'n':
        {   struct rusage buf;
            oflag |= O_DSYNC;
            if (verbose)
                fprintf(stdout, "--dsync\n");
            if (profile_flag)
            {
                getrusage(RUSAGE_SELF, &buf);
                printf("Time Elapsed for Option --dsync:   User: %lfs  System CPU: %lfs Total time: %lfs\n", buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0), buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0), buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0) + buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0));
                fflush(stdout);
            }
            //argno++;
        } break;
        case 'x':
        {   struct rusage buf;
            oflag |= O_EXCL;
            if (verbose)
                fprintf(stdout, "--excl\n");
            if (profile_flag)
            {
                getrusage(RUSAGE_SELF, &buf);
                printf("Time Elapsed for Option --excl:   User: %lfs  System CPU: %lfs Total time: %lfs\n", buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0), buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0), buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0) + buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0));
                fflush(stdout);
            }
            //argno++;
        } break;
        case 'o':
        {   struct rusage buf;
            oflag |= O_NOFOLLOW;
            if (verbose)
                fprintf(stdout, "--nofollow\n");
            if (profile_flag)
            {
                getrusage(RUSAGE_SELF, &buf);
                printf("Time Elapsed for Option --nofollow:   User: %lfs  System CPU: %lfs Total time: %lfs\n", buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0), buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0), buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0) + buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0));
                fflush(stdout);
            }
            //argno++;
        } break;
        case 's':
        {   struct rusage buf;
            oflag |= O_RSYNC;
            if (verbose)
                fprintf(stdout, "--rsync\n");
            if (profile_flag)
            {
                getrusage(RUSAGE_SELF, &buf);
                printf("Time Elapsed for Option --rsync:   User: %lfs  System CPU: %lfs Total time: %lfs\n", buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0), buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0), buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0) + buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0));
                fflush(stdout);
            }
            //argno++;
        } break;
        case 'y':
        {   struct rusage buf;
            oflag |= O_SYNC;
            if (verbose)
                fprintf(stdout, "--sync\n");
            if (profile_flag)
            {
                getrusage(RUSAGE_SELF, &buf);
                printf("Time Elapsed for Option --sync:   User: %lfs  System CPU: %lfs Total time: %lfs\n", buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0), buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0), buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0) + buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0));
                fflush(stdout);
            }
            //argno++;
        } break;
        case 'u':
        {   struct rusage buf;
            oflag |= O_TRUNC;
            if (verbose)
                fprintf(stdout, "--trunc\n");
            if (profile_flag)
            {
                getrusage(RUSAGE_SELF, &buf);
                printf("Time Elapsed for Option --trunc:   User: %lfs  System CPU: %lfs Total time: %lfs\n", buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0), buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0), buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0) + buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0));
                fflush(stdout);
            }
            //argno++;
        } break;
        case 'p':
        {   struct rusage buf;
            int fdes[2];
            if (pipe(fdes) == -1)
            {
                perror("pipe failure");
                fprintf(stderr, "Pipe Failed" );


                err = (1 > err) ? 1 : err;
            }
            else
            {
                if (verbose)
                    printf("--pipe\n");
            }
            if ((size_t)file_des > files_size)
            {
                files = (int*)realloc(files, files_size + 200 * sizeof(int));
                pipes = (int*)realloc(pipes, files_size + 200 * sizeof(int));
                files_size += 200;
            }
            //printf("Pipe: %d, %d, FD index: %d, %d\n", fdes[0], fdes[1], file_des, file_des+1);
            files[file_des] = fdes[0];
            pipes[file_des] = 1;
            file_des++;
            files[file_des] = fdes[1];
            pipes[file_des] = 1;
            file_des++;
            if (profile_flag)
            {
                getrusage(RUSAGE_SELF, &buf);
                printf("Time Elapsed for Option --pipe:   User: %lfs  System CPU: %lfs Total time: %lfs\n", buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0), buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0), buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0) + buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0));
                fflush(stdout);
            }

            //printf("Current Argument: %s\n", argv[argno]);
        }
        break;
        case 'g':
        {   struct rusage buf;
            if (verbose) {
                fprintf(stdout, "--abort\n");
                fflush(stdout);
            }
            segFaultFunction();
            if (profile_flag)
            {
                getrusage(RUSAGE_SELF, &buf);
                printf("Time Elapsed for Option --abort:   User: %lfs  System CPU: %lfs Total time: %lfs\n", buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0), buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0), buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0) + buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0));
                fflush(stdout);
            }
            fprintf(stderr, "\n");
            exit(139);
        } break;
        case 'h':
        {   struct rusage buf;

            //printf("ddd %s %s\n", argv[optind], argv[argno]);
            //fflush(stdout);
            //optind++;
            //argno++;
            if (optind < argc && isNumber(argv[optind]))
            {

                int sig = (int)(atoi(argv[optind]));
                //printf(">>>>%d\n", sig);
                //fflush(stdout);
                signal(sig, signal_handler);
                optind++;
                argno++;
                if (verbose) {
                    fprintf(stdout, "--catch %d\n", sig);
                    fflush(stdout);
                }
                if (profile_flag)
                {
                    getrusage(RUSAGE_SELF, &buf);
                    printf("Time Elapsed for Option --catch:   User: %lfs  System CPU: %lfs Total time: %lfs\n", buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0), buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0), buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0) + buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0));
                    fflush(stdout);
                }
            }
            else
            {
                fprintf(stderr, "No signal passed to the --catch option\n");
            }

        }
        break;
        case 'k':
        {   struct rusage buf;
            if (optind < argc && isNumber(argv[optind]))
            {
                int sig = (int)(atoi(argv[optind]));
                //printf(">>>>%d\n", sig);
                //fflush(stdout);
                signal(sig, SIG_DFL);
                if (verbose) {
                    fprintf(stdout, "--default %d\n", sig);
                    fflush(stdout);
                }
                if (profile_flag)
                {
                    getrusage(RUSAGE_SELF, &buf);
                    printf("Time Elapsed for Option --default:   User: %lfs  System CPU: %lfs Total time: %lfs\n", buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0), buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0), buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0) + buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0));
                    fflush(stdout);
                }
            } else
            {
                fprintf(stderr, "No signal passed to the --default option\n");
            }
        } break;
        case 'e':
        {   struct rusage buf;
            if (optind < argc && isNumber(argv[optind]))
            {
                int num = (int)(atoi(argv[optind]));
                //printf(">>>>%d,   %s\n", num, argv[optind]);
                //fflush(stdout);
                if (num < (int)(file_des)) {
                    close(files[num]);
                    files[num] = -1;
                    if (pipes[num] > 0)
                        pipes[num] = -1;
                    if (verbose)
                        printf("--close %d\n", num);
                }
                else
                {
                    fprintf(stderr, "Error closing file descriptor %d\n", num);
                    err = (1 > err) ? 1 : err;
                }
                if (profile_flag)
                {
                    getrusage(RUSAGE_SELF, &buf);
                    printf("Time Elapsed for Option --close:   User: %lfs  System CPU: %lfs Total time: %lfs\n", buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0), buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0), buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0) + buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0));
                    fflush(stdout);
                }
            } else
            {
                fprintf(stderr, "No argument was passed to --close\n");
                err = (1 > err) ? 1 : err;
            }
            optind++;
            argno++;
            //printf(">>>>   %s   %s\n", argv[optind], argv[argno]);
            //fflush(stdout);
        } break;
        case 'm':
        {   struct rusage buf;
            pause();
            if (profile_flag)
            {
                getrusage(RUSAGE_SELF, &buf);
                printf("Time Elapsed for Option --pause:   User: %lfs  System CPU: %lfs Total time: %lfs\n", buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0), buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0), buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0) + buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0));
                fflush(stdout);
            }
        } break;
        case 'j':
        {   struct rusage buf;
            if (optind < argc && isNumber(argv[optind]))
            {
                int sig = (int)(atoi(argv[optind]));
                //printf(">>>>%d\n", sig);
                //fflush(stdout);
                signal(sig, SIG_IGN);
                if (verbose)
                    printf("--ignore %d\n", sig);
                if (profile_flag)
                {
                    getrusage(RUSAGE_SELF, &buf);
                    printf("Time Elapsed for Option --ignore:   User: %lfs  System CPU: %lfs Total time: %lfs\n", buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0), buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0), buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0) + buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0));
                    fflush(stdout);
                }
            } else
            {
                fprintf(stderr, "Invalid argument for --ignore\n");
            }
        } break;
        case 'b':
        {   struct rusage buf;
            if (verbose)
                printf("--wait\n");
            if (profile_flag)
            {
                getrusage(RUSAGE_SELF, &buf);
                printf("Time Elapsed for Option --wait:   User: %lfs  System CPU: %lfs Total time: %lfs\n", buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0), buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0), buf.ru_utime.tv_sec + (buf.ru_utime.tv_usec / 1000000.0) + buf.ru_stime.tv_sec + (buf.ru_stime.tv_usec / 1000000.0));

                fflush(stdout);
            }
            wait_flag = numCommands;
        } break;
        case 'f':
        {
            if (verbose)
            {
                printf("--profile\n");
                fflush(stdout);
            }
        } profile_flag = 1;
        break;
        default:
            fprintf(stderr, "unrecognized option.\n");
            err = (1 > err) ? 1 : err;
            exit(err);
        }
        argno++;


        ///printf(">>>>%s, %s\n", argv[argno], argv[optind]);
    }

    int commands_processed = 0;
    int commands_finished[numCommands];
    int exit_stat;
    for (int k = 0; k < numCommands; k++)
        commands_finished[k] = 0;

    if (wait_flag > 0 && proc > 0)
    {
        while (commands_processed < wait_flag)
        {
            for (i = 0; i < wait_flag; i++)
            {
                if (commands_finished[i] != 1)
                {
                    if (waitpid(subprocesses[i], &exit_stat, 0) > 0)
                    {
                        if (profile_flag) {
                            getrusage(RUSAGE_CHILDREN, &waitbuf);
                            printf("Time Elapsed for All Children:   User: %lfs  System CPU: %lfs Total time: %lfs\n", waitbuf.ru_utime.tv_sec + (waitbuf.ru_utime.tv_usec / 1000000.0), waitbuf.ru_stime.tv_sec + (waitbuf.ru_stime.tv_usec / 1000000.0), waitbuf.ru_utime.tv_sec + (waitbuf.ru_utime.tv_usec / 1000000.0) + waitbuf.ru_stime.tv_sec + (waitbuf.ru_stime.tv_usec / 1000000.0));

                        }

                        if ( WIFEXITED(exit_stat) )
                        {
                            err = (err > WEXITSTATUS(exit_stat)) ? err : WEXITSTATUS(exit_stat);
                            fprintf(stdout, "exit %d %s\n", WEXITSTATUS(exit_stat), commands[i]);
                        }
                        else if ( WIFSIGNALED(exit_stat) )
                        {
                            err = (err > 128 + WTERMSIG(exit_stat)) ? err : 128 + WTERMSIG(exit_stat);
                            fprintf(stdout, "signal %d %s\n", WTERMSIG(exit_stat), commands[i]);
                        }
                        else if ( WIFSTOPPED(exit_stat) )
                        {
                            err = (err > 128 + WSTOPSIG(exit_stat)) ? err : 128 + WSTOPSIG(exit_stat);
                            fprintf(stdout, "signal %d %s\n", WSTOPSIG(exit_stat), commands[i]);
                        }
                        else
                        {
                            printf("Something else happened.\n");
                        }
                        commands_processed++;
                        commands_finished[i] = 1;
                    }
                }
            }
        }
    }



    commands_processed = 0;


    if (wait_flag < numCommands)
    {
        while (commands_processed < numCommands - wait_flag)
        {
            for (i = wait_flag; i < numCommands; i++)
            {
                if (commands_finished[i] != 1)
                {
                    if (waitpid(subprocesses[i], &exit_stat, WNOHANG) != 0)
                    {
                        commands_processed++;
                        commands_finished[i] = 1;
                    }
                }
            }
        }
    }



    /*
        while (file_des >= 0)
        {
            close(files[file_des]);
            file_des--;
        }
    */
    free(files);
    free(pipes);
    exit(err);
}

