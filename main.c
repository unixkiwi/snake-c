#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <curses.h>
#include <time.h>
#include <string.h>

void drawBorder(int width, int height);

void drawBtn(int y, int x, const char *text, int isHovered, WINDOW *win);

void game(WINDOW *win);

typedef struct {
    int x;
    int y;
} vec2;

int main(int argc, char *argv[]) {

    WINDOW *win = initscr();

    // ---Render setup stuff---
    keypad(win, true); // take keyboard input
    nodelay(win, true); // no keyboard input delay
    curs_set(0); // hide cursor
    noecho(); // don't print keys

    game(win);

    endwin();

    return 0;
}

void game(WINDOW *win) {
    struct winsize w;

    srand(time(NULL));

    ioctl(0, TIOCGWINSZ, &w);

    unsigned short height = w.ws_row;
    unsigned short width = w.ws_col;

    // ---has color test---
    if (has_colors()) {
            start_color();

            // pair number 1, fg color white, bg color red
            init_pair(1, COLOR_WHITE, COLOR_RED);
            // pair number 2, fg color white, bg color green
            init_pair(2, COLOR_WHITE, COLOR_GREEN);
            // pair number 3, fg color black, bg color black
            init_pair(3, COLOR_BLACK, COLOR_WHITE);
            // pair number 4, fg color white, bg color yellow
            init_pair(4, COLOR_WHITE, COLOR_YELLOW);
    }

    bool loose;

    int8_t score = 0;
    char scoreString[11];

    vec2 pos = {10, 5};
    vec2 food = {rand() % (width-2) + 1, rand() % (height-2) + 1};
    vec2 dir = {1, 0};

    vec2 tiles[100];

    while (true) {
        ioctl(0, TIOCGWINSZ, &w);
        height = w.ws_row;
        width = w.ws_col;

        // ---Set tiles positions---
        for (int k = score; k > 0; k--) {
            if (tiles[k].x == pos.x && tiles[k].y == pos.y) loose = true;
            tiles[k] = tiles[k-1];
        }

        tiles[0] = pos;

        // ---Key stuff---
        int pressed = wgetch(win);

        switch (pressed) {
            case KEY_LEFT: {
                dir.x = dir.x == 1 ? 1 : -1;
                dir.y = 0;
                break;
            }
            case KEY_RIGHT: {
                dir.x = dir.x == -1 ? -1 : 1;
                dir.y = 0;
                break;
            }
            case KEY_UP: {
                dir.x = 0;
                dir.y = dir.y == 1 ? 1 : -1;
                break;
            }
            case KEY_DOWN: {
                dir.x = 0;
                dir.y = dir.y == -1 ? -1 : 1;
                break;
            }
        }

        pos.x += dir.x;
        pos.y += dir.y;

        // ---Render---
        erase();

        // ---Border---
        drawBorder(width, height);

        // ---Score stuff---
        sprintf(scoreString, "Score: %d", score);

        mvaddstr(0, ((width-1)/2)-(strlen(scoreString)/2), scoreString);

        // ---Snake render---
        attron(COLOR_PAIR(1));
        mvaddstr(pos.y, pos.x, " ");
        attroff(COLOR_PAIR(1));

        for (int i = 0; i < score; i++) {
            attron(COLOR_PAIR(2));
            mvaddch(tiles[i].y, tiles[i].x, ' ');
            attroff(COLOR_PAIR(2));
        }

        attron(COLOR_PAIR(4));
        mvaddch(food.y, food.x, ' ');
        attroff(COLOR_PAIR(4));

        // ---Food stuff---

        if (food.x == pos.x && food.y == pos.y) {
            mvaddch(food.y, food.x, ' ');

            food.x = rand() % (width-2) + 1;
            food.y = rand() % (height-2) + 1;

            score++;

            if (score > 100) loose = false;
        }

        // ---Border stuff---
        if (pos.x <= 0 || pos.x >= width-1 || pos.y <= 0 || pos.y >= height-1) loose = true;

        // ---Lose :( or Win :) stuff---
        if (loose || score > 100) {
            while (true) {
                erase();

                if (!loose) mvaddstr((height-1)/2, ((width-1)/2)-(strlen("You won!")/2), "You won!");
                else mvaddstr((height-1)/2, ((width-1)/2)-(strlen("You lost!")/2), "You lost!");

                // little warning but idc
                mvaddstr(((height-1)/2)+((height-1) * 0.25), ((width-1)/2)-(strlen("Press any key to exit or 'Y' to play again...")/2), "Press any key to exit or 'Y' to play again...");

                nodelay(win, false);

                int endPressed = getch();

                if (endPressed == 'y' || endPressed == 'Y') {
                    nodelay(win, true);
                    loose = false;
                    score = 0;
                    pos = (vec2){10, 5};
                    food = (vec2){rand() % (width - 2) + 1, rand() % (height - 2) + 1};
                    dir = (vec2){1, 0};
                    memset(tiles, 0, sizeof(tiles));
                    break;
                } else {
                    erase();
                    return;
                }
            }
        } else {
            if (dir.x == 1 || dir.x == -1) usleep(50000);
            else usleep(100000);
        }
    }

    erase();
}

void drawBorder(int width, int height) {
    attron(COLOR_PAIR(3));
    for (int i = 0; i < width; i++) {
        mvaddch(0, i, '-');
        mvaddch(height-1, i, '-');
    }

    for (int j = 0; j < height; j++) {
        mvaddch(j, 0, '|');
        mvaddch(j, width-1, '|');
    }

    mvaddch(0, 0, '+');
    mvaddch(0, width-1, '+');
    mvaddch(height-1, 0, '+');
    mvaddch(height-1, width-1, '+');

    attroff(COLOR_PAIR(3));
}

void drawBtn(int y, int x, const char *text, int isHovered, WINDOW *win) {
    vec2 winSize;
    getmaxyx(win, winSize.y, winSize.x);

    if (isHovered) attron(A_REVERSE);
    mvprintw((winSize.y/2)-winSize.y/10, (winSize.x - strlen(text))/2, "[ %s ]", text);
    if (isHovered) attroff(A_REVERSE);
}
