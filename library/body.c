#include <stdlib.h>
#include <assert.h>
#include "polygon.h"
#include "body.h"
#include "color.h"

void body_info_free(void * aux){
    free(aux); 
}

struct body {
    vec_list_t *shape;
    vector_t velocity;
    vector_t center;
    vector_t force; 
    vector_t impulse; 
    double rotation_speed; 
    double radius;
    double mass;
    rgb_color_t color;
    bool removed; 
    free_func_t freer; 
    void *aux;
};


body_t *body_init(list_t *shape, double mass, rgb_color_t color) {
    body_t *body = malloc(sizeof(body_t));
    assert(body != NULL);
    vector_t v = {0.0, 0.0};
    vector_t c = {0.0, 0.0};
    vector_t f = {0.0, 0.0}; 
    vector_t im = {0.0, 0.0}; 
    body_t body_data = {shape, v, c, f, im, 0, 0, mass, color, false, NULL, NULL};
    *body = body_data;
    return body;
}

body_t *body_detailed_init(
        list_t *shape,
        vector_t velocity,
        vector_t center,
        vector_t force, 
        vector_t impulse,
        double rot_speed,
        double radius,
        double mass,
        rgb_color_t color,
        bool remove,
        free_func_t freer, 
        void *aux)
{
    // vec_print(center, "c"); 
    body_t *body = malloc(sizeof(body_t));
    assert(body != NULL);
    body_t body_data = {shape, velocity, center, force, impulse, rot_speed, radius, mass, color, NULL, NULL, aux};
    *body = body_data;
    body->removed = false; 
    body->freer = freer;  
    return body;
}

body_t *body_init_with_info(
    list_t *shape,
    double mass,
    rgb_color_t color,
    void *info,
    free_func_t info_freer
){
    return body_detailed_init(shape, VEC_ZERO, VEC_ZERO, VEC_ZERO, 
    VEC_ZERO, 0, 0, mass, color, false, info_freer, info); 
}

void body_free(body_t *body) {
    if (body->shape != NULL) {
        list_free(body->shape);
    }
    if (body->freer != NULL){
        body->freer(body->aux); 
    }
    free(body);
}


list_t *body_get_shape(body_t *body) {
    return body->shape;
}

double body_get_mass(body_t *body) {
    return body->mass;
}

vector_t body_get_centroid(body_t *body) {
    return body->center;
}

vector_t body_get_velocity(body_t *body) {
    return body->velocity;
}

double body_get_rotation_speed(body_t *body){
    return body->rotation_speed; 
}
void body_set_rotation_speed(body_t *body, double speed){
    body->rotation_speed = speed; 
}
vector_t body_get_force(body_t *body) {
    return body->force;
}

double body_get_radius(body_t *body) {
    return body->radius;
}

rgb_color_t body_get_color(body_t *body) {
    return body->color;
}

void body_set_centroid(body_t *body, vector_t x) {
    if (body->shape != NULL) {
        polygon_translate(body_get_shape(body), vec_add(x, vec_negate(body->center)));
    }
    body->center = x;
}

void body_set_velocity(body_t *body, vector_t v) {
    body->velocity = v;
}

void body_set_radius(body_t *body, double radius) {
    body->radius = radius; 
}

void body_set_shape(body_t *body, vec_list_t *shape) {
    body->shape = shape;
}

void body_set_rotation(body_t *body, double angle) {
    polygon_rotate((vec_list_t *)body->shape, angle, body->center);
}

void body_set_color(body_t *body, rgb_color_t color) {
    body->color = color;
}

void body_tick(body_t *body, double dt) {
    double angle = body->rotation_speed * dt; 
    body_set_rotation(body, angle); 
    
    vector_t dVel1 = vec_multiply(dt / body->mass, body->force); 
    
    vector_t dVel2 = vec_multiply(1 / body->mass, body->impulse); 
    // printf("Ix: %f, Iy: %f\n", dVel2.x, dVel2.y); 
    vector_t dVel = vec_add(dVel1, dVel2); 
    // printf("vx: %f, vy: %f\n", body->velocity.x, body->velocity.y); 
    vector_t avg_vel = vec_avg(body->velocity, vec_add(body->velocity, dVel)); 
    body_set_velocity(body, vec_add(body->velocity, dVel)); 
    vector_t dPos = vec_multiply(dt, avg_vel);
    body_set_centroid(body, vec_add(body->center, dPos));
    
    //sets force back to 0 after tick
    body_add_force(body, vec_negate(body->force)); 
    body_add_impulse(body, vec_negate(body->impulse)); 
}

void body_add_force(body_t *body, vector_t force){
    body->force = vec_add(body->force, force); 
}


void body_add_impulse(body_t *body, vector_t impulse){
    body->impulse = vec_add(body->impulse, impulse);    
}


// int body_is_enemy(body_t *body) {
//     assert(body->aux != NULL);
//     int ret = ((body_info_t*) body->aux)->character; 
//     return ret;
// }

void body_remove(body_t *body) {
    body->removed = true; 
}

bool body_is_removed(body_t *body) {
   return body->removed; 
}
void *body_get_info(body_t *body){
    return body->aux; 
}

void body_resize(body_t *body, double c){
    list_t *list = body_get_shape(body); 
    vector_t center = body_get_centroid(body); 

    for (int i = 0; i < list_size(list); i++){
        vector_t *cur = (vector_t*)list_get(list, i); 
        vector_t trans = vec_subtract(*cur, center); 
        trans = vec_multiply(c, trans); 
        *cur = vec_add(trans, center); 
    }
}