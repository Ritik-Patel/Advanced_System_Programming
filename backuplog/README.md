# Backup Log Maker
A bash script backup.sh that runs continuously in the background and performs the following operations:

### STEP 1 
Creates a complete backup of all the .txt files (only) found in the entire directory tree rooted at /home/username by tarring all the .txt files into cb*****.tar stored at ~/home/backup/cb  
 (2 minutes interval) 

### STEP 2 
Creates an incremental backup of only those .txt files in the entire directory tree rooted at /home/username that were newly created or modified (only) since the previous complete backup. If there are any newly created/modified .txt files since the previous complete backup, create a tar  file of those text files (only): ib***** tar at ~/home/backup/ib and update backup.log with the timestamp and the name of the tar file  
Else update backup.log with the timestamp and a message  
 (2 minutes interval) 

### STEP 3 
Creates an incremental backup of only those .txt files in the entire directory tree rooted at /home/username that were newly created or modified (only) since the previous complete backup. If there are any newly created/modified .txt files since the previous complete backup, create a tar  file of those text files (only): ib***** tar at ~/home/backup/ib and update backup.log with the timestamp and the name of the tar file  
Else update backup.log with the timestamp and a message  
 (2 minutes interval) 

### STEP 4 
Creates an incremental backup of only those .txt files in the entire directory tree rooted at /home/username that were newly created or modified (only) since the previous complete backup. If there are any newly created/modified .txt files since the previous complete backup, create a tar  file of those text files (only): ib***** tar at ~/home/backup/ib and update backup.log with the timestamp and the name of the tar file  
Else update backup.log with the timestamp and a message  
 (2 minutes interval) 

### STEP 5 
Create a complete backup of all the .txt files (only) found in the entire directory tree rooted at /home/username by tarring all the .txt files into cb*****.tar stored at ~/home/backup/cb  Update backup.log with the timestamp and the name of the tar file
 (2 minutes Interval) 

### (PROCEED TO STEP 2 ) //Continuous loop
