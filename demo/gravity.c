#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "color.h"
#include "sdl_wrapper.h"
#include "polygon.h"
#include "list.h"
#include "body.h"

// Canvas constant properties
const vector_t CANVAS_SIZE = {1000, 500};
const rgb_color_t CANVAS_COLOR = {0.3, 0.3, 1.0};

// Default Simulation Properties
const vector_t G = {0, -10000};

// Default polygon properties
const vector_t VELOCITY = {300, 0};

const int RESPAWN_INTERVAL = 1000;

const int side_factor = 7 - 3 + 1;
const int side_offset = 3;
const int radius_factor = 50 - 25 + 1;
const int radius_offset = 25;
const int mass_factor = 100 - 5 + 1;
const int mass_offset = 5;
const int min_side_num = 5;
const double elasticity_scale = 0.25;
const double elasticity_offset = 0.85;
const int initial_star_capacity = 10;

// Creates a list of points for the corners of the canvas to easily see the boundaries
body_t * build_background(void) {
    vec_list_t *background = simple_list_init(4);
    vector_t *bottomleft = vec_pointer_init(0, 0);
    vector_t *topleft = vec_pointer_init(0, CANVAS_SIZE.y);
    vector_t *bottomright = vec_pointer_init(CANVAS_SIZE.x, 0);
    vector_t *topright = vec_pointer_init(CANVAS_SIZE.x, CANVAS_SIZE.y);
    list_add(background, bottomright);
    list_add(background, bottomleft);
    list_add(background, topleft);
    list_add(background, topright);
    body_t *b = body_init(background, 0.0, CANVAS_COLOR);
    return b;
}

vec_list_t * build_vertices(
        int num_sides,
        vector_t initial_pos,
        int radius) {
    
    // If triangle or square, we don't want inner points
    if (num_sides < min_side_num) {
        vec_list_t *vertices = simple_list_init(num_sides);

        // Calculate outer points of the triangle/square
        vector_t top_outer_point = {0.0, radius};
        double angle;
        for (int i = 0; i < num_sides; i++) {
            angle = i * 2 * M_PI / num_sides;
            vector_t *p_outer = vec_pointer_init(0.0, 0.0);
            *p_outer = vec_add(initial_pos, vec_rotate(top_outer_point, angle));
            list_add(vertices, p_outer);
        }
        return vertices;
    } else {
        vec_list_t *vertices = simple_list_init(2 * num_sides);
        double inner_radius = radius * 2 / (3 + sqrt(num_sides));

        // Calculate outer and innter points of the polygon
        vector_t top_outer_point = {0.0, radius};
        vector_t top_inner_point = {0.0, inner_radius};
        double angle;
        double angle_offset = 2 * M_PI / num_sides / 2;
        for (int i = 0; i < num_sides; i++) {
            angle = i * 2 * M_PI / num_sides;
            vector_t *p_outer = vec_pointer_init(0.0, 0.0);
            vector_t *p_inner = vec_pointer_init(0.0, 0.0);     
            *p_outer = vec_add(initial_pos, vec_rotate(top_outer_point, angle));
            *p_inner = vec_add(initial_pos, vec_rotate(top_inner_point, angle + angle_offset));
            list_add(vertices, p_outer);
            list_add(vertices, p_inner);
        }
        return vertices;
    }
}

// Checks if any vertices of an object touched or went past a boundary
// And modifies the velocity as needed
void handle_collisions(
        body_t *p,
        vector_t *bottom_left,
        vector_t *top_right) {
    // Check if the object collided with a wall
    // We are only checking wall collisions in y direction not x
    int y_collision = 0;
    for (size_t i = 0; i < list_size(body_get_shape(p)); i++)
    {
        vector_t *pos = list_get(body_get_shape(p), i);
        if ((body_get_velocity(p).y < 0 && pos->y <= bottom_left->y) || 
            (body_get_velocity(p).y > 0 && pos->y >= top_right->y))
        {
            y_collision = 1;
        }
    }

    // Reverse the direction for y
    // And apply some variable elasticity (between 0.85 and 1.1)
    if (y_collision)
    {
        double elasticity = (elasticity_scale * (double)rand() / (double)RAND_MAX) + elasticity_offset;
        vector_t v_data = {body_get_velocity(p).x, -1.0 * elasticity * body_get_velocity(p).y};
        body_set_velocity(p, v_data);
    }
}

// Handles situation where an object moves off the screen
int is_off_screen(body_t *polygon, vector_t *top_right) {
    // Check if the object is past the right wall
    int off_screen_count = 0;
    for (size_t i = 0; i < list_size(body_get_shape(polygon)); i++) {
        vector_t *pos = list_get(body_get_shape(polygon), i);
        if (pos->x > top_right->x) {
            off_screen_count++;
        }
    }

    if (off_screen_count == list_size(body_get_shape(polygon))) {
        return 1;
    }
    return 0;
}

// Handles a complete move of an object for a single time step
void move_object(
        body_t *p,
        double dt,
        vector_t *bottom_left,
        vector_t *top_right) {
    // Only handle collisions if an object is in motion
    if (body_get_velocity(p).x != 0.0 && body_get_velocity(p).y != 0.0) {
        handle_collisions(p, bottom_left, top_right);
    }

    // Apply gravity
    vector_t gdt = vec_multiply(dt, G);
    body_set_velocity(p, vec_add(gdt, body_get_velocity(p)));
    body_tick(p, dt);
}

// Generates a polygon with random values for its properties and adds it to polygons
void generate_random_polygon(body_list_t *list) {

    int num_sides = rand() % side_factor + side_offset;
    int radius = rand() % radius_factor + radius_offset;
    //double angular_velocity = rand() % (15 + 15 + 1) - 15;    
    vector_t velocity = VELOCITY;
    vector_t f = {0,0};
    vector_t center = {radius, CANVAS_SIZE.y - radius};
    rgb_color_t color = {(float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX};
    double mass = rand() % mass_factor + mass_offset;
    vec_list_t *vertices = build_vertices(num_sides, center, radius);
    body_t *polygon = body_detailed_init((list_t *) vertices, velocity, center, f, f, radius, mass, color, NULL, NULL, NULL);
    list_add(list, polygon);
}

int main() {
    // Initialize the scene
    vector_t *bottom_left = vec_pointer_init(0.0, 0.0);
    vector_t *top_right = vec_pointer_init(CANVAS_SIZE.x, CANVAS_SIZE.y);
    sdl_init(*bottom_left, *top_right);

    // Initializes list of polygons with a polygon
    body_list_t *polygons = list_init(initial_star_capacity, (free_func_t)body_free);
    // body_t *square = build_background();
    generate_random_polygon(polygons);

    double dt;
    int respawn_counter = 0;
    while (!sdl_is_done(NULL)) {
        // At each time step the object has moved
        // dPos = dt * velocity
        // dRot = dt * rotation
        dt = time_since_last_tick();
        sdl_clear();
        // rgb_color_t newc = {(rand() % 3) / 3.0, (rand() % 3) / 3.0, (rand() % 3) / 3.0};
        // body_set_color(square, newc);
        // sdl_draw_polygon(body_get_shape(square), body_get_color(square));

        if (respawn_counter >= RESPAWN_INTERVAL) {
            generate_random_polygon(polygons);
            respawn_counter = 0;
        }

        size_t i = 0;
        while (i < list_size(polygons)) {
            body_t *p = list_get(polygons, i);

            move_object(
                p,
                dt,
                bottom_left,
                top_right);

            // Remove an object if it is off screen
            // Since we coalesce the list, we do not increment the 
            // index here because the new pointer will be at the
            // existing index
            if (is_off_screen(p, top_right) == 1) {
                p = list_remove(polygons, i);
                body_free(p);
            } else {

                sdl_draw_polygon(body_get_shape(p), body_get_color(p));
                i++;
            }
        }
        sdl_show();
        respawn_counter++;
    }

    free(bottom_left);
    free(top_right);
    list_free(polygons);
}