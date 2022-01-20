#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#define WIDTH 64
#define HEIGHT 64
#define FPS 60

enum {
    OFF = 0,
    ON  = 1
};

char pixel[2][2] = {
    { ' ', '_' },
    { '^', 'C' }
};

int buffer[WIDTH * HEIGHT];
char display[WIDTH * HEIGHT/2];

typedef struct { float x,y; } V2f;

V2f v2f(float x, float y) {
    V2f new_v = {x, y};
    return new_v;
}

V2f v2f_sub(V2f a, V2f b) {
    V2f result = {a.x-b.x, a.y-b.y};
    return result;
}

V2f v2f_add(V2f a, V2f b) {
    V2f result = {a.x+b.x, a.y+b.y};
    return result;
}

double v2f_dist(V2f a, V2f b) {
    V2f diff = {fabs(a.x-b.x), fabs(a.y-b.y)};
    return sqrt(diff.x*diff.x + diff.y*diff.y);
}

V2f v2f_scale(V2f v, float s) {
    V2f new_v = {v.x*s, v.y*s};
    return new_v;
}

void fill(int c) {
    for (int j=0; j<HEIGHT; j++)
        for (int i=0; i<WIDTH; i++)
            buffer[j*WIDTH+i] = c;
}

void circle(V2f center, int radius) {
    V2f top_left = v2f_sub(center, v2f(radius, radius));
    V2f bottom_right = v2f_add(center, v2f(radius, radius));

    for (int j=top_left.y; j<bottom_right.y; j++) {
        for (int i=top_left.x; i<bottom_right.x; i++) {
            if (j >= 0 && j < HEIGHT && i >= 0 && i < WIDTH && 
                v2f_dist(v2f(i,j), center) <= (double)radius)
            {
                buffer[j * WIDTH + i] = ON;
            }
        }
    }
}

void show(void) {
    for (int j=0; j<HEIGHT/2; j++) {
        for (int i=0; i<WIDTH; i++) {
            int top = buffer[j*2*WIDTH + i];
            int bottom = buffer[(j*2+1)*WIDTH + i];
            display[j*WIDTH+i] = pixel[top][bottom];
        }
        printf("%.*s\n", WIDTH, display + j*WIDTH);
    }
    /* return cursor to position before writing to stdout */
    printf("\x1b[%dA \x1b[%dD", HEIGHT/2, WIDTH);
}

int main(void) {
    printf("\e[?25l");
    V2f pos = { WIDTH/2, HEIGHT/2 };
    int radius = 12;
    bool quit = false;
    V2f vel = { 0.001f, 0.000f };
    V2f acc = { 0.0f, 0.000001f };
    
    double prev_time = clock()/CLOCKS_PER_SEC;
    double frame_time;

    while (!quit) {
        double now = clock()/CLOCKS_PER_SEC;
        frame_time += now - prev_time;

        if (frame_time < FPS / 1000)
            continue;

        frame_time -= FPS / 1000;

        vel = v2f_add(vel, acc);
        pos = v2f_add(pos, vel);

        if (pos.y + radius >= HEIGHT) {
            pos.y = HEIGHT - radius;
            vel.y *= -1;
        }

        if (pos.x + radius >= WIDTH) {
            pos.x = WIDTH - radius;
            vel.x *= -1;
        } else if (pos.x - radius <= 0) {
            pos.x = radius;
            vel.x *= -1;
        }

        fill(OFF);
        circle(pos, radius);
        show();
    }

    return 0;
}

