#include <menu.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>

// Include your custom header files
#include "src/lib/crypto_price.h"
#include "src/lib/error_handler.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define CTRLD 4

char *choices[] = {
    "Check Coin Values",
    "See Values in the last month",
    "Open graph",
    "Clean",
    "Exit",
    (char *)NULL,
};

void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string);
void scroll_content_window(WINDOW *content_win, int scroll_lines);

int main(int argc, char *argv[])
{
    ITEM **my_items;
    int c;
    MENU *my_menu;
    WINDOW *my_menu_win;
    int n_choices, i;

    char *symbol = ""; // Initialize symbol with an empty string

    /* Check command-line arguments */
    if (argc == 2)
    {
        symbol = argv[1]; // Save the symbol entered from the command-line argument
    }
    else if (argc > 2)
    {
        printf("Error: Too many command-line arguments.\n");
        return 1;
    }

    /* Initialize curses */
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    /* Create items */
    n_choices = ARRAY_SIZE(choices);
    my_items = (ITEM **)calloc(n_choices, sizeof(ITEM *));
    for (i = 0; i < n_choices; ++i)
        my_items[i] = new_item(choices[i], choices[i]);

    /* Create menu */
    my_menu = new_menu((ITEM **)my_items);

    /* Create the window to be associated with the menu */
    my_menu_win = newwin(10, 40, 0, 0); // Changed position to (0, 0)
    keypad(my_menu_win, TRUE);

    /* Set main window and sub window */
    set_menu_win(my_menu, my_menu_win);
    set_menu_sub(my_menu, derwin(my_menu_win, 6, 38, 3, 1));

    /* Set menu mark to the string " >> " */
    set_menu_mark(my_menu, " >> ");

    /* Print a border around the main window */
    box(my_menu_win, 0, 0);

    /* Print a title */
    print_in_middle(my_menu_win, 1, 0, 40, "Crypto Menu");

    /* Post the menu */
    post_menu(my_menu);
    wrefresh(my_menu_win);

    /* Create a blank window for the content */
    WINDOW *content_win = newwin(LINES - 10, COLS, 10, 0);
    box(content_win, 0, 0);
    wrefresh(content_win);

    int scroll_lines = 0; // Variable to track scroll lines

    while ((c = wgetch(my_menu_win)) != KEY_F(1))
    {
        switch (c)
        {
        case KEY_DOWN:
            menu_driver(my_menu, REQ_DOWN_ITEM);
            break;
        case KEY_UP:
            menu_driver(my_menu, REQ_UP_ITEM);
            break;
        case KEY_NPAGE: // Page Down
            scroll_lines += (LINES - 10) / 2; // Scroll by half a page
            scroll_content_window(content_win, scroll_lines);
            break;
        case KEY_PPAGE: // Page Up
            scroll_lines -= (LINES - 10) / 2; // Scroll by half a page
            if (scroll_lines < 0)
                scroll_lines = 0;
            scroll_content_window(content_win, scroll_lines);
            break;
        case '\n':
        case '\r':
            mvprintw(LINES - 1, 0, "Please wait, processing request...");
            refresh();
            usleep(2500000); // Sleep for 2.5 seconds
            mvprintw(LINES - 1, 0, "                        "); // Clear the message
            refresh();

            int selected_option = item_index(current_item(my_menu));
            switch (selected_option)
            {
            case 0:
                // Run ./src/market_analyzer with symbol as argument
                pid_t pid_market_analyzer = fork();
                if (pid_market_analyzer == 0)
                {
                    // Redirect output to /dev/null
                    int dev_null = open("/dev/null", O_WRONLY);
                    if (dev_null == -1) {
                        perror("open /dev/null failed");
                        exit(1);
                    }
                    if (dup2(dev_null, STDOUT_FILENO) == -1) {
                        perror("dup2 failed");
                        exit(1);
                    }
                    close(dev_null);
                    
                    execl("./src/market_analyzer", "market_analyzer", symbol, (char *)NULL);
                    perror("execl failed");
                    exit(1);
                }
                else if (pid_market_analyzer > 0)
                {
                    // Wait for market_analyzer to finish
                    int status;
                    waitpid(pid_market_analyzer, &status, 0);
                    if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
                    {
                        // Print success message if market_analyzer executed successfully
                        mvwprintw(content_win, 2, 2, "Market analysis completed.\n"); // Adjust position
                        wrefresh(content_win);

                        // Read and print the content of result_data/data.txt
                        FILE *file = fopen("result_data/data.txt", "r");
                        if (file)
                        {
                            char line[100];
                            while (fgets(line, sizeof(line), file))
                            {
                                wprintw(content_win, "%s", line);
                            }
                            fclose(file);
                            wrefresh(content_win);
                        }
                        else
                        {
                            mvwprintw(content_win, 4, 2, "Error: Failed to open result_data/data.txt\n"); // Adjust position
                            wrefresh(content_win);
                        }
                    }
                    else
                    {
                        mvwprintw(content_win, 4, 2, "Error: Failed to execute market_analyzer.\n"); // Adjust position
                        wrefresh(content_win);
                    }
                }
                else
                {
                    perror("fork failed");
                }
                break;
            case 1:
                // Run ./src/crypto_price with symbol as argument
                pid_t pid_crypto_price = fork();
                if (pid_crypto_price == 0)
                {
                    // Redirect output to /dev/null
                    int dev_null = open("/dev/null", O_WRONLY);
                    if (dev_null == -1) {
                        perror("open /dev/null failed");
                        exit(1);
                    }
                    if (dup2(dev_null, STDOUT_FILENO) == -1) {
                        perror("dup2 failed");
                        exit(1);
                    }
                    close(dev_null);
                    
                    execl("./src/crypto_price", "crypto_price", symbol, (char *)NULL);
                    perror("execl failed");
                    exit(1);
                }
                else if (pid_crypto_price > 0)
                {
                    // Wait for crypto_price to finish
                    int status;
                    waitpid(pid_crypto_price, &status, 0);
                    if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
                    {
                        // Print success message if crypto_price executed successfully
                        mvwprintw(content_win, 2, 2, "Crypto price data retrieved successfully.\n"); // Adjust position
                        wrefresh(content_win);

                        // Read and print the content of result_data/data.txt
                        FILE *file = fopen("src/target/price.txt", "r");
                        if (file)
                        {
                            char line[100];
                            while (fgets(line, sizeof(line), file))
                            {
                                wprintw(content_win, "%s", line);
                            }
                            fclose(file);
                            wrefresh(content_win);
                        }
                        else
                        {
                            mvwprintw(content_win, 4, 2, "Error: Failed to open lib/price.txt\n"); // Adjust position
                            wrefresh(content_win);
                        }
                    }
                    else
                    {
                        mvwprintw(content_win, 4, 2, "Error: Failed to execute crypto_price.\n"); // Adjust position
                        wrefresh(content_win);
                    }
                }
                else
                {
                    perror("fork failed");
                }
                break;

            case 2:
                // Run python3 src/graph.py
                pid_t pid_graph = fork();
                if (pid_graph == 0)
                {
                    execlp("python3", "python3", "src/graph.py", (char *)NULL);
                    perror("execlp failed");
                    exit(1);
                }
                else if (pid_graph > 0)
                {
                    // Print success message
                    wprintw(content_win, "Opening graph...\n");
                    wrefresh(content_win);
                }
                else
                {
                    perror("fork failed");
                }
                break;
            case 3:
                // Clear the content window
                wclear(content_win);
                wrefresh(content_win);
                break;
            case 4:
                // Exit the program
                goto end_loop;
                break;
            }
            break;
        }
        wrefresh(my_menu_win);
    }

end_loop:

    /* Unpost and free all the memory taken up */
    unpost_menu(my_menu);
    free_menu(my_menu);
    for (i = 0; i < n_choices; ++i)
        free_item(my_items[i]);
    endwin();
}

void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string)
{
    int length, x, y;
    float temp;

    if (win == NULL)
        win = stdscr;
    getyx(win, y, x);
    if (startx != 0)
        x = startx;
    if (starty != 0)
        y = starty;
    if (width == 0)
        width = 80;

    length = strlen(string);
    temp = (width - length) / 2;
    x = startx + (int)temp;
    mvwprintw(win, y, x, "%s", string);
    refresh();
}

void scroll_content_window(WINDOW *content_win, int scroll_lines)
{
    if (scroll_lines < 0)
        scroll_lines = 0;
    wclear(content_win);
    box(content_win, 0, 0);
    wrefresh(content_win);
    wmove(content_win, 1, 1);
    for (int i = scroll_lines; i < scroll_lines + (LINES - 10) && i < LINES - 10; i++)
    {
        mvwprintw(content_win, i - scroll_lines + 1, 1, "Line %d\n", i); // Replace this line with your content printing logic
    }
    wrefresh(content_win);
}
