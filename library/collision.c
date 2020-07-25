#include "collision.h"
#include "vector.h"
#include "list.h"
#include <math.h>
#include <float.h> 
#include <assert.h>
#include "body.h"
#include "color.h"
#include "scene.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct collision_data{
    vector_t axis; 
    double overlap; 
}collision_data_t;


// This function takes in a list of vectors that respresent a polygon, and returns
// a vector perpendicular to a side of the shape. This vector is centered at 0,0 and is unit
vector_t find_axis(list_t *shape, int num_side){ 
    vector_t s1; 
    if (num_side == 0){
        s1 = *(vector_t*)list_get(shape, list_size(shape) - 1); 
    } else {
        s1 = *(vector_t*)list_get(shape, num_side - 1); 
    }
    
    vector_t s2 = *(vector_t*)list_get(shape, num_side); 

    // This gives a vector perpendicular to the side
    // printf("s1_x: %f, s1_y: %f\n", s1.x, s1.y);
    // printf("s2_x: %f, s2_y: %f\n", s2.x, s2.y);   
    vector_t side = vec_subtract(s1, s2);
    // vector_t trans = vec_negate(s1); 
    // side = vec_add(side, trans); 
    // printf("cent_x: %f, cent_y: %f\n", side.x, side.y);
    side = vec_rotate(side, M_PI / 2); 
    
    double dist = vec_distance(side, VEC_ZERO); 
    side = vec_multiply(1 / dist, side); 
    if (side.x < 0){
        side.x = side.x * -1; 
    }
    if (side.y < 0){
    }
    // printf("perp_x: %f, perp_y: %f\n", side.x, side.y);
    return side; 
}

// this function always gets the axis from the side # of the first shape input
// returns true if there is no collision on axis and false otherwise 
collision_data_t find_collision_on_axis(list_t *shape1, list_t *shape2, int num_side){
    double min1 = DBL_MAX; 
    double max1 = -DBL_MAX; 
    double min2 = DBL_MAX; 
    double max2 = -DBL_MAX;

    collision_data_t ret = {VEC_ZERO, 0};  
    vector_t axis = find_axis(shape1, num_side); 
    // printf("num_side: %u, axisx: %f, axisy: %f\n", num_side, axis.x, axis.y); 
    double cur = 0;
    // project sides and find min and max vectors
    for (size_t i = 0; i < list_size(shape1); i++){
        //printf("shape 1: vector x: %f, y: %f\n", ((vector_t *)list_get(shape1, i))->x, ((vector_t *)list_get(shape1, i))->y);
        cur = vec_dot(*(vector_t*)list_get(shape1, i), axis); 
        //printf("shape 1: num_side: %u, point: %u, cur: %f\n", num_side, i, cur); 
        if (cur < min1){
            min1 = cur; 
        }

        if (cur > max1){
            max1 = cur; 
        }
    }

    for (size_t i = 0; i < list_size(shape2); i++){
        //printf("shape 2: vector x: %f, y: %f\n", ((vector_t *)list_get(shape2, i))->x, ((vector_t *)list_get(shape2, i))->y);
        cur = vec_dot(*(vector_t*)list_get(shape2, i), axis); 
        //printf("shape 2: num_side: %u, point: %u, cur: %f\n", num_side, i, cur);
        if (cur < min2){
            min2 = cur; 
        } 

        if (cur > max2){
            max2 = cur; 
        }
    }
    // printf("num_side: %u, min1: %f, max1: %f, min2: %f, max2: %f\n", num_side, min1, max1, min2, max2); 
    if ((max1 > min2  && max1 < max2) || (max2 > min1 && max2 < max1)) {
        ret.axis = axis; 
        
        if (max1 > min2  && max1 < max2){
            ret.overlap = max1 - min2; 
        } else {
            ret.overlap = max2 - min1;
        }
    }

    return ret; 
}

// This function returns true if the shapes are colliding and false if they are not colliding. 
collision_info_t find_collision(list_t *shape1, list_t *shape2){
    // printf("vector 1 shape 1: x: %f, y: %f\n", ((vector_t *)list_get(shape1, 1))->x, ((vector_t *)list_get(shape1, 1))->y);
    //printf("I'm here\n"); 
    collision_info_t info = {true, VEC_ZERO}; 
    collision_data_t check = {VEC_ZERO, 0};  
    collision_data_t cur; 
    for (int i = 0; i < list_size(shape1); i++){
        // printf("i: %u", i); 
        cur = find_collision_on_axis(shape1, shape2, i); 

        if (cur.overlap > 0){
            if (check.overlap == 0 || cur.overlap < check.overlap){
                // printf("reached here\n");
                check.overlap = cur.overlap; 
                info.axis = cur.axis; 
            }
        } else {
            info.collided = false; 
            break;
        }
    }

    for (int i = 0; i < list_size(shape2); i++){
        cur = find_collision_on_axis(shape2, shape1, i); 
        if (cur.overlap > 0){
            if (check.overlap == 0 || cur.overlap < check.overlap){
                check.overlap = cur.overlap; 
                info.axis = cur.axis; 
            }
        } else {
            info.collided = false; 
            break;
        }
    }
    
    return info; 
} 
