#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include "scene.h"
#include "sdl_wrapper.h"
#include "color.h"
#include "vector.h"
#include "scene.h"
#include "forces.h"
#include "collision.h"
#include "body.h"


// Canvas constant properties
const vector_t CANVAS_SIZE = {1000, 500};


// Game properties
const rgb_color_t CHARACTER_COLOR = {1.0f, 0.0f, 0.0f}; 
const double CHARACTER_MASS = INFINITY;
const vector_t CHARACTER_DIM = {75, 25};
const double CHARACTER_VELOCITY = 750.0; 
const vector_t BLOCK_DIM = {85, 25};
const int NUM_BLOCKS_PER_ROW = 10;
const int NUM_BLOCKS_PER_COL = 3;
const rgb_color_t BALL_COLOR = {0.0f, 1.0f, 0.0f}; 
const rgb_color_t WALL_COLOR = {1.0f, 1.0f, 1.0f}; 
const double BALL_RADIUS = 10;
const double BALL_MASS = 10;
const vector_t BALL_VELOCITY = {250, 250};
const double WALL_ELASTICITY = 1.0;
const double WALL_THICKNESS = 4;
const double BLOCK_ELASTICITY = 1.0;

typedef enum {
    BALL,
    BLOCK,
    WALL,
    FLOOR,
    PLAYER
} body_type_t;

body_type_t *make_type_info(body_type_t type) {
    body_type_t *info = malloc(sizeof(*info));
    *info = type;
    return info;
}

body_type_t get_type(body_t *body) {
    return *(body_type_t *) body_get_info(body);
}

rgb_color_t init_color(int circle, int num_circles) {
    rgb_color_t ret; 
    ret.r = ((double)(num_circles - circle)) / num_circles; 
    ret.g = 0.5; 
    ret.b = 0.5; 
    return ret; 
}

void on_key(char key, key_event_type_t type, double held_time, scene_t *scene) {
    body_t *character = scene_get_body(scene, 0); 
    vector_t temp = body_get_velocity(character);
    vector_t pos = body_get_centroid(character); 

    if (type == KEY_PRESSED) {
        switch (key) {
            case LEFT_ARROW:
                if (pos.x - 0.5*CHARACTER_DIM.x > 0) {
                    temp.x = CHARACTER_VELOCITY * -1; 
                    body_set_velocity(character, temp);
                } else {
                    body_set_velocity(character, VEC_ZERO);
                }
                break;
            case RIGHT_ARROW:
                if (pos.x + 0.5*CHARACTER_DIM.x < CANVAS_SIZE.x) {
                    temp.x = CHARACTER_VELOCITY; 
                    body_set_velocity(character, temp);
                } else {
                    body_set_velocity(character, VEC_ZERO);
                }
                break;
        }
    } else {
        switch (key) {
            case LEFT_ARROW:
                body_set_velocity(character, VEC_ZERO);
                break;
            case RIGHT_ARROW:
                body_set_velocity(character, VEC_ZERO);
                break;
        }
    }
}

list_t *create_ball_shape(vector_t center) {
    list_t *shape = simple_list_init(25);
    double angle_inc = 360/25 * M_PI / 180; 
    vector_t p_ref = {BALL_RADIUS, 0};

    for (int i = 0; i < 25; i++) {
        vector_t *p = vec_pointer_init(0, 0);
        *p = vec_add(center, vec_rotate(p_ref, i*angle_inc));
        list_add(shape, p);
    }

    return shape;
}

list_t *create_rectangle_shape(vector_t center, vector_t dim) { 
    list_t *shape = simple_list_init(4);

    vector_t *bottom_left = vec_pointer_init(center.x-0.5*dim.x, center.y-0.5*dim.y);
    list_add(shape, bottom_left);

    vector_t *top_left = vec_pointer_init(center.x-0.5*dim.x, center.y+0.5*dim.y);
    list_add(shape, top_left);

    vector_t *top_right = vec_pointer_init(center.x+0.5*dim.x, center.y+0.5*dim.y);
    list_add(shape, top_right);

    vector_t *bottom_right = vec_pointer_init(center.x+0.5*dim.x, center.y-0.5*dim.y);
    list_add(shape, bottom_right);

    return shape;
}

body_t *create_ball(vector_t center) {
    list_t *shape = create_ball_shape(center);
    body_t *ball = body_init_with_info(shape, BALL_MASS, BALL_COLOR, make_type_info(BALL), free);
    body_set_velocity(ball, BALL_VELOCITY);
    return ball; 
}

body_t *create_block(vector_t center, body_type_t status, rgb_color_t color) {
    list_t *shape = create_rectangle_shape(center, BLOCK_DIM);
    body_t *block = body_init_with_info(NULL, CHARACTER_MASS, color, make_type_info(status), body_info_free);
    body_set_centroid(block, center); 
    body_set_shape(block, shape); 
    return block; 
}

void create_walls(scene_t *scene) {
    vector_t center = {WALL_THICKNESS/2, CANVAS_SIZE.y/2};
    vector_t walldim = {WALL_THICKNESS, CANVAS_SIZE.y};
    body_t *leftwall = body_init_with_info(create_rectangle_shape(center, walldim), INFINITY, WALL_COLOR, make_type_info(WALL), free);
    scene_add_body(scene, leftwall);

    center = (vector_t) {CANVAS_SIZE.x/2, CANVAS_SIZE.y-WALL_THICKNESS/2};
    walldim = (vector_t) {CANVAS_SIZE.x, WALL_THICKNESS};
    body_t *topwall = body_init_with_info(create_rectangle_shape(center, walldim), INFINITY, WALL_COLOR, make_type_info(WALL), free);
    scene_add_body(scene, topwall);

    center = (vector_t) {CANVAS_SIZE.x-WALL_THICKNESS/2, CANVAS_SIZE.y/2};
    walldim = (vector_t) {WALL_THICKNESS, CANVAS_SIZE.y};
    body_t *rightwall = body_init_with_info(create_rectangle_shape(center, walldim), INFINITY, WALL_COLOR, make_type_info(WALL), free);
    scene_add_body(scene, rightwall);

    // floor has different type to trigger game restart
    center = (vector_t) {CANVAS_SIZE.x/2, WALL_THICKNESS/2};
    walldim = (vector_t) {CANVAS_SIZE.x, WALL_THICKNESS};
    body_t *bottomwall = body_init_with_info(create_rectangle_shape(center, walldim), INFINITY, WALL_COLOR, make_type_info(FLOOR), free);
    scene_add_body(scene, bottomwall);
}

void random_powerup(scene_t *scene);

void block_ball_collision_handler(body_t *ball, body_t *block, vector_t axis, void *aux) {
    // bounce the ball off of the block, then remove the block
    double impulse = body_get_mass(ball) * (1 + BLOCK_ELASTICITY);
    double u1 = vec_dot(body_get_velocity(ball), axis);
    impulse *= -1 * u1;
    vector_t imp_vec = vec_multiply(impulse, axis);
    //printf("impulse: %f, %f\n", imp_vec.x, imp_vec.y);
    body_add_impulse(ball, imp_vec);
    //body_set_velocity(ball, vec_add(body_get_velocity(ball), imp_vec));
    body_remove(block);

    int random = rand()%(5-0+1)+0;

    if (random > 2) {
        random_powerup(aux);
    }
}

void setup(scene_t *scene);

void restart(body_t *body1, body_t *body2, vector_t axis, void *aux) {
    //delete all bodies and forces from scene, add new ones using setup()
    //aux is the scene here
    scene_t *scene = aux;
    int ball_count = 0;
    for (int i = 0; i < scene_bodies(scene); i++) {
        body_t *body = scene_get_body(scene, i);
        if (get_type(body) == BALL) {
            ball_count++;
        }
    }
    if (ball_count <= 1) {
        for (size_t i = 0; i < scene_bodies(scene); i++) {
        body_t *body = scene_get_body(scene, i);
        body_remove(body);
    }
    setup(scene);
    } else {
        body_remove(body1);
    }
}

void initialize_ball(scene_t *scene) {
    // Initalize ball
    vector_t ball_delta = {0, 50};
    body_t *ball = create_ball(vec_add(ball_delta, body_get_centroid(scene_get_body(scene, 0))));
    size_t size = scene_bodies(scene);
    scene_add_body(scene, ball);

    // Create forces
    for (size_t i = 0; i < size; i++) {
        body_t *body = scene_get_body(scene, i);
        switch (get_type(body)) {
            case BLOCK:
                // when ball hits block, bounce ball and remove block
                create_collision(scene, ball, body, block_ball_collision_handler, scene, NULL);
                break;
            case WALL:
                // when ball hits wall, bounce ball
                create_physics_collision(scene, WALL_ELASTICITY, ball, body);
                break;
            case FLOOR:
                // when ball hits floor, restart game
                create_collision(scene, ball, body, restart, scene, NULL);
                break;
            case PLAYER:
                // when ball hits player, bounce ball
                create_physics_collision(scene, BLOCK_ELASTICITY, ball, body);
                break;
            case BALL:
                break;
        }
    }
}

void random_powerup(scene_t *scene) {
    int id = rand()%(2-0+1)+0;    
    // another ball powerup
    if (id == 0) {
        // another ball powerup
        initialize_ball(scene);
    }
    else if (id == 1) {
        // shorter paddle
        body_t *paddle = scene_get_body(scene, 0);
        vector_t new_paddle_dim = {25, 0};
        list_t *new_paddle_shape = create_rectangle_shape(body_get_centroid(paddle), vec_subtract(CHARACTER_DIM, new_paddle_dim));
        body_set_shape(paddle, new_paddle_shape);
    }
    else {
        // longer paddle
        body_t *paddle = scene_get_body(scene, 0);
        vector_t new_paddle_dim = {25, 0};
        list_t *new_paddle_shape = create_rectangle_shape(body_get_centroid(paddle), vec_add(CHARACTER_DIM, new_paddle_dim));
        body_set_shape(paddle, new_paddle_shape);
    }
}

void setup(scene_t *scene) {
    // Initialize character
    vector_t CHARACTER_LOCATION = {CANVAS_SIZE.x / 2, 25.0};
    scene_add_body(scene, create_block(CHARACTER_LOCATION, PLAYER, CHARACTER_COLOR)); 
    
    // Initialize blocks
    double gap_x = (CANVAS_SIZE.x - NUM_BLOCKS_PER_ROW*BLOCK_DIM.x)/(NUM_BLOCKS_PER_ROW+1);
    double gap_y = gap_x;
    for (int r = 0; r < NUM_BLOCKS_PER_COL; r++) {
        for (int c = 0; c < NUM_BLOCKS_PER_ROW; c++) {
            vector_t center = {(c+1)*gap_x + (c+0.5)*BLOCK_DIM.x, CANVAS_SIZE.y - ((r+1)*gap_y + (r+0.5)*BLOCK_DIM.y)};
            rgb_color_t color = init_color(c, NUM_BLOCKS_PER_ROW);
            scene_add_body(scene, create_block(center, BLOCK, color));
        }
    }

    // Create walls
    create_walls(scene);
    
    // Create ball
    initialize_ball(scene);
}

int main() {
    // Initialize the scene
    vector_t *bottom_left = vec_pointer_init(0.0, 0.0);
    vector_t *top_right = vec_pointer_init(CANVAS_SIZE.x, CANVAS_SIZE.y);

    sdl_init(*bottom_left, *top_right);

    scene_t *game_objects = scene_init();
    setup(game_objects);

    sdl_on_key(on_key);
    double dt;

    while (!sdl_is_done(game_objects)) {
        //if all blocks are gone, restart game
        if (scene_bodies(game_objects) <= 6) {
            restart(NULL, NULL, VEC_ZERO, game_objects);
        }

        dt = time_since_last_tick();
        scene_tick(game_objects, dt);
        
        sdl_render_scene(game_objects);
    }
}