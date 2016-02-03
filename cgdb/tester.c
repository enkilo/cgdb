/* tester.c:
 * ---------
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

/* System Includes */
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <curses.h>
#if HAVE_CURSES_H
#include <curses.h>
#elif HAVE_NCURSES_CURSES_H
#include <ncurses/curses.h>
#endif /* HAVE_CURSES_H */

/* Colors */
#define CGDB_COLOR_BLACK            0
#define CGDB_COLOR_GREEN            1
#define CGDB_COLOR_RED              2
#define CGDB_COLOR_CYAN             3
#define CGDB_COLOR_WHITE            4
#define CGDB_COLOR_MAGENTA          5
#define CGDB_COLOR_BLUE             6
#define CGDB_COLOR_YELLOW           7

#define CGDB_COLOR_INVERSE_GREEN    8
#define CGDB_COLOR_INVERSE_RED      9
#define CGDB_COLOR_INVERSE_CYAN     10
#define CGDB_COLOR_INVERSE_WHITE    11
#define CGDB_COLOR_INVERSE_MAGENTA  12
#define CGDB_COLOR_INVERSE_BLUE     13
#define CGDB_COLOR_INVERSE_YELLOW   14
#define CGDB_COLOR_STATUS_BAR       15

/* Local Includes */
#include "sources.h"
#include "cgdb.h"
#include "rline.h"

/* --------------- */
/* Local Variables */
/* --------------- */
struct sviewer *src_win = 0;
struct tgdb *tgdb;              /* The main TGDB context */
struct tgdb_request *last_request = NULL;

struct kui_manager *kui_ctx = NULL; /* The key input package */
struct kui_map_set *kui_map = NULL;
struct kui_map_set *kui_imap = NULL;

static int curses_initialized = 0;  /* Set when curses has been started */
static int curses_colors = 0;   /* Set if terminal supports color */
static char *my_name = NULL;    /* Name of this application (argv[0]) */

/**
 * This allows CGDB to know if it is acceptable to read more input from
 * the KUI at this particular moment. Under certain circumstances, CGDB may 
 * have to complete a particular communication message to GDB, in while doing
 * so, may not want to process's the users keystrokes. This flag is purely for
 * CGDB to keep track of if it wants to read from the KUI. The KUI is always
 * ready.
 *
 * If kui_input_acceptable is set to 1, then input can be read from
 * the kui. Otherwise, some part of CGDB is waiting for more events to happen.
 *
 * For example, when the user types 'o' at the CGDB source window, the
 * user is requesting the file dialog to open. CGDB must first ask GDB for 
 * the list of files. While it is retreiving these files, CGDB shouldn't 
 * shoot through the rest of the kui keys available. If the user had
 * typed 'o /main' they would want to open the file dialog and start 
 * searching for a file that had the substring 'main' in it. So, CGDB must
 * first ensure all the files are retrieved, displayed in the file
 * dialog, and ensure the file dialog is ready to recieve keys from the 
 * user before the input is allowed to hit the file dialog.
 */
int kui_input_acceptable = 1;

int resize_pipe[2] = { -1, -1 };

static struct rline *rline;

/**
 * If this is 1, then CGDB is performing a tab completion in the GDB window.
 * When this is true, there can be no data passed to readline. The completion
 * must be displayed to the user first.
 */
static int is_tab_completing = 0;

/* Original terminal attributes */
static struct termios term_attributes;

/* --------- */
/* Functions */
/* --------- */

int init_curses()
{
    initscr();                  /* Start curses mode */
    cbreak();                   /* Line buffering disabled */
    noecho();                   /* Do not echo characters typed by user */
    keypad(stdscr, TRUE);       /* Translate arrow keys, Fn keys, etc. */

    if ((curses_colors = has_colors())) {
        start_color();
        init_pair(CGDB_COLOR_BLACK, COLOR_BLACK, COLOR_BLACK);
        init_pair(CGDB_COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
        init_pair(CGDB_COLOR_RED, COLOR_RED, COLOR_BLACK);
        init_pair(CGDB_COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
        init_pair(CGDB_COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
        init_pair(CGDB_COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(CGDB_COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
        init_pair(CGDB_COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
    }

    curses_initialized = 1;
    return 0;
}

/* cleanup: Invoked by the various err_xxx funtions when dying.
 * -------- */
void cleanup()
{
    /* Shut down curses cleanly */
    if (curses_initialized) {
        endwin();
    }
}

/**
 * If the TGDB instance is not busy, it will run the requested command.
 * Otherwise, the command will get queued to run later.
 *
 * \param tgdb
 * An instance of the tgdb library to operate on.
 *
 * \param request
 * The requested command to have TGDB process.
 *
 * \return
 * 0 on success or -1 on error
 */
int handle_request(struct tgdb *tgdb, struct tgdb_request *request)
{
    int val, is_busy;

    if (!tgdb || !request)
        return -1;

    val = tgdb_is_busy(tgdb, &is_busy);
    if (val == -1)
        return -1;

    if (is_busy)
        tgdb_queue_append(tgdb, request);
    else {
        last_request = request;
        tgdb_process_command(tgdb, request);
    }
    
    return 0;
}

int err_quit(const char* msg, ...) {
  va_list args;
  va_start(args,msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  exit(1);
}

int run_shell_command(const char *command)
{
    int rv;

    /* Cleanly scroll the screen up for a prompt */
    scrl(1);
    move(LINES - 1, 0);
    printf("\n");

    /* Put the terminal in cooked mode and turn on echo */
    endwin();
    tty_set_attributes(STDIN_FILENO, &term_attributes);

    /* NULL or empty string means invoke user's shell */
    if (command == NULL || strlen(command) == 0) {

        /* Check for SHELL environment variable */
        char *shell = getenv("SHELL");

        if (shell == NULL) {
            /* Run /bin/sh instead */
            rv = system("/bin/sh");
        } else {
            rv = system(shell);
        }
    } else {
        /* Execute the command passed in via system() */
        rv = system(command);
    }

    /* Press any key to continue... */
    fprintf(stderr, "Hit ENTER to continue...");
    while (fgetc(stdin) != '\n') {
    }

    /* Turn off echo and put the terminal back into raw mode */
    tty_cbreak(STDIN_FILENO, &term_attributes);
    if_draw();

    return rv;
}

void rl_resize(int rows, int cols)
{
    rline_resize_terminal_and_redisplay(rline, rows, cols);
}

void rl_sigint_recved(void)
{
    rline_clear(rline);
    is_tab_completing = 0;
}

int main(int argc, char *argv[])
{
    /* NOTE:  This whole main is pretty much garbage right now.  Its only
     *        purpose is to show 2 source files using the sources module,
     *        for testing purposes only. */

    int c = 0;
    int num = 12;
    int line1 = 7, line2 = 7, *line = &line1;
    char *bar = "________________________________________"
            "________________________________________";

    /* Set up some data */
    my_name = argv[0];

    if (argc < 3)
        err_quit("  USAGE: %s <file 1> <file 2>", my_name);

    /* Initialize the display */
    if (init_curses())
        err_quit("%s: Unable to initialize the curses library\n", my_name);

    if_init();


    source_add(src_win,argv[1]);
    source_add(src_win,argv[2]);
    move(0, 0);
    attron(A_BOLD);
    wprintw(stdscr, "ESC                  : Exit\n");
    wprintw(stdscr, "TAB                  : Switch panes\n");
    wprintw(stdscr, "UP, DOWN, PGUP, PGDN : Scroll current pane\n");
    mvwprintw(stdscr, 4, 0, bar);
    mvwprintw(stdscr, 17, 0, bar);
    attroff(A_BOLD);
    mvwprintw(stdscr, 19, 0, bar);
    mvwprintw(stdscr, 32, 0, bar);
    do {
        switch (c) {
            case 9:
                if (line == &line1) {
                    mvwprintw(stdscr, 4, 0, bar);
                    mvwprintw(stdscr, 17, 0, bar);
                    attron(A_BOLD);
                    mvwprintw(stdscr, 19, 0, bar);
                    mvwprintw(stdscr, 32, 0, bar);
                    attroff(A_BOLD);
                    line = &line2;
                } else {
                    attron(A_BOLD);
                    mvwprintw(stdscr, 4, 0, bar);
                    mvwprintw(stdscr, 17, 0, bar);
                    attroff(A_BOLD);
                    mvwprintw(stdscr, 19, 0, bar);
                    mvwprintw(stdscr, 32, 0, bar);
                    line = &line1;
                }
                break;
            case KEY_DOWN:
                (*line)++;
                break;
            case KEY_UP:
                (*line)--;
                break;
            case KEY_PPAGE:
                (*line) -= 10;
                break;
            case KEY_NPAGE:
                (*line) += 10;
                break;
        }
        if (*line > source_length(src_win,argv[line == &line1 ? 1 : 2]) - 5)
            *line = source_length(src_win,argv[line == &line1 ? 1 : 2]) - 5;
        if (*line < 7)
            *line = 7;
        move(5, 0);
        //source_display(argv[1], stdscr, line1, num);
        source_display(src_win,0);
        move(20, 0);
        //source_display(argv[2], stdscr, line2, num);
        source_display(src_win,1);
    } while ((c = getch()) != 27);
    source_del(src_win, argv[1]);
    source_del(src_win, argv[2]);

    cleanup();
    return 0;
}
