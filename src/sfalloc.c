#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include "sfmm.h"

#include <stdio.h>

// Colors
#ifdef COLOR
    #define KNRM  "\x1B[0m"
    #define KRED  "\x1B[1;31m"
    #define KGRN  "\x1B[1;32m"
    #define KYEL  "\x1B[1;33m"
    #define KBLU  "\x1B[1;34m"
    #define KMAG  "\x1B[1;35m"
    #define KCYN  "\x1B[1;36m"
    #define KWHT  "\x1B[1;37m"
    #define KBWN  "\x1B[0;33m"
#else
    /* Color was either not defined or Terminal did not support */
    #define KNRM
    #define KRED
    #define KGRN
    #define KYEL
    #define KBLU
    #define KMAG
    #define KCYN
    #define KWHT
    #define KBWN
#endif

#ifdef DEBUG
    #define debug(S, ...)   fprintf(stdout, KMAG "DEBUG: %s:%s:%d " KNRM S, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
    #define error(S, ...)   fprintf(stderr, KRED "ERROR: %s:%s:%d " KNRM S, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
    #define warn(S, ...)    fprintf(stderr, KYEL "WARN: %s:%s:%d " KNRM S, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
    #define info(S, ...)    fprintf(stdout, KBLU "INFO: %s:%s:%d " KNRM S, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
    #define success(S, ...) fprintf(stdout, KGRN "SUCCESS: %s:%s:%d " KNRM S, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
    #define debug(S, ...)
    #define error(S, ...)   fprintf(stderr, KRED "ERROR: " KNRM S, ##__VA_ARGS__)
    #define warn(S, ...)    fprintf(stderr, KYEL "WARN: " KNRM S, ##__VA_ARGS__)
    #define info(S, ...)    fprintf(stdout, KBLU "INFO: " KNRM S, ##__VA_ARGS__)
    #define success(S, ...) fprintf(stdout, KGRN "SUCCESS: " KNRM S, ##__VA_ARGS__)
#endif

// Define 20 megabytes as the max heap size
#define MAX_HEAP_SIZE (20 * (1 << 20))
#define VALUE1_VALUE 320
#define VALUE2_VALUE 0xDEADBEEFF00D

#define press_to_cont() do { \
    printf("Press Enter to Continue"); \
    while(getchar() != '\n'); \
    printf("\n"); \
} while(0)

#define null_check(ptr, size) do { \
    if ((ptr) == NULL) { \
        error("Failed to allocate %lu byte(s) for an integer using sf_malloc.\n", (size)); \
        error("Aborting...\n"); \
        assert(false); \
    } else { \
        success("sf_malloc returned a non-null address: %p\n", (ptr)); \
    } \
} while(0)

#define payload_check(ptr) do { \
    if ((unsigned long)(ptr) % 16 != 0) { \
        warn("Returned payload address is not divisble by a quadword. %p %% 16 = %lu\n", (ptr), (unsigned long)(ptr) % 16); \
    } \
} while(0)

#define check_prim_contents(actual_value, expected_value, fmt_spec, name) do { \
    if (*(actual_value) != (expected_value)) { \
        error("Expected " name " to be " fmt_spec " but got " fmt_spec "\n", (expected_value), *(actual_value)); \
        error("Aborting...\n"); \
        assert(false); \
    } else { \
        success(name " retained the value of " fmt_spec " after assignment\n", (expected_value)); \
    } \
} while(0)

void printinfo(info *info)
{
    printf("internal:    %lu\n"
            "external:    %lu\n"
            "allocations: %lu\n"
            "frees:       %lu\n"
            "coalesces:   %lu\n", info->internal, info->external, info->allocations, info->frees, info->coalesce);
    sf_snapshot(true);
}

int main(int argc, char *argv[]) {
    // Initialize the custom allocator
    // extern void *heap_start;
    // info info;
        sf_mem_init(MAX_HEAP_SIZE);

    // Tell the user about the fields
    info("Initialized heap with %dmb of heap space.\n", MAX_HEAP_SIZE >> 20);
    info info;
    press_to_cont();

    sf_free_header *t;

    printf("allocate val1\n");
    int *val1 = sf_malloc(sizeof(int));
    int *val2 = sf_malloc(12 * sizeof(int));
    int *val3 = sf_malloc(12 * sizeof(int));
    int *val4 = sf_malloc(sizeof(int));

    *val1 = *val2 = *val3 = *val4 = 5;

    press_to_cont();

    t = freelist_head;
while (t) {
    sf_blockprint(t);
    t = t->next;
}

if (!sf_info(&info)) 
    printinfo(&info);
press_to_cont();

printf("freeing\n");

sf_free(val3);
sf_free(val1);

    t = freelist_head;
while (t) {
    sf_blockprint(t);
    t = t->next;
}

if (!sf_info(&info)) 
    printinfo(&info);
press_to_cont();

printf("alocate exact 64\n");
int *val5 = sf_malloc(12 * sizeof(int));
sf_varprint(val5);
press_to_cont();

    t = freelist_head;
while (t) {
    sf_blockprint(t);
    t = t->next;
}

if (!sf_info(&info)) 
    printinfo(&info);
press_to_cont();

printf("free all\n");
sf_free(val2);
sf_free(val4);
sf_free(val5);

press_to_cont();

    t = freelist_head;
while (t) {
    sf_blockprint(t);
    t = t->next;
}

if (!sf_info(&info)) 
    printinfo(&info);
press_to_cont();

sf_mem_fini();





// printf("allocate val1\n");
// int *val1 = sf_malloc(sizeof(int));
// null_check(val1, sizeof(int));
// payload_check(val1);
// sf_varprint(val1);
// press_to_cont();

// t = freelist_head;
// while (t) {
//     sf_blockprint(t);
//     t = t->next;
// }

// if (!sf_info(&info)) 
//     printinfo(&info);
// press_to_cont();

// printf("reallocate val1\n");
// val1 = sf_realloc(val1, 50 * sizeof(int));
// null_check(val1, 50 * sizeof(int));
// payload_check(val1);
// sf_varprint(val1);
// press_to_cont();

// t = freelist_head;
// while (t) {
//     sf_blockprint(t);
//     t = t->next;
// }

// if (!sf_info(&info)) 
//     printinfo(&info);
// press_to_cont();

// printf("free val\n");
// sf_free(val1);
// t = freelist_head;
// while (t) {
//     sf_blockprint(t);
//     t = t->next;
// }

// if (!sf_info(&info)) 
//     printinfo(&info);
// press_to_cont();

// printf("fail: allocate val2\n");
// long double *val2 = sf_realloc(val1, 15);

// if (!val2)
//     printf("val2 allcation failed\n");

// press_to_cont();

// printf("fail: allocate val2\n");
// val2 = sf_realloc(val2, 0);

// if (!val2)
//     printf("val2 allocation failed\n");

// press_to_cont();

// printf("allocate val2\n");
// val2 = sf_malloc(100 * sizeof(long double));
// null_check(val2, 100 * sizeof(long double));
// payload_check(val2);
// sf_varprint(val2);
// press_to_cont();

// t = freelist_head;
// while (t) {
//     sf_blockprint(t);
//     t = t->next;
// }

// if (!sf_info(&info)) 
//     printinfo(&info);
// press_to_cont();

// printf("allocate val3\n");
// char *val3 = sf_malloc(20 * sizeof(char));
// null_check(val3, 20 * sizeof(char));
// payload_check(val3);
// sf_varprint(val3);
// press_to_cont();

// t = freelist_head;
// while (t) {
//     sf_blockprint(t);
//     t = t->next;
// }

// if (!sf_info(&info)) 
//     printinfo(&info);
// press_to_cont();

// printf("allocate val4\n");
// double *val4 = sf_malloc(200 * sizeof(double));
// null_check(val4, 200 * sizeof(double));
// payload_check(val4);
// sf_varprint(val4);
// press_to_cont();

// t = freelist_head;
// while (t) {
//     sf_blockprint(t);
//     t = t->next;
// }

// if (!sf_info(&info)) 
//     printinfo(&info);
// press_to_cont();

// printf("fail allocation too big\n");
// int *val5 = sf_malloc(4096 * sizeof(int) - 16);
// if (!val5)
//     printf("allocation failed\n");
// press_to_cont();

// t = freelist_head;
// while (t) {
//     sf_blockprint(t);
//     t = t->next;
// }

// if (!sf_info(&info)) 
//     printinfo(&info);
// press_to_cont();

// printf("reallocate val2\n");
// val2 = sf_realloc(val2, 1000 * sizeof(double));
// null_check(val2, 1000 * sizeof(double));
// payload_check(val2);
// sf_varprint(val2);
// press_to_cont();

// t = freelist_head;
// while (t) {
//     sf_blockprint(t);
//     t = t->next;
// }

// if (!sf_info(&info)) 
//     printinfo(&info);
// press_to_cont();

// printf("allocate val6\n");
// int *val6 = sf_malloc(1000 * sizeof(int));
// null_check(val6, 1000 * sizeof(int));
// payload_check(val6);
// sf_varprint(val6);
// press_to_cont();

// t = freelist_head;
// while (t) {
//     sf_blockprint(t);
//     t = t->next;
// }

// if (!sf_info(&info)) 
//     printinfo(&info);
// press_to_cont();

// printf("allocate val7\n");
// char *val7 = sf_malloc(sizeof(char));
// null_check(val7, sizeof(char));
// payload_check(val7);
// sf_varprint(val7);
// press_to_cont();

// t = freelist_head;
// while (t) {
//     sf_blockprint(t);
//     t = t->next;
// }

// if (!sf_info(&info)) 
//     printinfo(&info);
// press_to_cont();

// printf("free val4\n");
// sf_free(val4);
// t = freelist_head;
// while (t) {
//     sf_blockprint(t);
//     t = t->next;
// }

// if (!sf_info(&info)) 
//     printinfo(&info);
// press_to_cont();

// printf("reallocate val7\n");
// val7 = sf_realloc(val7, 10 * sizeof(char));
// null_check(val7, 10 * sizeof(char));
// payload_check(val7);
// sf_varprint(val7);
// press_to_cont();

// t = freelist_head;
// while (t) {
//     sf_blockprint(t);
//     t = t->next;
// }

// if (!sf_info(&info)) 
//     printinfo(&info);
// press_to_cont();

// printf("free val7\n");
// sf_free(val7);
// t = freelist_head;
// while (t) {
//     sf_blockprint(t);
//     t = t->next;
// }

// if (!sf_info(&info)) 
//     printinfo(&info);
// press_to_cont();

// printf("free val2\n");
// sf_free(val2);
// t = freelist_head;
// while (t) {
//     sf_blockprint(t);
//     t = t->next;
// }

// if (!sf_info(&info)) 
//     printinfo(&info);
// press_to_cont();

// printf("free val3\n");
// sf_free(val3);
// t = freelist_head;
// while (t) {
//     sf_blockprint(t);
//     t = t->next;
// }

// if (!sf_info(&info)) 
//     printinfo(&info);
// press_to_cont();

// printf("free val6\n");
// sf_free(val6);
// t = freelist_head;
// while (t) {
//     sf_blockprint(t);
//     t = t->next;
// }

// if (!sf_info(&info)) 
//     printinfo(&info);
// press_to_cont();

// printf("allocate all\n");
// int *val8 = sf_malloc(4096 * sizeof(int) - 16);
// null_check(val8, 4096 * sizeof(int) - 16);
// payload_check(val8);
// sf_varprint(val8);
// press_to_cont();

// t = freelist_head;
// while (t) {
//     sf_blockprint(t);
//     t = t->next;
// }

// if (!sf_info(&info)) 
//     printinfo(&info);
// press_to_cont();

// printf("try to reallocate\n");
// sf_realloc(val8, 10000000);
// press_to_cont();

// printf("try to reallocate\n");
// val8 = sf_realloc(val8, 4095 * sizeof(int) - 16);
// sf_varprint(val8);
// press_to_cont();

// printf("free all\n");
// sf_free(val8);
// t = freelist_head;
// while (t) {
//     sf_blockprint(t);
//     t = t->next;
// }

// if (!sf_info(&info)) 
//     printinfo(&info);
// press_to_cont();

// sf_mem_fini();





    // printf("allocate big stuff\n");
    // int *val1 = sf_malloc(4096 * sizeof(int) - 16);
    //     null_check(val1, 4096 * sizeof(int) - 16);
    // payload_check(val1);
    // sf_varprint(val1);
    // press_to_cont();

    //     sf_snapshot(true);
    // sf_free_header *t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }

    // if (!sf_info(&info)) 
    //     printinfo(&info);
    // press_to_cont();

    // printf("free stuff\n");
    // sf_free(val1);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // if (!sf_info(&info)) 
    //     printinfo(&info);

    // sf_mem_fini();

    // printf("allocate val1\n");
    // int *val1 = sf_malloc(4);
    // null_check(val1, sizeof(int));
    // payload_check(val1);
    // sf_varprint(val1);
    // press_to_cont();

    // printf("allocate val2\n");
    // int *val2 = sf_malloc(4);
    // null_check(val2, sizeof(int));
    // payload_check(val2);
    // sf_varprint(val2);
    // press_to_cont();

    // printf("allocate val3\n");
    // int *val3 = sf_malloc(4);
    // null_check(val3, sizeof(int));
    // payload_check(val3);
    // sf_varprint(val3);
    // press_to_cont();

    // sf_snapshot(true);
    // sf_free_header *t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allocate val4\n");
    // int *val4 = sf_malloc(1000 * sizeof(int) - 16);
    // null_check(val4, 1000 * sizeof(int) - 16);
    // payload_check(val4);
    // sf_varprint(val4);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // sf_free(val3);
    // sf_free(val1);

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // sf_free(val4);

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allocate val5\n");
    // int *val5 = sf_malloc(1004 * sizeof(int));
    // null_check(val5, 1004 * sizeof(int));
    // payload_check(val5);
    // sf_varprint(val5);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allocate val1\n");
    // int *val1 = sf_malloc(sizeof(int));
    // null_check(val1, sizeof(int));
    // payload_check(val1);
    // sf_varprint(val1);
    // press_to_cont();

    // if (!sf_info(&info)) 
    //     printinfo(&info);

    // sf_snapshot(true);
    // sf_free_header *t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("reallocate val1\n");
    // val1 = sf_realloc(val1, 12 * sizeof(int));
    // null_check(val1, 12 * sizeof(int));
    // payload_check(val1);
    // sf_varprint(val1);
    // press_to_cont();

    // if (!sf_info(&info)) 
    //     printinfo(&info);

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();


    // printf("allocate bunch of stuffs\n");
    // int *val1 = sf_malloc(48);
    // double *val2 = sf_malloc(5 * sizeof(double));
    // long double *val3 = sf_malloc(10 * sizeof(long double));
    // float *val4 = sf_malloc(2 * sizeof(float));

    // *val1 = 2; *val2 = 3; *val3 = 4; *val4 = 5;
    // press_to_cont();

    // sf_snapshot(true);
    // sf_free_header *t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // sf_free(val1);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // sf_free(val3);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("reallocate val2\n");
    // val2 = sf_realloc(val2, sizeof(int));
    // null_check(val2, sizeof(int));
    // payload_check(val2);
    // sf_varprint(val2);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // null_check(val1, sizeof(int));
    // payload_check(val1);
    // sf_varprint(val1);




    // printf("allocate val1\n");
    // int *val1 = sf_malloc(4);
    // null_check(val1, sizeof(int));
    // payload_check(val1);
    // sf_varprint(val1);
    // press_to_cont();

    // sf_snapshot(true);
    // sf_free_header *t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allocate val2\n");
    // int *val2 = sf_malloc(4);
    // null_check(val2, sizeof(int));
    // payload_check(val2);
    // sf_varprint(val2);
    // press_to_cont();

    // printf("free val1\n");
    // sf_free(val1);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allocate 4 pages\n");
    // int *val3 = sf_malloc(4096 * sizeof(int) - 16);
    // if (!val3)
    //     printf("was not allcoated correctly\n");
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // sf_free(heap_start - 8);
//     printf("allocate val1\n");
//     int *x = sf_malloc(400 * sizeof(int));
//     long *y = sf_malloc(sizeof(long));
//     double *z = sf_malloc(2 * sizeof(double));
//     long double *a = sf_malloc(5 * sizeof(long double));
//     press_to_cont();

//     *x = 1; *y = 2; *z = 3; *a = 1512121212;
//     printf("%d %lu %.0f %Lf\n", *x, *y, *z, *a);

//     if (!sf_info(&info)) 
//         printinfo(&info);

//     sf_snapshot(true);
//     sf_free_header *t = freelist_head;
//     while (t) {
//         sf_blockprint(t);
//         t = t->next;
//     }
//     press_to_cont();

//     printf("free x\n");
//     sf_free(x);
//     sf_snapshot(true);
//     t = freelist_head;
//     while (t) {
//         sf_blockprint(t);
//         t = t->next;
//     }
//     if (!sf_info(&info)) 
//         printinfo(&info);
//     press_to_cont();

//     printf("free z\n");
//     sf_free(z);
//     sf_snapshot(true);
//     t = freelist_head;
//     while (t) {
//         sf_blockprint(t);
//         t = t->next;
//     }
//         if (!sf_info(&info)) 
//         printinfo(&info);
//     press_to_cont();

//     printf("reallocate a\n");
//     a = sf_realloc(a, 5000);
//     printf("here\n");
//     null_check(a, 5000 * sizeof(char));
//     payload_check(a);
//     sf_varprint(a);

//     printf("a again: %Lf\n", *a);
//     press_to_cont();


//     sf_snapshot(true);
//     t = freelist_head;
//     while (t) {
//         sf_blockprint(t);
//         t = t->next;
//     }
//         if (!sf_info(&info)) 
//         printinfo(&info);
//     press_to_cont();


//     //      CAUTIONAOISNDSANDKASNDKANDASNDKA
//     // press_to_cont();
//     // printf("free y\n");
//     // sf_free(y);
//     // sf_snapshot(true);
//     // t = freelist_head;
//     // while (t) {
//     //     sf_blockprint(t);
//     //     t = t->next;
//     // }
//     // press_to_cont();

//     printf("free a\n");
//     sf_free(a);
//     sf_snapshot(true);
//     t = freelist_head;
//     while (t) {
//         sf_blockprint(t);
//         t = t->next;
//     }
//         if (!sf_info(&info)) 
//         printinfo(&info);
// press_to_cont();

// printf("free y\n");
//     sf_free(y);
//     sf_snapshot(true);
//     t = freelist_head;
//     while (t) {
//         sf_blockprint(t);
//         t = t->next;
//     }
//         if (!sf_info(&info)) 
//         printinfo(&info);
//     press_to_cont();
//     // printf("free y\n");
//     // sf_free(y);
//     // sf_snapshot(true);
//     // t = freelist_head;
//     // while (t) {
//     //     sf_blockprint(t);
//     //     t = t->next;
//     // }
//     // press_to_cont();

//     sf_mem_fini();
    // printf("free a\n");
    // sf_free(a);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("free y\n");
    // sf_free(y);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allocate val1\n");
    // int *x = sf_malloc(16368);
    // null_check(x, 16388 * sizeof(char));
    // payload_check(x);
    // sf_varprint(x);
    // press_to_cont();

    //  sf_snapshot(true);
    // sf_free_header *t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allocate val2\n");
    // int *y = sf_malloc(400);
    // if (!y)
    //     printf("too little mem\n");
    
    // printf("allocate 3 vals\n");
    // int *x = sf_malloc(4);
    // int *y = sf_malloc(12 * sizeof(int));
    // int *z = sf_malloc(4);

    // *x = 3333333333;
    // *y = 10;
    // *z = 4;

    // printf("x: %d\n", *x);

    // if (!sf_info(&info)) 
    //     printinfo(&info);

    // sf_snapshot(true);
    // sf_free_header *t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("free y\n");
    // sf_free(y);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("reallocate x to 64\n");

    // x = sf_realloc(x, 8 * sizeof(int));
    // null_check(x, 8 * sizeof(int));
    // payload_check(x);
    // sf_varprint(x);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("x: %d\n", *x);

    // printf("allocate val1\n");
    // int *val1 = sf_malloc(sizeof(int));
    // null_check(val1, 12 * sizeof(int));
    // payload_check(val1);
    // sf_varprint(val1);
    // press_to_cont();

    // if (!sf_info(&info)) 
    //     printinfo(&info);

    // sf_snapshot(true);
    // sf_free_header *t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("reallocate val1 to 12 ints\n");
    // val1 = sf_realloc(val1, 10);
    // payload_check(val1);
    // sf_varprint(val1);
    // press_to_cont();

    // if (!sf_info(&info)) 
    //     printinfo(&info);

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }

    // printf("allocate val2\n");
    // int *val2 = sf_malloc(100 *sizeof(int));
    // null_check(val2, 100*sizeof(int));
    // payload_check(val2);
    // sf_varprint(val2);
    // press_to_cont();

    // if (!sf_info(&info)) 
    //     printinfo(&info);

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allocate val3\n");
    // int *val3 = sf_malloc(200 *sizeof(int));
    // null_check(val3, 200*sizeof(int));
    // payload_check(val3);
    // sf_varprint(val3);
    // press_to_cont();

    // if (!sf_info(&info)) 
    //     printinfo(&info);

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allocate val4\n");
    // int *val4 = sf_malloc(500 *sizeof(int));
    // null_check(val4, 500*sizeof(int));
    // payload_check(val4);
    // sf_varprint(val4);
    // press_to_cont();

    // if (!sf_info(&info)) 
    //     printinfo(&info);

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("free val4\n");
    // sf_free(val4);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // if (!sf_info(&info)) 
    //     printinfo(&info);
    // press_to_cont();

    // printf("free val2\n");
    // sf_free(val2);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // if (!sf_info(&info)) 
    //     printinfo(&info);
    // press_to_cont();

    // printf("free val3\n");
    // sf_free(val3);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // if (!sf_info(&info)) 
    //     printinfo(&info);
    // press_to_cont();
    
    // printf("free val1\n");
    // sf_free(val1);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // if (!sf_info(&info)) 
    //     printinfo(&info);
    // press_to_cont();

    // sf_mem_fini();

    // Print out title for first test
    // printf("=== Test1: Allocation test ===\n");
    // // Test #1: Allocate an integer
    // int *value1 = sf_malloc(sizeof(int));
    // null_check(value1, sizeof(int));
    // payload_check(value1);
    // // Print out the allocator block
    // sf_varprint(value1);
    // press_to_cont();

    // // Now assign a value
    // printf("=== Test2: Assignment test ===\n");
    // info("Attempting to assign value1 = %d\n", VALUE1_VALUE);
    // // Assign the value
    // *value1 = VALUE1_VALUE;
    // // Now check its value
    // check_prim_contents(value1, VALUE1_VALUE, "%d", "value1");
    // press_to_cont();

    // printf("=== Test3: Allocate a second variable ===\n");
    // info("Attempting to assign value2 = %ld\n", VALUE2_VALUE);
    // long *value2 = sf_malloc(sizeof(long));
    // null_check(value2, sizeof(long));
    // payload_check(value2);
    // sf_varprint(value2);
    // // Assign a value
    // *value2 = VALUE2_VALUE;
    // // Check value
    // check_prim_contents(value2, VALUE2_VALUE, "%ld", "value2");
    // press_to_cont();

    // printf("=== Test4: does value1 still equal %d ===\n", VALUE1_VALUE);
    // check_prim_contents(value1, VALUE1_VALUE, "%d", "value1");
    // press_to_cont();

    // // Snapshot the freelist
    // printf("=== Test5: Perform a snapshot ===\n");
    // sf_snapshot(true);
    // press_to_cont();

    // // Free a variable
    // printf("=== Test6: Free a block and snapshot ===\n");
    // info("Freeing value1...\n");
    // sf_free(value1);
    // sf_snapshot(true);
    // press_to_cont();

    // // Allocate more memory
    // printf("=== Test7: 8192 byte allocation ===\n");
    // void *memory = sf_malloc(8192);
    // sf_varprint(memory);
    // sf_free(memory);
    // press_to_cont();

    // sf_mem_fini();
    // sf_free_header *t;
    // sf_mem_init(MAX_HEAP_SIZE);
    
    // //Tell the user about the fields heap space.\n", MAX_HEAP_SIZE >> 20);
    // press_to_cont();

    // printf("allocate val1\n");
    // int *val1 = sf_malloc(4092 * sizeof(int));
    // null_check(val1, 4092 * sizeof(int));
    // payload_check(val1);
    // sf_varprint(val1);

    // if (val1 == NULL)
    //     printf("val1 didnt allocate properly\n");
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }

    // printf("allocate val2\n");
    // int *val2 = sf_malloc(sizeof(int));
    // if (val2 == NULL)
    //     printf("val2 didnt allocate properly\n");
    // press_to_cont();

    // printf("reallocate val1\n");
    // val1 = sf_realloc(val1, 100 * sizeof(int));
    // null_check(val1, 100 * sizeof(int));
    // payload_check(val1);
    // sf_varprint(val1);

    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("freeing val1\n");
    // sf_free(val1);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }

    // sf_mem_fini();

    // printf("allocate val1\n");
    // int *val1 = sf_malloc(12 * sizeof(int));
    // null_check(val1, 12 * sizeof(int));
    // payload_check(val1);
    // sf_varprint(val1);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allocate val2\n");
    // int *val2 = sf_malloc(100 * sizeof(int));
    // null_check(val2, 100 * sizeof(int));
    // payload_check(val2);
    // sf_varprint(val2);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allocate val3\n");
    // int *val3 = sf_malloc(900 * sizeof(int));
    // null_check(val3, 900 * sizeof(int));
    // payload_check(val3);
    // sf_varprint(val3);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allocate val3 to 1000 ints\n");
    // val3 = sf_realloc(val3, 1000 * sizeof(int));
    // null_check(val3, 1000 * sizeof(int));
    // payload_check(val3);
    // sf_varprint(val3);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allocate val1 to 15 ints\n");
    // val1 = sf_realloc(val1, 15 * sizeof(int));
    // null_check(val1, 15 * sizeof(int));
    // payload_check(val1);
    // sf_varprint(val1);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allocate val3 to 1001 ints\n");
    // val3 = sf_realloc(val3, 1020 * sizeof(int));
    // null_check(val3, 1001 * sizeof(int));
    // payload_check(val3);
    // sf_varprint(val3);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("free val3\n");
    // sf_free(val3);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("free val2\n");
    // sf_free(val2);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("free val1\n");
    // sf_free(val1);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // sf_mem_fini();


    // press_to_cont();
    // printf("allocated val1\n");
    // int *val1 = sf_malloc(12 * sizeof(int));
    // null_check(val1, 12 * sizeof(int));
    // payload_check(val1);
    // sf_varprint(val1);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allocated val2\n");
    // int *val2 = sf_malloc(100 * sizeof(int));
    // null_check(val2, 100 * sizeof(int));
    // payload_check(val2);
    // sf_varprint(val2);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("reallocate val1\n");
    // val1 = sf_realloc(val1, sizeof(int));
    // null_check(val1, sizeof(int));
    // payload_check(val1);
    // sf_varprint(val1);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allcoated val1\n");
    // int *val1 = sf_malloc(1000 * sizeof(int));
    // null_check(val1, 1000 * sizeof(int));
    // payload_check(val1);
    // sf_varprint(val1);
    // press_to_cont();

    // sf_snapshot(true);
    // sf_free_header *t;
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allcoated val2\n");
    // int *val2 = sf_malloc(1000 * sizeof(int));
    // null_check(val2, 1000 * sizeof(int));
    // payload_check(val2);
    // sf_varprint(val2);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allcoated val3\n");
    // int *val3 = sf_malloc(sizeof(int));
    // null_check(val3, sizeof(int));
    // payload_check(val3);
    // sf_varprint(val3);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allcoated val4\n");
    // long double *val4 = sf_malloc(2 * sizeof(long double));
    // null_check(val4, 2 * sizeof(long double));
    // payload_check(val4);
    // sf_varprint(val4);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();


    // printf("free val1\n");
    // sf_free(val1);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("free val2\n");
    // sf_free(val2);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("free val4\n");
    // sf_free(val4);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("free val3\n");
    // sf_free(val3);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allocate val5\n");
    // int *val5 = sf_malloc(4000 * sizeof(int));
    // null_check(val5, 4000 * sizeof(int));
    // payload_check(val5);
    // sf_varprint(val5);
    // press_to_cont();

    // sf_snapshot(true);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("free val5\n");
    // sf_free(val5);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // sf_mem_fini();

    // printf("free value2\n");
    // sf_free(val2);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("free value4\n");
    // sf_free(val4);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("free value1\n");
    // sf_free(val1);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("attempt to allocate large value\n");
    // int *val5 = sf_malloc(4000 * sizeof(int));
    // if (!val5)
    //     printf("no allocated\n");
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allocate val6\n");
    // double *val6 = sf_malloc(1000 * sizeof(double));
    // null_check(val6, 1000 * sizeof(double));
    // payload_check(val6);
    // sf_varprint(val6);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("free val6\n");
    // sf_free(val6);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("free val3\n");
    // sf_free(val3);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // sf_mem_fini();




    // printf("allcoated val1\n");
    // int *val1 = sf_malloc(1000 * sizeof(int));
    // null_check(val1, 1000 * sizeof(int));
    // payload_check(val1);
    // sf_varprint(val1);
    // press_to_cont();

    // sf_snapshot(true);
    // sf_free_header *t;
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allcoated val2\n");
    // int *val2 = sf_malloc(2000 * sizeof(int));
    // null_check(val2, 2000 * sizeof(int));
    // payload_check(val2);
    // sf_varprint(val2);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("free val1\n");
    // sf_free(val1);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allocate val3\n");
    // int *val3 = sf_malloc(500 * sizeof(int));
    // null_check(val3, 500 * sizeof(int));
    // payload_check(val3);
    // sf_varprint(val3);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allocate val4\n");
    // int *val4 = sf_malloc(sizeof(int));
    // null_check(val4, sizeof(int));
    // payload_check(val4);
    // sf_varprint(val4);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("free val3\n");
    // sf_free(val3);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("free val2\n");
    // sf_free(val2);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("free val4\n");
    // sf_free(val4);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // sf_mem_fini();

    /********* BRANCHING HERE *************/ 
    // for head to the left
    // printf("free val2\n");
    // sf_free(val2);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("free val3\n");
    // sf_free(val3);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("free val4\n");
    // sf_free(val4);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allocate val5\n");
    // int *val5 = sf_malloc(4092 * sizeof(int));
    // null_check(val5, 4092 * sizeof(int));
    // payload_check(val5);
    // sf_varprint(val5);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }

    // sf_free(val5);


    // sf_mem_fini();
    //for head to the right



    // printf("allocated val1\n");
    // int *val1 = sf_malloc(sizeof(int));
    // null_check(val1, sizeof(int));
    // payload_check(val1);
    // sf_varprint(val1);
    // press_to_cont();

    // sf_free_header *t = freelist_head;
    // sf_snapshot(true);
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allocated val3\n");
    // int *val3 = sf_malloc(1012 * sizeof(int));
    // null_check(val3, 1012 * sizeof(int));
    // payload_check(val3);
    // sf_varprint(val3);
    // press_to_cont();

    // t = freelist_head;
    // sf_snapshot(true);
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }

    // printf("allocated val2\n");
    // int *val2 = sf_malloc(1000 * sizeof(int));
    // null_check(val2, 1000 * sizeof(int));
    // payload_check(val2);
    // sf_varprint(val2);
    // press_to_cont();

    // t = freelist_head;
    // sf_snapshot(true);
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("freeing val1\n");
    // sf_free(val1);
    // t = freelist_head;
    // sf_snapshot(true);
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("freeing val3\n");
    // sf_free(val3);
    // t = freelist_head;
    // sf_snapshot(true);
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("freeing val2\n");
    // sf_free(val2);
    // t = freelist_head;
    // sf_snapshot(true);
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }

    // sf_mem_fini();

    // printf("allocated big val1\n");
    // int *val1 = sf_malloc(4050 * sizeof(int));
    // null_check(val1, 4050 * sizeof(int));
    // payload_check(val1);
    // sf_varprint(val1);
    // press_to_cont();


    // printf("check free list\n");
    // sf_snapshot(true);
    // sf_free_header *t = freelist_head;
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("free the giant penis\n");
    // sf_free(val1);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // printf("alocated too big val\n");
    // int *value = sf_malloc(5000 * sizeof(int));
    // if (!value) {
    //     printf("not enough mem\n");
    // }

    // printf("allocated val2\n");
    // int *val2 = sf_malloc(3000 * sizeof(int));
    // null_check(val2, 3000 * sizeof(int));
    // payload_check(val2);
    // sf_varprint(val2);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allocate val3\n");
    // char *val3 = sf_malloc(sizeof(char));
    // null_check(val3, sizeof(char));
    // payload_check(val3);
    // sf_varprint(val3);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allocate val4\n");
    // char *val4 = sf_malloc(4050 * sizeof(char));
    // null_check(val4, 4050 * sizeof(char));
    // payload_check(val4);
    // sf_varprint(val4);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("free val3\n");
    // sf_free(val3);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allocate val5\n");
    // char *val5 = sf_malloc(4050 * sizeof(char));
    // if (val5 == NULL)
    //     printf("yes\n");

    // press_to_cont();

    // printf("allocate val6\n");
    // char *val6 = sf_malloc(240 * sizeof(char));
    // null_check(val6, 240 * sizeof(char));
    // payload_check(val6);
    // sf_varprint(val6);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // sf_free(val2);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();
    // sf_free(val4);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();
    // sf_free(val6);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }

    // sf_mem_fini();

    // printf("allocate val1\n");
    // int *val1 = sf_malloc(4080);
    // null_check(val1, 1020 * sizeof(int));
    // payload_check(val1);
    // sf_varprint(val1);
    // press_to_cont();

    // sf_snapshot(true);
    // sf_free_header *t = freelist_head;
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }

    // press_to_cont();

    // printf("allocate val2\n");
    // int *val2 = sf_malloc(sizeof(int));
    // null_check(val2, sizeof(int));
    // payload_check(val2);
    // sf_varprint(val2);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }

    // press_to_cont();

    // printf("allocate val3\n");
    // int *val3 = sf_malloc(sizeof(int));
    // null_check(val3, sizeof(int));
    // payload_check(val3);
    // sf_varprint(val3);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }

    // press_to_cont();

    // printf("allocate val4\n");
    // int *val4 = sf_malloc(950 * sizeof(int));
    // null_check(val4, 950 * sizeof(int));
    // payload_check(val4);
    // sf_varprint(val4);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }

    // press_to_cont();

    // printf("allocate val5\n");
    // int *val5 = sf_malloc(950 * sizeof(int));
    // null_check(val5, 950 * sizeof(int));
    // payload_check(val5);
    // sf_varprint(val5);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }


    // printf("allocate val2\n");
    // int *val2 = sf_malloc(sizeof(int));
    // null_check(val2, sizeof(int));
    // payload_check(val2);
    // sf_varprint(val2);
    // press_to_cont();

    // printf("allocate val3\n");
    // int *val3 = sf_malloc(sizeof(int));
    // null_check(val3, sizeof(int));
    // payload_check(val3);
    // sf_varprint(val3);
    // press_to_cont();

    // printf("allocate val4\n");
    // int *val4 = sf_malloc(sizeof(int));
    // null_check(val4, sizeof(int));
    // payload_check(val4);
    // sf_varprint(val4);
    // press_to_cont();

    // printf("allocate val5\n");
    // int *val5 = sf_malloc(sizeof(int));
    // null_check(val5, sizeof(int));
    // payload_check(val5);
    // sf_varprint(val5);
    // press_to_cont();

    // sf_free_header *t;
    // printf("free val2\n");
    // sf_free(val2);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allocate val6\n");
    // int *val6 = sf_malloc(sizeof(int));
    // null_check(val6, sizeof(int));
    // payload_check(val6);
    // sf_varprint(val6);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }



    // printf("allocate val1\n");
    // int *val1 = sf_malloc(sizeof(int));
    // memset(val1, 0, sizeof(*val1));
    // null_check(val1, sizeof(int));
    // payload_check(val1);
    // sf_varprint(val1);
    // press_to_cont();

    // sf_snapshot(true);
    // sf_free_header *t = freelist_head;
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allocate val2\n");
    // long double *val2 = sf_malloc(sizeof(long double));
    // memset(val2, 0, sizeof(*val2));
    // null_check(val2, sizeof(long double));
    // payload_check(val2);
    // sf_varprint(val2);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allocate val3\n");
    // double *val3 = sf_malloc(sizeof(double));
    // memset(val3, 0, sizeof(*val3));
    // null_check(val3, sizeof(double));
    // payload_check(val3);
    // sf_varprint(val3);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allocate val4\n");
    // long *val4 = sf_malloc(sizeof(long));
    // memset(val4, 0, sizeof(*val4));
    // null_check(val4, sizeof(long));
    // payload_check(val4);
    // sf_varprint(val4);
    // press_to_cont();

    // sf_snapshot(true);
    // t = freelist_head;
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allocate val5\n");
    // long double *val5 = sf_malloc(5 * sizeof(long double));
    // memset(val5, 0, sizeof(*val5));
    // null_check(val5, 5 * sizeof(long double));
    // payload_check(val5);
    // sf_varprint(val5);
    // press_to_cont();

    // printf("check freelist");
    // sf_snapshot(true);
    // press_to_cont();

    // printf("free val4\n");
    // sf_free(val4);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("free val1\n");
    // sf_free(val1);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("free val3\n");
    // sf_free(val3);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }

    // printf("allocate val6\n");
    // int *val6 = sf_malloc(sizeof(int));
    // memset(val6, 0, sizeof(*val6));
    // null_check(val6,sizeof(int));
    // payload_check(val6);
    // sf_varprint(val6);

    // press_to_cont();

    // printf("check freelist\n");
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }
    // press_to_cont();

    // printf("allocate val7\n");
    // int *val7 = sf_malloc(sizeof(int));
    // memset(val7, 0, sizeof(*val7));
    // null_check(val7,sizeof(int));
    // payload_check(val7);
    // sf_varprint(val7);
    // press_to_cont();

    // printf("check freelist\n");
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }

    // press_to_cont();

    // printf("free val2\n");
    // sf_free(val2);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }

    // press_to_cont();

    // printf("free val6\n");
    // sf_free(val6);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }

    // press_to_cont();

    // printf("free val7\n");
    // sf_free(val7);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }

    // press_to_cont();

    // printf("free val5\n");
    // sf_free(val5);
    // sf_snapshot(true);
    // t = freelist_head;
    // while (t != NULL) {
    //     sf_blockprint(t);
    //     t = t->next;
    // }

    // sf_mem_fini();

    // Print out title for first test
    // printf("=== Test1: Allocation test ===\n");
    // Test #1: Allocate an integer
    // int *value1 = sf_malloc(sizeof(int));
    // null_check(value1, sizeof(int));
    // payload_check(value1);
    // // Print out the allocator block
    // sf_varprint(value1);
    // press_to_cont();

    
    // // Now assign a value
    // printf("=== Test2: Assignment test ===\n");
    // info("Attempting to assign value1 = %d\n", VALUE1_VALUE);
    // // Assign the value
    // *value1 = VALUE1_VALUE;
    // // Now check its value
    // check_prim_contents(value1, VALUE1_VALUE, "%d", "value1");
    // sf_snapshot(true);

    
    // press_to_cont();

    
    // printf("=== Test3: Allocate a second variable ===\n");
    // info("Attempting to assign value2 = %ld\n", VALUE2_VALUE);
    // long *value2 = sf_malloc(sizeof(long));
    // null_check(value2, sizeof(long));
    // payload_check(value2);
    // sf_varprint(value2);
    // // Assign a value
    // *value2 = VALUE2_VALUE;
    // // Check value
    // check_prim_contents(value2, VALUE2_VALUE, "%ld", "value2");
    // press_to_cont();

    // printf("=== Test4: does value1 still equal %d ===\n", VALUE1_VALUE);
    // check_prim_contents(value1, VALUE1_VALUE, "%d", "value1");
    // press_to_cont();


    // // Snapshot the freelist
    // printf("=== Test5: Perform a snapshot ===\n");
    // sf_snapshot(true);
    // press_to_cont();

    // printf("=== Test6: Allocate a third variable ===\n");
    // info("Attempting to assign value3 = 15\n");
    // char *value3 = sf_malloc(sizeof(char));
    // null_check(value3, sizeof(char));
    // payload_check(value3);
    // sf_varprint(value3);
    
    
    // // Free a variable
    // printf("=== Test6: Free a block and snapshot ===\n");
    // info("Freeing value1...\n");
    // sf_free(value1);
    // sf_snapshot(true);
    // press_to_cont();

    // // printf("=== Test7: Free a block and snapshot ===\n");
    // // info("Freeing value2...\n");
    // // sf_free(value2);
    // // sf_snapshot(true);
    // // press_to_cont();

    // printf("=== Test7: Free a block and snapshot ===\n");
    // info("Freeing value3...\n");
    // sf_free(value3);
    // sf_snapshot(true);
    
    // Allocate more memory
    // printf("=== Test7: 8192 byte allocation ===\n");
    // void *memory = sf_malloc(8192);
    // sf_varprint(memory);
    // sf_free(memory);
    // press_to_cont();

    // sf_mem_fini();

    

    return EXIT_SUCCESS;
}
