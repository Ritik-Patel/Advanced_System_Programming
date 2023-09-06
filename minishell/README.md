# MiniShell
A C program shell23s (mshell$) that goes into an infinite loop waiting for user's commands. Once a command is entered, the program will assemble and execute each 
command using fork(), exec() and other system calls as required with the following rules and conditions. 

## Rule 1: 
The argc of any individual command or program should be >=1 and < =5 
-> mshell$ ls -1 ~/chapter2 -S -n //valid 
-> mshell$ cat new.txt //valid 

## Rule 2: 
The argc of induvial commands or programs that are used along with the special  characters listed below should be >=1 and <=5 
-> Ex: mshell$ ls -1 ~/chapter2 -S -n | wc -w //the first command has argc=5 and the second command has argc=2 which are used along with the special | character 

## Special Characters 
The program should handle the following special characters (In accordance to Rule 2 and  the additional rules listed below) 

### | Piping (up to 7 piping operations should be supported) 
 Ex mshell$ cat ex1.c|grep std|wc| wc -w  // Every command/program can have argc >=1 and <=6 as per Rule 2

### >, <, >> Redirection 
 Ex: mshell$ ls -1 >>dislist.txt

### && Conditional Execution 
 upto 7 conditional execution operators should be supported and could possibly be a combination of && and || 
 Ex : mshell$ ex1 && ex2 && ex3 && ex4 && ex5 
      mshell$ c1 && c2 || c3 && c4 

### || Conditional Execution // see && 

### & Background Processing 
 mshell$ ex1 & //should run ex1 in the background 

### ; Sequential execution of commands 
(up to 7 commands) the argc of each command should be >=1 and <=6 as per Rule 1 
 Ex: mshell$ cat e1.txt; cat e2.txt ; ls ; date