#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "scene.h"
#include "sdl_wrapper.h"
#include "color.h"
#include "vector.h"
#include "scene.h"
#include "body.h"

// Canvas constant properties
const vector_t CANVAS_SIZE = {1000, 500};

// Pacman and pellet constant properties
const rgb_color_t YELLOW = {1.0, 1.0, 0.0}; 
const double PACMAN_MASS = 25; 
const double PACMAN_RADIUS = 25;
const double PACMAN_MOUTH_ANGLE = 85*M_PI/180;
const double PELLET_MASS = 3;
const int PELLET_RADIUS = 5;
const int NUM_PELLETS = 10;
const double PACMAN_BASE_SPEED = 1000; 
const double ARC_RADIUS_MULTIPLIER = 1.05;
const double ACCELERATION = 4; 
const int RESPAWN_INTERVAL = 50;
const double eat_threshold = 0.75;

void check_edges(body_t *body) {
    if (body_get_centroid(body).x > CANVAS_SIZE.x){
        vector_t temp = {0, body_get_centroid(body).y};
        body_set_centroid(body, temp);
    } else if (body_get_centroid(body).x < 0){
        vector_t temp = {CANVAS_SIZE.x, body_get_centroid(body).y};
        body_set_centroid(body, temp);
    }
    if (body_get_centroid(body).y > CANVAS_SIZE.y){
        vector_t temp = {body_get_centroid(body).x, 0};
        body_set_centroid(body, temp);
    } else if (body_get_centroid(body).y < 0){
        vector_t temp = {body_get_centroid(body).x, CANVAS_SIZE.y};
        body_set_centroid(body, temp);
    }
}


void button_pressed(body_t *body, int flag, double heldtime){
    // flag 0 = no button, flag = 1 means right, flag = 2 means up, flag = 3 means left, flag = 4 means down
    vector_t temp = body_get_velocity(body); 
    double angle = 0;
    if ((flag == 1 && temp.y > 0) || (flag == 2 && temp.x < 0)
                || (flag == 3 && temp.y < 0) || (flag == 4 && temp.x > 0)) {
        angle = M_PI / -2;
    } else if ((flag == 1 && temp.x < 0) || (flag == 3 && temp.x > 0)
                || (flag == 2 && temp.y < 0) || (flag == 4 && temp.y > 0)) {
        angle = M_PI;
    } else if ((flag == 1 && temp.y < 0) || (flag == 2 && temp.x > 0)
                || (flag == 3 && temp.y > 0) || (flag == 4 && temp.x < 0)) {
        angle = M_PI / 2;
    }

    if (flag == 1) {
        temp.y = 0;
        temp.x = PACMAN_BASE_SPEED * (1 + heldtime);
    } else if (flag == 2) {
        temp.x = 0; 
        temp.y = PACMAN_BASE_SPEED * (1 + heldtime);
    } else if (flag == 3) {
        temp.x = -PACMAN_BASE_SPEED * (1 + heldtime);
        temp.y = 0; 
    } else if (flag == 4) {
        temp.x = 0; 
        temp.y = -PACMAN_BASE_SPEED * (1 + heldtime); 
    }

    body_set_rotation(body, angle);
    body_set_velocity(body, temp);
}


void on_key(char key, key_event_type_t type, double held_time, scene_t *scene) {
    body_t *pacman = scene_get_body(scene, 0); 
    if (type == KEY_PRESSED) {
        switch (key) {
            case LEFT_ARROW:
                button_pressed(pacman, 3, held_time);
                break;
            case RIGHT_ARROW:
                button_pressed(pacman, 1, held_time);
                break;
            case UP_ARROW:
                button_pressed(pacman, 2, held_time);
                break;
            case DOWN_ARROW:
                button_pressed(pacman, 4, held_time);
                break;
        }
    } else {
        button_pressed(pacman, 0, held_time);
    }
}


// Creates a list of points that correspond to an arc (pacman's mouth)
list_t *create_arc(vector_t center, double radius) {
    size_t num_points = radius * PACMAN_MOUTH_ANGLE;
    list_t *arc = simple_list_init(num_points + 1);
    vector_t *cent = vec_pointer_init(center.x, center.y);
    list_add(arc, cent);
    double angle_increment = PACMAN_MOUTH_ANGLE / num_points;

    // Calculates outer points along the arc
    double angle = PACMAN_MOUTH_ANGLE / -2;
    for (size_t i = 0; i < num_points; i++) {
        vector_t *p_rot = vec_pointer_init(radius, 0);
        *p_rot = vec_add(center, vec_rotate(*p_rot, angle));
        list_add(arc, p_rot);
        angle += angle_increment;
    }
    return arc;
}


// Creates a body that represents pacman
body_t *create_pacman(double mass, vector_t center) {
    body_t *pacman = body_init(NULL, mass, YELLOW); 
    body_set_centroid(pacman, center); 
    body_set_radius(pacman, PACMAN_RADIUS);
    list_t *pac_shape = create_arc(body_get_centroid(pacman), body_get_radius(pacman) + 1);
    body_set_shape(pacman, pac_shape);
    body_set_color(pacman, YELLOW); 
    vector_t velocity = {PACMAN_BASE_SPEED, 0}; 
    body_set_velocity(pacman, velocity); 
    return pacman; 
}


// Creates a body that represents a pellet in a random location on the canvas
body_t *spawn_random_pellet() {
    int x = (rand() % ((int)(CANVAS_SIZE.x - PELLET_RADIUS + 1))) + PELLET_RADIUS;
    int y = (rand() % ((int)(CANVAS_SIZE.y - PELLET_RADIUS + 1))) + PELLET_RADIUS;
    vector_t center = {x, y};
    body_t *ret = body_init(NULL, PELLET_MASS, YELLOW);
    body_set_radius(ret, PELLET_RADIUS); 
    body_set_color(ret, YELLOW); 
    body_set_centroid(ret, center); 
    return ret; 
}


// Checks if this pellet has been eaten by pacman
int is_eaten(body_t *pacman, body_t *pellet) {
    double distance = sqrt(pow(body_get_centroid(pellet).x - body_get_centroid(pacman).x, 2) 
            + pow(body_get_centroid(pellet).y - body_get_centroid(pacman).y, 2));
    if (distance <= eat_threshold * PACMAN_RADIUS) {
        return 1;
    }
    return 0;
}


int main() {
    // Initialize the scene
    vector_t *bottom_left = vec_pointer_init(0.0, 0.0);
    vector_t *top_right = vec_pointer_init(CANVAS_SIZE.x, CANVAS_SIZE.y);
    
    sdl_init(*bottom_left, *top_right);
    
    // Initialize pacman and 10 game_objects
    vector_t CANVAS_CENTER = {CANVAS_SIZE.x / 2, CANVAS_SIZE.y / 2};
    body_t *pacman = create_pacman(PACMAN_MASS, CANVAS_CENTER);
    
    scene_t *game_objects = scene_init(); 
    scene_add_body(game_objects, pacman); 
    for (size_t i = 0; i < NUM_PELLETS; i++) {
        scene_add_body(game_objects, spawn_random_pellet());
    }
    
    sdl_on_key(on_key);
    double dt;
    int respawn_counter = 0;
    while (!sdl_is_done(game_objects)) {
        dt = time_since_last_tick();
        check_edges(pacman); 
        scene_tick(game_objects, dt);

        //creates new game_objects after certain intervals
        if (respawn_counter >= RESPAWN_INTERVAL) {
            scene_add_body(game_objects, spawn_random_pellet());
            respawn_counter = 0;
        }

        // Pacman is the 1st object in the scene
        size_t i = 1;
        while (i < scene_bodies(game_objects)) {
            body_t *p = scene_get_body(game_objects, i);

            // Remove pellet if eaten by pacman
            if (is_eaten(pacman, p) == 1) {
                scene_remove_body(game_objects, i);
            } 
            else {
                i++;
            }
        }
        
        sdl_render_scene(game_objects);
        respawn_counter++;
    }
}