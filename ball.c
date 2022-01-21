#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define WIDTH 78
#define HEIGHT 64
#define FPS 30
#define DT (1.0f / FPS)

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
bool quit = false;

typedef struct { float x,y; } V2f;

V2f v2f(float x, float y) {
    V2f new_v = {x, y};
    return new_v;
}

V2f v2f_sub(V2f a, V2f b) {
    a.x -= b.x;
    a.y -= b.y;
    return a;
}

V2f v2f_add(V2f a, V2f b) {
    a.x += b.x;
    a.y += b.y;
    return a;
}

float v2f_dist(V2f a, V2f b) {
    V2f diff = {fabs(a.x-b.x), fabs(a.y-b.y)};
    return sqrt(diff.x*diff.x + diff.y*diff.y);
}

V2f v2f_scale(V2f v, float s) {
    v.x *= s;
    v.y *= s;
    return v;
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
                v2f_dist(v2f(i,j), center) <= (float)radius)
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

void sigint_handler(int signum) {
    quit = true;
}

int main(void) {
    /* hide cursor */
    printf("\e[?25l");

    struct sigaction sa = {0};
    sa.sa_handler = sigint_handler;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction"); exit(1);
    }

    int radius = 12;
    V2f pos = { radius, radius };
    V2f vel = { 30.0f, 0.0f };
    V2f acc = { 0.0f, 169.0f };

    while (!quit) {
        vel = v2f_add(vel, v2f_scale(acc, DT));
        pos = v2f_add(pos, v2f_scale(vel, DT));

        if (pos.y + radius > HEIGHT) {
            pos.y = HEIGHT - radius;
            vel.y *= -1.0f;
        } else if (pos.y - radius <= 0) {
            pos.y = radius;
            vel.y *= -1.0f;
        }

        if (pos.x + radius > WIDTH) {
            pos.x = WIDTH - radius;
            vel.x *= -1;
        } else if (pos.x - radius < 0) {
            pos.x = radius;
            vel.x *= -1;
        }

        fill(OFF);
        circle(pos, radius);
        show();

        usleep(1000*1000 / FPS);
    }

    /* show cursor */
    printf("\e[?25h");

    return 0;
}

