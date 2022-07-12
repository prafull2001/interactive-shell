#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <fcntl.h>

// Copyright 2022 Dyuthi Nair, Prafull Sharma

struct alias_node {
    struct alias_node *prev;
    struct alias_node *next;
    char *alias;
    char *replaced;
};

struct alias_node *head = NULL;



void print(struct alias_node *node) {
    char *str = calloc(sizeof(char), 100);
    str[0] = '\0';
    strncat(str, node -> alias, strlen(node -> alias));
    strncat(str, " ", 2);
    strncat(str, node -> replaced, strlen(node -> replaced));
    strncat(str, "\n", 2);
    write(STDOUT_FILENO, str, strlen(str));
    free(str);
}

void parse_input(char *user_input, bool has_batch) {
    char *cur_line = strdup(user_input);

    if (strcmp(cur_line, "\n") == 0) {
        write(STDOUT_FILENO, "\n", 1);
        return;
    }

    // remove trailing new line
    char new_line[] = "\n";
    if (strcmp(user_input, new_line) != 0) {
        for (int i = strlen(cur_line) - 1; i >= 0; --i) {
        if (cur_line[i] == '\n')
            cur_line[i] = '\0';
        }
    }



    if (has_batch) {
        write(STDOUT_FILENO, user_input, strlen(user_input));
    }

    // if just enter (no input)
    if (strlen(cur_line) <= 0) {
        return;
    }

    char exitstr[] = "exit\n";
    if (strcmp(user_input, exitstr) == 0) {
        exit(0);
    }

    struct alias_node *cur = head;

    bool hasRedirection = false;

    if (strstr(user_input, ">") != NULL) {
        hasRedirection = true;
        char *fin = calloc(sizeof(char*), (strlen(cur_line) + 2));
        int offset = 0;
        for (int i = 0; i < strlen(cur_line); i++) {
            if (cur_line[i] == '>') {
                fin[i] = ' ';
                fin[i + 1] = '>';
                fin[i + 2] = ' ';
                offset = 2;
            } else {
                fin[i + offset] = cur_line[i];
            }
        }
        cur_line = fin;
    }

    if (strstr(user_input, "unalias") != NULL) {
        int i = 0;
        char *unalias_args[2];
        // char *unalias_token = strtok(cur_line, " \t");
        char *unalias_token = strtok_r(cur_line, " \t", &cur_line);
        while (unalias_token != NULL) {
            unalias_args[i] = strdup(unalias_token);
            // unalias_token = strtok(NULL, " \t");
            unalias_token = strtok_r(NULL, " \t", &cur_line);
            i++;
        }
        if (i != 2) {
            char msg[] = "unalias: Incorrect number of arguments.\n";
            write(STDOUT_FILENO, msg, strlen(msg));
        }
        if (cur == NULL) {
            return;
        }
        while (cur != NULL) {
            if (strcmp(cur -> alias, unalias_args[1]) == 0) {
                struct alias_node *garbage_node = cur;
                if (cur == head) {
                    head = cur -> prev;
                }
                if (cur -> next != NULL) {
                    cur -> next -> prev = cur -> prev;
                }
                if (cur -> prev != NULL) {
                    cur -> prev -> next = cur -> next;
                }
                cur -> prev = NULL;
                cur -> next = NULL;
                free(garbage_node);
            }
            cur = cur -> prev;
        }
    } else if (strstr(user_input, "alias") != NULL) {
        int i = 0;
        char *alias_args[2];
        // char *alias_token = strtok(cur_line, " \t");
        char *alias_token = strtok_r(cur_line, " \t", &cur_line);
        while (alias_token != NULL) {
            alias_args[i] = strdup(alias_token);

            // alias_token = strtok(NULL, " \t");
            alias_token = strtok_r(NULL, " \t", &cur_line);
            i++;
        }
        free(alias_token);
        if (i == 1) {
            while (cur != NULL) {
                print(cur);
                cur = cur -> prev;
            }
        } else if (i == 2) {
            while (cur != NULL) {
                if (strcmp(alias_args[1], cur -> alias) == 0) {
                    print(cur);
                }
                cur = cur -> prev;
            }
        } else if (i >= 3) {
            bool isReplaced = false;
            char *newCommand = calloc(sizeof(char), 100);
            for (int c = 2; c < i; c++) {
                // strcat(newCommand, alias_args[c]);
                // str[0] = '\0';
                strncat(newCommand, alias_args[c], strlen(alias_args[c]));
                if (c != i - 1) {
                    // strcat(newCommand, " ");
                    strncat(newCommand, " ", 2);
                }
            }
            if ((strcmp(alias_args[1], "alias") == 0) ||
            (strcmp(alias_args[1], "unalias") == 0) ||
            (strcmp(alias_args[1], "exit") == 0)) {
                char msg[] = "alias: Too dangerous to alias that.\n";
                write(STDERR_FILENO, msg, strlen(msg));
                _exit(1);
            }
            while (cur != NULL && isReplaced == false) {
                if (strcmp(alias_args[1], cur -> alias) == 0) {
                    cur -> replaced = newCommand;
                    isReplaced = true;
                }
                cur = cur -> prev;
            }
            if (isReplaced == false) {
                struct alias_node *newNode = malloc(sizeof*newNode);
                newNode -> alias = alias_args[1];
                newNode -> replaced =  newCommand;
                if (head == NULL) {
                    head = newNode;
                } else {
                    head -> next = newNode;
                    newNode -> prev = head;
                    head = newNode;
                }
            }
        }
    } else {
        struct alias_node *iterator = head;
        bool isAlias = false;
        char *stringToCheck;
        while (iterator != NULL && isAlias == false) {
            if (strcmp(cur_line, iterator -> alias) == 0) {
                stringToCheck = iterator -> replaced;
                isAlias = true;
                // write(STDOUT_FILENO, cur_line, strlen(cur_line));
            }
            iterator = iterator -> prev;
        }
        if (isAlias == false) {
            stringToCheck = cur_line;
        }
        int pid = fork();
        if (pid != -1) {
            if (pid == 0) {
                // child
                int n = 0;
                // char *tokenizer;
                /*if(hasRedirection) {
                    tokenizer = "> \t";
                } else {
                    tokenizer = " \t";	
                }*/
                // char *token = strtok(stringToCheck, " \t");
                char *token = strtok_r(stringToCheck, " \t", &stringToCheck);
                char *args[128];
                while (token != NULL) {
                    args[n] = strdup(token);
                    token = strtok_r(NULL, " \t", &stringToCheck);
                    n++;
                    if (n > 128) {
                        char msg[] = "Too many command line inputs.";
                        write(STDERR_FILENO, msg, strlen(msg));
                        _exit(1);
                    }
                }
                args[n] = NULL;
                if (hasRedirection) {
                    if (strcmp(args[n-2], ">") != 0 ||
                    strcmp(args[0], ">") == 0) {
                        write(STDERR_FILENO, "Redirection misformatted.\n", 26);
                        _exit(1);
                    }
                    int redirCount = 0;
                    for (int index = 0; index < n; index++) {
                        if (strcmp(args[index], ">") == 0) {
                            redirCount++;
                        }
                    }
                    if (redirCount != 1) {
                        write(STDERR_FILENO, "Redirection misformatted.\n", 26);
                        _exit(1);
                    }

                    FILE *outfp = fopen(args[n-1], "w");
                    if (outfp == NULL) {
                        char err[] = "Error: Cannot open file ";
                        // char *finerr = strcat(err, args[n-1]);
                        int err_len = strlen(args[n-1]);
                        char *finerr = strncat(err, args[n-1], err_len);
                        // finerr = strcat(finerr, ".\n");
                        finerr = strncat(finerr, ".\n", 3);
                        write(STDERR_FILENO, finerr, strlen(finerr));
                        exit(1);
                    }

                    int out = open(args[n-1], O_TRUNC | O_WRONLY);
                    if (out == -1) {
                        char err[] = "Cannot write to file ";
                        // char *finerr = strcat(err, args[n-1]);
                        int err_len = strlen(args[n-1]);
                        char *finerr = strncat(err, args[n-1], err_len);
                        // finerr = strcat(finerr, ".\n");
                        finerr = strncat(finerr, ".\n", 3);
                        write(STDERR_FILENO, finerr, strlen(finerr));
                        return;
                    }
                    int ret = dup2(out, 1);
                    if (ret == -1) {
                        char err[] = "Cannot write to file ";
                        int err_len = strlen(args[n-1]);
                        char *finerr = strncat(err, args[n-1], err_len);
                        // finerr = strcat(finerr, ".\n");
                        finerr = strncat(finerr, ".\n", 3);
                        write(STDERR_FILENO, finerr, strlen(finerr));
                        return;
                    }
                    args[n-2] = NULL;
                    args[n-1] = NULL;
                }
                execv(args[0], args);
                write(STDERR_FILENO, args[0], strlen(args[0]));
                write(STDERR_FILENO, ": Command not found.\n", 21);
                _exit(1);
            } else {
                int status;
                waitpid(pid, &status, 0);
            }
        }
    }
}

int main(int argc, char **argv) {
    char user_input[512];
    bool isBatch;
    FILE *fp;
    if (argc == 1) {
        fp = stdin;
        isBatch = false;
    } else if (argc == 2) {
        fp = fopen(argv[1], "r");
        if (fp == NULL) {
            char err[] = "Error: Cannot open file ";
            // char *finerr = strcat(err, argv[1]);
            char *finerr = strncat(err, argv[1], strlen(argv[1]));
            // finerr = strcat(finerr, ".\n");
            finerr = strncat(finerr, ".\n", 3);
            write(STDERR_FILENO, finerr, strlen(finerr));
            exit(1);
        }
        isBatch = true;
    } else {
        char err[] = "Usage: mysh [batch-file]\n";
        write(STDERR_FILENO, err, strlen(err));
        exit(1);
    }
    while (1) {
        if (isBatch == 0) {
            write(STDOUT_FILENO, "mysh> ", 6);
        }
        char *ret = fgets(user_input, 512, fp);
        if (ret == NULL) {
            // char msg[] = "not able to take input!\n";
            // write(STDOUT_FILENO, msg, strlen(msg));
            exit(0);
        }
        parse_input(user_input, isBatch);
    }
    free(fp);
}


