#include <ncurses.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

#define MAXPARTICLES 256
#define ROWS LINES

const unsigned char GRAVITY = 128;
const unsigned char ROLLRES = 2;

const char FRAMERATE = 32;
const float PERIOD = 1.0f / FRAMERATE;

const float ONEBILLION = 1000000000;

volatile unsigned char state = 1;

typedef struct ParticleStruct {
    float x, y;
    float vx, vy;
    unsigned int ch;
} Particle;

void sighandler() {
    state = 0;
}

float getTime() {
    return (float)clock() / CLOCKS_PER_SEC;
}

void wait() {
    static float tLast = 0;

    if (PERIOD - getTime() + tLast <= 0) {
        printf("error: loop overrun\n");
        exit(1);
    }

    struct timespec waitTime = {
        0, (long)((PERIOD - getTime() + tLast) * ONEBILLION)
    };

    nanosleep(&waitTime, &waitTime);

    tLast = getTime();
}

float constrain(float num, float min, float max) {
    return num < min ? min : num > max ? max : num;
}

void randrangeInit() {
    srand((unsigned int)time(NULL));
}

/* min inclusive, max exclusive */
int randrange(int min, int max) {
    return (rand() % (max - min)) + min;
}

void particleInit(Particle *p) {
    p->x = (float)randrange(0, COLS);
    p->y = (float)(LINES - 2);
    p->vx = (float)randrange(-40, 40);
    p->vy = (float)randrange(-15, 15);
    p->ch = (unsigned int)randrange(33, 127);
}

void particleUpdate(Particle *p, int mx, int my, float dt) {
    if (p->x == 0 || p->x == mx) { /* bounce off sides */
        p->vx *= -1;
    }
    if (p->y == 0 || p->y == my) { /* bounce off bottom */
        p->vy *= -1;
    }

    if (p->y == 0) { /* apply rolling resistive force */
        if (p->vx < 0) {
            p->vx += ROLLRES * dt;
        }
        if (p->vx > 0) {
            p->vx -= ROLLRES * dt;
        }
    }

    p->vy -= GRAVITY * dt;

    p->x += p->vx * dt;
    p->y += p->vy * dt;

    p->x = constrain(p->x, 0, (float)mx);
    p->y = constrain(p->y, 0, (float)my);
}

void mapClear(int *map, int size) {
    for (int i = 0; i < size; ++i) {
        map[i] = -1;
    }
}

int mapIndex(int row, int col) {
    return (row * COLS) + col;
}

int main(void) {
    signal(SIGINT, sighandler);

    initscr();
    noecho();
    curs_set(0);
    timeout(0);

    randrangeInit();

    const int maxParticles = (ROWS * COLS) / 2;

    int numParticles = 1;

    Particle *particles = malloc(sizeof(Particle) * (unsigned)maxParticles);

    particleInit(particles);

    const int mapSize = ROWS * COLS;
    int *map = malloc(sizeof(int) * (unsigned)mapSize);
    mapClear(map, mapSize);

    while (state) {
        clear();

        mapClear(map, mapSize);

        for (int i = 0; i < numParticles; ++i) {
            particleUpdate(particles + i, COLS - 1, ROWS - 1, PERIOD);

            int xPos = (int)round(particles[i].x);
            int yPos = (int)round(particles[i].y);

            map[mapIndex(xPos, yPos)] = i;

            mvaddch(ROWS - yPos - 1, xPos, particles[i].ch);
        }

        refresh();
        wait();

        /* add particle on key press */
        if (numParticles < maxParticles && getch() != ERR) {
            particleInit(particles + numParticles);
            ++numParticles;
        }
    }

    endwin();

    free(particles);

    exit(0);
}
