    
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <regex.h>
#include <ctype.h>

#define MAX_CMDS 7
#define MAX_IN 2048
#define MAX_ARGS 6
#define DEBUGGER_MOD 0
#define INCOR_CMD 2
#define EXC_MAX_CMD 3
#define CMD_NOT_CORR 4
#define VALIDATION_SUCCESSFUL 0

// Check for character to trim 
int escapingCharacters(const char *str, int st_ind)
{
  int omission = 0;

  while (st_ind > 0 && str[st_ind - 1] == '\\')
  {
    omission = !omission;
    st_ind--;
  }

  return omission;
}

char *rm_space(const char *s)
{
    // Get 1st non-whitespace character
    while (isspace(*s))
    {
        s++;
    }

    // Find len of the trimm_str
    size_t totrm = strlen(s);
    while (totrm > 0 && isspace(s[totrm - 1]))
    {
        totrm--;
    }

    // Mem Alloc
    char *rm = (char *)malloc(totrm + 1);
    if (!rm)
    {
        fprintf(stderr, "Err in mem alloc.\n");
        exit(EXIT_FAILURE);
    }

    // Copy to buffer
    strncpy(rm, s, totrm);
    rm[totrm] = '\0';

    return rm;
}

// Convert string to formatted form to add spaces around the special char
char *cmd_shell(const char *cmd) {
    const char *spc = "<>|&;";
    const char *db_spc = ">|&";
    size_t cmd_sz = strlen(cmd);
    char *cmd_form = malloc(cmd_sz * 3 + 1); 
    size_t i = 0, j = 0;

    while (i < cmd_sz) {
        if (strchr(spc, cmd[i]) != NULL && !escapingCharacters(cmd, i)) {
            // Add the required space around the spec char
            if (strchr(db_spc, cmd[i]) != NULL && i + 1 < cmd_sz && cmd[i] == cmd[i + 1]) {
                cmd_form = realloc(cmd_form, (j + 5) * sizeof(char)); // Buffer size increment
                sprintf(&cmd_form[j], " %c%c  ", cmd[i], cmd[i]);
                j += 5;
                i++;
            } else {
                cmd_form = realloc(cmd_form, (j + 3) * sizeof(char)); // Buffer size increment
                sprintf(&cmd_form[j], " %c ", cmd[i]);
                j += 3;
            }
        } else {
            // For reg char
            cmd_form = realloc(cmd_form, (j + 2) * sizeof(char)); // Buffer size increment
            cmd_form[j++] = cmd[i];
        }
        i++;
    }

    cmd_form[j] = '\0';
    return cmd_form;
}

// Checking validity of the arguments passed in the input by user
int args_valid(const char *cm)
{
  int tot_arg = 0;
  int arg_access = 0; 
  int len = strlen(cm);
  // Find number of arguments
  for (int i = 0; i < len; i++)
  {
    // Start of new arg when non-space char is found
    if (!isspace(cm[i]))
    {
      if (!arg_access)
      {
        tot_arg++;
        arg_access = 1; 
      }
    }
    else
    {
      arg_access = 0; 
    }
  }
  // True is returned when number of args is less than allowed num of args
  return tot_arg <= MAX_ARGS && tot_arg > 0;
}

int seq_validator(char **cm, int nm_cm)
{
    // Check for number of commands is even
    if (nm_cm % 2 == 0)
    {
        return INCOR_CMD;
    }
    // Check for number of commands does not exceed max limit
    if (nm_cm > 2 * MAX_CMDS - 1)
    {
        return EXC_MAX_CMD;
    }

    // Validate each command in the sequence
    for (int i = 0; i < nm_cm; i++)
    {
        if (i % 2 == 1)
        {
            continue; // Skip validation for delimeters (e.g., '&&', '||')
        }

        if (DEBUGGER_MOD)
        {
            printf("Validating command: %s\n", cm[i]);
        }

        int is_cm_ok = args_valid(cm[i]);
        if (!is_cm_ok)
        {
            return CMD_NOT_CORR;
        }
    }

    return VALIDATION_SUCCESSFUL;
}

// Helper function to tokenize the input string based on dlmiters
int get_tkn_frm_str(char *str, char **tkn)
{
    int nm_tkn = 0;
    int str_sz = strlen(str);
    int k;
    int st_ind = 0;
    int for_db = 2;
    int for_sn = 1;

    for (k = 0; k < str_sz; k++)
    {
        if (strncmp(str + k, "&&", for_db) == 0)
        {
            tkn[nm_tkn] = malloc((k - st_ind + for_sn) * sizeof(char));
            strncpy(tkn[nm_tkn++], str + st_ind, k - st_ind);
            tkn[nm_tkn++] = "&&";
            k++;
            st_ind = k + for_db;
        }
        else if (strncmp(str + k, "||", for_db) == 0)
        {
            tkn[nm_tkn] = malloc((k - st_ind + for_sn) * sizeof(char));
            strncpy(tkn[nm_tkn++], str + st_ind, k - st_ind);
            tkn[nm_tkn++] = "||";
            k++;
            st_ind = k + for_db;
        }
        else if (str[k] == '|')
        {
            tkn[nm_tkn] = malloc((k - st_ind + for_sn) * sizeof(char));
            strncpy(tkn[nm_tkn++], str + st_ind, k - st_ind);
            tkn[nm_tkn++] = "|";
            st_ind = k + for_sn;
        }
        else if (str[k] == ';')
        {
            tkn[nm_tkn] = malloc((k - st_ind + for_sn) * sizeof(char));
            strncpy(tkn[nm_tkn++], str + st_ind, k - st_ind);
            tkn[nm_tkn++] = ";";
            st_ind = k + for_sn;
        }
    }

    // Copy the last token
    tkn[nm_tkn] = malloc((str_sz - st_ind + for_sn) * sizeof(char));
    strncpy(tkn[nm_tkn++], str + st_ind, str_sz - st_ind);

    return nm_tkn;
}

// Function to change the directory
int fn_for_chdir(const char *dir) {
    if (chdir(dir) != 0) {
        perror("Directory Change Err");
        return 1;
    }
    return 0;
}

// Function to redirect input/output
void redirect_for_io(char *ip_fl, char *op_fl, int op_ap) {
    if (ip_fl != NULL) {
        int fd_in = open(ip_fl, O_RDONLY);
        if (fd_in < 0) {
            perror("Error: could not open input file");
            exit(1);
        }

        dup2(fd_in, STDIN_FILENO);
        close(fd_in);
    }

    if (op_fl != NULL) {
        int fd_out;
        int flags = O_WRONLY | O_CREAT;
        if (op_ap) {
            flags |= O_APPEND;
        } else {
            flags |= O_TRUNC;
        }

        fd_out = open(op_fl, flags, 0666);
        if (fd_out < 0) {
            perror("Error: could not open output file");
            exit(1);
        }

        dup2(fd_out, STDOUT_FILENO);
        close(fd_out);
    }
}

// Helper function to execute execvp()
void cmd_exc(char *ags[], int to_fork, int bg) {
    if (execvp(ags[0], ags) < 0) {
        perror("Exec Err");
        exit(1);
    }
}

// Function to execute the input given by the user by using tokenzing
int fn_to_exc(char *cmd, int to_fork) {
    char *cm = strdup(cmd);
    cm = rm_space(cm);
    char *ags[MAX_ARGS];
    char *tkn;
    char *dlm = " ";
    int bg = 0;
    char *ip_fl = NULL;
    char *op_fl = NULL;
    int op_ap = 0;

    // Remove newline character @ EOC
    if (cm[strlen(cm) - 1] == '&') {
        bg = 1;
        cm[strlen(cm) - 1] = '\0';
        cm = rm_space(cm);
    }

    // Check if user entered "cd"
    if ((strcmp(cm, "cd") == 0) || (strstr(cm, "cd ~") == cm)) {
        fn_for_chdir(getenv("HOME"));
        return 0;
    } else if (strstr(cm, "cd ") == cm) {
        char *dir = cm + 3;
        dir = rm_space(dir);
        fn_for_chdir(dir);
        return 0;
    }

    //Check for IO redirect and create arg array
    int i = 0;
    char *nx_tkn;
    tkn = strtok_r(cm, dlm, &nx_tkn);
    while (tkn != NULL) {
        if (strcmp(tkn, "<") == 0) {
            // Input redirection
            ip_fl = strtok_r(NULL, dlm, &nx_tkn);
        } else if (strcmp(tkn, ">") == 0) {
            // Output redirection using replace
            op_fl = strtok_r(NULL, dlm, &nx_tkn);
            op_ap = 0;
        } else if (strcmp(tkn, ">>") == 0) {
            // Append to output redirection
            op_fl = strtok_r(NULL, dlm, &nx_tkn);
            op_ap = 1;
        } else {
            ags[i++] = tkn;
        }

        tkn = strtok_r(NULL, dlm, &nx_tkn);
    }

    // Null-termination
    ags[i] = NULL;

    // Fork process to execute the command
    if (to_fork) {
        pid_t find_proc_id = fork();
        if (find_proc_id < 0) {
            fprintf(stderr, "Error: fork failed\n");
            exit(1);
        } else if (find_proc_id == 0) {
            // Child process active
            redirect_for_io(ip_fl, op_fl, op_ap);
            cmd_exc(ags, to_fork, bg);
            exit(1);
        } else {
            // Parent process active
            if (!bg) {
                // Wait for the child process termination
                int st;
                wait(&st);
                return st;
            } else {
                printf("PID: %d pushed to bakcground\n", find_proc_id);
                return 0;
            }
        }
    } else {
        // If to_fork is 0, execute the command directly
        redirect_for_io(ip_fl, op_fl, op_ap);
        cmd_exc(ags, to_fork, bg);
        return 0;
    }
}

// Function to execute pipe commands
int fn_to_exc_pipe(char **cm, int nm_cm)
{
    int i;
    int fork1 = fork();
    if (!fork1) {
        for (i = 0; i < nm_cm - 1; i += 2) {
            int file_desc_for_pipe[2];
            if (pipe(file_desc_for_pipe) < 0) {
                perror("pipe err");
                exit(EXIT_FAILURE);
            }

            if (!fork()) {
                // Set output to the enter in the current pipe
                if ((dup2(file_desc_for_pipe[1], STDOUT_FILENO)) == -1) {
                    perror("dup2 err");
                    exit(EXIT_FAILURE);
                }

                close(file_desc_for_pipe[0]);
                close(file_desc_for_pipe[1]);
                // Execute the command present in pipe
                fn_to_exc(cm[i], 0);
                exit(EXIT_FAILURE); 
            }

            // Set input to the previous pipe
            if ((dup2(file_desc_for_pipe[0], STDIN_FILENO)) == -1) {
                perror("dup2");
                exit(EXIT_FAILURE);
            }

            close(file_desc_for_pipe[0]);
            close(file_desc_for_pipe[1]);
        }

        // Execute final command
        fn_to_exc(cm[i],0);
    } else if (fork1 > 0) {
        char *fin_cm = strdup(cm[nm_cm - 1]);
        fin_cm = rm_space(fin_cm);
        if (fin_cm[strlen(fin_cm) - 1] != '&') {
            waitpid(fork1, NULL, 0);
        } else {
            printf("PID: %d for the Backgroud prc\n", fork1);
        }
    } else {
        perror("fork");
        exit(EXIT_FAILURE);
    }
}

// Function to execute commands sequentially
void cmd_exc_seq(char **cm, int nm_cm) {
    int i = 0;
    int res = 0;
    int prv_res = 1;

    while (i < nm_cm) {
        
        // Handle conditional execution for "||"
        if (strcmp(cm[i], "||") == 0) {
            if (prv_res == 0) {
                // Skip the next command
                i++;
                while (i < nm_cm && strcmp(cm[i], "&&") != 0 && strcmp(cm[i], "|") != 0 && strcmp(cm[i], ";") != 0) {
                    i++;
                }

                if (i > 0 && i < nm_cm && strcmp(cm[i], "|") == 0) {
                    i += 2;
                }
            } else {
                i++;
            }
        }        // Handle conditional execution for "&&" 
        else if (strcmp(cm[i], "&&") == 0) {
            if (prv_res != 0) {
                i++;
                while (i < nm_cm && strcmp(cm[i], "||") != 0 && strcmp(cm[i], "|") != 0 && strcmp(cm[i], ";") != 0) {
                    i++;
                }

                if (i > 0 && i < nm_cm && strcmp(cm[i], "|") == 0) {
                    i += 2;
                }
            } else {
                i++;
            }
        } else if (strcmp(cm[i], "|") == 0) {
            i++;
        } else if (strcmp(cm[i], ";") == 0) {
            i++;
        } else if (i < nm_cm - 1 && strcmp(cm[i + 1], "|") == 0) {
            int start = i;
            // Gather all piping commands and finally append input redirection
            while (i < nm_cm - 1 && strcmp(cm[i + 1], "|") == 0) {
                i += 2;
            }
            fn_to_exc_pipe(cm + start, i - start + 1);
            i++;
        } else {
            res = fn_to_exc(cm[i], 1);
            prv_res = res;
            i++;
        }
    }
}
int main()
{
    while (1)
    {
        char *buf_for_cmd = malloc(4096);
        char *in = malloc(4096);
        char *in_fmtted;
        char *tkns[150];

        fflush(stdout);
        printf("mshell$ ");
        fgets(buf_for_cmd, MAX_IN, stdin);
        // Remove unneccessary spaces
        in = rm_space(buf_for_cmd);

        // exit program 
        if (strcmp(in, "exit") == 0)
        {
            printf("Exiting mshell...\n");
            break;
        }
        // Skip empty 
        if (strlen(in) == 0)
        {
            continue;
        }

        //Do tokenizing, validations and execution of input
        in_fmtted = cmd_shell(in);
        int nm_tkns = get_tkn_frm_str(in_fmtted, tkns);
        int res_for_valid = seq_validator(tkns, nm_tkns);
        if (res_for_valid != VALIDATION_SUCCESSFUL) {
            // Handle the error based on the res_for_valid
            switch (res_for_valid) {
                case INCOR_CMD:
                    printf("Not a valid input. Please input valid command\n");
                    break;
                case EXC_MAX_CMD:
                    printf("%d commands allowed at max.\n", MAX_CMDS);
                    break;
                case CMD_NOT_CORR:
                    printf("Entered command is valid\n");
                    break;
                default:
                    break;
            }
            continue; // Restart loop
        }

        if (res_for_valid == VALIDATION_SUCCESSFUL) {

            cmd_exc_seq(tkns, nm_tkns);
    }}

    return 0;
}


