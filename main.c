#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int mysh_cd(char **args);
int mysh_help(char **args);
int mysh_exit(char **args);

char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

int (*builtin_func[]) (char **) = {
    &mysh_cd,
    &mysh_help,
    &mysh_exit
};

int mysh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

#define MYSH_RL_BUFFSIZE 1024
char *mysh_read(void) {
    int buffsize = MYSH_RL_BUFFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * buffsize);
    int c;

    if(!buffer) {
        fprintf(stderr, "mysh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        c = getchar();
        
        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        
        position++;

        if (position >= buffsize) {
            buffsize += MYSH_RL_BUFFSIZE;
            buffer = realloc(buffer, buffsize);

            if (!buffer) {
                fprintf(stderr, "mysh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

char *mysh_read_line(void) {
    char *line = NULL;
    ssize_t bufsize = 0;
    getline(&line, &bufsize, stdin);
    return line;
}

#define MYSH_TOK_BUFSIZE 64
#define MYSH_TOK_DELIM " \t\r\n\a"

char **mysh_split_line(char *line) {
    int bufsize = MYSH_TOK_BUFSIZE;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "mysh: allcation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, MYSH_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += MYSH_RL_BUFFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "mysh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, MYSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

int mysh_launch(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("mysh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("mysh");
    } else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int mysh_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "mysh: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("mysh");
        }
    }
    return 1;
}

int mysh_help(char **args) {
    int i;
    printf("Stephen Brennan's MYSH\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are build in:\n");

    for (i = 0; i < mysh_num_builtins(); i++) {
        printf(" %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return 1;
}

int mysh_exit(char **args) {
    return 0;
}

int mysh_execute(char **args) {
    int i;

    if (args[0] == NULL) {
        return 1;
    }

    for (i = 0; i < mysh_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return mysh_launch(args);
}

void mysh_loop(void){
    char *line;
    char **args;
    int status;

    do {
        printf(">");
        line = mysh_read();
        args = mysh_split_line(line);
        status = mysh_execute(args);

        free(line);
        free(args);
    } while(status);
}

int main(int argc, char **argv){

    mysh_loop();

    return EXIT_SUCCESS;
}
