#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include "polygon.h"
#include "list.h"

const int MIN_NUM_SIDES_POLYGON = 3;

const double two = 2.0;
const double six = 6.0;

double polygon_area(vec_list_t *polygon) {
    assert(list_size(polygon) >= MIN_NUM_SIDES_POLYGON);
    double area = 0.0;
    for(size_t i = 0; i < list_size(polygon); i++) {
        vector_t *p1 = list_get(polygon, i);
        vector_t *p2 = list_get(polygon, (i+1)%list_size(polygon));

        // https://en.wikipedia.org/wiki/Shoelace_formula#Statement
        area += vec_cross(*p1, *p2);
    }
    return fabs(area) / two;
}

vector_t polygon_centroid(vec_list_t *polygon) {
    assert(list_size(polygon) >= MIN_NUM_SIDES_POLYGON);
    vector_t centroid = {0.0, 0.0};
    for (size_t i = 0; i < list_size(polygon); i++) {
        // Last vertex wraps around to first vertex
        size_t i1 = (i + 1) % list_size(polygon);
        vector_t *v1 = list_get(polygon, i);
        vector_t *v2 = list_get(polygon, i1);

        // https://en.wikipedia.org/wiki/Centroid#Of_a_polygon
        centroid = vec_add(centroid, 
            vec_multiply(vec_cross(*v1, *v2), vec_add(*v1, *v2)));
    }
    centroid = vec_multiply(1.0 / (six * polygon_area(polygon)), 
            centroid);    
    return centroid;
}

void polygon_translate(vec_list_t *polygon, vector_t translation) {
    assert(list_size(polygon) >= MIN_NUM_SIDES_POLYGON);
    for (size_t i = 0; i < list_size(polygon); i++) {
        vector_t *v = list_get(polygon, i);
        *v = vec_add(translation, *v);
    }
}


void polygon_rotate(vec_list_t *polygon, double angle, vector_t point) {
    assert(list_size(polygon) >= MIN_NUM_SIDES_POLYGON);
    for(size_t i = 0; i < list_size(polygon); i++) {
        // Rotates as if one of the points is the origin and then translates 
        // the point back
        vector_t *v = list_get(polygon, i);
        vector_t diff = vec_subtract(*v, point);
        vector_t rot = vec_add(point, vec_rotate(diff, angle));
        *v = rot;
    }
}
