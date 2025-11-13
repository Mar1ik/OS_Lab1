#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sched.h>
#include <signal.h>
#include <errno.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS 64
#define STACK_SIZE (1024 * 1024)

// Структура для передачи аргументов в clone
typedef struct {
    char **argv;
    char *command;
} child_args_t;

// Функция для дочернего процесса (используется с clone)
int child_func(void *arg) {
    child_args_t *args = (child_args_t *)arg;

    execvp(args->argv[0], args->argv);

    // Если execvp не удался
    fprintf(stderr, "Error: command not found: %s\n", args->argv[0]);
    return 127;
}

// Получение текущего времени в микросекундах
long long get_time_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000000 + tv.tv_usec;
}

// Разбор строки команды на аргументы
int parse_command(char *command, char **argv) {
    int argc = 0;
    char *token = strtok(command, " \t\n");

    while (token != NULL && argc < MAX_ARGS - 1) {
        argv[argc++] = token;
        token = strtok(NULL, " \t\n");
    }

    argv[argc] = NULL;
    return argc;
}

// Выполнение одной команды с использованием clone
int execute_command(char *command) {
    if (command == NULL || strlen(command) == 0) {
        return 0;
    }

    // Убираем пробелы в начале и конце
    while (*command == ' ' || *command == '\t') command++;
    if (*command == '\0') return 0;

    char *argv[MAX_ARGS];
    char cmd_copy[MAX_COMMAND_LENGTH];
    strncpy(cmd_copy, command, MAX_COMMAND_LENGTH - 1);
    cmd_copy[MAX_COMMAND_LENGTH - 1] = '\0';

    int argc = parse_command(cmd_copy, argv);
    if (argc == 0) return 0;

    // Встроенные команды
    if (strcmp(argv[0], "exit") == 0) {
        exit(0);
    }

    if (strcmp(argv[0], "cd") == 0) {
        if (argc < 2) {
            fprintf(stderr, "cd: missing argument\n");
            return 1;
        }
        if (chdir(argv[1]) != 0) {
            perror("cd");
            return 1;
        }
        return 0;
    }

    // Подготовка аргументов для дочернего процесса
    child_args_t args = {
            .argv = argv,
            .command = command
    };

    // Выделяем стек для дочернего процесса
    char *stack = malloc(STACK_SIZE);
    if (stack == NULL) {
        perror("malloc");
        return 1;
    }
    char *stack_top = stack + STACK_SIZE;

    // Засекаем время начала выполнения
    long long start_time = get_time_us();

    // Запускаем процесс через clone
    int flags = SIGCHLD; // Минимальные флаги для создания процесса
    pid_t pid = clone(child_func, stack_top, flags, &args);

    if (pid == -1) {
        perror("clone");
        free(stack);
        return 1;
    }

    // Ждем завершения дочернего процесса
    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        free(stack);
        return 1;
    }

    // Засекаем время завершения
    long long end_time = get_time_us();
    long long elapsed = end_time - start_time;

    free(stack);

    // Выводим время выполнения
    printf("[Execution time: %lld.%06lld seconds]\n",
           elapsed / 1000000, elapsed % 1000000);

    // Возвращаем код возврата дочернего процесса
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }

    return 1;
}

// Обработка команды с операторами (для shell-seq: поддержка ;)
void process_command_line(char *line) {
    char *commands[MAX_ARGS];
    int cmd_count = 0;

    // Разбиваем по разделителю ;
    char *token = strtok(line, ";");
    while (token != NULL && cmd_count < MAX_ARGS) {
        commands[cmd_count++] = token;
        token = strtok(NULL, ";");
    }

    // Выполняем команды последовательно
    for (int i = 0; i < cmd_count; i++) {
        execute_command(commands[i]);
    }
}

int main() {
    char command[MAX_COMMAND_LENGTH];

    printf("Simple Shell with clone and sequential execution (;)\n");
    printf("Type 'exit' to quit\n\n");

    while (1) {
        printf("shell> ");
        fflush(stdout);

        if (fgets(command, sizeof(command), stdin) == NULL) {
            printf("\n");
            break;
        }

        // Убираем перевод строки
        command[strcspn(command, "\n")] = 0;

        if (strlen(command) == 0) {
            continue;
        }

        process_command_line(command);
    }

    return 0;
}
