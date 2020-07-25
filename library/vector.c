#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include "vector.h"

const vector_t VEC_ZERO = {0, 0};

vector_t* vec_pointer_init(double x, double y) {
    vector_t *v = malloc(sizeof(vector_t));
    assert(v != NULL);
    vector_t v_data = {x, y};
    *v = v_data;
    return v;
}

vector_t vec_add(vector_t v1, vector_t v2) {
    vector_t v3 = {v1.x + v2.x, v1.y + v2.y};
    return v3;
}

vector_t vec_subtract(vector_t v1, vector_t v2) {
    vector_t v3 = {v1.x - v2.x, v1.y - v2.y};
    return v3;
}

vector_t vec_negate(vector_t v) {
    vector_t v_new = {-v.x, -v.y};
    return v_new;
}

vector_t vec_multiply(double scalar, vector_t v) {
    vector_t v_new = {scalar * v.x, scalar * v.y};
    return v_new;
}

vector_t vec_multiply_vec(vector_t v1, vector_t v2) {
    vector_t v3 = {v1.x * v2.x, v1.y * v2.y};
    return v3;
}

double vec_dot(vector_t v1, vector_t v2) {
    return v1.x * v2.x + v1.y * v2.y;
}

double vec_cross(vector_t v1, vector_t v2) {
    return v1.x * v2.y - v1.y * v2.x;
}

vector_t vec_rotate(vector_t v, double angle) {
    vector_t v_new = {v.x * cos(angle) - v.y * sin(angle), v.x * sin(angle) + v.y * cos(angle)};
    return v_new;
}

double vec_distance(vector_t v1, vector_t v2){
    return sqrt(pow((v1.x - v2.x), 2) + pow((v1.y - v2.y), 2)); 
}
vector_t vec_avg(vector_t v1, vector_t v2){
    return (vector_t) {(v1.x + v2.x) / 2, (v1.y + v2.y) / 2}; 
}
vector_t vec_print(vector_t v1, char pre[]){
    printf("vx%s: %f, vy%s: %f\n", pre, v1.x, pre, v1.y); 
} 

