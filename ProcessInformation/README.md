# Process Information

## Part A 
A C program that searches for processes in the process tree (rooted at a specified  process) and outputs the requested information based on the input parameters. 
### Synopsis : prcinfo [root_process] [process_id1] [process_id2]… [process_id(n)] [OPTION] 
-> 1 >=n <= 5 
-> Lists the PID and PPID of each process_id(n) if process_id(n) belongs to the process tree rooted at root_process 
o root_process is the PID of a process that is a descendant of the current bash process. 
o process_id(n) is the PID of a process that is a descendant of the current bash process. 

### OPTIONS 
-nd additionally lists the PIDs of all the non-direct descendants of process_id1 (only) 
-dd additionally lists the PIDs of all the immediate descendants of process_id1 
-sb additionally lists the PIDs of all the sibling processes of process_id1
-sz additionally Lists the PIDs of all sibling processes of process_id1 that are defunct
-gc additionally lists the PIDs of all the grandchildren of process_id1
-zz additionally prints the status of process_id1 (Defunct/ Not Defunct) 
-zc additionally lists the PIDs of all the direct descendants of process_id1 that are  currently in the defunct state 


## Part B 
A C program that searches for defunct processes in a process tree(rooted at a  specified process) and forcefully terminates their parent processes based on the input parameters 
### Synopsis: deftreeminus [root_process] [OPTION1] [OPTION2] [-processid] 
o Forcefully terminates all the parent processes (as long as it is not the  current BASH process) of all the defunct processes that belongs to the process tree rooted at root_process. 
o root_process is the PID of a process that is a descendant of the current  bash process. 
o -processid: if specified, the process with PID= processid will not be terminated even if it happens to be the parent of a defunct process that belongs to the process tree rooted at root_process. 

-> -processid can be specified without the options: 
$ deftreeminus 1004 -1010 (Terminates 1005 only in the 
process tree shown in sample runs below) whereas, 
$ deftreeminus 1004 (Terminates both 1005 and 1010) 

### OPTION1 
o - t: forcefully terminates parent processes (whose elapsed time is  greater than PROC_ELTIME) of all the defunct processes in the process tree rooted at root_process 
o - b: forcefully terminates all the processes in the process tree rooted at root_process that have >= NO_OF_DFCS defunct child processes. 
 
### OPTION2 
o PROC_ELTIME: The elapsed time of the process in minutes since it was created (>=1) 
o NO_OF_DFCS: The number of default child processes (>=1) 
