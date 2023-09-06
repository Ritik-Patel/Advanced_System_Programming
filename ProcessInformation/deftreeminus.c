// Submitted by Ritik Patel 110100962, Assignment 2.2

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <ctype.h>
#include <time.h>

// Defining Global Variables for maximum number of child, depth and value of degubber
#define MAX_CHLD 1000
#define MAX_DEPTH 1000
#define VAL_DEBUGGER 1 // will output log messages for each step when set 1

int child[MAX_CHLD];
int bash_pid;

// Define a function to return the parent process id
int find_parent_pid(int pid) {
    char statfilepath[50];
    FILE *statfileptr;
    int parentpid;

    // using sprintf to send formatted values
    sprintf(statfilepath, "/proc/%d/stat", pid);
    statfileptr = fopen(statfilepath, "r");
    // checking if stat file has be opened
    if (statfileptr == NULL) {
        perror("Error in opening /proc/stat file for given process");
        exit(EXIT_FAILURE);
    }

    fscanf(statfileptr, "%*d %*s %*c %d", &parentpid);
    fclose(statfileptr);

    return parentpid;
}

// function to check if a process is in defunct state
int process_status_finder(int pid) {

    char processStatusFilePath[50];
    FILE *processStatusPtr;
    char process_status_line[100];
    char *status;
    int result = 0;

    // using sprintf to obtain status file's path 
    sprintf(processStatusFilePath, "/proc/%d/status", pid);
    processStatusPtr = fopen(processStatusFilePath, "r");
    if (processStatusPtr == NULL) {
        perror("Error opening status file");
        exit(EXIT_FAILURE);
    }

    //checking the status of the file the PID given by user whether its zombie or not
    while (fgets(process_status_line, sizeof(process_status_line), processStatusPtr) != NULL) {
        if (strncmp(process_status_line, "State:", 6) == 0) {
            status = process_status_line + 7;
            if (strstr(status, "Z (zombie)") != NULL) {
                result = 1;
            }
            break;
        }
    }

    //closing the file pointer
    fclose(processStatusPtr);
    return result;
}

// function to get all children of given PID
int *find_child_of_given_PID(int pid) {
    char proc[100];
    DIR *dp;
    struct dirent *dirp;
    int cpid, cppid;

    int count = 0;

    //using sprintf getting to /proc directory
    sprintf(proc, "/proc/");

    //checking id process is running
    if ((dp = opendir(proc)) == NULL) {
        printf("Process is not running: %s\n", proc);
        return NULL;
    }

    //if process is found running then obtain the children PID
    while ((dirp = readdir(dp)) != NULL) {
        if (dirp->d_type == DT_DIR) {
            if (!isdigit(dirp->d_name[0]))
            { 
            continue; // skip non-digit folders
            }
            cpid = atoi(dirp->d_name);
            if (cpid == 1) 
            {
            continue; // skip init PID
            }
            if (cpid == pid) 
            {
            continue;    // skip PID itself
            }
            // get child ppid
            cppid = find_parent_pid(cpid);

            if (cppid == pid) {
                child[count] = cpid;
                count++;
            }
        }
    }

    closedir(dp);
    return child;
}

// function to get elapsed time in minutes of input PID
int time_used_finder(int pid) {
    char fname[100], line[100], *str;
    FILE *fileptr;
    int ck_tics;
    struct timespec ts;

    // Opening /proc/[pid]/stat file using sprintf
    sprintf(fname, "/proc/%d/stat", pid);
    fileptr = fopen(fname, "r");
    if (fileptr == NULL) {
        printf("Process with given PID %d is not running\n", pid);
        return -1;
    }

    // Get time of start of PID from the stat file
    if (fgets(line, 100, fileptr) == NULL) {
        perror("fgets");
        fclose(fileptr);
        return -1;
    }
    str = strtok(line, " ");
    for (int i = 1; i < 22; i++) {
        str = strtok(NULL, " ");
    }
    ck_tics = atoi(str);

    // Close the stat file
    fclose(fileptr);

    // Get the current time then calculate the time that the PID has been active for
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
        perror("Error in getting time");
        return -1;
    }
    return (int)(ts.tv_sec - (ck_tics / sysconf(_SC_CLK_TCK))) / 60;
}

// function to check if children array has any values filled 
// Check if child is present
int cheker_for_child(int child[]) {
    for (int i = 0; i < MAX_CHLD; i++) {
        if (child[i] > 0){
         return 1;
        }
    }
    return 0;
}

// Define a function to kill the process using kill signal - can also be done using terminate signal
void sig_kill_process(int pid) {
    if (pid == bash_pid) {
        printf("\nBash Process can not be killed\n");
        return;
    }
    printf("PID: %d has been terminated\n", pid);
    kill(pid, SIGKILL);
}

// Define a function to establish process' tree using a function recursively
void pid_tree_generator(pid_t pid, int d, int PROC_ELTIME, int NO_OF_DFCS, int processid) {

    if (d >= MAX_DEPTH) {
        printf("Process tree reached maximum depth\n");
        return;
    }

    int* array_of_discovered_children;
    int chl_loc[MAX_CHLD]; //local child array
    int cnt_def = 0, time_used = 0;

    // get time for which PID has been active
    time_used = time_used_finder(pid);

    // get children of parent
    array_of_discovered_children = find_child_of_given_PID(pid);
    memcpy(chl_loc, array_of_discovered_children, MAX_CHLD * sizeof(int)); // copy children locally
    memset(child, 0, sizeof(child)); // clear the global array

    // check if any children
    if (!cheker_for_child(chl_loc)) {
        return;
    }

    // count defunct children
    for (int i = 0; i < MAX_CHLD; i++) {
        if (chl_loc[i] == 0) {
        continue; // skip empty values
        }

        if (VAL_DEBUGGER) {
            printf("%d", chl_loc[i]);
        }

        if (process_status_finder(chl_loc[i])) {
            cnt_def++;
            if (VAL_DEBUGGER) {
                printf(" is defunct\n");
            }
        }
        else {
            if (VAL_DEBUGGER) {
                printf("\n");
            }
        }

        // Recursive call using DFS
        if (chl_loc[i] != processid) { // Skipping PID which is needed to be skipped as entered by user
            pid_tree_generator(chl_loc[i], d + 1, PROC_ELTIME, NO_OF_DFCS, processid);
        }
    }

    if (VAL_DEBUGGER) {
        printf("\n %d has %d zombie children\n", pid, cnt_def);
    }

    // kill parent process with elapsed time > PROC_ELTIME
    if (PROC_ELTIME > 0 && cnt_def > 0) {

        if (time_used > PROC_ELTIME) {
            printf("PID: %d has been running for %d mins\n",pid,time_used);
            sig_kill_process(pid);
            return;
        }
    }

    // kill parent process with number of defunct child >= NO_OF_DFCS
    else if (NO_OF_DFCS > 0 && cnt_def > 0) {

        if (cnt_def >= NO_OF_DFCS) {
            sig_kill_process(pid);
            return;
        }
    }

    // kill all parent process if the below condition is reached
    else if (PROC_ELTIME == 0 && NO_OF_DFCS == 0 && cnt_def > 0) {
        sig_kill_process(pid);
        return;
    }

    if (VAL_DEBUGGER) {
        printf("\nNot killing %d\n", pid);
    }
}

int main(int argc, char* argv[]) {
    int PROC_ELTIME = 0, NO_OF_DFCS = 0;
    pid_t root_pid;

    // Parse command line arguments
    if (argc != 2 && argc != 4 && argc != 5) {
        printf("Synopsis: deftreeminus [root_process] [OPTION1] [OPTION2] [-processid]\n");
        printf("Option 1: -t || -b, Option 2 must be a digit\n");
        return 1;
    }

    // Read root pid
    char* endptr;
    long int pid_long = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || errno == ERANGE || pid_long <= 0) {
        printf("Invalid root process PID\n");
        return 1;
    }
    //store root_pid
    root_pid = (pid_t)pid_long;

    if (argc >= 4) {
        char* opt1 = argv[2];
        char* opt2 = argv[3];
        
        //checking for options, which option hass been entered
        if (strcmp(opt1, "-t") == 0) {
            long int elapsed_long = strtol(opt2, &endptr, 10);
            if (*endptr != '\0' || errno == ERANGE || elapsed_long < 1) {
                printf("Invalid PROC_ELTIME value\n");
                return 1;
            }
            PROC_ELTIME = (int)elapsed_long;
        }
        else if (strcmp(opt1, "-b") == 0) {
            long int dfcs_long = strtol(opt2, &endptr, 10);
            if (*endptr != '\0' || errno == ERANGE || dfcs_long < 1) {
                printf("Invalid NO_OF_DFCS value\n");
                return 1;
            }
            NO_OF_DFCS = (int)dfcs_long;
        }
        else {
            printf("Invalid option has been entered\n");
            return 1;
        }
    }

    // checking if valid pid is given or not
    int p_id = 0;
    if (argc == 5) {
        char* option3 = argv[4];
        long int pid_long = strtol(option3, &endptr, 10);
        if (*endptr != '\0' || errno == ERANGE || pid_long <= 0) {
            printf("Enter valid PID\n");
            return 1;
        }
        p_id = (int)pid_long;
    }

    bash_pid = getppid();// store bash_pid so that it can't be killed when kill is called

    // Start searching process tree
    pid_tree_generator(root_pid, 0,PROC_ELTIME, NO_OF_DFCS, p_id);

    return 0;
}