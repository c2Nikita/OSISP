#define _GNU_SOURCE
#define MAX_LEN 255

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <locale.h>
#include <stdint.h>

void dir_walk(char *path, char *flags);

// Comparison function for qsort
int compare_entries(const void *a, const void *b) {
    return strcoll(*(const char **)a, *(const char **)b);
}

int main(int argc, char *argv[]) {
    char *flags = (char*)calloc(6, sizeof(char)); // Increased size to hold "-s" flag
    char *path = ".";
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            if (strstr(argv[i], "-"))
                strcat(flags, argv[i]);
            else if (argv[i][0] == '/')
                path = argv[i];
        }
    } else
        strcat(flags, "-ldf");

    if (!strstr(flags, "l") && !strstr(flags, "d") && !strstr(flags, "f"))
        strcat(flags, "-ldf");

    if (strstr(flags, "d"))
        printf("d - %s/\n", path);

    dir_walk(path, flags);
    free(flags);
    return 0;
}

void dir_walk(char *path, char *flags) {
    DIR *dir = opendir(path);
    struct dirent *read_dir;
    struct stat st;
    char **entries = NULL;
    size_t entry_count = 0;

    if (!dir) {
        if (errno) {
            fprintf(stderr, "lab1 : '%s' : Permission denied\n", path);
            errno = 0;
            return;
        }
    }

    // Collect directory entries
    while ((read_dir = readdir(dir))) {
        if (!(strcmp(read_dir->d_name, ".") && strcmp(read_dir->d_name, ".."))) {
            continue;
        }
        char tmp[MAX_LEN];
        strcpy(tmp, path);
        strcat(tmp, "/");
        strcat(tmp, read_dir->d_name);
        entries = realloc(entries, (entry_count + 1) * sizeof(char *));
        entries[entry_count] = strdup(tmp);
        entry_count++;
    }

    // Sort entries if -s flag is provided
    if (strstr(flags, "s")) {
        setlocale(LC_COLLATE, "");
        qsort(entries, entry_count, sizeof(char *), compare_entries);
    }

    // Process sorted or unsorted entries
    for (size_t i = 0; i < entry_count; i++) {
        lstat(entries[i], &st);
        if (S_ISDIR(st.st_mode)) {
            if (strstr(flags, "d"))
                printf("d - %s\n", entries[i]);
            dir_walk(entries[i], flags);
        }
        if (S_ISREG(st.st_mode)) {
            if (strstr(flags, "f"))
                printf("f - %s\n", entries[i]);
        }
        if (S_ISLNK(st.st_mode)) {
            if (strstr(flags, "l"))
                printf("l - %s\n", entries[i]);
        }
        free(entries[i]);
    }
    free(entries);

    if (!read_dir) {
        if (errno) {
            fprintf(stderr, "lab1: '%s' : File can't be read\n", path);
            errno = 0;
        }
    }
    closedir(dir);
}