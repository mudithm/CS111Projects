#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <mraa/gpio.h>
#include <mraa/aio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <time.h>
#include <poll.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/evp.h>

sig_atomic_t volatile run_flag = 1;

// NTC Thermistor constants
static const int thermistor_B = 4275;
static const int divider_resistance = 100000;

static FILE *output_file = NULL;
static FILE *f_recv = NULL;
static char* log_file = NULL;
static int report = 1;
static int socket_fd = -1;

static SSL_CTX* context;
static SSL* ssl;

static int id = -1;
static char* host = NULL;
static int port_no = -1;
static char* port = NULL;

static int sample_rate = 1000;
static int fahrenheit = 1;

static mraa_gpio_context button;
static mraa_aio_context temp;


// print to STDOUT and to the log file, if present
void log_report(FILE * output, int comd, char* format_string, ...) {
    if (!report)
        return;
    va_list args;

    // rather than stdout, print to tls
    va_start(args, format_string);
    if (!comd) {
        char output_string[1000];
        vsprintf(output_string, format_string, args);
        if (SSL_write(ssl, output_string, strlen(output_string)) < 0)
        {
            fprintf(stderr, "Error printing to tls server!\n");
            exit(2);
        }
    }
    va_end(args);


    va_start(args, format_string);
    if (output != NULL)
        if (vfprintf(output, format_string, args) < 0) {
            fprintf(stderr, "Error printing to log file!\n");
            exit(2);
        }
    va_end(args);

    fflush(stdout);
    fflush(output_file);
}

void do_when_interrupted()
{
    time_t current_time;
    struct tm *time_info;
    char timebuf[80];
    time (&current_time);
    time_info = localtime( &current_time );

    strftime (timebuf, 80, "%H:%M:%S", time_info);

    log_report(output_file, 0, "%s SHUTDOWN\n", timebuf);
    run_flag = 0;
    report = 0;

    if (output_file != NULL)
        fclose(output_file);
    if (f_recv != NULL)
        fclose(f_recv);

    SSL_shutdown(ssl);
    close(socket_fd);
    mraa_gpio_close(button);
    mraa_aio_close(temp);

    exit(0);
}

void process_commands(char* buffer, int size)
{
    // processing multiple commands
    int i = 0, curr_cmd = 0;

    while (i < size)
    {
        if (buffer[i] == '\n') {
            buffer[i] = 0;

            if (strcmp(buffer + curr_cmd, "OFF") == 0) {
                if (!report) {
                    report = 1;
                    log_report(output_file, 1, "OFF\n");
                }
                do_when_interrupted();
            }


            log_report(output_file, 1, "%s\n", buffer + curr_cmd);

            if (strcmp(buffer + curr_cmd, "STOP") == 0) {
                //        printf("stop command received\n");
                //        fflush(stdout);
                report = 0;
            }

            if (strcmp(buffer + curr_cmd, "START") == 0 && !report)
            {
                //           printf("start command received\n");
                //         fflush(stdout);
                report = 1;
                log_report(output_file, 1, "START\n");
            }


            if (strstr(buffer + curr_cmd, "LOG") == buffer + curr_cmd)
            {
                if (!report)
                {
                    report = 1;
                    log_report(output_file, 1, buffer + curr_cmd);
                    report = 0;
                }
            }


            if ( ( strlen(buffer + curr_cmd) == 7 ) && (strstr((buffer + curr_cmd), "SCALE=") == (buffer + curr_cmd)))
            {
                //           printf("Scale command read. %c\n", buffer[6 + curr_cmd]);
                //         fflush(stdout);
                if (buffer[6 + curr_cmd] == 'C')
                    fahrenheit = 0;
                else if (buffer[6 + curr_cmd] == 'F')
                    fahrenheit = 1;
                else
                    fprintf(stderr, "Incorrect argument for SCALE %s\n", buffer  + curr_cmd + 6 );
            }


            if (strlen(buffer + curr_cmd) > 7 && strstr((buffer + curr_cmd), "PERIOD=") == (buffer + curr_cmd))
            {
                //        printf("period command read %s\n", buffer + curr_cmd);
                //      fflush(stdout);
                unsigned int c = 7 + curr_cmd;
                while (c < strlen(buffer + curr_cmd))
                {
                    if (buffer[c] == '\n')
                    {
                        buffer[c] = '\0';
                        break;
                    }
                    if (! isdigit(buffer[c]))
                        return;
                    c++;
                }
                sample_rate = atoi(buffer + 7 + curr_cmd) * 1000;
            }

            curr_cmd = i + 1;

        }
        i++;
    }


}

int main(int argc, char** argv)
{

    time_t current_time;
    struct tm *time_info;
    char timebuf [80];


    const struct option opts[] =
    {
        {"period", required_argument, 0, 'p'},
        {"scale", required_argument, 0, 's'},
        {"log", required_argument, 0, 'l'},
        {"id", required_argument, 0, 'i'},
        {"host", required_argument, 0, 'h'},
        {NULL, 0, NULL, 0}
    };

    // holds the current index of the opts[] array for getopt_long
    int option_index = 0;

    long c = 0;
    while (option_index < argc)
    {
        // parse the options passed to the program, if any
        c = getopt_long(argc, argv, "", opts, &option_index);

        // check non-switch arguments or end of argument list
        if (c == -1)
        {
            break;
        }


        switch ((char)(c))
        {
        case 'p':
            if (optarg) {
                unsigned int samp = 0;
                while (samp < strlen(optarg))
                {
                    if (optarg[samp] == '\n')
                    {
                        optarg[samp] = '\0';
                        break;
                    }
                    if (! isdigit(optarg[samp]))
                    {
                        fprintf(stderr, "Invalid sample rate %s\n", optarg);
                        exit(1);
                    }
                    samp++;
                }
                sample_rate = atoi(optarg);
                if (sample_rate < 1)
                {
                    fprintf(stderr, "Invalid sample rate: %s seconds/sample\n", optarg);
                    exit(1);
                }
                sample_rate *= 1000;
            }
            else
            {
                fprintf(stderr, "No argument passed to --period\n");
                exit(1);
            }
            break;
        case 's':
            if (optarg) {
                if (! (strcmp("C", optarg) == 0 || strcmp("F", optarg) == 0) )
                {
                    fprintf(stderr, "Invalid scale: %s\n", optarg);
                    exit(1);
                }
                else if (strcmp("C", optarg) == 0)
                    fahrenheit = 0;
            }
            else
            {
                fprintf(stderr, "No argument passed to --scale\n");
                exit(1);
            }
            break;
        case 'l':
            if (optarg) {
                if (strcmp(optarg, "") == 0)
                {
                    fprintf(stderr, "Invalid output file %s!\n", optarg);
                    exit(1);
                }
                else
                {
                    log_file = optarg;

                    output_file = fopen(optarg, "ab");
                    if (output_file == NULL)
                    {
                        fprintf(stderr, "Unable to open file %s\n", optarg);
                        exit(1);
                    }
                }
            }
            else
            {
                fprintf(stderr, "No argument passed to --log\n");
                exit(1);
            }
            break;
        case 'i':
            if (optarg) {
                if (strlen(optarg) != 9)
                {
                    fprintf(stderr, "ID has invalid length!\n");
                    exit(1);
                }
                else {
                    unsigned int ind = 0;
                    while (ind < 9)
                    {
                        if (optarg[ind] == '\n')
                        {
                            optarg[ind] = '\0';
                            break;
                        }
                        if (! isdigit(optarg[ind]))
                        {
                            fprintf(stderr, "Invalid ID number: %s\n", optarg);
                            exit(1);
                        }
                        ind++;
                    }
                    id = atoi(optarg);
                    if (id < 1)
                    {
                        fprintf(stderr, "Invalid ID number: %s\n", optarg);
                        exit(1);
                    }
                }
            }
            break;
        case 'h':
            if (optarg)
            {
                host = optarg;
            }
            break;
        default:
            fprintf(stderr, "Bad argument.\n");
            exit(1);

            break;
        }

    }

    if (argc - optind  > 1)
    {
        fprintf(stderr, "multiple non-switch arguments passed.\n");
        exit(1);
    }
    else if (optind == argc)
    {
        printf("%s\n", argv[0]);
        fprintf(stderr, "No non-switch argument was passed!\n");
        exit(1);
    }
    else
    {
        char *b = argv[optind];
        unsigned int ind = 0;
        while (ind < strlen(b))
        {
            if (b[ind] == '\n')
            {
                b[ind] = '\0';
                break;
            }
            if (! isdigit(b[ind]))
            {
                fprintf(stderr, "Invalid port number: %s\n", b);
                exit(1);
            }
            ind++;
        }
        port_no = atoi(b);
        if (port_no < 1)
        {
            fprintf(stderr, "Invalid port number: %s\n", b);
            exit(1);
        }
        port = argv[optind];

    }


    if (port_no == -1)
    {
        fprintf(stderr, "No port number passed!\n");
        exit(1);
    }
    else if (id == -1)
    {
        fprintf(stderr, "No id passed!\n");
        exit(1);
    }
    else if (host == NULL)
    {
        fprintf(stderr, "No host address passed!\n");
        exit(1);
    }
    else if (log_file == NULL)
    {
        fprintf(stderr, "No log file passed!\n");
        exit(1);
    }

    // printf("port number: %s\n", port);
    // printf("host: %s\n", host);


// open tcp connection
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s;


    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM; //TCP connection
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    s = getaddrinfo(host, port, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "Error in getaddrinfo! %s\n",  gai_strerror(s));
        exit(2);
    }


    rp = result;
    while (rp != NULL)
    {
        fflush(stdout);
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
        {
            rp = rp->ai_next;
            continue;
        }

        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break; // success

        close (sfd);
        rp = rp->ai_next;
    }

    if (rp == NULL) {
        fprintf(stderr, "Could not connect to host!\n");
        exit(2);
    }


    freeaddrinfo(result);

    socket_fd = sfd;
    f_recv = fdopen(socket_fd, "r+");


    // tls connections

    // get ssl library functions
    if (SSL_library_init() < 0)
    {
        fprintf(stderr, "Error initializing ssl library\n");
        exit(2);
    }

    // Import ssl algorithms
    OpenSSL_add_all_algorithms();
    // load error strings
    SSL_load_error_strings();

    // begin new tls context
    context = SSL_CTX_new(TLSv1_client_method());
    if (context == NULL)
    {
        fprintf(stderr, "Failed to initialize ssl context!\n");
        exit(2);
    }

    // create ssl fd
    ssl = SSL_new(context);
    int ssl_status = SSL_set_fd(ssl, socket_fd);
    if (ssl_status == 0)
    {
        fprintf(stderr, "Error setting SSL file descriptor\n");
        exit(2);
    }

    ssl_status = SSL_connect(ssl);
    if (ssl_status != 1)
    {
        fprintf(stderr, "Error establishing ssl connection\n");
        exit(2);
    }

    // write id
    char id_no[20];
    sprintf(id_no, "ID=%d\n", id);
    ssl_status = SSL_write(ssl, id_no, strlen(id_no));
    if (ssl_status < 0)
    {
        fprintf(stderr, "Error sending id number\n");
        exit(2);
    }



    //dprintf(socket_fd, "ID=%d\n", id);
    fprintf(output_file, "ID=%d\n", id);

    mraa_gpio_context button;
    mraa_aio_context temp;
    float temperature = 0.0;

    button = mraa_gpio_init(60);
    if (button == NULL)
    {
        fprintf(stderr, "Failed to initialize button gpio!\n");
        exit(2);
    }

    temp = mraa_aio_init(1);
    if (temp == NULL)
    {
        fprintf(stderr, "Failed to initialize temperature sensor aio!\n");
        exit(2);
    }

    mraa_gpio_dir(button, MRAA_GPIO_IN);
    mraa_gpio_isr(button, MRAA_GPIO_EDGE_FALLING, &do_when_interrupted, NULL);

    int poll_ret = -1;
    struct pollfd poll_cmds;
    poll_cmds.fd = socket_fd;
    poll_cmds.events = POLLIN;

    time_t start_time;
    time_t run_time;

    char command_buffer[1000];
    while (run_flag) {

        int reading =  mraa_aio_read(temp);
        float resistance = divider_resistance * (1023.0 / (float)(reading) - 1.0);

        temperature = 1.0 / (log(resistance / divider_resistance) / thermistor_B + 1 / 298.15) - 273.15;

        if (fahrenheit)
            temperature = temperature * 9.0 / 5.0 + 32;

        time (&current_time);
        time_info = localtime( &current_time );

        strftime (timebuf, 80, "%H:%M:%S", time_info);
        log_report(output_file, 0, "%s %.1f\n", timebuf, temperature);

        start_time = time(NULL);
        run_time = time(NULL);
        while ((run_time - start_time) < (sample_rate / 1000))
        {
            poll_ret = poll(&poll_cmds, 1, 100);
            if (poll_ret < 0)
            {
                fprintf(stderr, "Error polling!\n");
                exit(2);
            }
            else if (poll_ret > 0 && (poll_cmds.revents & POLLIN))
            {
                int size = SSL_read(ssl, command_buffer, 1000);

                process_commands(command_buffer, size);
            }
            run_time = time(NULL);

        }
    }

    if (output_file != NULL)
        fclose(output_file);
    if (f_recv != NULL)
        fclose(f_recv);
    SSL_shutdown(ssl);
    close(socket_fd);
    mraa_gpio_close(button);
    mraa_aio_close(temp);

    return 0;
}
