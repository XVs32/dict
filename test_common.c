#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bench.c"
#include "qf.h"
#include "tst.h"

/** constants insert, delete, max word(s) & stack nodes */
enum { INS, DEL, WRDMAX = 256, STKMAX = 512, LMAX = 1024 };
char common_word[WRDMAX] = "";

#define REF INS
#define CPY DEL

#ifdef REFMODE
#define MEM_MODE REF
long poolsize = 2000000 * WRDMAX;
char *pool;
char *buf;
#define BENCH_TEST_FILE "bench_ref.txt"
#else  // default cpy
#define MEM_MODE CPY
#define BENCH_TEST_FILE "bench_cpy.txt"
char *buf = common_word;
#endif


/* simple trim '\n' from end of buffer filled by fgets */
static void rmcrlf(char *s)
{
    size_t len = strlen(s);
    if (len && s[len - 1] == '\n')
        s[--len] = 0;
}

char *space_handler(char *input)
{
    if (input == NULL) {
        return NULL;
    }
    while (*input == ' ') {
        input++;
    }
    return input;
}

char *format_handler(char *input)
{
    if (input == NULL) {
        return NULL;
    }
    while (*input == ' ') {
        input++;
    }

    int id = 0;
    while (*(input + id) != '\0') {
        if (*(input + id) == '\n' || *(input + id) == '\r') {
            *(input + id) = '\0';
            return input;
        }
        id++;
    }

    return input;
}


unsigned int BKDRHash(char *str, unsigned int len)
{
    unsigned int seed = 131; /* 31 131 1313 13131 131313 etc.. */
    unsigned int hash = 0;
    unsigned int i = 0;

    for (i = 0; i < len; str++, i++) {
        hash = (hash * seed) + (*str);
    }
    return hash;
}



int main(int argc, char **argv)
{
#ifdef REFMODE
    pool = malloc(poolsize * sizeof(char));
    buf = pool;
#endif

    char *sgl[LMAX] = {NULL};
    tst_node *root = NULL, *res = NULL;
    int idx = 0, sidx = 0;
    double t1, t2;

    /*separate city name fix by marco*/
    // char city_file_buf[128];
    char *name;
    /*separate city name fix by marco*/

    FILE *fp = fopen(argv[1], "r");

    if (!fp) { /* prompt, open, validate file for reading */
        fprintf(stderr, "error: file open failed '%s'.\n", argv[1]);
        return 1;
    }

    t1 = tvgetf();

    /*quotient_filter by marco*/
    struct quotient_filter qf;
    qf_init(&qf, 24, 8);
    /*quotient_filter by marco*/

    /* memory pool */
    int Toplen;

    /*separate city name fix by marco*/
    while (fgets(buf, 128, fp) != NULL) {
        Toplen = strlen(buf);
        name = strtok(buf, ",");
        name = format_handler(name);
        while (name != NULL) {
            /* insert reference to each string */
            /*******************************************/
            if (!tst_ins_del(&root, name, INS, MEM_MODE)) { /* fail to insert */
                /*******************************************/
                fprintf(stderr, "error: memory exhausted, tst_insert.\n");
                fclose(fp);
                return 1;
            }

            /*quotient_filter by marco*/
            qf_insert(&qf, BKDRHash(name, strlen(name)));
            /*quotient_filter by marco*/

            idx++;

            name = strtok(NULL, ",");
            name = format_handler(name);
        }
#ifdef REFMODE
        buf += Toplen + 1;
#endif
        /*separate city name fix by marco*/
    }
    t2 = tvgetf();
    fclose(fp);
    printf("ternary_tree, loaded %d words in %.6f sec\n", idx, t2 - t1);

    /*if (argc == 4 && strcmp(argv[2], "--bench") == 0) {
        int stat = bench_test(root, BENCH_TEST_FILE, LMAX);
        tst_free(root);
        return stat;
    }*/

    FILE *output;
    output = fopen("ref.txt", "a");
    if (output != NULL) {
        fprintf(output, "%.6f\n", t2 - t1);
        fclose(output);
    } else
        printf("open file error\n");

    FILE *command;
    if (argc > 2 && strcmp(argv[2], "--bench") == 0) {  // a for auto
        command = fopen(argv[3], "r");
        if (command == NULL) {
            fprintf(stderr, "error: cannot open command file.\n");
            return 1;
        }
    }

    for (;;) {
        printf(
            "\nCommands:\n"
            " a  add word to the tree\n"
            " f  find word in tree\n"
            " s  search words matching prefix\n"
            " d  delete word from the tree\n"
            " q  quit, freeing all data\n\n"
            "choice: ");

        if (argc > 2 && strcmp(argv[2], "--bench") == 0) {  // a for auto

            fgets(common_word, 128, command);

            printf("\n");
            printf("common_word:%c\n", common_word[0]);  // debug only
        } else {
            fgets(common_word, sizeof(common_word), stdin);
        }

        switch (common_word[0]) {
        case 'q':
            goto quit;

        case 'a':
            printf("enter word to add: ");
            if (argc > 2 && strcmp(argv[2], "--bench") == 0) {
                fgets(buf, 128, command);
                printf("buf:%s\n", buf);  // debug only
            } else if (!fgets(buf, sizeof(char) * 256, stdin)) {
                fprintf(stderr, "error: insufficient input.\n");
                break;
            }
            rmcrlf(buf);

            /*quotient_filter by marco*/
            t1 = tvgetf();
            if (qf_may_contain(&qf, BKDRHash(buf, strlen(buf))) == false) {
                qf_insert(&qf, BKDRHash(buf, strlen(buf)));

                /*******************************************/
                if (!tst_ins_del(&root, buf, INS,
                                 MEM_MODE)) { /* fail to insert */
                    fprintf(stderr, "error: memory exhausted, tst_insert.\n");
                    fclose(fp);
                    return 1;
                }
                /*******************************************/
            }
            t2 = tvgetf();

            printf("qf in %.6f sec.\n", t2 - t1);
            /*quotient_filter by marco*/

            break;
        case 'f':
            printf("find word in tree: ");

            if (argc > 2 && strcmp(argv[2], "--bench") == 0) {
                fgets(buf, 128, command);
                printf("buf:%s\n", buf);  // debug only
            } else if (!fgets(buf, sizeof(char) * 256, stdin)) {
                fprintf(stderr, "error: insufficient input.\n");
                break;
            }
            rmcrlf(buf);
            printf("word: %s\n", buf);

            t1 = tvgetf();
            /*quotient_filter by marco*/
            if (qf_may_contain(&qf, BKDRHash(buf, strlen(buf)))) {
                t2 = tvgetf();
                printf("  qf found %s in %.6f sec.\n", buf, t2 - t1);
            } else {
                printf("  %s not found by qf.\n", buf);
            }
            /*quotient_filter by marco*/

            break;
        case 's':
            printf("find words matching prefix (at least 1 char): ");

            if (argc > 2 && strcmp(argv[2], "--bench") == 0) {
                fgets(buf, 128, command);
                printf("buf:%s\n", buf);  // debug only
            } else if (!fgets(buf, sizeof(char) * 256, stdin)) {
                fprintf(stderr, "error: insufficient input.\n");
                break;
            }
            rmcrlf(buf);

            printf("1\n");  // debug only
            t1 = tvgetf();
            res = tst_search_prefix(root, buf, sgl, &sidx, LMAX);
            t2 = tvgetf();
            printf("2\n");  // debug only

            if (res) {
                printf("  %s - searched prefix in %.6f sec\n\n", buf, t2 - t1);
                for (int i = 0; i < sidx; i++) {
                    printf("suggest[%d] : %s\n", i, sgl[i]);
                }
            } else {
                printf("  %s - not found\n", buf);
            }

            break;
        case 'd':
            printf("enter word to del: ");
            if (argc > 2 && strcmp(argv[2], "--bench") == 0) {
                fgets(buf, 128, command);
                printf("buf:%s\n", buf);  // debug only
            } else if (!fgets(buf, sizeof(char) * 256, stdin)) {
                fprintf(stderr, "error: insufficient input.\n");
                break;
            }
            rmcrlf(buf);
            printf("  deleting %s\n", buf);
            t1 = tvgetf();

            res = tst_ins_del(&root, buf, DEL, MEM_MODE);

            t2 = tvgetf();
            if (res)
                printf("  delete failed.\n");
            else {
                printf("  deleted %s in %.6f sec\n", buf, t2 - t1);
                idx--;
            }

            /*quotient_filter by marco*/
            if (qf_remove(&qf, BKDRHash(buf, strlen(buf)))) {
                printf("  qf found %s in %.6f sec.\n", buf, t2 - t1);
            } else {
                printf("  %s not found by qf.\n", buf);
            }
            /*quotient_filter by marco*/

            break;

        default:
            fprintf(stderr, "error: invalid selection.\n");
            break;
        }
    }

quit:
    tst_free(root);
    qf_destroy(&qf);

    return 0;
}
