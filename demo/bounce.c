#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
// #include "color.h"
#include "sdl_wrapper.h"
#include "list.h"
#include "polygon.h"

// Canvas constant properties
const double CANVAS_SIZE = 800;
const rgb_color_t CANVAS_COLOR = {0.0, 0.7, 1.0};

// Default star properties
const int STAR_RADIUS = 75;
const double ANGULAR_VELOCITY = -20;
const double CENTER_X = 200;
const double CENTER_Y = 200;
const double VELOCITY_X = 2000;
const double VELOCITY_Y = 1000;
const int NUM_SIDE_STAR = 5;
const double one_half = 0.5;
rgb_color_t STAR_COLOR = {0.5f, 0.5f, 0.5f};

// Creates a list of points for the corners of the canvas to easily see the boundaries
vec_list_t *build_square() {
    vec_list_t *square = simple_list_init(4);
    vector_t *bottomleft = vec_pointer_init(0, 0);
    vector_t *topleft = vec_pointer_init(0, CANVAS_SIZE);
    vector_t *bottomright = vec_pointer_init(CANVAS_SIZE, 0);
    vector_t *topright = vec_pointer_init(CANVAS_SIZE, CANVAS_SIZE);
    list_add(square, bottomright);
    list_add(square, bottomleft);
    list_add(square, topleft);
    list_add(square, topright);
    return square;
}

vec_list_t *build_star(
    vector_t initial_pos,
    int radius)
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

// Checks if any vertices of an object touched or went past a boundary
// And modifies the velocity as needed
void handle_collisions(
    vec_list_t *polygon,
    vector_t *velocity,
    vector_t *bottom_left,
    vector_t *top_right)
{
    // Check if the object collided with a wall
    int x_collision = 0;
    int y_collision = 0;
    for (size_t i = 0; i < list_size(polygon); i++)
    {
        vector_t *v = list_get(polygon, i);
        if ((velocity->x < 0 && v->x <= bottom_left->x) ||
            (velocity->x > 0 && v->x >= top_right->x))
        {
            x_collision = 1;
        }
        if ((velocity->y < 0 && v->y <= bottom_left->y) ||
            (velocity->y > 0 && v->y >= top_right->y))
        {
            y_collision = 1;
        }
    }

    // Reverse the direction in either x or y (or both)
    if (x_collision)
    {
        vector_t v_data = {-1.0 * velocity->x, velocity->y};
        *velocity = v_data;
    }
    if (y_collision)
    {
        vector_t v_data = {velocity->x, -1.0 * velocity->y};
        *velocity = v_data;
    }
}

// Handles a complete move of an object for a single time step
void move_object(
    vec_list_t *polygon,
    vector_t *velocity,
    double dt,
    double angular_velocity,
    vector_t *center,
    vector_t *bottom_left,
    vector_t *top_right)
{
    // Only handle collisions if an object is in motion
    if ((velocity->x != 0.0) && (velocity->y != 0.0))
    {
        handle_collisions(polygon, velocity, bottom_left, top_right);
    }

    vector_t dPos = vec_multiply(dt, *velocity);
    double dRot = angular_velocity * dt;
    polygon_rotate(polygon, dRot, *center);
    polygon_translate(polygon, dPos);
    // translate the center of rotation as well
    *center = vec_add(dPos, *center);
}

void check_arguments(
    int argc,
    char *argv[],
    int *radius,
    double *angular_velocity,
    vector_t *velocity,
    vector_t *center)
{
    // Validate usage
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-radius") == 0)
        {
            *radius = atoi(argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "-pos") == 0)
        {
            center->x = atof(argv[i + 1]);
            center->y = atof(argv[i + 2]);
            i += 2;
        }
        else if (strcmp(argv[i], "-v") == 0)
        {
            velocity->x = atof(argv[i + 1]);
            velocity->y = atof(argv[i + 2]);
            i += 2;
        }
        else if (strcmp(argv[i], "-a") == 0)
        {
            *angular_velocity = atof(argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "-r") == 0)
        {
            STAR_COLOR.r = atof(argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "-g") == 0)
        {
            STAR_COLOR.g = atof(argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "-b") == 0)
        {
            STAR_COLOR.b = atof(argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "-help") == 0)
        {
            printf("Usage: %s \n"
                   "-radius <star radius (DEFAULT=%d)>\n"
                   "-pos <initial position x y (DEFAULT=%0.2f, %0.2f)>\n"
                   "-v <velocity x y (DEFAULT=%0.2f %0.2f)>\n"
                   "-a <angular velocity (DEFAULT=%0.2f)>\n"
                   "-r <red value (DEFAULT=%0.2f)>\n"
                   "-g <green value (DEFAULT=%0.2f)>\n"
                   "-b <blue value (DEFAULT=%0.2f)>\n",
                   argv[0],
                   STAR_RADIUS,
                   CENTER_X,
                   CENTER_Y,
                   VELOCITY_X,
                   VELOCITY_Y,
                   ANGULAR_VELOCITY,
                   STAR_COLOR.r,
                   STAR_COLOR.g,
                   STAR_COLOR.b);
            exit(1);
        }
    }
}

int main(int argc, char *argv[])
{
    // Initialize default parameters of bouncing star
    int *radius = malloc(sizeof(int));
    *radius = STAR_RADIUS;
    double *angular_velocity = malloc(sizeof(double));
    *angular_velocity = -ANGULAR_VELOCITY;
    vector_t *velocity = vec_pointer_init(VELOCITY_X, VELOCITY_Y);
    vector_t *center = vec_pointer_init(CENTER_X, CENTER_Y);

    check_arguments(argc, argv, radius, angular_velocity, velocity, center);

    // Initialize the scene
    vector_t *bottom_left = vec_pointer_init(0.0, 0.0);
    vector_t *top_right = vec_pointer_init(CANVAS_SIZE, CANVAS_SIZE);
    sdl_init(*bottom_left, *top_right);

    // Draw the star with initial translation and angular velocity
    vec_list_t *star = build_star(*center, *radius);
    vec_list_t *square = build_square();

    double dt;
    while (!sdl_is_done(NULL))
    {
        // At each time step the object has moved
        // dPos = dt * velocity
        // dRot = dt * rotation
        dt = time_since_last_tick();
        move_object(
            star,
            velocity,
            dt,
            *angular_velocity,
            center,
            bottom_left,
            top_right);

        sdl_clear();
        sdl_draw_polygon(square, CANVAS_COLOR);
        sdl_draw_polygon(star, STAR_COLOR);
        sdl_show();
    }

    free(bottom_left);
    free(top_right);
    free(velocity);
    free(radius);
    free(angular_velocity);
    free(center);
    list_free(star);
    return 0;
}