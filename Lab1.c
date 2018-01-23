/*
 * Lab 1, variant 13
 * Task:
 * Написать программу подсчета числа строк, слов и символов указан-ного по запросу текстового файла.
 * Полученную информацию пере-дать через межпроцессный канал в параллельный процесс и в нем вывести
 * только информацию о количестве слов, записав ее при этом в файл. Предусмотреть возможность
 * неоднократного прерывания по сигналу <CTRL>+<C>. При поступлении каждого нечетного преры-вания
 * выводить информацию обо всех текстовых файлах текущего каталога.
 */

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#define NO_PARAMETERS_ERROR 1
#define NOT_FOUND_ERROR 2
#define CANT_OPEN_ERROR 3

int interrupt_counter = 0;     // counter of keyboard interruptions

void interrupt_handler()       // handler of keyboard interrupt signal
{
    interrupt_counter++;       // increase the counter of keyboard interruptions
    if (interrupt_counter % 2) // do it every odd counter value
    {
        printf("\n");          // beautifier

        if (fork() == 0)       // execute action in parallel process
        {
            // child process
            // this command outputs all non-binary files in current directory
            execl("/usr/bin/find",  "find",  ".", "-maxdepth", "1", "-type", "f", "-exec", "grep", "-Iq", ".", "{}", "\;", "-print", 0);
        }
        else
        {
            // wait until child process die
            wait();
        }

    }
}

int main(int argc, char* argv[])
{
    // rewrite the KeyboardInterrupt signal handler
    struct sigaction keyboard_interrupt;
    keyboard_interrupt.sa_handler = interrupt_handler; // we make our function the handler of KeyboardInterrupt handler
    sigemptyset(&keyboard_interrupt.sa_mask);          // no additional blocking signals
    sigprocmask(0,0,&keyboard_interrupt.sa_mask);      // save current mask
    keyboard_interrupt.sa_flags = 0;                   // not to perform special actions
    sigaction(SIGINT, &keyboard_interrupt, 0);         // we set our handler

    if(argc<2)
    {
        printf("USAGE: test <filename>\n");
        exit(NO_PARAMETERS_ERROR);
    }

    char* filename = argv[1];

    if (access(filename, F_OK))
    {
        printf("File %s not found\n", filename);
        exit(NOT_FOUND_ERROR);
    }

    int pipe_descriptors[2];                           // pipe IN/OUT descriptors
    pipe(pipe_descriptors);                            // init pipe

    if (fork() == 0)                                   // make parallel process
    {
        // child process
        // we perform some actions in it
        close(pipe_descriptors[0]);  // close pipe IN descriptor in child process because there is no need in it
        close(1);                    // close STDOUT descriptors
        dup2(pipe_descriptors[1], 1);// copy pipe OUT descriptor into the STDOUT place. All STDOUT information will be in pipe.

        printf("%d\n", 0);           // beautifier
        execl("/usr/bin/wc",  "wc",  filename, 0); // we need info about rows, words and bytes in file
    }
    else
    {
        wait();                                                  // wait untill child process die

        char buffer[1000];                                       // information buffer
        memset(&buffer, 0, 1000);                                // null the buffer memory
        read(pipe_descriptors[0], buffer, 1000);                 // read info from pipe IN channel

        int words = 0;                                           // Words counter
        sscanf(buffer, "%*d %d", &words);                        // parse info from buffer into counter
        printf("Count WORDS in FILE %s: %d\n", filename, words); // print results

        FILE* file;                                              // file descriptor
        if ((file=fopen("info.txt", "w"))==NULL)                 // open file for write. If it isn't exist, create
        {
            fprintf(stderr, "Can't open file info.txt");         // if file is not accesseble, quit
            exit(CANT_OPEN_ERROR);
        }
        else
        {
            fprintf(file, "Count WORDS in FILE %s: %d\n", filename, words); // if file is accesseble, rewrite information in it to our
            fclose(file);
        }
        while(getchar() != 'q') // infitite cycle to catch KeyboardInterrupt signals
        {
        }

        printf("FINISHED\n");
    }
}
