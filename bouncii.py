#!/usr/bin/python

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

class Screen:
    def __init__(self, width: int, height: int) -> None:
        self.width = width + 1
        self.height = height
        self.arr = [ "" for _ in range(self.width * self.height) ]
        self.clear()
        return

    def toString(self) -> str:
        return "".join(self.arr)

    def get(self, x: int, y: int) -> str:
        return(self.arr[(self.width * (self.height - y - 1)) + x])

    def set(self, x:int, y:int, val: str) -> None:
        self.arr[(self.width * (self.height - y - 1)) + x] = val

    def clear(self) -> None:
        for i in range(self.width * self.height):
            if i % self.width == self.width - 1:
                self.arr[i] = "\n"
            else:
                self.arr[i] = " "
        return

def cursesMain(stdscr: "curses._CursesWindow") -> int:
    curses.curs_set(0)
    stdscr.nodelay(1)

    height, width = stdscr.getmaxyx()
    screen = Screen(width, height)

    balls = [ Ball(width - 1, height - 1) ]

    while True:
        screen.clear()
        stdscr.clear()

        for b in balls:
            screen.set(b.getX(), b.getY(), b.getChr())
            b.update()

        stdscr.insstr(0, 0, screen.toString())

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
