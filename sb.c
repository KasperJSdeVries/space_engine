#include "bits/types/siginfo_t.h"
#include "sys/types.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int source_filter(const struct dirent *dir) {
    if (dir->d_type != DT_REG) {
        return 0;
    }

    const char *extension = dir->d_name + strlen(dir->d_name) - 2;
    if (strncmp(extension, ".spv", 2) == 0) {
        return 0;
    }

    return 1;
}

int shader_filter(const struct dirent *dir) {
    if (dir->d_type != DT_REG) {
        return 0;
    }

    const char *extension = dir->d_name + strlen(dir->d_name) - 4;
    if (strncmp(extension, ".spv", 4) == 0) {
        return 0;
    }

    return 1;
}

void run_command(char *command) {
    pid_t fork_result = fork();
    if (fork_result == 0) {
        int result = system(command);
        (void)result;
    } else {
        siginfo_t info;
        waitid(P_ALL, 0, &info, WEXITED);
    }
}

int main(int argc, char *argv[]) {
    char *command = "generate";

    if (argc > 1) {
        command = argv[1];
    }

    if (strcmp(command, "generate") == 0) {
        printf("Generating build.ninja\n");

        FILE *fp = fopen("build.ninja", "w");

        char *vars_and_rules = "cflags = -Wall -Wextra -Wpedantic -std=gnu11 -ggdb\n"
                               "builddir = build\n"
                               "libraries =\n"
                               "includes =\n"
                               "libtype = shared\n\n"
                               "rule link\n"
                               "  command = gcc $in $libraries -o $out\n\n"
                               "rule link_library\n"
                               "  command = gcc -$libtype $in $libraries -o $out\n\n"
                               "rule cc\n"
                               "  depfile = $out.d\n"
                               "  command = gcc -MD -MF $out.d $cflags $includes -c $in -o $out\n\n"
                               "rule glslc\n"
                               "  command = glslc $in -o $out\n\n"
                               "build $builddir/sb.c.o: cc sb.c\n"
                               "build sb: link $builddir/sb.c.o\n\n";
        fwrite(vars_and_rules, 1, strlen(vars_and_rules), fp);

        char *newline = "\n";
        {
            struct dirent **source_entries;
            int n = scandir("./src/", &source_entries, source_filter, alphasort);
            if (n == -1) {
                perror("scandir");
                exit(EXIT_FAILURE);
            }

            for (int i = 0; i < n; i++) {
                char *build_string;
                int build_string_length = asprintf(&build_string,
                                                   "build $builddir/%s.o: cc src/%s\n",
                                                   source_entries[i]->d_name,
                                                   source_entries[i]->d_name);
                if (build_string_length == -1) {
                    perror("asprintf");
                    exit(EXIT_FAILURE);
                }
                fwrite(build_string, 1, strlen(build_string), fp);
                free(build_string);
            }

            fwrite(newline, 1, strlen(newline), fp);

            char *build_link = "build game: link";
            fwrite(build_link, 1, strlen(build_link), fp);
            for (int i = 0; i < n; i++) {
                char *link_object;
                int link_object_length =
                    asprintf(&link_object, " $builddir/%s.o", source_entries[i]->d_name);
                if (link_object_length == -1) {
                    perror("asprintf");
                    exit(EXIT_FAILURE);
                }
                fwrite(link_object, 1, strlen(link_object), fp);
                free(link_object);
            }
            fwrite(newline, 1, strlen(newline), fp);
            char *build_link_libraries = "  libraries = -lm -lX11\n";
            fwrite(build_link_libraries, 1, strlen(build_link_libraries), fp);
        }

        fwrite(newline, 1, strlen(newline), fp);

        {
            struct dirent **shader_entries;
            int n = scandir("./shaders/", &shader_entries, shader_filter, alphasort);
            if (n == -1) {
                perror("scandir");
                exit(EXIT_FAILURE);
            }

            for (int i = 0; i < n; i++) {
                char *build_string;
                int build_string_length = asprintf(&build_string,
                                                   "build shaders/%s.spv: glslc shaders/%s\n",
                                                   shader_entries[i]->d_name,
                                                   shader_entries[i]->d_name);
                if (build_string_length == -1) {
                    perror("asprintf");
                    exit(EXIT_FAILURE);
                }
                fwrite(build_string, 1, strlen(build_string), fp);
                free(build_string);
            }
        }
        fclose(fp);

        printf("Generating compile_commands.json\n");
        run_command("ninja -t compdb > compile_commands.json");
    } else if (strcmp(command, "clean") == 0) {
        run_command("ninja -t clean");
        remove("./compile_commands.json");
    } else if (strcmp(command, "watch") == 0) {
        // TODO: command that watches for new files and file changes to then automatically
        // regenerate and rebuild.
        printf("TODO");
    } else {
        fprintf(stderr, "Unknown command");
        exit(EXIT_FAILURE);
    }

    return 0;
}
