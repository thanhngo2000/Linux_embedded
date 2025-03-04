#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

// Define exit status codes
#define EXIT_GENERAL_ERROR       1   // General error
#define EXIT_FILE_NOT_FOUND      2   // File not found
#define EXIT_COMMAND_CANNOT_EXEC 126 // Command invoked cannot execute
#define EXIT_COMMAND_NOT_FOUND    127 // Command not found
#define EXIT_INTERRUPTED          130 // Process terminated by Ctrl+C
#define EXIT_KILLED               137 // Process killed (received SIGKILL)

int main()
{
    //create list of valid status variable with exit()
    int valid_exit_codes[] = {0,1,126,127,130,137};
    int num_exit_codes = sizeof(valid_exit_codes) / sizeof(valid_exit_codes[0]);

    //create random
    srand(time(NULL)); 

    //choose random status variable
    int user_exit_code = valid_exit_codes[rand() % num_exit_codes];
    printf("User input exit code: %d\n", user_exit_code);

    pid_t pid = fork();
    int status;//save info exit of child process

    if(pid <0)
    {
        printf("Fork failed\n");
        
        return EXIT_FAILURE;
    }
    else if (pid ==0)//child process
    {
        printf("child process\n");
        printf("Child PID = %d\nParent PID = %d\n", getpid(), getppid());
        sleep(2);// do something
        printf("Exit child process\n");
        exit(user_exit_code);
    }
    else //parent process
    {
        printf("Parent process\n");
        wait(&status);

        if (WIFEXITED(status)) //check status code is valid
        {
            int exit_code = WEXITSTATUS(status); //get  the exit code

            switch (exit_code){
            case EXIT_GENERAL_ERROR:
                printf("Child process exit with General error status code: %d\n", exit_code);
                break;
            case EXIT_FILE_NOT_FOUND:
                printf("Child process exit with File not found status code: %d\n", exit_code);
                break;
            case EXIT_COMMAND_CANNOT_EXEC:
                printf("Child process exit with Command invoked cannot execute status code: %d\n", exit_code);
                break;
            case EXIT_COMMAND_NOT_FOUND:
                printf("Child process exit with Command not found status code: %d\n", exit_code);
                break;
            case EXIT_INTERRUPTED:
                printf("Child process exit with Interrupted status code: %d\n", exit_code);
                break;
            case EXIT_KILLED:
                printf("Child process exit with Killed status code: %d\n", exit_code);
                break;

            default: 
                printf("Child process exit with status code");
                break;
            }
        }
        else
        {
            printf("Child process exit not normally\n");
        }
    }
    return EXIT_SUCCESS;
}