# CopyAndMove
A C program ncpmvdir that copies or moves an entire directory tree rooted at a  specific path in the home directory to a specific destination folder in the home directory minus the file types specified in the extension list.

### Synopsis : ncpmvdir [source_dir] [destination_dir] [options] <extension list> 

* Both source_dir and destination_dir can be either absolute or relative paths but must belong to the home directory hierarchy. 
* If the destination_dir is not present in the home directory hierarchy, it will be newly created. 

## OPTIONS 
* -cp: copy the directory rooted at source_dir to destination_dir and do not delete the directory (and contents) rooted at source_dir
* -mv: move the directory rooted at source_dir to destination_dir and delete the directory (and contents) rooted at source_dir 
* extension list: up to 6 file extensions can provided ( c , pdf, txt etc.)

-> If the extension list is provided with -cp:
* The entire sub-tree rooted at source_dir along with all its folders/subfolders/files (minus the file types listed in the extension list) must be copied onto the destination_dir. 
* All the folders/sub-folders/files must be copied onto destination_dir as per the original hierarchy at source_dir. 
* If desintation_dir does not exist, it must be created.

-> If the extension list is provided with -mv:
* The entire sub-tree rooted at source_dir along with all its folders/subfolders/files (minus the file types listed in the extension list) must be moved onto the destination_dir. 
* All the folders/sub-folders/files must be moved onto destination_dir as per the original hierarchy at source_dir. 
* If desintation_dir does not exist, it must be created.
* The original subtree rooted at source_dir must be deleted entirely along with its folders/sub-folders/files etc. 
* If the extension list is not provided, all files and folders must be copied or moved  as per the option chosen. 

## Sample Runs 
* $ ncpmvdir ./folder1 ./folder2/folder3 -cp txt pdf 
-> This will copy the directory tree rooted at ./folder1 to ./folder2/folder3 as per the source_dir hierarchy and will not copy the .txt and .pdf files 

* $ ncpmvdir ~/folder1 ~/folder3 -mv 
-> This will move the entire directory tree rooted at ~/folder1 to ~/folder 3 along with all the files and folders as per the source_dir hierarchy 

* If the source directory does not exist or does not belong to the home directory hierarchy, an  appropriate error message will be displayed. 