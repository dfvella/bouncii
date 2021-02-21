#!/usr/bin/env python3

import curses

from time import sleep
from random import randrange

FRAMERATE = 32

GRAVITY = 128
ROLLRES = 2

class Ball:
    def __init__(self, xMax: int, yMax: int) -> None:
        self.x = randrange(1, xMax)
        self.y = yMax - 1
        self.vx = randrange(-40, 40)
        self.vy = randrange(-20, 20)
        self.mx = xMax
        self.my = yMax
        self.dt = 1 / FRAMERATE
        self.char = randrange(33, 126)

    def update(self) -> None:
        if self.x == 0 or self.x == self.mx:
            self.vx *= -1
        if self.y == 0 or self.y == self.my:
            self.vy *= -1

        if self.getY() == 0:
            if self.vx < 0:
                self.vx += ROLLRES * self.dt
            if self.vx > 0:
                self.vx -= ROLLRES * self.dt
        self.vy -= GRAVITY * self.dt

        self.x += self.vx * self.dt
        self.y += self.vy * self.dt

        self.x = min(self.mx, max(0, self.x))
        self.y = min(self.my, max(0, self.y))

    def getX(self) -> int:
        return round(self.x)

    def getY(self) -> int:
        return round(self.y)

    def getChr(self) -> str:
        return chr(self.char)

def cursesMain(stdscr: "curses._CursesWindow") -> int:
    curses.use_default_colors()
    curses.curs_set(0)
    stdscr.nodelay(1)

    height, width = stdscr.getmaxyx()
    balls = [ Ball(width - 1, height - 1) ]

    while True:
        stdscr.clear()

        for b in balls:
            stdscr.insstr(height - b.getY() - 1, b.getX(), b.getChr())
            b.update()

        try:
            if stdscr.getkey():
                balls.append(Ball(width - 1, height - 1))
        except curses.error:
           sleep(1 / FRAMERATE)

def main() -> int:
    try:
        return curses.wrapper(cursesMain)
    except KeyboardInterrupt:
        return 0

if __name__ == "__main__":
    exit(main())
