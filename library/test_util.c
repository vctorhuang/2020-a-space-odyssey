#include "test_util.h"

#include <assert.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define DEFAULT_EPSILON 1e-7

bool isclose(double d1, double d2) {
    return within(DEFAULT_EPSILON, d1, d2);
}

bool vec_equal(vector_t v1, vector_t v2) {
    return v1.x == v2.x && v1.y == v2.y;
}

bool vec_isclose(vector_t v1, vector_t v2) {
    return vec_within(DEFAULT_EPSILON, v1, v2);
}

bool within(double epsilon, double d1, double d2) {
    return fabs(d1 - d2) < epsilon;
}

bool vec_within(double epsilon, vector_t v1, vector_t v2) {
    return within(epsilon, v1.x, v2.x) && within(epsilon, v1.y, v2.y);
}

void read_testname(char *filename, char *testname, size_t testname_size) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        printf("Couldn't open file %s\n", filename);
        exit(1);
    }
    // Generate format string
    char fmt[12];
    snprintf(fmt, sizeof(fmt), "%%%lus", testname_size - 1);
    fscanf(f, fmt, testname);
    fclose(f);
}

bool test_assert_fail(void (*run)(void *aux), void *aux) {
    if (fork()) { // parent process
        int *status = malloc(sizeof(*status));
        assert(status != NULL);
        wait(status);
        // Check whether child process aborted
        bool aborted = WIFSIGNALED(*status) && WTERMSIG(*status) == SIGABRT;
        free(status);
        return aborted;
    } else {                               // child process
        freopen("/dev/null", "w", stderr); // suppress assertion message
        run(aux);
        exit(0); // should not be reached
    }
}
