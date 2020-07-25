#include "scene.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "body.h"
#include "color.h"
#include "list.h"

const size_t initial_body_count = 40;

// Wraps functionality of calling a force on a scene to 
// one generalizable format
struct force_wrapper {
    force_creator_t forcer;
    free_func_t freer;
    void *aux;
    list_t *bodies;
};

// Frees a force wrapper with all of its parameters
void force_free(force_wrapper_t *force) {
    if (force->freer != NULL){
        force->freer(force->aux);
    }
    free(force);
}

// A scene will have a collection of bodies and all applicable forces
struct scene {
    body_list_t *bodies;
    force_list_t *forces;
};

scene_t *scene_init(void){
    scene_t *res = malloc(sizeof(scene_t));
    res->bodies = list_init(initial_body_count, (free_func_t) body_free);
    res->forces = list_init(initial_body_count * initial_body_count, (free_func_t) force_free);
    return res;
}

void scene_free(scene_t *scene){
    list_free(scene->bodies); 

    list_free(scene->forces);

    free(scene);

}

size_t scene_bodies(scene_t *scene){
    return list_size(scene->bodies);
}

body_t *scene_get_body(scene_t *scene, size_t index){
    body_t *item = list_get(scene->bodies, index);
    return item;
}

void scene_add_body(scene_t *scene, body_t *body){
    list_add(scene->bodies, body);
}

void scene_remove_body(scene_t *scene, size_t index){
    body_remove(scene_get_body(scene, index)); 
}


void scene_add_force_creator(
        scene_t *scene, 
        force_creator_t forcer, 
        void *aux,
        free_func_t freer) {
            scene_add_bodies_force_creator(scene, forcer, aux, NULL, freer);
}


void scene_add_bodies_force_creator(
    scene_t *scene,
    force_creator_t forcer,
    void *aux,
    list_t *bodies,
    free_func_t freer) {
        force_wrapper_t *force = malloc(sizeof(force_wrapper_t));
        force->forcer = forcer;
        force->aux = aux;
        force->freer = freer;
        force->bodies = bodies;
        list_add(scene->forces, force);
}

void scene_remove_forces(scene_t *scene){
    if (scene->forces != NULL){
        for (int i = list_size(scene->forces) - 1; i >= 0; i--){ 
            force_wrapper_t *cur_wrap = list_get(scene->forces, i);
            // printf("reached here\n"); 
            list_t *list_of_bodies = cur_wrap->bodies; 
            // printf("reached here though\n"); 
            if (list_of_bodies != NULL){
                // printf("reached here\n"); 
                for (int j = 0; j < list_size(list_of_bodies); j++){
                    body_t *check = list_get(list_of_bodies, j); 
                    if (body_is_removed(check)){
                        force_free(list_remove(scene->forces, i));
                        break; 
                    }
                }
            }
        }
    }
}

void scene_tick(scene_t *scene, double dt){

    // Apply all forces added to the scene
    for (int  i = 0; i < list_size(scene->forces); i++) {
        force_wrapper_t *force = list_get(scene->forces, i);
        force->forcer(force->aux);
    }

    // Tell all the bodies to explicitly move
    for (int i = 0; i < scene_bodies(scene); i++) {
        body_t *b = scene_get_body(scene, i);
        body_tick(b, dt);
        //vector_t center = body_get_centroid(b); 
        // printf("xc: %f, yc: %f\n", center.x, center.y); 
    }

    scene_remove_forces(scene); 
    
    for (int i = scene_bodies(scene) - 1; i >= 0; i--) {
        body_t *cur = scene_get_body(scene, i); 
        if (body_is_removed(cur)){
            //scene_remove_body(scene, i); 
            body_t *r = list_remove(scene->bodies, i);
            body_free(r);
        }
    }
}