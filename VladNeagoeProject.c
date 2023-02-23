#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#define MAX_LINE_LENGTH 256
#define MAX_ARGUMENTS 64

int compare(const void *a, const void *b)
{
    return strcmp((*(struct dirent **)a)->d_name, (*(struct dirent **)b)->d_name);
}

char *execute_ls(char **arguments, int num_arguments)
{
    char result[10000] = {};
    // initializing the arguments
    int l_flag = 0;
    int s_flag = 0;
    int a_flag = 0;
    int F_flag = 0;
    for (int i = 1; i < num_arguments; i++)
    {
        if (strcmp(arguments[i], "-l") == 0)
        {
            l_flag = 1;
        }
        else if (strcmp(arguments[i], "-s") == 0)
        {
            s_flag = 1;
        }
        else if (strcmp(arguments[i], "-a") == 0)
        {
            a_flag = 1;
        }
        else if (strcmp(arguments[i], "-F") == 0)
        {
            F_flag = 1;
        }
    }
    // checking directory
    char *directory;
    if (num_arguments > 1 && arguments[num_arguments - 1][0] != '-' && arguments[num_arguments - 2][0] != '>')
    {
        directory = arguments[num_arguments - 1];
    }
    else
    {
        directory = ".";
    }

    // list dir
    DIR *dir = opendir(directory);
    if (dir == NULL)
    {
        perror("Error opening directory");
        return NULL;
    }

    struct dirent *entry[MAX_LINE_LENGTH];
    int cnt = 0;
    while ((entry[cnt] = readdir(dir)) != NULL)
    {
        cnt++;
    }
    qsort(entry, cnt, sizeof(struct dirent *), compare);
    for (int i = 0; i < cnt; i++)
    {
        if (a_flag || entry[i]->d_name[0] != '.')
        {
            if (l_flag)
            {
                char path[MAX_LINE_LENGTH];
                int n = snprintf(path, MAX_LINE_LENGTH, "%s/%s", directory, entry[i]->d_name);
                if (n < 0 || n >= MAX_LINE_LENGTH)
                {
                    printf("Error: buffer overflow\n");
                    return NULL;
                }
                struct stat statbuf;
                stat(path, &statbuf);
                strcat(result, (S_ISDIR(statbuf.st_mode)) ? "d" : "-");
                printf((S_ISDIR(statbuf.st_mode)) ? "d" : "-");

                strcat(result, (statbuf.st_mode & S_IRUSR) ? "r" : "-");
                printf((statbuf.st_mode & S_IRUSR) ? "r" : "-");

                strcat(result, (statbuf.st_mode & S_IWUSR) ? "w" : "-");
                printf((statbuf.st_mode & S_IWUSR) ? "w" : "-");

                strcat(result, (statbuf.st_mode & S_IXUSR) ? "x" : "-");
                printf((statbuf.st_mode & S_IXUSR) ? "x" : "-");

                strcat(result, (statbuf.st_mode & S_IRGRP) ? "r" : "-");
                printf((statbuf.st_mode & S_IRGRP) ? "r" : "-");

                strcat(result, (statbuf.st_mode & S_IWGRP) ? "w" : "-");
                printf((statbuf.st_mode & S_IWGRP) ? "w" : "-");

                strcat(result, (statbuf.st_mode & S_IXGRP) ? "x" : "-");
                printf((statbuf.st_mode & S_IXGRP) ? "x" : "-");

                strcat(result, (statbuf.st_mode & S_IROTH) ? "r" : "-");
                printf((statbuf.st_mode & S_IROTH) ? "r" : "-");

                strcat(result, (statbuf.st_mode & S_IWOTH) ? "w" : "-");
                printf((statbuf.st_mode & S_IWOTH) ? "w" : "-");

                strcat(result, (statbuf.st_mode & S_IXOTH) ? "x" : "-");
                printf((statbuf.st_mode & S_IXOTH) ? "x" : "-");

                char *temp;
                asprintf(&temp, " %ld", statbuf.st_nlink);
                strcat(result, temp);
                free(temp);

                printf(" %ld", statbuf.st_nlink);

                asprintf(&temp, " %s", getpwuid(statbuf.st_uid)->pw_name);
                strcat(result, temp);
                free(temp);

                printf(" %s", getpwuid(statbuf.st_uid)->pw_name);

                asprintf(&temp, " %s", getgrgid(statbuf.st_gid)->gr_name);
                strcat(result, temp);
                free(temp);

                printf(" %s", getgrgid(statbuf.st_gid)->gr_name);

                asprintf(&temp, " %s", ctime(&statbuf.st_mtime));
                strcat(result, temp);
                free(temp);

                printf(" %s", ctime(&statbuf.st_mtime));
            }
            if (s_flag)
            {
                char path[MAX_LINE_LENGTH];
                int n = snprintf(path, MAX_LINE_LENGTH, "%s/%s", directory, entry[i]->d_name);
                if (n < 0 || n >= MAX_LINE_LENGTH)
                {
                    printf("Error: buffer overflow\n");
                    return NULL;
                }
                struct stat statbuf;
                if (stat(path, &statbuf) == 0)
                {
                    char *temp;
                    asprintf(&temp, " %jd ", statbuf.st_blocks / 2);
                    strcat(result, temp);
                    free(temp);
                    printf(" %jd ", statbuf.st_blocks / 2);
                }
            }

            char *temp;
            asprintf(&temp, "%s", entry[i]->d_name);
            strcat(result, temp);
            free(temp);

            printf("%s", entry[i]->d_name);
            // prints "/" if it's directory
            if (entry[i]->d_type == DT_DIR)
            {
                strcat(result, "/");
                printf("/");
            }

            // prints "*" if it's an exe
            if (F_flag && entry[i]->d_type == DT_REG)
            {
                char path[MAX_LINE_LENGTH];
                int n = snprintf(path, MAX_LINE_LENGTH, "%s/%s", directory, entry[i]->d_name);
                if (n < 0 || n >= MAX_LINE_LENGTH)
                {
                    printf("Error: buffer overflow\n");
                    return NULL;
                }
                if (access(path, X_OK) == 0)
                {
                    strcat(result, "*");
                    printf("*");
                }
            }
            strcat(result, "\n");
            printf("\n");
        }
    }

    size_t len = strlen(result);
    char *ret = malloc((len + 1) * sizeof(char));
    strcpy(ret, result);

    closedir(dir);
    return ret;
}

char *execute_tac(char **arguments, int num_arguments)
{
    char result[10000] = {};

    // -b check
    int b_flag = 0;
    for (int i = 1; i < num_arguments; i++)
    {
        if (strcmp(arguments[i], "-b") == 0)
        {
            b_flag = 1;
            break;
        }
    }

    char *filename = arguments[1];

    // -s check
    int s_flag = 0;
    char *s_arg = NULL;
    for (int i = 1; i < num_arguments; i++)
    {
        if (strcmp(arguments[i], "-s") == 0)
        {
            if (i + 1 < num_arguments && i + 2 < num_arguments)
            {
                s_flag = 1;
                s_arg = arguments[i + 1];
                filename = arguments[i + 2];
                break;
            }
            else
            {
                printf("Error: -s flag must be followed by an argument\n");
                return NULL;
            }
        }
    }

    if (b_flag && !s_flag)
    {
        filename = arguments[2];
    }

    // get file
    char *input_file;
    if (num_arguments > 1 && arguments[num_arguments - 1][0] != '-' && arguments[num_arguments - 2][0] != '>')
    {
        input_file = arguments[num_arguments - 1];
    }
    else if (filename)
    {
        input_file = filename;
    }
    else
    {
        printf("Error: no input file specified");
        return NULL;
    }

    // open file
    FILE *in = fopen(input_file, "r");
    if (in == NULL)
    {
        perror("Error opening input file\n");
        return NULL;
    }

    // read lines
    char *lines[MAX_LINE_LENGTH];
    int line_count = 0;
    char line[MAX_LINE_LENGTH];

    // ACTUAL bug fix for s_flag :)
    if (s_flag)
    {
        char delim = *s_arg;
        char *s_line;
        ssize_t read;
        size_t len = 0;
        while (read = (getdelim(&s_line, &len, delim, in)) != -1)
        {
            lines[line_count] = strdup(s_line);
            line_count++;
        }
    }
    else
    {
        while (fgets(line, MAX_LINE_LENGTH, in) != NULL)
        {
            lines[line_count] = strdup(line);
            size_t line_len = strlen(lines[line_count]);

            if (lines[line_count][line_len - 1] != '\n')
            {
                // string_manipulation.exe
                char temp[line_len + 3];
                strcpy(temp, lines[line_count]);
                temp[line_len + 1] = '\0';
                temp[line_len] = '\n';
                strcpy(lines[line_count], temp);
            }

            line_count++;
        }
    }

    fclose(in);
    // reverse if -b is present
    if (b_flag)
    {
        for (int i = 0; i < line_count / 2; i++)
        {
            char *temp = lines[i];
            lines[i] = lines[line_count - i - 1];
            lines[line_count - i - 1] = temp;
        }
    }
    // reverse lines if -b isn't specified
    if (!b_flag)
    {
        for (int i = 0; i < line_count / 2; i++)
        {
            char *temp = lines[i];
            lines[i] = lines[line_count - i - 1];
            lines[line_count - i - 1] = temp;
        }
    }

    // printing in reverse
    for (int i = 0; i < line_count; i++)
    {
        if (i == line_count - 2 && b_flag)
        {
            lines[i][strlen(lines[i]) - 1] = '\0';
            lines[i + 1][strlen(lines[i + 1]) - 1] = '\0';
            // Concatenate the last two lines
            strcat(lines[i], lines[i + 1]);
            strcat(result, lines[i]);
            printf("%s", lines[i]);
            break;
        }
        else if (i == line_count - 1 && !b_flag)
        {
            if (lines[i][strlen(lines[i]) - 1] == '\n')
            {
                lines[i][strlen(lines[i]) - 1] = '\0';
            }
            strcat(result, lines[i]);
            strcat(result, "\n");
            printf("%s\n", lines[i]);
            break;
        }
        else
        {
            strcat(result, lines[i]);
            printf("%s", lines[i]);
        }
    }
    size_t len = strlen(result);
    char *ret = malloc(len + 1);
    strcpy(ret, result);

    return ret;
}

char *execute_dir(char **arguments, int num_arguments)
{
    char result[10000] = {};

    // get dir
    char *directory;
    if (num_arguments > 1 && arguments[num_arguments - 1][0] != '-' && arguments[num_arguments - 2][0] != '>')
    {
        directory = arguments[num_arguments - 1];
    }
    else
    {
        directory = ".";
    }

    // list dir
    DIR *dir = opendir(directory);
    if (dir == NULL)
    {
        perror("Error opening directory");
        return NULL;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        // print entry
        if (entry->d_name[0] != '.')
        {
            char *temp;

            asprintf(&temp, "%s\n", entry->d_name);
            strcat(result, temp);
            free(temp);

            printf("%s\n", entry->d_name);
        }
    }
    closedir(dir);

    size_t len = strlen(result);
    char *ret = malloc(len + 1);
    strcpy(ret, result);

    return ret;
}

// checks which command was specified in the terminal

char *process_command(char **arguments, int num_arguments)
{
    if (strcmp(arguments[0], "ls") == 0)
    {
        return execute_ls(arguments, num_arguments);
    }
    else if (strcmp(arguments[0], "tac") == 0)
    {
        return execute_tac(arguments, num_arguments);
    }
    else if (strcmp(arguments[0], "dir") == 0)
    {
        return execute_dir(arguments, num_arguments);
    }
    else
    {
        printf("Unrecognized command: %s\n", arguments[0]);
        return NULL;
    }
}

int main(int argc, char *argv[])
{
    char line[MAX_LINE_LENGTH];
    char *arguments[MAX_ARGUMENTS];
    int num_arguments;
    char *last_result;

    int has_pipe = 0;
    char *output_file;

    while (1)
    {
        int pipe_set = 0;
        has_pipe = 0;
        printf("$ ");

        if (fgets(line, MAX_LINE_LENGTH, stdin) == NULL)
        {
            break;
        }

        // split the line
        num_arguments = 0;
        char *token = strtok(line, " \t\n");
        while (token != NULL)
        {
            if (pipe_set == 1 && has_pipe == 1)
            {
                output_file = token;
                pipe_set = 0;
            }
            arguments[num_arguments] = token;
            num_arguments++;
            if (strcmp(token, ">") == 0 && pipe_set == 0)
            {
                has_pipe = 1;
                pipe_set = 1;
            }
            token = strtok(NULL, " \t\n");
        }
        arguments[num_arguments] = NULL;

        // if there is something written, checks if it's an exiting command
        if (num_arguments > 0)
        {
            last_result = process_command(arguments, num_arguments);
        }

        if (last_result && has_pipe && output_file)
        {
            FILE *file;
            file = fopen(output_file, "w");
            fprintf(file, "%s", last_result);

            fclose(file);
        }
        if (last_result)
        {
            free(last_result);
        }
    }

    return 0;
}