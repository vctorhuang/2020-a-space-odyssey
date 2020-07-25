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


// Canvas constant properties
const vector_t CANVAS_SIZE = {1000, 500};

// Pacman and pellet constant properties
const double MIN_RADIUS = 15;
const double MAX_RADIUS = 40; 
const double MASS_RATIO = 40; 
const double MAX_INIT_VELOCITY = 150; 
const double NUM_STARS = 80; 
const double NUM_SIDE_STAR = 4; 
const double PERCENT_SCREEN = .8; 
const double GRAVITY = 100; 


vec_list_t *build_star_list(
    vector_t initial_pos,
    double radius)
{
    vec_list_t *star = simple_list_init(2 * NUM_SIDE_STAR);
    double inner_radius = radius * 2 / (3 + sqrt(NUM_SIDE_STAR));

    // Calculate outer and innter points of the star
    vector_t top_outer_point = {0.0, radius};
    vector_t top_inner_point = {0.0, inner_radius};
    double angle;
    double angle_offset = 2 * M_PI / NUM_SIDE_STAR / 2;
    for (int i = 0; i < NUM_SIDE_STAR; i++)
    {
        angle = i * 2 * M_PI / NUM_SIDE_STAR;
        vector_t *p_outer = vec_pointer_init(0.0, 0.0);
        vector_t *p_inner = vec_pointer_init(0.0, 0.0);
        *p_outer = vec_add(initial_pos, vec_rotate(top_outer_point, angle));
        *p_inner = vec_add(initial_pos, vec_rotate(top_inner_point, angle + angle_offset));
        list_add(star, p_outer);
        list_add(star, p_inner);
    }
    return star;
}

rgb_color_t random_color(){

    double r = (double) rand() / RAND_MAX;  
    double g = (double) rand() / RAND_MAX;  
    double b = (double) rand() / RAND_MAX;  
    rgb_color_t color = {r, g, b}; 
    return color; 
}

double random_radius(double lo, double hi){
    double ret = (double) rand() / RAND_MAX * (hi - lo) + lo; 
    return ret; 
}

//mass is proportional to radius squared and the mass ratio constant 
double generate_mass(double radius){
    return pow(radius, 2) * MASS_RATIO; 
}

//initialize random velocity 
vector_t init_velocity(double max_velocity){
    double x = (double) rand() / RAND_MAX * max_velocity - max_velocity/2; 
    double y = (double) rand() / RAND_MAX * max_velocity - max_velocity/2; 
    vector_t ret = {x, y}; 
    return ret; 
}

vector_t rand_center(vector_t canvas, double scale_down){
    double move_right = canvas.x * (1 - scale_down) / 2; 
    double move_up = canvas.y * (1 - scale_down) / 2; 
    canvas.x *= scale_down; 
    canvas.y += scale_down; 
    double x = (double) rand() / RAND_MAX * canvas.x + move_right; 
    double y = (double) rand() / RAND_MAX * canvas.y + move_up; 
    vector_t ret = {x, y}; 
    return ret;
}
// Creates a body that represents a random star
body_t *create_star() {

    //initialize random variables
    double radius = random_radius(MIN_RADIUS, MAX_RADIUS);
    vector_t force = {0, 0};
    vector_t impulse = {0,0}; 
    double mass = generate_mass(radius);
    
    vector_t velocity = init_velocity(MAX_INIT_VELOCITY); 
    vector_t center = rand_center(CANVAS_SIZE, PERCENT_SCREEN); 
    rgb_color_t color = random_color(); 
    vec_list_t *star_list = build_star_list(center, radius); 
    body_t *star = body_detailed_init(
        star_list, 
        velocity,
        center,
        force,
        impulse,
        0,
        mass, 
        color,
        NULL, NULL, NULL);

    return star; 
}

int main() { 
    // Initialize the scene
    vector_t *bottom_left = vec_pointer_init(0.0, 0.0);
    vector_t *top_right = vec_pointer_init(CANVAS_SIZE.x, CANVAS_SIZE.y);

    srand(time(NULL));
    
    sdl_init(*bottom_left, *top_right);
    
    scene_t *game_objects = scene_init(); 

    for (size_t i = 0; i < NUM_STARS; i++) {
        scene_add_body(game_objects, create_star());
    }

    for (size_t i = 0; i < scene_bodies(game_objects); i++){
        for (size_t j = 0; j < scene_bodies(game_objects); j++){
            create_newtonian_gravity(game_objects, GRAVITY,
            scene_get_body(game_objects, i), scene_get_body(game_objects, j)); 
        }
    }

    double dt;
    while (!sdl_is_done(game_objects)) {
        dt = time_since_last_tick();
        scene_tick(game_objects, dt);
        sdl_render_scene(game_objects);
    }
}