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

sig_atomic_t volatile run_flag = 1;

// NTC Thermistor constants
static const int thermistor_B = 4275;
static const int divider_resistance = 100000;

static FILE *output_file = NULL;
static int report = 1;


static int sample_rate = 1000;
static int fahrenheit = 1;

static mraa_gpio_context button;
static mraa_aio_context temp;


// print to STDOUT and to the log file, if present
void log_report(FILE * output, int comd, char* format_string, ...) {
    if (!report)
        return;
    va_list args;


    va_start(args, format_string);
    if (!comd)
        if (vprintf(format_string, args) < 0)
        {
        	fprintf(stderr, "Error printing to stdout!\n");
        	exit(2);
        }
    va_end(args);

    va_start(args, format_string);
    if (output != NULL)
        if (vfprintf(output, format_string, args) < 0){
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
    mraa_gpio_close(button);
    mraa_aio_close(temp);

    exit(0);
}

void process_commands(char* buffer)
{

    log_report(output_file, 1, "%s", buffer);
    if (strcmp(buffer, "OFF\n") == 0) {
        if (!report) {
            report = 1;
            log_report(output_file, 1, "OFF\n");
        }
        do_when_interrupted();
    }
    if (strcmp(buffer, "STOP\n") == 0)
        report = 0;
    if (strcmp(buffer, "START\n") == 0 && !report)
    {
        report = 1;
        log_report(output_file, 1, "START\n");

    }
    if (strstr(buffer, "LOG") == buffer)
    {
        if (!report)
        {
            report = 1;
            log_report(output_file, 1, buffer);
            report = 0;
        }
    }
    if (strlen(buffer) == 8 && strstr(buffer, "SCALE=") == buffer)
    {
        if (buffer[6] == 'C')
            fahrenheit = 0;
        else if (buffer[6] == 'F')
            fahrenheit = 1;
        else
            fprintf(stderr, "Incorrect argument for SCALE %s\n", buffer + 6 );
    }
    if (strlen(buffer) > 8 && strstr(buffer, "PERIOD=") == buffer)
    {
        unsigned int c = 7;
        while (c < strlen(buffer))
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
        sample_rate = atoi(buffer + 7) * 1000;
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
        {NULL, 0, NULL, 0}
    };

    // holds the current index of the opts[] array for getopt_long
    int option_index = 0;

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
        case 'p':
            if (optarg) {
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

        default:
            fprintf(stderr, "Bad argument.\n");
            exit(1);
            break;
        }

    }


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
    poll_cmds.fd = STDIN_FILENO;
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
                fgets(command_buffer, 1000, stdin);
                process_commands(command_buffer);
            }
            run_time = time(NULL);

        }
    }

    if (output_file != NULL)
        fclose(output_file);
    mraa_gpio_close(button);
    mraa_aio_close(temp);

    return 0;
}
