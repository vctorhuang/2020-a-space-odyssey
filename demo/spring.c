#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "scene.h"
#include "sdl_wrapper.h"
#include "color.h"
#include "vector.h"
#include "scene.h"
#include <time.h> 
#include "forces.h"
#include "body.h"

const vector_t CANVAS_SIZE = {1000, 500};

const int RADIUS = 15;
const int CANVAS_WIDTH = 1000;
const double K = 600;
const double GAMMA_SMALL = 10;
const double GAMMA_BIG = 50; 
const double MASS = 20;
const double GREEN = 0.5;
const double BLUE = 0.5;


vector_t init_center(int circle, int num_circles, vector_t canvas) {
    double x = 0; //(double) circle * ((double) canvas.y / (double) num_circles); 
    double y = canvas.y / 2; 
    if (circle < num_circles) {
        x = RADIUS + canvas.x * (double) circle / (double) num_circles; 
    } else {
        x = RADIUS + canvas.x * (double) (circle - num_circles) / num_circles; 
        if (circle - num_circles <= num_circles / 3){
            //scaling from 0 to canvas.y / 2
            y = (double) canvas.y * (double) 3/2 * ((double)circle - num_circles) / (num_circles); 
        } else if (circle - num_circles >= 2 * num_circles / 3){
            //scaling from canvas.y / 2 to 3/2 * canvas
            y = (double) canvas.y / 2 * (double) circle / num_circles - canvas.y / 4; 
        } 
    }
    vector_t ret = {x, y}; 
    // printf("xc: %f, yc: %f, num: %u\n", ret.x, ret.y, circle); 
    return ret; 
}


rgb_color_t init_color(int circle, int num_circles) {

    rgb_color_t ret; 
    if (circle <  num_circles){
        ret.r = 1; 
        ret.g = 1; 
        ret.b = 1;     
    }else {
        ret.r = ((double)(circle - num_circles)) / num_circles; 
        ret.g = GREEN; 
        ret.b = BLUE;  
    }
    return ret; 
}


body_t *create_circle(int circle, int num_circles) {

    //initialize variables
    vector_t force = {0, 0};
    vector_t impulse = {0,0}; 
    double mass = MASS;
    
    vector_t velocity = VEC_ZERO; 
    vector_t center = init_center(circle, num_circles, CANVAS_SIZE);  
    rgb_color_t color = init_color(circle, num_circles); 
    double radius = (double) RADIUS; 
    body_t *ret_circle = body_detailed_init(
        NULL, 
        velocity,
        center,
        force,
        impulse,
        radius,
        mass, 
        color,
        NULL, NULL, NULL);
    // printf("xc: %f, yc: %f, num: %u\n", center.x, center.y, circle); 
    
    return ret_circle; 
}


int main() { 
    // Initialize the scene
    vector_t *bottom_left = vec_pointer_init(0.0, 0.0);
    vector_t *top_right = vec_pointer_init(CANVAS_SIZE.x, CANVAS_SIZE.y);

    srand(time(NULL));
    
    sdl_init(*bottom_left, *top_right);
    
    scene_t *game_objects = scene_init(); 

    int num_circles = CANVAS_WIDTH / (2 * RADIUS);

    for (int i = 0; i < 2 * num_circles; i++) {
        scene_add_body(game_objects, create_circle(i, num_circles));
    }
    
    int num_objects = (int) scene_bodies(game_objects);
    for (int i = num_circles; i < num_objects; i++){
        if (i > ((double)3)/4 * num_objects){
            create_drag(game_objects, GAMMA_BIG, scene_get_body(game_objects, i));
        } else {
            create_drag(game_objects, GAMMA_SMALL, scene_get_body(game_objects, i));
        } 
        create_spring(game_objects, K,
            scene_get_body(game_objects, i), scene_get_body(game_objects, i - num_circles));
        
        
    }

    double dt;
    while (!sdl_is_done(game_objects)) {
        dt = time_since_last_tick();
        scene_tick(game_objects, dt);
        sdl_render_scene(game_objects);
    }
}