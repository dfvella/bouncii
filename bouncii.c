#include <ncurses.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

#define MAXPARTICLES 256
#define ROWS LINES

#define MAPSIZE (unsigned)(LINES*COLS)

#define EMPTY -1
#define BLANK 32
#define pi (float)(3.14159265)

const unsigned char GRAVITY = 128;
const unsigned char ROLLRES = 2;

const char FRAMERATE = 32;
const float PERIOD = 1.0f / FRAMERATE;

const float ONEBILLION = 1000000000;

volatile unsigned char state = 1;

typedef struct ParticleStruct {
    float x, y;
    float xLast, yLast;
    float vx, vy;
    chtype ch;
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
    p->xLast = 0;
    p->yLast = 0;
    p->vx = (float)randrange(-40, 40);
    p->vy = (float)randrange(-15, 15);
    p->ch = (chtype)randrange(33, 127);
}

void particleUpdate(Particle *p, int mx, int my, float dt) {
    p->xLast = constrain(p->x, 0, (float)mx);
    p->yLast = constrain(p->y, 0, (float)my);

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

int mapIndex(int row, int col) {
    return (row * COLS) + col;
}

void mapClear(int *map) {
    for (unsigned int i = 0; i < MAPSIZE; ++i) {
        map[i] = EMPTY;
    }
}

int mapAt(int *map, int row, int col) {
    return map[mapIndex(row, col)];
}

int isCollision(int *map, int row, int col) {
    return map[mapIndex(row, col)] != EMPTY;
}

int checkCollsion(int *map, int row, int col) {
    int checkLeft = col > 0;
    int checkRight = col < (COLS - 1);
    int checkTop = row < (ROWS - 1);
    int checkBottom = row > 0;

    if (checkTop && isCollision(map, row+1, col)) {
        return mapAt(map, row+1, col);
    } else if (checkTop && checkRight && isCollision(map, row+1, col+1)) {
        return mapAt(map, row+1, col+1);
    } else if (checkRight && isCollision(map, row, col+1)) {
        return mapAt(map, row, col+1);
    } else if (checkRight && checkBottom && isCollision(map, row-1, col+1)) {
        return mapAt(map, row-1, col+1);
    } else if (checkBottom && isCollision(map, row-1, col)) {
        return mapAt(map, row-1, col);
    } else if (checkBottom && checkLeft && isCollision(map, row-1, col-1)) {
        return mapAt(map, row-1, col-1);
    } else if (checkLeft && isCollision(map, row, col-1)) {
        return mapAt(map, row, col-1);
    } else if (checkLeft && checkTop && isCollision(map, row+1, col-1)) {
        return mapAt(map, row+1, col-1);
    }
    return EMPTY;
}

void printMap(int *map) {
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            fprintf(stderr,"%d ",mapAt(map,i,j));
        }
        fprintf(stderr,"\n");
    }
    fprintf(stderr,"\n");
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

    int *map = malloc(sizeof(int) * (unsigned)MAPSIZE);
    mapClear(map);

    while (state) {
        #ifdef PRINTSTUFF
        printMap(map);
        #endif

        clear();

        for (int i = 0; i < numParticles; ++i) {
            particleUpdate(particles + i, COLS - 1, ROWS - 1, PERIOD);

            int xPos = (int)round(particles[i].x);
            int yPos = ROWS - (int)round(particles[i].y) - 1;
            int xPosLast = (int)round(particles[i].xLast);
            int yPosLast = ROWS - (int)round(particles[i].yLast) - 1;

            map[mapIndex(yPosLast, xPosLast)] = EMPTY;

            int pIndex = checkCollsion(map, yPos, xPos);

            if (pIndex != EMPTY) { /* then collision */
                float vy1 = particles[i].vy;
                float vx1 = particles[i].vx;

                float vy2 = particles[pIndex].vy;
                float vx2 = particles[pIndex].vx;

                /* compute angle between velocity vectors */
                float theta1 = atan2f(particles[i].vy, particles[i].vx);
                float theta2 = atan2f(particles[pIndex].vy, particles[pIndex].vx);

                /* compute plane tangent to the point of contact */
                float tPlane = (theta1 + theta2) / 2;

                /* reflect velocity vectors across point of contact */
                float vNet1 = sqrtf((vy1 * vy1) + (vx1 * vx1));
                float vNet2 = sqrtf((vy2 * vy2) + (vx2 * vx2));

                /* rotate angles so tangent plane is vertical */
                theta1 += (pi / 2) - tPlane;
                theta2 += (pi / 2) - tPlane;

                float vPerp1 = vNet1 * cosf(theta1);
                float vPare1 = vNet1 * sinf(theta1);

                float vPerp2 = vNet2 * cosf(theta2);
                float vPare2 = vNet2 * sinf(theta2);

                /* reflect velocity component parallel to tangent plane */
                vPerp1 *= -1;
                vPerp2 *= -1;

                theta1 = atan2f(vPare1, vPerp1);
                theta2 = atan2f(vPerp2, vPare2);

                vNet1 = sqrtf((vPare1 * vPare1) + (vPerp1 * vPerp1));
                vNet2 = sqrtf((vPare2 * vPare2) + (vPerp2 * vPerp2));

                vNet1 = vNet2 = (vNet1 + vNet2) / 2;

                /* change back to original coordinates */
                theta1 -= (pi / 2) - tPlane;
                theta2 -= (pi / 2) - tPlane;

                particles[i].vx = vNet1 * cosf(theta1);
                particles[i].vy = vNet1 * sinf(theta1);

                particles[pIndex].vx = vNet2 * cosf(theta2);
                particles[pIndex].vy = vNet2 * sinf(theta2);

                #ifdef PRINTSTUFF
                fprintf(stderr,"Collision with %d at (%d,%d)\n",pIndex,yPos,xPos);
                #endif
            }
            map[mapIndex(yPos, xPos)] = i;
            mvaddch(yPos, xPos, particles[i].ch);
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
