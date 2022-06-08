#include <getopt.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STR_BUFFER 256
#define MY_REG 2048
#define MAX_ERR_LENGTH 80

struct flags {
    int e;
    int i;
    int v;
    int c;
    int l;
    int n;
    int h;
    int s;
    int f;
    int o;
    int empty_line;
};

void grep_flags(int argc, char* argv[], struct flags* flag, char* ef_arg);
void add_e_arg(char* optarg, int* e_count, char* ef_arg, struct flags*);
void add_f_arg(char* optarg, int* e_count, char* ef_arg, struct flags*);

void open_files(int argc, char* argv[], char* ef_arg, struct flags flag);
void reg_compile(char* argv[], char* my_reg, FILE* fp, struct flags flag,
                 int files_count);

int main(int argc, char* argv[]) {
    struct flags flag = {0};
    char ef_arg[MY_REG] = {0};

    if (argc > 1) {
        grep_flags(argc, argv, &flag, ef_arg);
        if (!flag.e && !flag.f) {
            if (!*argv[optind]) argv[optind] = ".";
            strcat(ef_arg, argv[optind]);
            optind += 1;
        }
        open_files(argc, argv, ef_arg, flag);
    }
    return 0;
}

void grep_flags(int argc, char* argv[], struct flags* flag, char* ef_arg) {
    int ch = 0;
    int e_count = 0;
    char* optstring = "e:ivclnhsf:o";

    while ((ch = getopt_long(argc, argv, optstring, NULL, NULL)) != -1) {
        switch (ch) {
            case 'e':
                flag->e = 1;
                add_e_arg(optarg, &e_count, ef_arg, flag);
                break;
            case 'i':
                flag->i = 1;
                break;
            case 'v':
                flag->v = 1;
                break;
            case 'c':
                flag->c = 1;
                break;
            case 'l':
                flag->l = 1;
                break;
            case 'n':
                flag->n = 1;
                break;
            case 'h':
                flag->h = 1;
                break;
            case 's':
                flag->s = 1;
                break;
            case 'f':
                flag->f = 1;
                add_f_arg(optarg, &e_count, ef_arg, flag);
                break;
            case 'o':
                flag->o = 1;
                break;
            case '?':
                exit(1);
            default:
                break;
        }
    }
    if (flag->empty_line) flag->o = 0;
}

void add_e_arg(char* optarg, int* e_count, char* ef_arg, struct flags* flag) {
    if (*e_count) {
        char* ch = "|";
        strcat(ef_arg, ch);
    }
    if (!*optarg) {
        optarg = ".";
        flag->empty_line++;
    }
    strcat(ef_arg, optarg);
    *e_count += 1;
}

void add_f_arg(char* optarg, int* e_count, char* ef_arg, struct flags* flag) {
    FILE* fp_f = NULL;
    int empty_counter = flag->empty_line;

    if ((fp_f = fopen(optarg, "r")) == NULL) {
        printf("grep: %s: No such file or directory\n", optarg);
        exit(2);
    } else {
        while (!feof(fp_f)) {
            if (*e_count) {
                char* ch = "|";
                strcat(ef_arg, ch);
            }

            size_t len = strlen(ef_arg);
            size_t new_len;

            fgets(ef_arg + len, STR_BUFFER, fp_f);
            char* nch = strrchr(ef_arg, 10);
            if (nch) *nch = '\0';
            new_len = strlen(ef_arg);
            if (new_len == len) {
                flag->empty_line++;
                char* all = ".";
                strcat(ef_arg, all);
            }
            *e_count += 1;
        }
        fclose(fp_f);
    }
    if (ef_arg[strlen(ef_arg) - 1] == '.') {
        if ((flag->empty_line - empty_counter) == 1)
            flag->empty_line = empty_counter;
        ef_arg[strlen(ef_arg) - 1] = '\0';
        ef_arg[strlen(ef_arg) - 1] = '\0';
    }
}

void open_files(int argc, char* argv[], char* ef_arg, struct flags flag) {
    FILE* fp;
    int files_count = argc - optind;

    for (; optind < argc; optind++) {
        if ((fp = fopen(argv[optind], "r")) == NULL) {
            if (!flag.s) {
                printf("grep: %s: No such file or directory\n", argv[optind]);
            }
        } else {
            reg_compile(argv, ef_arg, fp, flag, files_count);
            fclose(fp);
        }
    }
}

void reg_compile(char* argv[], char* my_reg, FILE* fp, struct flags flag,
                 int files_count) {
    regex_t myre;
    regmatch_t pmatch[1] = {0};
    size_t nmatch = 1;

    int err;
    char err_msg[MAX_ERR_LENGTH];
    char str[STR_BUFFER];
    size_t n_line = 1;
    int lines_count = 0;
    int i_option = 0;

    if (flag.i) i_option = REG_ICASE;
    err = regcomp(&myre, my_reg, REG_EXTENDED | i_option);

    if (err != 0) {
        regerror(err, &myre, err_msg, MAX_ERR_LENGTH);
        printf("Error analyzing regular expression '%s': %s.\n", my_reg,
               err_msg);
        exit(3);
    }

    while (!feof(fp)) {
        if (fgets(str, STR_BUFFER, fp)) {
            int new_line_o_counter = 1;
            err = regexec(&myre, str, nmatch, pmatch, 0);
            if (flag.v) err = err ? 0 : 1;

            if (!err) {
                if (!flag.c && !flag.l) {
                    if (files_count > 1 && !flag.h) {
                        printf("%s:", argv[optind]);
                    }
                    if (flag.n) {
                        printf("%lu:", n_line);
                    }
                    if (flag.o && !flag.v) {
                        new_line_o_counter = 0;
                        char* ptr = str;
                        while (!err) {
                            if (pmatch[0].rm_eo == pmatch[0].rm_so) break;
                            printf("%.*s\n",
                                   (int)(pmatch[0].rm_eo - pmatch[0].rm_so),
                                   ptr + pmatch[0].rm_so);
                            ptr += pmatch[0].rm_eo;
                            err =
                                regexec(&myre, ptr, nmatch, pmatch, REG_NOTBOL);
                        }
                    }
                    if (!flag.o || flag.v) printf("%s", str);
                    if (str[strlen(str) - 1] != '\n' && new_line_o_counter) {
                        printf("\n");
                    }
                }
                lines_count++;
            }
            n_line++;
        }
    }
    if (flag.c) {
        if (files_count > 1 && !flag.h) {
            printf("%s:", argv[optind]);
        }
        if (flag.l && lines_count)
            printf("1\n");
        else
            printf("%d\n", lines_count);
    }
    if (flag.l && lines_count) {
        printf("%s\n", argv[optind]);
    }
    regfree(&myre);
}
