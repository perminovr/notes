#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define MEMINFO "/proc/meminfo"


void getmeminfo(size_t *avail, size_t *total)
{
    int res;
    char *line;
    size_t buf_sz = 0;
    ssize_t size;
    FILE *f = fopen(MEMINFO, "r");

    // total
    size = getline(&line, &buf_sz, f);
    if (size) {
        res = sscanf(line, "%*s%zu", total);
        // free
        if (res) {
            size = getline(&line, &buf_sz, f);
            if (size) {
                // avail
                size = getline(&line, &buf_sz, f);
                if (size) {
                    res = sscanf(line, "%*s%zu", avail);
                }
            }
        }
    }

    *avail *= 1024;
    *total *= 1024;

    fclose(f);
}



#define TEST_MALLOC
// #define TEST_REALLOC

#define TEST_MULT 1.03
// #define TEST_SUMM 128

#define POINTERS 2000
#define MCL_BASE_SIZE 5



int main(int argc, char **argv)
{
    int cnt = 0;
    void *newmem[POINTERS];
    float memk;
    size_t avail, total, need;
    size_t mlc_size, prevsize;

    mlc_size = MCL_BASE_SIZE;

    while (1) {
        getmeminfo(&avail, &total);
        need = (total-avail) + mlc_size*POINTERS;
        memk = (float)(need) / (float)(total);
        if (memk < 0.49) {
            for (int i = 0; i < POINTERS; ++i) {
                #ifdef TEST_REALLOC
                    newmem[i] = realloc(newmem[i], mlc_size+i);
                    memset(newmem[i], 1, mlc_size);
                #endif
                #ifdef TEST_MALLOC
                    newmem[i] = malloc(mlc_size+i);
                    memset(newmem[i], (uint8_t)i, mlc_size);
                #endif
            }
            for (int i = 0; i < POINTERS; ++i) {
                #ifdef TEST_MALLOC
                    free(newmem[i]);
                    newmem[i] = 0;
                #endif
            }
        } else {
            for (int i = 0; i < POINTERS; ++i) {
                if (newmem[i])
                    free(newmem[i]);
                newmem[i] = 0;
            }
            time_t t = time(NULL);
            struct tm tm = *localtime(&t);
            printf("[%02d-%02d %02d:%02d:%02d] check point %d: maxsize = %zu, memk = %f\n", 
                tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, 
                cnt++, prevsize, memk);
            mlc_size = MCL_BASE_SIZE+7;
            if (mlc_size > 100*1024) {
                printf("end\n");
                exit(0);
            }
        }
        prevsize = mlc_size;
        #ifdef TEST_MULT
            double mult = TEST_MULT;
            double mcl_size_f = mlc_size * mult;
            mlc_size = (size_t)mcl_size_f;
            if (prevsize == mlc_size)
                mlc_size++;
        #endif
        #ifdef TEST_SUMM
            mlc_size+=TEST_SUMM;
        #endif
    }


    return 0;
}



#if defined(TEST_REALLOC) && defined(TEST_MALLOC)
#error "TEST_REALLOC && TEST_MALLOC"
#endif
#if defined(TEST_MULT) && defined(TEST_SUMM)
#error "TEST_MULT && TEST_SUMM"
#endif
