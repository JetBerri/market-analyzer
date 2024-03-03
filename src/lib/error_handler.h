// error_handler.h

#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h> // Add ncurses.h for WINDOW*

void handle_error(const char *message, WINDOW *content_win);

#endif
