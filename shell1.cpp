#include <iostream>
#include <string.h>
#include <unistd.h>
#include <cstdlib>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>
#include <termios.h>
#include <string.h>


#define TOKENSIZE 100
#define HISTORY_SIZE 20

using namespace std;


void StrTokenizer(char *line, char **argv);
void myExecvp(char **argv);
void displayHistory(const char *history[], int count);
void removeFile(const char *filename);
void copyFile(const char *srcFilename, const char *destFilename);
void moveFile(const char *srcFilename, const char *destFilename);
void printCurrentWorkingDirectory();
void changeFilePermissions(const char *filename, mode_t mode);
void listProcesses();
void sendSignal(pid_t pid, int signal);
void executeInBackground(char **argv);
void freeHistory(const char *history[], int count);

int main()
{
    char input[250];
    char *argv[TOKENSIZE];
    const char *history[HISTORY_SIZE];
    int historyCount = 0;
    while (true)
    {
        cout << "cwushell-> ";
        cin.getline(input, 250);

        // Add the command to history
        history[historyCount] = strdup(input);
        historyCount = (historyCount + 1) % HISTORY_SIZE;

        StrTokenizer(input, argv);

        if (strcmp(argv[0], "exit") == 0)
        {
            break;
        }
        else if (strcmp(argv[0], "history") == 0)
        {
            displayHistory(history, historyCount);
        }
        else if (strcmp(argv[0], "rm") == 0)
        {
            if (argv[1] != NULL)
            {
                removeFile(argv[1]);
            }
            else
            {
                cout << "Usage: rm <filename>" << endl;
            }
        }
        else if (strcmp(argv[0], "cp") == 0)
        {
            if (argv[1] != NULL && argv[2] != NULL)
            {
                copyFile(argv[1], argv[2]);
            }
            else
            {
                cout << "Usage: cp <source> <destination>" << endl;
            }
        }
        else if (strcmp(argv[0], "mv") == 0)
        {
            if (argv[1] != NULL && argv[2] != NULL)
            {
                moveFile(argv[1], argv[2]);
            }
            else
            {
                cout << "Usage: mv <source> <destination>" << endl;
            }
        }
        else if (strcmp(argv[0], "pwd") == 0)
        {
            printCurrentWorkingDirectory();
        }
        else if (strcmp(argv[0], "chmod") == 0)
        {
            if (argv[1] != NULL && argv[2] != NULL)
            {
                mode_t mode = strtol(argv[2], NULL, 8);
                changeFilePermissions(argv[1], mode);
            }
            else
            {
                cout << "Usage: chmod <filename> <permissions>" << endl;
            }
        }
        else if (strcmp(argv[0], "ps") == 0)
        {
            listProcesses();
        }
        else if (strcmp(argv[0], "kill") == 0)
        {
            if (argv[1] != NULL && argv[2] != NULL)
            {
                pid_t pid = atoi(argv[2]);


                int signal = atoi(argv[1]);
                sendSignal(pid, signal);
            }
            else
            {
                cout << "Usage: kill <signal> <pid>" << endl;
            }
        }
        else if (strcmp(argv[0], "bg") == 0)
        {
            // Execute the command in the background
            executeInBackground(argv);
        }
        else if (strcmp(argv[0], "fg") == 0)
        {
            // Execute the command in the foreground (default behavior)
            myExecvp(argv);
        }
        else if (strcmp(input, "\n") == 0)
        {
            continue;
        }

        myExecvp(argv);
    }

    // Free memory allocated for history
    freeHistory(history, historyCount);

    return 0;
}

void freeHistory(const char *history[], int count)
{
    for (int i = 0; i < count; i++)
    {
        free((void *)history[i]);
    }
}

void myExecvp(char **argv)
{
    pid_t pid;
    int status;
    int childStatus;

    pid = fork();
    if (pid == 0)
    {
        childStatus = execvp(*argv, argv);
        if (childStatus < 0)
        {
            cout << "ERROR: wrong input" << endl;
        }
        exit(0);
    }
    else if (pid < 0)
    {
        cout << "something went wrong!" << endl;
    }
    else
    {
        waitpid(pid, &status, 0);
    }
}

void StrTokenizer(char *input, char **argv)
{
    char *stringTokenized;
    stringTokenized = strtok(input, " ");
    while (stringTokenized != NULL)
    {
        *argv++ = stringTokenized;
        stringTokenized = strtok(NULL, " ");
    }

    *argv = NULL;
}

void displayHistory(const char *history[], int count)
{
    cout << "Command History:" << endl;
    for (int i = 0; i < count; i++)
    {
        cout << i + 1 << ": " << history[i] << endl;
    }
}

void removeFile(const char *filename)
{
    int status = unlink(filename);
    if (status != 0)
    {
        cout << "Error deleting file or directory." << endl;
    }
}

void copyFile(const char *srcFilename, const char *destFilename)
{
    int srcFile = open(srcFilename, O_RDONLY);
    int destFile = open(destFilename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    char buffer[4096];
    ssize_t bytesRead;

    while ((bytesRead = read(srcFile, buffer, sizeof(buffer))) > 0)
    {
        write(destFile, buffer, bytesRead);
    }

    close(srcFile);
    close(destFile);
}

void moveFile(const char *srcFilename, const char *destFilename)
{
    int status = rename(srcFilename, destFilename);
    if (status != 0)
    {
        cout << "Error moving file or directory." << endl;
    }
}

void printCurrentWorkingDirectory()
{
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        cout << "Current working directory: " << cwd << endl;
    }
    else
    {
        cout << "Error getting current working directory." << endl;
    }
}

void changeFilePermissions(const char *filename, mode_t mode)
{
    int status = chmod(filename, mode);
    if (status != 0)
    {
        cout << "Error changing file permissions." << endl;
    }
}

void listProcesses()
{
    pid_t pid = getpid();
    pid_t ppid = getppid();
    cout << "Process ID: " << pid << endl;
    cout << "Parent Process ID: " << ppid << endl;
}

void sendSignal(pid_t pid, int signal)
{
    int status = kill(pid, signal);
    if (status != 0)
    {
        cout << "Error sending signal to process." << endl;
    }
}

void executeInBackground(char **argv)
{
    pid_t pid;
    int status;
    int childStatus;

    pid = fork();
    if (pid == 0)
    {
        // Ignore the SIGINT signal to prevent termination by Ctrl+C
        signal(SIGINT, SIG_IGN);
        childStatus = execvp(*argv, argv);
        if (childStatus < 0)
        {
            cout << "ERROR: wrong input" << endl;
        }
        exit(0);
    }
    else if (pid < 0)
    {
        cout << "something went wrong!" << endl;
    }
    else
    {
        // Parent process does not wait for the background process
        cout << "Background process started with PID: " << pid << endl;
    }
}