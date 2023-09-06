// Submitted by Ritik Patel - 110100962, Assignment 2.1



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>

#define MAX_PRCID 5
#define MAX_opt_LENGTH 3
#define MAX_CMDS 100
#define MAX_PATH 512


// Define a function to check if a process ID is valid
int checkValidProcessID(int prcID) {
    if (prcID <= 0) { // If the process ID is less than or equal to zero
        fprintf(stderr, "Invalid process ID: %d\n", prcID); // Print error message to standard error
        return 0; // Return 0 to indicate invalid process ID
    }
    return 1; // Return 1 to indicate valid process ID
}

// Define a function to obtain the parent process's ID
int getParentProcessID(int prcID) {
    char filePath[MAX_PATH];
    int ppid = 0;

    // Create the file path
    int numCharsWritten = snprintf(filePath, MAX_PATH, "/proc/%d/status", prcID);
    if (numCharsWritten >= MAX_PATH || numCharsWritten < 0) {
        fprintf(stderr, "Failed to generate file path for process ID: %d\n", prcID);
        return -1;
    }

    // Open the file
    FILE* filePtr = fopen(filePath, "r");
    if (filePtr == NULL) {
        fprintf(stderr, "Failed to open file for process ID: %d\n", prcID);
        return -1;
    }

    // Read the file line by line
    char line[256];
    while (fgets(line, sizeof(line), filePtr) != NULL) {
        if (strncmp(line, "PPid:", 5) == 0) {
            // Extract the PPID value
            char* ppidString = line + 6;
            ppid = atoi(ppidString);
            break;
        }
    }
    // Close the file pointer
    fclose(filePtr);
    return ppid; // Return the Parent process's PID
}

// ------------------- THIS FUNCTION WILL PRINT PROCESSID AND PARENT PROCESSID OF EVERY PROCESSID GIVEN AFTER ROOTID  ------------------- //
// Define a function to list the PIDs and PPIDs of specified process IDs in the process tree rooted at root_process
void printProcessIDsAndParents(int rpID, int* pIDs, int numProcessIDs) {
    for (int i = 0; i < numProcessIDs; i++) {
        int currentProcessID = pIDs[i];
        // Check if the current process ID is valid
        if (!checkValidProcessID(currentProcessID)) {
            continue; // Skip to the next process ID
        }
        // Get the parent process ID
        int parentPID = getParentProcessID(currentProcessID);
        // Print the process ID and its parent process ID
        if (parentPID > 0) {
            printf("Process ID: %d, Parent Process ID: %d\n", currentProcessID, parentPID);
        }
    }
}

// ------------------- FOLLOWING FUNCTIONS ARE FOR THE OPTION ENTERED BY THE USER  ------------------- //

// Define a function to find and list non-direct descendants using ps cmnd
void op_nd_non_direct_desc(int rpID, int pID, int depth){
printf("Listing non-direct descendants of process %d\n", pID);

    char cmnd[100];
    //using ps cmnd to obtain required PIDs 
    snprintf(cmnd, sizeof(cmnd), "ps -o pid,ppid --ppid %d --no-headers", pID);

    FILE* pipeptr = popen(cmnd, "r");
    if (pipeptr == NULL) {
        printf("Failed to execute cmnd: %s\n", cmnd);
        return;
    }

    int excludedPID = pID;
    int childPID, parentPID;

    while (fscanf(pipeptr, "%d %d", &childPID, &parentPID) == 2) {
        if (parentPID != excludedPID)
            printf("%d\n", childPID);
    }

    pclose(pipeptr);
}

// Define a function to list the PIDs of immediate descendants of prcID1 (root process)
void op_dd_direct_desc(int rpID) {
 printf("Listing immediate descendants of %d\n", rpID);

    // Command string to list the immediate descendants using ps command
    char cmnd[50];
    snprintf(cmnd, sizeof(cmnd), "ps -o pid --ppid %d --noheaders", rpID);

    // Using pipeptr to read the obtained output
    FILE* pipeptr = popen(cmnd, "r");
    if (pipeptr == NULL) {
        printf("Failed to execute cmnd: %s\n", cmnd);
        return;
    }

    char pidStr[10];
    while (fgets(pidStr, sizeof(pidStr), pipeptr) != NULL) {
        int pid = atoi(pidStr);
        printf("%d\n", pid);
    }

    pclose(pipeptr);
}

// Define a function to list the PIDs of sibling processes of processID1
void op_sb_sibling_proc(int prcID) {
 printf("Listing sibling processes of %d\n", prcID);

    //using pgrep command using snprintf
    char cmnd[100];
    snprintf(cmnd, sizeof(cmnd), "pgrep -P $(ps -o ppid= -p %d)", prcID);

    //reading value into fileptr
    FILE* pipeptrptr = popen(cmnd, "r");
    if (pipeptrptr == NULL) {
        printf("command execution failed: %s\n", cmnd);
        return;
    }

    //Obtain required sibling PIDs of given processID
    char line[256];
    while (fgets(line, sizeof(line), pipeptrptr) != NULL) {
        int siblingPID = atoi(line);
        printf("%d\n", siblingPID);
    }

    pclose(pipeptrptr);
}

// Define a function to list the PIDs of sibling processes that are defunct of prcID1 (root process)
void op_sz_defunct_siblings(int rpID, int prcID) {
   printf("Listing defunct sibling processes of %d\n", prcID);

    //use ps command to get required PIDs out
    char cmnd[100];
    snprintf(cmnd, sizeof(cmnd), "ps -o pid,state --ppid $(ps -o ppid= -p %d)", prcID);

    //reading obtained output into fileptr
    FILE* pipeptrptr = popen(cmnd, "r");
    if (pipeptrptr == NULL) {
        printf("command execution failed.\n");
        return;
    }

    // checking if obtained PID has a zombie state or not
    char line[256];
    while (fgets(line, sizeof(line), pipeptrptr) != NULL) {
        int siblingProcessID;
        char state[3];
        sscanf(line, "%d %s", &siblingProcessID, state);

        if (strncmp(state, "Z", 1) == 0 && siblingProcessID != prcID) {
            printf("%d\n", siblingProcessID);
        }
    }

    pclose(pipeptrptr);
}

// Define a function to list grandchildren processes of prcID
void op_gc_grandchild(int rpID, int prcID) {
    printf("Listing grandchildren of %d within the tree of %d\n", prcID, rpID);

    // Find the children of the specified prcID uding ps command
    char cmnd[MAX_CMDS];
    snprintf(cmnd, sizeof(cmnd), "ps -o pid --no-headers --ppid %d", prcID);

    //reading value into fp pointer
    FILE* fp = popen(cmnd, "r");
    if (fp == NULL) {
        printf("command execution failed\n");
        return;
    }

    int childProcessID;
    while (fscanf(fp, "%d", &childProcessID) == 1) {
        // Find and list the children of each child process
        snprintf(cmnd, sizeof(cmnd), "ps -o pid --no-headers --ppid %d", childProcessID);

        FILE* childFp = popen(cmnd, "r");
        if (childFp == NULL) {
            printf("command execution failed\n");
            pclose(fp);
            return;
        }

        // get and print the grandchild PID for any entered PID
        int grandchildProcessID;
        while (fscanf(childFp, "%d", &grandchildProcessID) == 1) {
            printf("%d\n", grandchildProcessID);
        }

        pclose(childFp);
    }

    pclose(fp);
}

// Function to check if a process is defunct or not
void op_zz_proc_status(int prcID) {
    printf("Checking status of process %d: ", prcID);
    fflush(stdout);

    // Create the cmnd to read the process status from /proc/<pid>/stat file
    char cmnd[MAX_CMDS];
    snprintf(cmnd, sizeof(cmnd), "cat /proc/%d/stat | awk '{print $3}'", prcID);

    // Execute the cmnd and read the output
    FILE* fp = popen(cmnd, "r");
    if (fp == NULL) {
        printf("command execution failed\n");
        return;
    }

    //check the status of the PID provided to see whether it is zombie or not
    char status;
    if (fscanf(fp, " %c", &status) == 1) {
        if (status == 'Z') {
            printf("Defunct\n");
        } else {
            printf("Not Defunct\n");
        }
    }

    pclose(fp);
}

// Function to list the PIDs of direct descendants that are currently defunct
void op_zc_def_desc(int prcID) {
 printf("Listing defunct descendants of %d\n", prcID);

    //use combination of pgrep and ps (reference taken to complete this tasl)
    char shellCommand[MAX_CMDS];
    snprintf(shellCommand, sizeof(shellCommand), "pgrep -P %d | xargs ps -o pid,state --no-headers | awk '/[Zz]/{print $1}'", prcID);

    //reading values into shelloutput pointer
    FILE* shellOutput = popen(shellCommand, "r");
    if (shellOutput == NULL) {
        printf("Failed to execute shell command\n");
        return;
    }

    char pid[32];
    int defunctCount = 0;
    while (fgets(pid, sizeof(pid), shellOutput) != NULL) {
        printf("%s\n", pid);
        defunctCount++;
    }

    pclose(shellOutput);

    if (defunctCount == 0) {
        printf("No defunct descendants found\n");
    }
}



int main(int argc, char* argv[]) {
    
    // Get the option
    char* opt = argv[argc - 1];

    // Checker for number of args
    // Check if the user has inputed any option, i.e total argcs = root PID + maximum number of dervied process IDs + option (1+5+1)
    if(strlen(opt) == 3){
        if (argc < 3 || argc > 8) {
        printf("You entered too many or too less arguments\n");
        return 1;
        }
    }
    // If no option is given by user then count for root process + maximum number of dervied process IDs (1+5)
    else{
        if (argc < 3 || argc > 7) {
        printf("You entered too many or too less arguments\n");
        return 1;
        }
    }

    // Check if the root PID provided by user is valid or not
    int rpID = atoi(argv[1]);
    if (!checkValidProcessID(rpID)) {
        return 1;
    }

    // Get the PIDs from the user's inputs
    int prcIDs[MAX_PRCID];
    int numProcessIDs = 0;
    
    if(strlen(opt) == 3){
        numProcessIDs = argc - 3;
    }
    else{
        numProcessIDs = argc - 2;
    }
    
    // Check if the Process IDs provided by user are valid or not
    for (int i = 0; i < numProcessIDs; i++) {
        prcIDs[i] = atoi(argv[i + 2]);
        if (!checkValidProcessID(prcIDs[i])) {
            return 1;
        }
    }
    
    // Perform the action based on the option selected
    if(strlen(opt) == 3){
    // if the user has inputed nd option then call the non direct descendant finder function
    if (strcmp(opt, "-nd") == 0 || strcmp(opt, "-ND") == 0) {
        if (numProcessIDs >= 1) {
            op_nd_non_direct_desc(rpID, prcIDs[0],0);
        } else {
            printf("Invalid number of process IDs\n");
            return 1;
        }
    } 
    
    // if the user has inputed dd option then call the immidiate direct descendant finder function
    else if (strcmp(opt, "-dd") == 0 || strcmp(opt, "-DD") == 0){
        if (numProcessIDs >= 1) {
            op_dd_direct_desc(prcIDs[0]);
        } else {
            printf("Invalid number of process IDs\n");
            return 1;
        }
    } 
    
    // if the user has inputed sb option then call the sibling process finder function
    else if (strcmp(opt, "-sb") == 0 || strcmp(opt, "-SB") == 0) {
        if (numProcessIDs >= 1) {
            op_sb_sibling_proc(prcIDs[0]);
        } else {
            printf("Invalid number of process IDs\n");
            return 1;
        }
    }
    
    // if the user has inputed sz option then call the defunct siblings finder function 
    else if (strcmp(opt, "-sz") == 0 || strcmp(opt, "-SZ") == 0) {
        if (numProcessIDs >= 1) {
            op_sz_defunct_siblings(rpID, prcIDs[0]);
        } else {
            printf("Invalid number of process IDs\n");
            return 1;
        }
    } 
    
    // if the user has inputed gc option then call the grandchildren finder function
    else if (strcmp(opt, "-gc") == 0 || strcmp(opt, "-GC") == 0) {
        if (numProcessIDs >= 1) {
            op_gc_grandchild(rpID, prcIDs[0]);
        } else {
            printf("Invalid number of process IDs\n");
            return 1;
        }
    } 
    
    // if the user has inputed zz option then call the current process status checker function
    else if (strcmp(opt, "-zz") == 0 || strcmp(opt, "-ZZ") == 0) {
        if (numProcessIDs >= 1) {
            op_zz_proc_status(prcIDs[0]);
        } else {
            printf("Invalid number of process IDs\n");
            return 1;
        }
    } 
    
    // if the user has inputed zc option then call the defunct direct descendant finder function
    else if (strcmp(opt, "-zc") == 0 || strcmp(opt, "-ZC") == 0) {
        if (numProcessIDs >= 1) {
            op_zc_def_desc(prcIDs[0]);
        } else {
            printf("Invalid number of process IDs\n");
            return 1;
        }
    } 

    // if the user has inputed an option other than 7 above mentioned ones
    else {
        printf("Invalid opt: %s\n", opt);
        printf("Accepted Options: -nd, -dd, -sb, -sc, -zz, -zc and -gc");
        return 1;
    }
}

    // List the PIDs and PPIDs of specified process IDs in the process tree rooted at rpID
    printProcessIDsAndParents(rpID, prcIDs, numProcessIDs);
    
    return 0;
}



