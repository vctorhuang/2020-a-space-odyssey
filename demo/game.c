#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include "audio.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "color.h"
#include "vector.h"
#include "scene.h"
#include "forces.h"
#include "collision.h"
#include "body.h"
#include "sdnoise1234.h"

// Allocations of game objects in scene
/* 
0. background 
1. player 
2. cursor
3. Wall below player 
Onwards: asteroids, enemies, bullets, etc.
*/

// Canvas constant properties
const vector_t CANVAS_SIZE = {1000, 500};

// Game properties
int EXIT_GAME = 0;
const double ASTER_DIM_MAX = 4; 
const double ASTER_DIM_MIN = 1;  
const double ASTER_SIDES = 8; 
const vector_t ASTER_DIM = {5, 8}; 
const vector_t PLAYER_DIM = {40, 20}; 
const vector_t ENEMY_DIM = {5, 5}; 


const double PLAYER_Y_START = .1; 
const double Y_START = .8; 
const double WIDTH_TOP = .005; 
const double WIDTH_BOT = .4; 
const double ASTER_MASS = 20;
const double WALL_THICKNESS = 5.0;
const double CURSOR_RADIUS = 5;
const double BULLET_RADIUS = 8;
const double BULLET_MASS = 3;

int *glitch_frames;
const int GLITCH_FRAMECOUNT = 5;

// Colors
const rgb_color_t CANVAS_COLOR = {0, 0, 0};
const rgb_color_t ASTER_COLOR = {1, 0, 0};
const rgb_color_t PLAYER_COLOR = {0, 1, 0};
const rgb_color_t ENEMY_COLOR = {0, 0, 0};
const rgb_color_t BULLET_COLOR = {1, 1, 1};

// Speed properties 
const double PLAYER_VELOCITY = 300;
const double PLAYER_BULLET_SPEED = 1000;
const double ENEMY_BULLET_SPEED = 500; 
const double ENEMY_SPEED = 30;  
const double INIT_ASTER_ROT_SPEED = .25; 

// Scaling properties
const double NOISE_SCALE = .3;
const double SPEED_SCALE = 50; 
const double PLAYER_SIZE= 20; 
const double ENEMY_STOP = .6; 
const double ASTER_GROWTH = 1.05; 
const double PLAYER_BULLET_GROWTH = .80; 
const double ENEMY_BULLET_GROWTH = 1.02; 
const double BULLET_DECAY_Y = .6; 
const double ASTER_ROT_SPEED_SCALE = 1.05; 
const double SCORE_SCALING = 10000.0; 

// Health Properties 
const int PLAYER_HEALTH = 100;
const int ENEMY_HEALTH = 50;
const int ASTER_HEALTH = 20; 
const int BULLET_HEALTH = 1; 
const int BULLET_DAMAGE = 20;
const int ASTER_DAMAGE = 10;

// Spawn intervals
const double ASTER_SPAWN_INT = .15; 
const double ENEMY_SPAWN_INT = 1; 
const double INITIAL_ENEMY_DELAY = 1; 
const double ENEMY_FIRE_INT = .5; 
const double ASTER_BOUND_SPAWN = 1; 
const int FPS = 60; 
const double ENV_GROWTH = .20; 

// UI elements
const vector_t SCORE_XY = {10, 10};
const vector_t HEALTH_BAR_DIM = {500, 20};
const rgb_color_t HEALTH_BAR_BG_COLOR = {1, 1, 1};
const rgb_color_t HEALTH_BAR_COLOR = {1, 0, 0};
const vector_t MAIN_MENU_1 = {250, 50};
const vector_t MAIN_MENU_2 = {450, 150};
const vector_t MAIN_MENU_3 = {425, 250};
const vector_t MAIN_MENU_4 = {400, 325};
const vector_t MAIN_MENU_5 = {325, 350};
const vector_t MAIN_MENU_6 = {300, 375};
const vector_t MAIN_MENU_7 = {350, 400};
const vector_t GAME_OVER_1 = {385, 50};
const vector_t GAME_OVER_2 = {425, 150};
const vector_t GAME_OVER_3 = {450, 250};


// Force elements
const double ASTER_ENEMY_REPULSE = -100; 

// Score properties
int *score;
char *score_str;
char *prefix;
const int KILL_ENEMY_SCORE = 500;
const int KILL_ASTEROID_SCORE = 25;

// Sound properties
const double VOLUME = .3;
const char *MUSIC_PATH = "highwaytohell.wav";
const char *PEW_PATH = "pew.wav";
Mix_Music *music = NULL;
Mix_Chunk *pew = NULL;

// Body auxiliary types
typedef enum {
    PLAYER,
    ASTEROID,
    ENEMY,
    PLAYER_BULLET,
    ENEMY_BULLET,
    WALL,
    BACKGROUND,
    HEALTHBAR,
    CURSOR
} body_type_t;

typedef struct body_info {
    body_type_t character;
    int highD; // tells whether or not the body will move in 3D perspective (high dimension)
    int health;
} body_info_t;

rgb_color_t random_color(){

    double r = (double) rand() / RAND_MAX;  
    double g = (double) rand() / RAND_MAX;  
    double b = (double) rand() / RAND_MAX;  
    rgb_color_t color = {r, g, b}; 
    return color; 
}

body_info_t *make_info(body_type_t type, int highD) {
    body_info_t *info = malloc(sizeof(body_info_t));
    info->character = type;
    info->highD = highD;
    switch (type) {
        case PLAYER:
            info->health = PLAYER_HEALTH;
            break;
        case ENEMY:
            info->health = ENEMY_HEALTH;
            break;
        case ASTEROID:
            info->health = ASTER_HEALTH;
            break;
        case PLAYER_BULLET:
            info->health = BULLET_HEALTH;
            break;
        case ENEMY_BULLET:
            info->health = BULLET_HEALTH;
            break;
        case WALL:
            info->health = 1000;
            break;
        case BACKGROUND:
            info->health = 1000;
            break;
        case HEALTHBAR:
            info->health = 1000;
            break;
        case CURSOR:
            info->health = 1000;
            break;
    }
    return info;
}

body_type_t get_type(body_t *body) {
    body_info_t *info = body_get_info(body); 
    return info->character;
}

int get_highd(body_t *body) {
    body_info_t *info = body_get_info(body);
    return info->highD;
}

int get_health(body_t *body) {
    body_info_t *info = body_get_info(body);
    return info->health;
}

void set_health(body_t *body, int health) {
    body_info_t *info = body_get_info(body);
    info->health = health;
}

list_t *create_player_shape(vector_t center) { 
    list_t *shape = simple_list_init(12);
    vector_t *p1 = vec_pointer_init(center.x-1*PLAYER_SIZE, center.y-1*PLAYER_SIZE);
    list_add(shape, p1);
    vector_t *p2 = vec_pointer_init(center.x-1*PLAYER_SIZE, center.y-.5*PLAYER_SIZE);
    list_add(shape, p2);
    vector_t *p3 = vec_pointer_init(center.x-2*PLAYER_SIZE, center.y-2.5*PLAYER_SIZE);
    list_add(shape, p3);
    vector_t *p12 = vec_pointer_init(center.x-2*PLAYER_SIZE, center.y+2.5*PLAYER_SIZE);
    list_add(shape, p12);
    vector_t *p4 = vec_pointer_init(center.x-1*PLAYER_SIZE, center.y+.5*PLAYER_SIZE);
    list_add(shape, p4);
    vector_t *p5 = vec_pointer_init(center.x-1*PLAYER_SIZE, center.y+1*PLAYER_SIZE);
    list_add(shape, p5);
    vector_t *p6 = vec_pointer_init(center.x+1*PLAYER_SIZE, center.y+1*PLAYER_SIZE);
    list_add(shape, p6);
    vector_t *p7 = vec_pointer_init(center.x+1*PLAYER_SIZE, center.y+.5*PLAYER_SIZE);
    list_add(shape, p7);
    vector_t *p8 = vec_pointer_init(center.x+2*PLAYER_SIZE, center.y+2.5*PLAYER_SIZE);
    list_add(shape, p8);
    vector_t *p9 = vec_pointer_init(center.x+2*PLAYER_SIZE, center.y-2.5*PLAYER_SIZE);
    list_add(shape, p9);
    vector_t *p10 = vec_pointer_init(center.x+1*PLAYER_SIZE, center.y-.5*PLAYER_SIZE);
    list_add(shape, p10);
    vector_t *p11 = vec_pointer_init(center.x+1*PLAYER_SIZE, center.y-1*PLAYER_SIZE);
    list_add(shape, p11);
    return shape;
}

vec_list_t *create_aster_shape(
    vector_t initial_pos,
    int radius) {
    vec_list_t *aster = simple_list_init(ASTER_SIDES);
    double angle;
    for (int i = 0; i < ASTER_SIDES; i++)
    {
        angle = i * 2 * M_PI / ASTER_SIDES;
        double len = (rand()/(double)RAND_MAX) * (ASTER_DIM_MAX - ASTER_DIM_MIN) + ASTER_DIM_MIN; 
        vector_t *p = vec_pointer_init(0.0, len);
        *p = vec_add(initial_pos, vec_rotate(*p, angle));
        list_add(aster, p);
    }
    return aster;
}

list_t *create_rectangle_shape(vector_t center, vector_t dim) { 
    // printf("xc: %f, yc: %f\n", center.x, center.y); 
    list_t *shape = simple_list_init(4);
    vector_t *bottom_left = vec_pointer_init(center.x-0.5*dim.x, center.y-0.5*dim.y);
    // printf("bottom left xb: %f, yb: %f\n", bottom_left->x, bottom_left->y); 
    list_add(shape, bottom_left);
    vector_t *top_left = vec_pointer_init(center.x-0.5*dim.x, center.y+0.5*dim.y);
    list_add(shape, top_left);
    vector_t *top_right = vec_pointer_init(center.x+0.5*dim.x, center.y+0.5*dim.y);
    list_add(shape, top_right);
    vector_t *bottom_right = vec_pointer_init(center.x+0.5*dim.x, center.y-0.5*dim.y);
    list_add(shape, bottom_right);
    return shape;
}

list_t *create_circle_shape(vector_t center, double radius) {
    double num_points = 15;
    list_t *shape = simple_list_init(num_points);
    double angle_increment = 2*M_PI/num_points;
    double angle = 0;
    for (int i = 0; i < num_points; i++) {
        vector_t *p_rot = vec_pointer_init(radius, 0);
        *p_rot = vec_add(center, vec_rotate(*p_rot, angle));
        list_add(shape, p_rot);
        angle += angle_increment;
    }
    return shape;
}

void spawn_player(scene_t * scene) {
    vector_t center = {CANVAS_SIZE.x / 2 , CANVAS_SIZE.y * PLAYER_Y_START}; 
    list_t *shape = create_player_shape(center); 
    body_t *ret = body_detailed_init(shape, VEC_ZERO, center, VEC_ZERO, VEC_ZERO, 0, 0, 1, PLAYER_COLOR, false, body_info_free, make_info(PLAYER, false));
    scene_add_body(scene, ret); 
}

void build_background(scene_t *scene) {
    vector_t center = vec_multiply(.5, CANVAS_SIZE); 
    list_t * shape = create_rectangle_shape(center, CANVAS_SIZE); 
    body_t *ret = body_detailed_init(shape, VEC_ZERO, center, VEC_ZERO, VEC_ZERO, 0, 0, 1, CANVAS_COLOR, false, body_info_free, make_info(BACKGROUND, false));
    scene_add_body(scene, ret); 
}

void add_cursor(scene_t *scene, vector_t center) {
    body_t *cursor = body_detailed_init(create_circle_shape(center, CURSOR_RADIUS), VEC_ZERO, center, VEC_ZERO, VEC_ZERO,0, 0, 1, PLAYER_COLOR, false, body_info_free, make_info(CURSOR, false));
    scene_add_body(scene, cursor); 
}

body_t *create_bottom_wall() {
    vector_t center = (vector_t) {CANVAS_SIZE.x/2, -5 * ASTER_DIM.y};
    vector_t walldim = (vector_t) {2.0 * CANVAS_SIZE.x, WALL_THICKNESS};
    body_t *bottomwall = body_init_with_info(create_rectangle_shape(center, walldim), INFINITY, CANVAS_COLOR, make_info(WALL, false), free);
    return bottomwall;
}

body_t *create_bullet(vector_t center, body_info_t *aux) {
    list_t *shape = create_circle_shape(VEC_ZERO, BULLET_RADIUS);
    body_t *bullet = body_init_with_info(shape, BULLET_MASS, BULLET_COLOR, aux, body_info_free); 
    
    body_set_centroid(bullet, center); 
    return bullet; 
}

void render_ui(scene_t *scene) {

    // remove all ui elements
    for (int i = 0; i < scene_bodies(scene); i++) {
        body_t *body = scene_get_body(scene, i);
        if (get_type(body) == HEALTHBAR) {
            body_remove(body);
        }
    }

    // add ui elements
    body_t *player = scene_get_body(scene, 1);
    int health = get_health(player);

    vector_t bg_center = {CANVAS_SIZE.x / 2, CANVAS_SIZE.y - 35};
    vector_t bg_dim = {HEALTH_BAR_DIM.x + 10, HEALTH_BAR_DIM.y + 10};
    list_t *bg_shape = create_rectangle_shape(bg_center, bg_dim);
    body_t *bg = body_detailed_init(bg_shape, VEC_ZERO, bg_center, VEC_ZERO, VEC_ZERO, 0, 0, 100, HEALTH_BAR_BG_COLOR, false, body_info_free, make_info(HEALTHBAR, false));
    scene_add_body(scene, bg);

    if (health >= 0) {
        vector_t hb_center = {CANVAS_SIZE.x / 2, CANVAS_SIZE.y - 35};
        vector_t hb_dim = {((double) health / (double) PLAYER_HEALTH) * HEALTH_BAR_DIM.x, HEALTH_BAR_DIM.y};
        list_t *hb_shape = create_rectangle_shape(hb_center, hb_dim); 
        body_t *hb = body_detailed_init(hb_shape, VEC_ZERO, hb_center, VEC_ZERO, VEC_ZERO, 0, 0, 100, HEALTH_BAR_COLOR, false, body_info_free, make_info(HEALTHBAR, false));
        vector_t translate = {-1 * ((HEALTH_BAR_DIM.x - hb_dim.x) / 2), 0};
        body_set_centroid(hb, vec_add(translate, hb_center));
        scene_add_body(scene, hb);
    }
}

void screen_glitch(scene_t *scene, int *glitch_frames) {
    body_t *bg = scene_get_body(scene, 0);
    if (*glitch_frames > 1) {
        body_set_color(bg, random_color());
    } else if (*glitch_frames == 1) {
        body_set_color(bg, CANVAS_COLOR);
    }
    *glitch_frames = *glitch_frames - 1;
}

void bullet_collision_handler(body_t *bullet, body_t *target, vector_t axis, void *aux) {
    scene_t *scene = aux;
    body_type_t type = get_type(target);
    if (type == ENEMY || type == ASTEROID || type == PLAYER || type == PLAYER_BULLET) {
        int health = get_health(target);
        set_health(bullet, 0);
        set_health(target, health - BULLET_DAMAGE);
        render_ui(scene);
        if (type == PLAYER) {
            *glitch_frames = GLITCH_FRAMECOUNT;
        }
    }
}

void wall_collision_handler(body_t *obj, body_t *wall, vector_t axis, void *aux) {
    body_remove(obj);
}


void fire(scene_t *scene, body_t *body) {
    body_t *player = scene_get_body(scene, 1);
    body_t *cursor = scene_get_body(scene, 2);
    body_t *bullet = NULL;

    vector_t v = {0, 0};

    // If body is enemy, fire downwards targeted at player. If body is player, fire upwards.
    // If enemy bullet: create collision with player and bottom wall.
    // If player bullet: create collisions with existing asteroids and enemies.
    if (get_type(body) == ENEMY) {
        bullet = create_bullet(body_get_centroid(body), make_info(ENEMY_BULLET, true));
        v = vec_subtract(body_get_centroid(player), body_get_centroid(bullet));
        create_collision(scene, bullet, scene_get_body(scene, 1), bullet_collision_handler, scene, NULL);
        create_collision(scene, bullet, scene_get_body(scene, 3), wall_collision_handler, scene, NULL);
        for (size_t i = 0; i < scene_bodies(scene); i++){
            body_t *target = scene_get_body(scene, i);
            if (get_type(target) == PLAYER_BULLET) {
                create_collision(scene, bullet, target, bullet_collision_handler, scene, NULL);
                create_collision(scene, target, bullet, bullet_collision_handler, scene, NULL);
            }
        }
    } else if (get_type(body) == PLAYER) {
        bullet = create_bullet(body_get_centroid(body), make_info(PLAYER_BULLET, false));
        v = vec_subtract(body_get_centroid(cursor), body_get_centroid(bullet));
        v = vec_multiply(ENEMY_BULLET_SPEED/sqrt(pow(v.x, 2) + pow(v.y, 2)), v);
        for (size_t i = 0; i < scene_bodies(scene); i++){
            body_t *target = scene_get_body(scene, i);
            if (get_type(target) == ENEMY || get_type(target) == ASTEROID) {
                create_collision(scene, bullet, scene_get_body(scene, i), bullet_collision_handler, scene, NULL);
            }
            v = vec_multiply(PLAYER_BULLET_SPEED/sqrt(pow(v.x, 2) + pow(v.y, 2)), v);
        } 
    }
    
    body_set_velocity(bullet, v); 
    scene_add_body(scene, bullet);

    if (get_type(body) == PLAYER) {
        Mix_PlayChannel(-1, pew, 0);
    }
}


body_t *create_boundary_aster(int orientation, double time){
    double noise_x = (sdnoise1(time * NOISE_SCALE, NULL) + 1) / 2.0; 
    //printf("noise: %f\n", noise_x); 
    double y = CANVAS_SIZE.y * Y_START; 
    double x = CANVAS_SIZE.x * noise_x + orientation * WIDTH_TOP * CANVAS_SIZE.x; 

    vector_t top = {x,y}; 
    list_t *shape = create_aster_shape(top, ASTER_SIDES); 
    double noise_xb = (sdnoise1(time * NOISE_SCALE, NULL) + 1) / 2.0; 
    double xb = CANVAS_SIZE.x * noise_xb + orientation * WIDTH_BOT * CANVAS_SIZE.x; 
    double yb = 0; 
    vector_t bot = {xb, yb}; 

    vector_t unit = vec_subtract(bot, top); 
    unit = vec_multiply(1 / vec_distance(unit, VEC_ZERO), unit); 
    unit = vec_multiply(SPEED_SCALE, unit); 
    body_t *ret = body_detailed_init(shape, unit, top, VEC_ZERO, VEC_ZERO, INIT_ASTER_ROT_SPEED, 0, ASTER_MASS, ASTER_COLOR, false, body_info_free, make_info(ASTEROID, true));
    double c = top.y / CANVAS_SIZE.y; 
    body_resize(ret, c); 
    return ret; 
} 

body_t *create_enemy(double time){
    double noise_x = (sdnoise1(time * NOISE_SCALE, NULL) + 1) / 2.0; 
    //printf("noise: %f\n", noise_x); 
    double y = CANVAS_SIZE.y * Y_START; 
    double x = CANVAS_SIZE.x * noise_x; 
    double noise_xb = (sdnoise1(time * NOISE_SCALE, NULL) + 1) / 2.0; 
    double xb = CANVAS_SIZE.x * noise_xb; 
    double yb = 0; 
    vector_t top = {x,y}; 
    vector_t bot = {xb, yb}; 
    body_info_t *aux = make_info(ENEMY, true); 
    vector_t unit = vec_subtract(bot, top); 
    unit = vec_multiply(1 / vec_distance(unit, VEC_ZERO), unit); 
    unit = vec_multiply(SPEED_SCALE, unit); 
    list_t *shape = create_rectangle_shape(top, ENEMY_DIM); 
    body_t *ret = body_detailed_init(shape, unit, top, VEC_ZERO, VEC_ZERO, 0, 0, ASTER_MASS, ENEMY_COLOR, false, body_info_free, aux);
    double c = top.y / CANVAS_SIZE.y; 
    body_resize(ret, c); 
    return ret; 
}

void spawn_enemies(scene_t *scene, double time){
    body_t *enemy = create_enemy(time); 
    scene_add_body(scene, enemy);
    // printf("spawning enemy\n");
    
}

void asteroid_collision_handler(body_t *asteroid, body_t *target, vector_t axis, void *aux) {
    scene_t *scene = aux;
    vector_t velocity = body_get_velocity(target); 
    vector_t center = body_get_centroid(target);
    if (get_type(target) == ENEMY && center.y < CANVAS_SIZE.y * .6 ){
        vector_t b1c = body_get_centroid(asteroid); 
        if (center.x <  b1c.x){
            velocity.x = ENEMY_SPEED; 
        } else {
            velocity.x = -1 * ENEMY_SPEED; 
        }
        body_set_velocity(target, velocity);
    } else {
        int health = get_health(target);
        set_health(asteroid, 0);
        set_health(target, health - ASTER_DAMAGE);
        if (get_type(target) == PLAYER) {
            *glitch_frames = GLITCH_FRAMECOUNT;
        }
    }

    render_ui(scene);
}

void spawn_boundary(scene_t *scene, double time) {
    // if (fmod(time, ASTER_SPAWN_INT) < 1.3 / (double) FPS) {
        body_t *obj1 = create_boundary_aster(-1, time); 
        body_t *obj2 = create_boundary_aster(1, time); 

        size_t size = scene_bodies(scene);
        scene_add_body(scene, obj1); 
        scene_add_body(scene, obj2);  

        // create collisions
        // collide with player: take off player health
        // collide with wall: remove
        // we don't create collisions with bullet here. create those collisions when bullet is created
        // convention: asteroid first
        for (int i = 0; i < size; i++) {
            body_t *body = scene_get_body(scene, i);
            switch (get_type(body)) {
                case PLAYER:
                    create_collision(scene, obj1, body, asteroid_collision_handler, scene, NULL);
                    create_collision(scene, obj2, body, asteroid_collision_handler, scene, NULL);
                    break;
                case ENEMY:
                    create_collision(scene, obj1, body, asteroid_collision_handler, scene, NULL);
                    create_collision(scene, obj2, body, asteroid_collision_handler, scene, NULL);
                    break;
                case WALL:
                    create_collision(scene, obj1, body, wall_collision_handler, scene, NULL);
                    create_collision(scene, obj2, body, wall_collision_handler, scene, NULL);
                    break;
                case PLAYER_BULLET:
                    break;
                case ENEMY_BULLET:
                    break;
                case BACKGROUND:
                    break;
                case HEALTHBAR:
                    break;
                case ASTEROID:
                    break;
                case CURSOR:
                    break;
            }
        }
}



void scene_update_3D(scene_t *scene){
    for (int i = 0; i < scene_bodies(scene); i++){
        body_t *cur = scene_get_body(scene, i); 
        body_info_t* info = body_get_info(cur); 
        vector_t center = body_get_centroid(cur); 
        if (info->character == PLAYER_BULLET && center.y > BULLET_DECAY_Y * CANVAS_SIZE.y && !info->highD){
            info->highD = true; 
        }
        if (get_highd(cur)) {
            double c = 0;  
            double d = 0; 
            if (get_type(cur) == ASTEROID || get_type(cur) == ENEMY){
                c = ASTER_GROWTH; 
                d = ASTER_ROT_SPEED_SCALE; 
            } else if (get_type(cur) == PLAYER_BULLET) {
                c = PLAYER_BULLET_GROWTH; 
            } else if (get_type(cur) == ENEMY_BULLET) {
                c = ENEMY_BULLET_GROWTH;
            }
            
            body_set_rotation_speed(cur, body_get_rotation_speed(cur) * d); 
            body_resize(cur, c); 
            vector_t speed = body_get_velocity(cur);
            body_set_velocity(cur, vec_multiply(c, speed));  
            if ((info->character == ENEMY) && center.y < ENEMY_STOP * CANVAS_SIZE.y && info->highD) {
                info->highD = false;
                speed.x = ENEMY_SPEED;
                speed.y = 0; 
                body_set_velocity(cur, speed); 
                // for (int i = 3; i < scene_bodies(scene) - 1; i ++) {
                    // if (get_type(scene_get_body(scene, i)) == ASTEROID) {
                    //     create_newtonian_gravity(scene, 10000, cur, scene_get_body(scene,i));
                    // } 
                //}    
            }
        }
    }
}

double calc_bounding_box_time(void){
    double frame = 0; 
    scene_t *scene = scene_init(); 
    body_t *enemy = create_enemy(0);
    scene_add_body(scene, enemy);  
    vector_t center = body_get_centroid(enemy); 
    while (center.y > ENEMY_STOP * CANVAS_SIZE.y){
        scene_update_3D(scene); 
        scene_tick(scene, 1 / FPS);
        frame += 1 /FPS;  
    }
    scene_free(scene); 
    return frame; 
}

void setup(scene_t *scene) {
    build_background(scene); 
    spawn_player(scene);

    vector_t center = vec_multiply(0.5, CANVAS_SIZE);
    add_cursor(scene, center); 

    body_t *wall = create_bottom_wall();
    scene_add_body(scene, wall);

    render_ui(scene);
}

int is_player_dead(scene_t *scene) {
    body_t *player = scene_get_body(scene, 1);
    if (get_health(player) <= 0) {
        return 1;
    }
    return 0;
}

void check_objects(scene_t *scene, int *score, char *score_str, char *prefix) {
    for (int i = 0; i < scene_bodies(scene); i++) {
        body_t *body = scene_get_body(scene, i);
        if (get_health(body) <= 0) {
            char res[1024]; 
            sprintf(res, "%u", *score); 
            strcat(prefix, res);
            strcpy(score_str, prefix);
            strcpy(prefix, "SCORE: ");
            
            if (get_type(body) == ENEMY) {
                *score += KILL_ENEMY_SCORE;
            } else if (get_type(body) == ASTEROID) {
                *score += KILL_ASTEROID_SCORE;
            }
            body_remove(body);
        }
        if (get_type(body) == PLAYER_BULLET){
            vector_t center = body_get_centroid(body); 
            vector_t *s1 = list_get(body_get_shape(body), 0); 
            if (vec_distance(*s1, center) < 1){
                body_remove(body); 
            }
        }
        if (get_type(body) == ENEMY && !get_highd(body)) {
            vector_t eyeball = {15, 100};
            render_text(vec_subtract(body_get_centroid(body), eyeball), "2020", 12);
        }
    }
}

void enemy_fire(scene_t *scene){
    for (int i = 0; i < scene_bodies(scene); i++){
        body_t *cur = scene_get_body(scene, i); 
        if (get_type(cur) == ENEMY && get_highd(cur) == false){
            fire(scene, cur); 
            break; 
        }
    }
}

void reset_score() {
    *score = 0;
    strcpy(score_str, "SCORE: 0");
    strcpy(prefix, "SCORE: ");
}

void restart_game(scene_t *scene) {
    reset_score();
    *glitch_frames = 0;
    body_remove(scene_get_body(scene, 0));
    scene_tick(scene, 0.0); 
    setup(scene);
    render_ui(scene); 
}

void end_game(scene_t *game_objects, int *score, char *score_str) {
    for (int i = 1; i < scene_bodies(game_objects); i++){
        body_t *cur = scene_get_body(game_objects, i); 
        body_remove(cur); 
    }
    render_text(SCORE_XY, score_str, 25);
    scene_tick(game_objects, 0.001); 
}

void render_main_menu() {
    render_text(MAIN_MENU_1, "2020 SPACE ODYSSEY", 40);
    render_text(MAIN_MENU_2, "PLAY [R]", 25);
    render_text(MAIN_MENU_3, "HOW TO PLAY:", 20);
    render_text(MAIN_MENU_4, "Aim with mouse cursor", 20);
    render_text(MAIN_MENU_5, "Shoot blue enemies with spacebar", 20);
    render_text(MAIN_MENU_6, "Dodge the asteroids to avoid losing health [A,D]", 20);
    render_text(MAIN_MENU_7, "Get the highest possible score!", 20);
}

void render_game_over_menu() {
    render_text(GAME_OVER_1, "GAME OVER", 40);
    render_text(GAME_OVER_2, "RESTART [R]", 25);
    render_text(GAME_OVER_3, "GIVE UP [G]", 20);
}

void on_key(char key, key_event_type_t type, double held_time, scene_t *scene) {
    if (type == KEY_PRESSED && scene_bodies(scene) == 1){
        switch (key) {
            case R:
                restart_game(scene); 
                Mix_PlayMusic(music, -1);
                break;
            case G:
                EXIT_GAME = 1;
                break;
        }

    }
    if (scene_bodies(scene) < 2){
        return; 
    }
    body_t *player = scene_get_body(scene, 1); 
    if (get_type(player) != PLAYER) {
        return;
    }
    vector_t temp = body_get_velocity(player);
    vector_t pos = body_get_centroid(player); 
    if (type == KEY_PRESSED) {
        switch (key) {
            case A:
                if (pos.x > 0) {
                    temp.x = PLAYER_VELOCITY * -1; 
                    body_set_velocity(player, temp);
                } 
                else {
                    body_set_velocity(player, VEC_ZERO);
                }
                break;
            case D:
                if (pos.x < CANVAS_SIZE.x) {
                    temp.x = PLAYER_VELOCITY; 
                    body_set_velocity(player, temp);
                }
                else {
                    body_set_velocity(player, VEC_ZERO);
                }
                break;
            case SPACEBAR:
                if (held_time == 0) {
                    fire(scene, player);
                    break;
                }
        }
    }
    else {
        switch (key) {
            case A:
                body_set_velocity(player, VEC_ZERO);
                break;
            case D:
                body_set_velocity(player, VEC_ZERO);
                break;
        }
    }
}

double scoring_function(int* score, double rate){
    return (double) (*score + SCORE_SCALING) / (double) *score * rate; 
}
int main() {
    // Initialize the scene
    vector_t *bottom_left = vec_pointer_init(0.0, 0.0);
    vector_t *top_right = vec_pointer_init(CANVAS_SIZE.x, CANVAS_SIZE.y);
    
    sdl_init(*bottom_left, *top_right);
    
    scene_t *game_objects = scene_init(); 
    build_background(game_objects); 
    sdl_on_key(on_key);

    body_t *cursor;

    int played_before = 0;
    double spawn_time_enemy = 0; 
    double dt = 0; 
    double spawn_time_aster = 0; 
    double running_time = 0;
    double shoot_time_enemy = 0; 
    double time = 0; 
    
    int frequency = 22050;
    Uint16 format = AUDIO_S16SYS;
    int canal = 2;
    int buffer = 4096;
    Mix_OpenAudio(frequency, format, canal, buffer);
    
    music = Mix_LoadMUS(MUSIC_PATH);
    pew = Mix_LoadWAV(PEW_PATH);
    Mix_PlayMusic(music, -1);
    
    glitch_frames = malloc(sizeof(int *));
    *glitch_frames = 0;

    score = malloc(sizeof(int *));
    *score = 0;
    score_str = malloc(sizeof(char) * 1024);
    strcpy(score_str, "SCORE: 0");
    prefix = malloc(sizeof(char) * 1024);
    strcpy(prefix, "SCORE: ");
    
    while (!sdl_is_done(game_objects)) {
        dt = time_since_last_tick();
        time += dt; 
        spawn_time_aster += dt; 
        spawn_time_enemy += dt; 
        shoot_time_enemy += dt; 
        running_time += dt; 

        if (EXIT_GAME == 1) {
            break;
        }
        
        if (time > 1 / (double)FPS){
            sdl_clear();
            sdl_render_scene(game_objects);
            if (scene_bodies(game_objects) == 1){
                if (played_before == 0) {
                    render_main_menu(); 
                }
                else {
                    render_game_over_menu();
                }
            }
            
            if (scene_bodies(game_objects) > 1) {
                cursor = scene_get_body(game_objects, 2); 
                if(spawn_time_aster > ASTER_SPAWN_INT){
                    spawn_boundary(game_objects, running_time);
                    spawn_time_aster = 0; 
                }
                if(spawn_time_enemy > scoring_function(score, ENEMY_SPAWN_INT) && running_time > INITIAL_ENEMY_DELAY && *score > 0){
                    spawn_enemies(game_objects, running_time);
                    spawn_time_enemy = 0; 
                }
                if (*score > 0 && shoot_time_enemy > scoring_function(score, ENEMY_FIRE_INT)){
                    enemy_fire(game_objects); 
                    shoot_time_enemy = 0; 
                }
                scene_update_3D(game_objects);
                vector_t cursor_pos = sdl_get_mouse_position();
                body_set_centroid(cursor, cursor_pos);

                screen_glitch(game_objects, glitch_frames);
                scene_tick(game_objects, time);
                check_objects(game_objects, score, score_str, prefix);
                render_text(SCORE_XY, score_str, 25);
                time = 0; 
                if (is_player_dead(game_objects)) {
                    spawn_time_enemy = 0;  
                    spawn_time_aster = 0; 
                    running_time = 0;
                    shoot_time_enemy = 0; 
                    time = 0; 
                    end_game(game_objects, score, score_str);
                    render_game_over_menu(); 
                    played_before = 1;
                    Mix_HaltMusic();
                }
            }
            sdl_show();
        }
    }

    free(glitch_frames);
    free(score);
    free(score_str);
    Mix_FreeChunk(pew);
    Mix_Quit();
    return 0;
}