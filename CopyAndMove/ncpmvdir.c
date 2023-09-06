/*
    COMP 8567-1-R-2023S: Advanced System Programming 
    Section 1
    Assignment 1 - ncpmvdir.c
    Created by: Ritik Patel - 110100962
*/

// This code runs in Debian 11.x - 64bit

#include <stdio.h>
#include <stdlib.h>
#include <ftw.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

// Define Maximum number of extensions as 6 and path length as 1024
#define MAX_EXTS 6
#define MAX_PATH 1024

// Initiate source path, destination path, extensions, copy/mode option and extension coun variable
char Source_Path[MAX_PATH], Dest_Path[MAX_PATH];
char *exts[MAX_EXTS];
int CM_Option, ext_count = 0;


// Function to check the status of the file taken into consideration
int checkFileStatus(const char *File_Path)
{
    if (ext_count == 0)
        return 1;

    // Find the last dot in the file path
    char file_ext[MAX_PATH];
    char *last_dot = strrchr(File_Path, '.');
    if (last_dot != NULL)
    {
        // Copy the extension after the dot into the file_ext array
        strncpy(file_ext, last_dot + 1, MAX_PATH);
        file_ext[MAX_PATH - 1] = '\0';

        // Compare obtained file extension with the extensions entered by user in <extension list>
        for (int i = 0; i < ext_count; i++)
        {
            if (strcmp(file_ext, exts[i]) == 0)
                return 0; // File extension matches one of the extensions
        }
    }

    return 1;
}


// Function to copy the file from source directory to destination directory
void copyFile(const char *source, const char *destination)
{
    // Open the source file for reading in binary mode
    FILE *src_file = fopen(source, "rb");
    if (src_file == NULL)
    {
        perror("Error opening source file");
        return;
    }

    // Open the destination file for writing in binary mode
    FILE *dest_file = fopen(destination, "wb");
    if (dest_file == NULL)
    {
        perror("Error opening destination file");
        fclose(src_file);
        return;
    }

    char buffer[10240];
    size_t bytes_read;

    // Read data from the source file and write it to the destination file
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), src_file)) > 0)
    {
        fwrite(buffer, 1, bytes_read, dest_file);
    }

    // Close the files
    fclose(src_file);
    fclose(dest_file);

    // If move is selected, remove the source file after copying
    if (!CM_Option)
    {
        if (remove(source) != 0)
        {
            perror("Error while moving file");
        }
    }
}


// Function to create the destination directory if it does not exist
void createDir(const char *path, mode_t mode)
{
    if (mkdir(path, mode) != 0 && errno != EEXIST)
    {
        perror("Error while creating directory");
    }
}

// Function to remove the source directory when 'move' option is used
void removeDir(const char *path)
{
    // Open the directory
    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        perror("Error opening directory");
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        // Exclude current directory (.) and parent directory (..)
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            char subpath[MAX_PATH];
            int subpath_index = 0;

            // Copy the path of the subdirectory or file
            for (int i = 0; path[i] != '\0'; i++)
            {
                subpath[subpath_index++] = path[i];
            }

            subpath[subpath_index++] = '/';

            // Append the name of the subdirectory or file
            for (int i = 0; entry->d_name[i] != '\0'; i++)
            {
                subpath[subpath_index++] = entry->d_name[i];
            }

            subpath[subpath_index] = '\0';

            // Recursively call removeDir for directories, or use remove to delete files
            if (entry->d_type == DT_DIR)
            {
                removeDir(subpath);
            }
            else
            {
                if (remove(subpath) != 0)
                {
                    perror("Error while removing file");
                }
            }
        }
    }

    // Close the directory
    closedir(dir);

    // Remove the directory itself
    if (rmdir(path) != 0)
    {
        perror("Error while removing directory");
    }
}


// updateFileLocation will be recursively called in nftw()
// Function to trigger copyFile function if file is encountered and/or createDir function to create destination directory
int updateFileLocation(const char *File_Path, const struct stat *statbase, int File_Type, struct FTW *ftwbuf)
{
    char Dest_Loc[MAX_PATH];

     // Check if the concatenated destination path exceeds the maximum path length
    if (strlen(Dest_Path) + strlen(File_Path) - strlen(Source_Path) >= MAX_PATH)
    {
        printf("Destination path exceeds MAX_PATH");
        return -1;
    }

    // Generate the destination location by concatenating the destination path and the file path relative to the source path
    strcpy(Dest_Loc, Dest_Path);
    strcat(Dest_Loc, "/");
    strcat(Dest_Loc, File_Path + strlen(Source_Path) + 1);

    if (File_Type == FTW_F && checkFileStatus(File_Path))
    {
        copyFile(File_Path, Dest_Loc);
    }
    else if (File_Type == FTW_D)
    {
        createDir(Dest_Loc, 0777); 
    }

    return 0;
}



int main(int argc, char *argv[])
{
    // If the user does not enter proper format of command then exit
    if (argc < 4)
    {
        printf("Synopsis: ncpmvdir [ source_dir] [destination_dir] [options] <extension list>\n");
        exit(1);
    }

    // If 'cp' is found at options in command then set CM_Option to 1
    if ((strcmp(argv[3], "-cp") == 0) || (strcmp(argv[3], "-CP") == 0) || (strcmp(argv[3], "-Cp") == 0))
    {
        CM_Option = 1;
    }

    // If 'mv' is found at options in command then set CM_Option to 0
    else if ((strcmp(argv[3], "-mv") == 0) || (strcmp(argv[3], "-MV") == 0) || (strcmp(argv[3], "-Mv") == 0))
    {
        CM_Option = 0;
    }

    // If neither cp nor mv is found in the command then prompt user to enter proper option
    else
    {
        printf("You entered a wrong option. Supported options are: -cp or -mv\n");
        exit(1);
    }

    // If either cp or mv are included, then check for number of extensions provided by user
    if (argc > 4)
    {
        for (int i = 4; i < argc; i++)
        {
            // If extensions provided by user are more than 6 then prompt the user with an error message
            if (ext_count >= MAX_EXTS)
            {
                printf("You gave too many extensions, maximum number of allowed extensions are 6\n");
                exit(1);
            }

            // If extensions entered are less than 6 then add those extensions to the array
            exts[ext_count++] = argv[i];
        }
    }

    // Copy the 2nd argument passsed by user into source path and 3rd argument into destination path
    strncpy(Source_Path, argv[1], MAX_PATH);
    strncpy(Dest_Path, argv[2], MAX_PATH);

    // Using nftw() to traverse file location
    if (nftw(Source_Path, updateFileLocation, 20, FTW_NS) != 0)
    {
        perror("Error while traversing file tree");
    }

    // Initiate removal of directory when move is selected
    if (!CM_Option)
    {
        removeDir(Source_Path);
    }

    return 0;
}


