// #include <math.h>
// #include <stdlib.h>
// #include <stdio.h>
// #include <time.h>
// #include "scene.h"
// #include "sdl_wrapper.h"
// #include "color.h"
// #include "vector.h"
// #include "scene.h"
// #include "forces.h"
// #include "collision.h"
// #include <assert.h>
// #include "body.h"

// // Canvas constant properties
// const vector_t CANVAS_SIZE = {1000, 500};


// // Game properties
// const rgb_color_t CHARACTER_COLOR = {1.0f, 0.0f, 0.0f}; 
// const double CHARACTER_MASS = 25.0;
// const vector_t CHARACTER_DIM = {75, 25};
// const int NUM_ENEMIES_PER_ROW = 8;
// const int NUM_ENEMIES_PER_COL = 3;
// const vector_t ENEMY_VELOCITY = {200, 0};
// const vector_t BULLET_DIM = {2, 7};
// const vector_t BULLET_SPEED = {0, 1000}; 
// const double FIRE_INTERVAL = .1;  
// const double PLAYER_VELOCITY = 1000.0; 

// // By our convention, we want the first element in our scene to be our player
// // The next elements are the enemies, then the last are the bullets

// list_t *create_rectangle_shape(vector_t center, vector_t dim) { 
//     // printf("xc: %f, yc: %f\n", center.x, center.y); 
//     list_t *shape = simple_list_init(4);
//     vector_t *bottom_left = vec_pointer_init(center.x-0.5*dim.x, center.y-0.5*dim.y);
//     // printf("bottom left xb: %f, yb: %f\n", bottom_left->x, bottom_left->y); 
//     list_add(shape, bottom_left);
//     vector_t *top_left = vec_pointer_init(center.x-0.5*dim.x, center.y+0.5*dim.y);
//     list_add(shape, top_left);
//     vector_t *top_right = vec_pointer_init(center.x+0.5*dim.x, center.y+0.5*dim.y);
//     list_add(shape, top_right);
//     vector_t *bottom_right = vec_pointer_init(center.x+0.5*dim.x, center.y-0.5*dim.y);
//     list_add(shape, bottom_right);
//     return shape;
// }


// body_t *create_bullet(vector_t center) {
//     body_info_t *info = malloc(sizeof(body_info_t));
//     assert(info != NULL);
//     info->character = 1;
//     list_t *shape = create_rectangle_shape(VEC_ZERO, BULLET_DIM);
//     body_t *bullet = body_init_with_info(shape, CHARACTER_MASS, CHARACTER_COLOR, info, body_info_free); 
//     body_set_centroid(bullet, center); 
//     return bullet; 
// }

// void fire(scene_t *scene, body_t *body) {
//     // If body is character: fire upwards. create a bullet and create_destructive_collision with every enemy body.
//     // If body is enemy: fire downwards create a bullet and create_destructive_collision with character body.
//     // vector_t center = body_get_centroid(body); 
//     // printf("xc: %f, yc: %f\n", center.x, center.y);  
    
//     body_t *bullet = create_bullet(body_get_centroid(body)); 
//     vector_t speed = BULLET_SPEED; 

//     if (!body_is_enemy(body)) {
//         speed = vec_negate(speed);  
//         //printf("reached here\n"); 
//         create_destructive_collision(scene, bullet, scene_get_body(scene, 0)); 
//         // printf("reached here also\n"); 
//         create_destructive_collision(scene, scene_get_body(scene, 0), bullet); 
//     } else {
//         for (size_t i = 1; i < scene_bodies(scene); i++){
//             create_destructive_collision(scene, scene_get_body(scene, i), bullet); 
//             create_destructive_collision(scene, bullet, scene_get_body(scene, i)); 
//         } 
//     }
//     body_set_velocity(bullet, speed); 
//     scene_add_body(scene, bullet);
// }


// void on_key(char key, key_event_type_t type, double held_time, scene_t *scene) {
//     body_t *character = scene_get_body(scene, 0); 
//     if (!body_is_enemy(character)) {
//         return;
//     }
//     vector_t temp = body_get_velocity(character);
//     vector_t pos = body_get_centroid(character); 
//     if (type == KEY_PRESSED) {
//         switch (key) {
//             case LEFT_ARROW:
//                 if (pos.x - 0.5*CHARACTER_DIM.x > 0) {
//                     temp.x = PLAYER_VELOCITY * -1; 
//                     body_set_velocity(character, temp);

//                 } 
//                 else {
//                     body_set_velocity(character, VEC_ZERO);
//                 }
//                 break;
//             case RIGHT_ARROW:
//                 if (pos.x + 0.5*CHARACTER_DIM.x < CANVAS_SIZE.x) {
//                     temp.x = PLAYER_VELOCITY; 
//                     body_set_velocity(character, temp);
//                 }
//                 else {
//                     body_set_velocity(character, VEC_ZERO);
//                 }
//                 break;
//             case SPACEBAR:
//                 if (held_time == 0){
//                 // printf("type: %u\n", body_is_enemy(character)); 
//                     fire(scene, character);
//                     break;
//                 }
//         }
//     }
//     else {
//         switch (key) {
//             case LEFT_ARROW:
//                 body_set_velocity(character, VEC_ZERO);
//                 break;
//             case RIGHT_ARROW:
//                 body_set_velocity(character, VEC_ZERO);
//                 break;
//         }
//     }
// }


// int count_of_type(scene_t *scene, int id){
//     int count = 0; 
//     for (size_t i = 0; i < scene_bodies(scene); i++){ 
//         if (body_is_enemy(scene_get_body(scene, i)) == id){
//             count++; 
//         }
//     } 
//     return count; 
// }


// void enemy_shoot(scene_t *scene){
//     int count = count_of_type(scene, 0); 
//     int num_shoot = rand() % count + 1; 
//     fire(scene, scene_get_body(scene, num_shoot)); 
// }


// body_t *create_character(vector_t center, int status) {
//     // printf("status: %u", status); 
//     body_info_t *info = malloc(sizeof(body_info_t));
//     assert(info != NULL);
//     info->character = status;


//     list_t *shape = create_rectangle_shape(VEC_ZERO, CHARACTER_DIM);
//     body_t *character = body_init_with_info(shape, CHARACTER_MASS, CHARACTER_COLOR, info, body_info_free);
//     body_set_centroid(character, center); 
//     // printf("type: %u\n", body_is_enemy(character)); 
//     if (status == 0) {
//         body_set_velocity(character, ENEMY_VELOCITY);
//     } 
//     return character; 
// }


// void create_enemies(scene_t *scene) {
//     double gap_x = (CANVAS_SIZE.x - NUM_ENEMIES_PER_ROW*CHARACTER_DIM.x)/(NUM_ENEMIES_PER_ROW+1);
//     double gap_y = (CANVAS_SIZE.y/3 - NUM_ENEMIES_PER_COL*CHARACTER_DIM.y)/(NUM_ENEMIES_PER_COL+1);
//     for (int r = 0; r < NUM_ENEMIES_PER_COL; r++) {
//         for (int c = 0; c < NUM_ENEMIES_PER_ROW; c++) {
//             vector_t center = {(c+1)*gap_x + (c+0.5)*CHARACTER_DIM.x, CANVAS_SIZE.y - ((r+1)*gap_y + (r+0.5)*CHARACTER_DIM.y)};
//             // printf("row number: %d, x: %f, y: %f\n", r, center.x, center.y);
//             scene_add_body(scene, create_character(center, 0));
//         }
//     }
// }


// void check_edges(scene_t *scene) {
//     double gap_y = (CANVAS_SIZE.y/3 - NUM_ENEMIES_PER_COL*CHARACTER_DIM.y)/(NUM_ENEMIES_PER_COL+1);
//     for (size_t i = 0; i < scene_bodies(scene); i++) {
//         body_t *body = scene_get_body(scene, i);
//         if (!body_is_enemy(body)) {
//             vector_t vel = body_get_velocity(body); 
//             // if (i % 3 == 0){
//             //     printf("body number: %zu, x: %f, y: %f\n", i, print.x, print.y);
//             // }
            
//             if ((vel.x > 0 && body_get_centroid(body).x + 0.5*CHARACTER_DIM.x > CANVAS_SIZE.x) || 
//             (vel.x < 0 && body_get_centroid(body).x - 0.5*CHARACTER_DIM.x < 0)) {
//                 vector_t cur = body_get_centroid(body); 
//                 vector_t temp = {cur.x, cur.y - 3*(gap_y+CHARACTER_DIM.y)};
//                 // printf("body number: %zu, x: %f, y: %f\n", i, temp.x, temp.y); 
//                 body_set_centroid(body, temp);
//                 body_set_velocity(body, vec_negate(body_get_velocity(body)));
//             }
//         }
//     }
// }


int main() {
}
//     // Initialize the scene
//     vector_t *bottom_left = vec_pointer_init(0.0, 0.0);
//     vector_t *top_right = vec_pointer_init(CANVAS_SIZE.x, CANVAS_SIZE.y);
    
//     sdl_init(*bottom_left, *top_right);
    
//     // Initialize character and enemies
//     vector_t CHARACTER_LOCATION = {CANVAS_SIZE.x / 2, 25.0};
//     body_t *character = create_character(CHARACTER_LOCATION, 1);
    
//     scene_t *game_objects = scene_init(); 

//     scene_add_body(game_objects, character); 
//     create_enemies(game_objects);
//     sdl_on_key(on_key);
//     double dt;
//     double fire_counter = 0;
//     while (!sdl_is_done(game_objects)) {
//         dt = time_since_last_tick();
//         // printf("first pass\n"); 
//         scene_tick(game_objects, dt);
//         check_edges(game_objects);
//         // printf("second pass\n");
//         // creates new game_objects after certain intervals
//         if (fire_counter >= FIRE_INTERVAL) {
//             enemy_shoot(game_objects);
//             fire_counter = -1;
//         }

//         if (count_of_type(game_objects, 0) == 0) {
//             break;
//         }

//         // printf("passed here\n"); 
//         sdl_render_scene(game_objects);
//         fire_counter += dt;
//     }
// }