#include "forces.h"
#include <math.h>
#include <stdlib.h>
#include "body.h"
#include "scene.h"
#include "collision.h"
#include <stdio.h> 
#include <stdbool.h>
#include "list.h"
const double THRESHOLD = 50;
const double THRESHOLD_S = 1e-7;  

// Info required to define 
struct gravity_info {
    double G;
    body_t *b1;
    body_t *b2;
};

// Forcer function to apply the gravity at each tick
void apply_gravity(void *aux) {

    gravity_info_t *gravity_aux = aux;

    // Apply formula and add force to each body
    vector_t center1 = body_get_centroid(gravity_aux->b1);
    vector_t center2 = body_get_centroid(gravity_aux->b2);
    double distance = vec_distance(center2, center1); 

    // if (distance < THRESHOLD){
    //     return; 
    // }

    double m1 = body_get_mass(gravity_aux->b1);  
    double m2 = body_get_mass(gravity_aux->b2);
    // printf("dist: %f, m1: %f, m2: %f\n", distance, m1, m2); 
    vector_t unit = vec_multiply(1 / distance, vec_subtract(center2, center1));  
    vector_t force = vec_multiply(gravity_aux->G * m1 * m2 / 
                pow(distance, 2), unit); 
    force.y = 0; 
    // vec_print(force, "f:"); 
    body_add_force(gravity_aux->b1, force);
    // body_add_force(gravity_aux->b2, vec_negate(force));
}

// Register the gravity function with the scene
void create_newtonian_gravity(
        scene_t *scene, 
        double G, 
        body_t *body1, 
        body_t *body2) {
    
    // Build the aux info
    gravity_info_t *gravity_aux = malloc(sizeof(gravity_info_t));
    gravity_aux->G = G;
    gravity_aux->b1 = body1;
    gravity_aux->b2 = body2;

    list_t *bodies = list_init(2, free);
    list_add(bodies, body1); 
    list_add(bodies, body2);  

    // Register force with scene
    scene_add_bodies_force_creator(
    scene,
    apply_gravity,
    gravity_aux,
    bodies,
    free); 

}

 struct spring_info {
    double K;
    body_t *b1;
    body_t *b2;
};

void apply_spring(void *aux) {

    spring_info_t *spring_aux = aux;

    // Apply formula and add force to each body

    vector_t center1 = body_get_centroid(spring_aux->b1);
    vector_t center2 = body_get_centroid(spring_aux->b2);
    

    double distance = vec_distance(center2, center1); 
    // printf("distance: %f\n", distance); 
    // if (distance < THRESHOLD_S){
    //     printf("this is true\n"); 
    //     return; 
    // }
    vector_t unit = vec_multiply(1 / distance, vec_subtract(center2, center1));
    vector_t force = vec_multiply(spring_aux->K * distance, unit);
    
    body_add_force(spring_aux->b1, force);
}

void create_spring(scene_t *scene, double k, body_t *body1, body_t *body2) {
    // Build the aux info
    spring_info_t *spring_aux = malloc(sizeof(spring_info_t));
    spring_aux->K = k;
    spring_aux->b1 = body1;
    spring_aux->b2 = body2;

    // Register force with scene
    list_t *bodies = list_init(2, free);
    list_add(bodies, body1); 
    list_add(bodies, body2);  

    // Register force with scene
    scene_add_bodies_force_creator(
    scene, 
    apply_spring,
    spring_aux,
    bodies,
    free); 
}

struct drag_info {
    double gamma; 
    body_t *b1; 
}; 

void apply_drag(void *aux){
    drag_info_t *drag_aux = aux; 

    vector_t velocity = body_get_velocity(drag_aux->b1); 
    vector_t force = vec_negate(vec_multiply(drag_aux->gamma, velocity)); 
    body_add_force(drag_aux->b1, force); 
}

void create_drag(scene_t * scene, double gamma, body_t *body) {
    drag_info_t *drag_aux = malloc(sizeof(drag_info_t)); 
    drag_aux->gamma = gamma; 
    drag_aux->b1 = body; 
    
    list_t *bodies = list_init(1, free);
    list_add(bodies, body); 

    // Register force with scene
    scene_add_bodies_force_creator(
    scene,
    apply_drag,
    drag_aux,
    bodies,
    free);  
}
struct collision_force_info {
    collision_handler_t handler;
    void *handler_aux;
    body_t *b1;
    body_t *b2;
    free_func_t freer;
    bool prev_collided;
};

void apply_collision(void *aux) {
    collision_force_info_t *collision_aux = aux;
    
    list_t *shape1 = body_get_shape(collision_aux->b1);
    list_t *shape2 = body_get_shape(collision_aux->b2);
    
    collision_info_t collision = find_collision(shape1, shape2);
    if (collision.collided) {
        if(collision_aux->prev_collided == false) {
            collision_aux->handler(collision_aux->b1, collision_aux->b2, collision.axis, collision_aux->handler_aux); 
            collision_aux->prev_collided = true;
        }
    } else {
        collision_aux->prev_collided = false;
    }
}

void create_collision(
    scene_t *scene,
    body_t *body1,
    body_t *body2,
    collision_handler_t handler,
    void *aux,
    free_func_t freer
) {
    collision_force_info_t *collision_aux = malloc(sizeof(collision_force_info_t));
    collision_aux->b1 = body1;
    collision_aux-> b2 = body2;
    collision_aux->handler = handler;
    collision_aux->handler_aux = aux;
    collision_aux->freer = freer;
    collision_aux->prev_collided = false;

    list_t *bodies = list_init(2, (free_func_t) body_free);
    list_add(bodies, body1); 
    list_add(bodies, body2); 
    scene_add_bodies_force_creator(
        scene,
        apply_collision,
        collision_aux,
        bodies,
        free
    );
}

struct destructive_collision_info {
    body_t *b1; 
    body_t *b2; 
};

void apply_destructive_collision(void *aux) {
    destructive_collision_info_t *destructive_collision_aux = aux;
    
    list_t *shape1 = body_get_shape(destructive_collision_aux->b1);
    list_t *shape2 = body_get_shape(destructive_collision_aux->b2);
    
    collision_info_t collision = find_collision(shape1, shape2);
    if (collision.collided) {
        body_remove(destructive_collision_aux->b1);
        body_remove(destructive_collision_aux->b2); 
    }

}

void create_destructive_collision(scene_t *scene, body_t *body1, body_t *body2) {
    destructive_collision_info_t *destructive_collision_aux = malloc(sizeof(destructive_collision_info_t));
    destructive_collision_aux->b1 = body1;
    destructive_collision_aux->b2 = body2;

    list_t *bodies = list_init(2, (free_func_t) body_free);
    list_add(bodies, body1); 
    list_add(bodies, body2); 
    scene_add_bodies_force_creator(
        scene,
        apply_destructive_collision,
        destructive_collision_aux,
        bodies,
        free
    );
}

struct physics_collision_info {
    body_t *b1;
    body_t *b2;
    double elasticity;
    bool prev_collided;
};

void apply_physics_collision(void *aux) {
    physics_collision_info_t *physics_collision_aux = aux;

    body_t *b1 = physics_collision_aux->b1;
    body_t *b2 = physics_collision_aux->b2;
    list_t *shape1 = body_get_shape(b1);
    list_t *shape2 = body_get_shape(b2);
    
    collision_info_t collision = find_collision(shape1, shape2);
    if (collision.collided) {
        //printf("axis: %f, %f\n", collision.axis.x, collision.axis.y);
        if (physics_collision_aux->prev_collided == false) {
            double reduced_mass;
            double mass1 = body_get_mass(b1);
            double mass2 = body_get_mass(b2);
            if (mass1 == INFINITY) {
                reduced_mass = mass2;
            } else if (mass2 == INFINITY) {
                reduced_mass = mass1;
            } else {
                reduced_mass = (mass1 * mass2) / (mass1 + mass2);
            }
            double impulse = reduced_mass * (1 + physics_collision_aux->elasticity);
            double u1 = vec_dot(body_get_velocity(b1), collision.axis);
            double u2 = vec_dot(body_get_velocity(b2), collision.axis);
            impulse *= (u2 - u1);
            vector_t imp_vec = vec_multiply(impulse, collision.axis);
            body_add_impulse(b1, imp_vec);
            body_add_impulse(b2, vec_negate(imp_vec));
            physics_collision_aux->prev_collided = true; 
        }
    } else {
        physics_collision_aux->prev_collided = false;
    }
}


void create_physics_collision(
    scene_t *scene,
    double elasticity,
    body_t *body1,
    body_t *body2
) {
    physics_collision_info_t *physics_collision_aux = malloc(sizeof(physics_collision_info_t));
    physics_collision_aux->b1 = body1;
    physics_collision_aux->b2 = body2;
    physics_collision_aux->elasticity = elasticity;
    physics_collision_aux->prev_collided = false;

    list_t *bodies = list_init(2, (free_func_t) body_free);
    list_add(bodies, body1); 
    list_add(bodies, body2); 
    scene_add_bodies_force_creator(
        scene,
        apply_physics_collision,
        physics_collision_aux,
        bodies,
        free
    );
}