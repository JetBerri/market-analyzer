// error_handler.c

#include "error_handler.h"

void handle_error(const char *message, WINDOW *content_win) {
    wclear(content_win);
    box(content_win, 0, 0);
    wattron(content_win, COLOR_PAIR(1)); // Assuming you've defined color pair 1 as red
    mvwprintw(content_win, 1, 1, "Error: %s\n", message);
    wattroff(content_win, COLOR_PAIR(1));
    wrefresh(content_win);
    perror(message);
    exit(EXIT_FAILURE);
}
