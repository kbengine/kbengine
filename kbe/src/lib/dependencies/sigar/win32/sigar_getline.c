/*
 * Copyright (C) 1991, 1992 by Chris Thewalt (thewalt@ce.berkeley.edu)
 *
 * Permission to use, copy, modify, and distribute this software
 * for any purpose and without fee is hereby granted, provided
 * that the above copyright notices appear in all copies and that both the
 * copyright notice and this permission notice appear in supporting
 * documentation.  This software is provided "as is" without express or
 * implied warranty.
 */
/*
*************************** Motivation **********************************

Many interactive programs read input line by line, but would like to
provide line editing and history functionality to the end-user that
runs the program.

The input-edit package provides that functionality.  As far as the
programmer is concerned, the program only asks for the next line
of input. However, until the user presses the RETURN key they can use
emacs-style line editing commands and can traverse the history of lines
previously typed.

Other packages, such as GNU's readline, have greater capability but are
also substantially larger.  Input-edit is small, since it uses neither
stdio nor any termcap features, and is also quite portable.  It only uses
\b to backspace and \007 to ring the bell on errors.  Since it cannot
edit multiple lines it scrolls long lines left and right on the same line.

Input edit uses classic (not ANSI) C, and should run on any Unix
system (BSD or SYSV), PC's with the MSC compiler, or Vax/VMS (untested by me).
Porting the package to new systems basicaly requires code to read a
character when it is typed without echoing it, everything else should be OK.

I have run the package on:

        DECstation 5000, Ultrix 4.2 with cc and gcc
        Sun Sparc 2, SunOS 4.1.1, with cc
        SGI Iris, IRIX System V.3, with cc
        PC, DRDOS 5.0, with MSC 6.0

The description below is broken into two parts, the end-user (editing)
interface and the programmer interface.  Send bug reports, fixes and
enhancements to:

Chris Thewalt (thewalt@ce.berkeley.edu)
2/4/92

PS: I don't have, and don't want to add, a vi mode, sorry.

************************** End-User Interface ***************************

Entering printable keys generally inserts new text into the buffer (unless
in overwrite mode, see below).  Other special keys can be used to modify
the text in the buffer.  In the description of the keys below, ^n means
Control-n, or holding the CONTROL key down while pressing "n". M-B means
Meta-B (or Alt-B). Errors will ring the terminal bell.

^A/^E   : Move cursor to beginning/end of the line.
^F/^B   : Move cursor forward/backward one character.
^D      : Delete the character under the cursor.
^H, DEL : Delete the character to the left of the cursor.
^K      : Kill from the cursor to the end of line.
^L      : Redraw current line.
^O      : Toggle overwrite/insert mode. Initially in insert mode. Text
          added in overwrite mode (including yanks) overwrite
          existing text, while insert mode does not overwrite.
^P/^N   : Move to previous/next item on history list.
^R/^S   : Perform incremental reverse/forward search for string on
          the history list.  Typing normal characters adds to the current
          search string and searches for a match. Typing ^R/^S marks
          the start of a new search, and moves on to the next match.
          Typing ^H or DEL deletes the last character from the search
          string, and searches from the starting location of the last search.
          Therefore, repeated DEL's appear to unwind to the match nearest
          the point at which the last ^R or ^S was typed.  If DEL is
          repeated until the search string is empty the search location
          begins from the start of the history list.  Typing ESC or
          any other editing character accepts the current match and
          loads it into the buffer, terminating the search.
^T      : Toggle the characters under and to the left of the cursor.
^U      : Kill from beginning to the end of the line.
^Y      : Yank previously killed text back at current location.  Note that
          this will overwrite or insert, depending on the current mode.
M-F/M-B : Move cursor forward/backward one word.
M-D     : Delete the word under the cursor.
^SPC    : Set mark.
^W      : Kill from mark to point.
^X      : Exchange mark and point.
TAB     : By default adds spaces to buffer to get to next TAB stop
          (just after every 8th column), although this may be rebound by the
          programmer, as described below.
NL, CR  : returns current buffer to the program.

DOS and ANSI terminal arrow key sequences are recognized, and act like:

  up    : same as ^P
  down  : same as ^N
  left  : same as ^B
  right : same as ^F

************************** Programmer Interface ***************************

The programmer accesses input-edit through five functions, and optionally
through three additional function pointer hooks.  The five functions are:

char *Getline(char *prompt)

        Prints the prompt and allows the user to edit the current line. A
        pointer to the line is returned when the user finishes by
        typing a newline or a return.  Unlike GNU readline, the returned
        pointer points to a static buffer, so it should not be free'd, and
        the buffer contains the newline character.  The user enters an
        end-of-file by typing ^D on an empty line, in which case the
        first character of the returned buffer is '\0'.  Getline never
        returns a NULL pointer.  The getline function sets terminal modes
        needed to make it work, and resets them before returning to the
        caller.  The getline function also looks for characters that would
        generate a signal, and resets the terminal modes before raising the
        signal condition.  If the signal handler returns to getline,
        the screen is automatically redrawn and editing can continue.
        Getline now requires both the input and output stream be connected
        to the terminal (not redirected) so the main program should check
        to make sure this is true.  If input or output have been redirected
        the main program should use buffered IO (stdio) rather than
        the slow 1 character read()s that getline uses (note: this limitation
        has been removed).

char *Getlinem(int mode, char *prompt)

        mode: -1 = init, 0 = line mode, 1 = one char at a time mode, 2 = cleanup

        More specialized version of the previous function. Depending on
        the mode, it behaves differently. Its main use is to allow
        character by character input from the input stream (useful when
        in an X eventloop). It will return NULL as long as no newline
        has been received. Its use is typically as follows:
        1) In the program initialization part one calls: Getlinem(-1,"prompt>")
        2) In the X inputhandler: if ((line = Getlinem(1,NULL))) {
        3) In the termination routine: Getlinem(2,NULL)
        With mode=0 the function behaves exactly like the previous function.

void Gl_config(const char *which, int value)

        Set some config options. Which can be:
          "noecho":  do not echo characters (used for passwd input)
          "erase":   do erase line after return (used for text scrollers)

void Gl_setwidth(int width)

        Set the width of the terminal to the specified width. The default
        width is 80 characters, so this function need only be called if the
        width of the terminal is not 80.  Since horizontal scrolling is
        controlled by this parameter it is important to get it right.

void Gl_histinit(char *file)

        This function reads a history list from file. So lines from a
        previous session can be used again.

void Gl_histadd(char *buf)

        The Gl_histadd function checks to see if the buf is not empty or
        whitespace, and also checks to make sure it is different than
        the last saved buffer to avoid repeats on the history list.
        If the buf is a new non-blank string a copy is made and saved on
        the history list, so the caller can re-use the specified buf.

The main loop in testgl.c, included in this directory, shows how the
input-edit package can be used:

extern char *Getline();
extern void  Gl_histadd();
main()
{
    char *p;
    Gl_histinit(".hist");
    do {
        p = Getline("PROMPT>>>> ");
        Gl_histadd(p);
        fputs(p, stdout);
    } while (*p != 0);
}

In order to allow the main program to have additional access to the buffer,
to implement things such as completion or auto-indent modes, three
function pointers can be bound to user functions to modify the buffer as
described below.  By default gl_in_hook and gl_out_hook are set to NULL,
and gl_tab_hook is bound to a function that inserts spaces until the next
logical tab stop is reached.  The user can reassign any of these pointers
to other functions.  Each of the functions bound to these hooks receives
the current buffer as the first argument, and must return the location of
the leftmost change made in the buffer.  If the buffer isn't modified the
functions should return -1.  When the hook function returns the screen is
updated to reflect any changes made by the user function.

int (*gl_in_hook)(char *buf)

        If gl_in_hook is non-NULL the function is called each time a new
        buffer is loaded. It is called when getline is entered, with an
        empty buffer, it is called each time a new buffer is loaded from
        the history with ^P or ^N, and it is called when an incremental
        search string is accepted (when the search is terminated). The
        buffer can be modified and will be redrawn upon return to Getline().

int (*gl_out_hook)(char *buf)

        If gl_out_hook is non-NULL it is called when a line has been
        completed by the user entering a newline or return. The buffer
        handed to the hook does not yet have the newline appended. If the
        buffer is modified the screen is redrawn before getline returns the
        buffer to the caller.

int (*gl_tab_hook)(char *buf, int prompt_width, int *cursor_loc)

        If gl_tab_hook is non-NULL, it is called whenever a tab is typed.
        In addition to receiving the buffer, the current prompt width is
        given (needed to do tabbing right) and a pointer to the cursor
        offset is given, where a 0 offset means the first character in the
        line.  Not only does the cursor_loc tell the programmer where the
        TAB was received, but it can be reset so that the cursor will end
        up at the specified location after the screen is redrawn.
*/

/* forward reference needed for gl_tab_hook */
static int gl_tab(char *buf, int offset, int *loc);

/********************* exported interface ********************************/

static int (*gl_in_hook)(char *buf) = 0;
static int (*gl_out_hook)(char *buf) = 0;
static int (*gl_tab_hook)(char *buf, int prompt_width, int *loc) = gl_tab;

/******************** imported interface *********************************/
#ifdef DMALLOC
/* reports leaks, which is the history buffer.  dont care */ 
#undef DMALLOC
#endif
#include "sigar_getline.h"
#include "sigar_private.h"
#include "sigar_util.h"
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#pragma warning(disable:4996)
#pragma warning(disable:4819)
#pragma warning(disable:4049)
#pragma warning(disable:4217)
#pragma warning(disable:4013)
#pragma warning(disable:4018)
#pragma warning(disable:4244)
#pragma warning(disable:4133)
#pragma warning(disable:4307)
#pragma warning(disable:4293)

/******************** internal interface *********************************/

static char   *sigar_getlinem(int mode, char *prompt); /* allows reading char by char */

static void    sigar_getline_config(const char *which, int value); /* set some options */

static void sigar_getline_clear_screen(void);

#define BUF_SIZE 8096

static int      gl_init_done = -1;      /* terminal mode flag  */
static int      gl_notty = 0;           /* 1 when not a tty */
static int      gl_eof = 0;             /* 1 when not a tty and read() == -1 */
static int      gl_termw = 80;          /* actual terminal width */
static int      gl_scroll = 27;         /* width of EOL scrolling region */
static int      gl_width = 0;           /* net size available for input */
static int      gl_extent = 0;          /* how far to redraw, 0 means all */
static int      gl_overwrite = 0;       /* overwrite mode */
static int      gl_no_echo = 0;         /* do not echo input characters */
static int      gl_passwd = 0;          /* do not echo input characters */
static int      gl_erase_line = 0;      /* erase line before returning */
static int      gl_pos, gl_cnt = 0;     /* position and size of input */
static char     gl_buf[BUF_SIZE];       /* input buffer */
static char     gl_killbuf[BUF_SIZE]=""; /* killed text */
static char    *gl_prompt;              /* to save the prompt string */
static char     gl_intrc = 0;           /* keyboard SIGINT char */
static char     gl_quitc = 0;           /* keyboard SIGQUIT char */
static char     gl_suspc = 0;           /* keyboard SIGTSTP char */
static char     gl_dsuspc = 0;          /* delayed SIGTSTP char */
static int      gl_search_mode = 0;     /* search mode flag */
static int      gl_bell_enabled = 0;    /* bell mode */
static int      gl_savehist = 0;        /* # of lines to save in hist file */
static char     gl_histfile[256];       /* name of history file */

static void     gl_init();              /* prepare to edit a line */
static void     gl_bell();              /* ring bell */
static void     gl_cleanup();           /* to undo gl_init */
static void     gl_char_init();         /* get ready for no echo input */
static void     gl_char_cleanup();      /* undo gl_char_init */

static void     gl_addchar(int c);      /* install specified char */
static void     gl_del(int loc);        /* del, either left (-1) or cur (0) */
static void     gl_error(char *buf);    /* write error msg and die */
static void     gl_fixup(char *p, int c, int cur); /* fixup state variables and screen */
static int      gl_getc();              /* read one char from terminal */
static void     gl_kill();              /* delete to EOL */
static void     gl_newline();           /* handle \n or \r */
static void     gl_putc(int c);         /* write one char to terminal */
static void     gl_puts(char *buf);     /* write a line to terminal */
static void     gl_transpose();         /* transpose two chars */
static void     gl_yank();              /* yank killed text */

static int      is_whitespace(char c);  /* "whitespace" very loosely interpreted */
static void     gl_back_1_word();       /* move cursor back one word */
static void     gl_kill_1_word();       /* kill to end of word */
static void     gl_kill_region(int i, int j); /* kills from i to j */
static void     gl_fwd_1_word();        /* move cursor forward one word */
static void     gl_set_mark();          /* sets mark to be at point */
static void     gl_exch();              /* exchanges point and mark */
static void     gl_wipe();              /* kills from mark to point */
static int      gl_mark = -1;           /* position of mark. gl_mark<0 if not set */

static void     hist_init();            /* initializes hist pointers */
static char    *hist_next();            /* return ptr to next item */
static char    *hist_prev();            /* return ptr to prev item */
static char    *hist_save(char *p);     /* makes copy of a string, without NL */

static void     search_addchar(int c);  /* increment search string */
static void     search_term();          /* reset with current contents */
static void     search_back(int s);     /* look back for current string */
static void     search_forw(int s);     /* look forw for current string */

/************************ nonportable part *********************************/

#ifdef MSDOS
#include <bios.h>
#endif

#ifdef WIN32
#  define MSDOS
#  include <io.h>
#  include <windows.h>
#endif /* WIN32 */

#ifdef __MWERKS__
#define R__MWERKS
#endif

#ifdef R__MWERKS
#  include <unistd.h>
#endif

#if defined(_AIX) || defined(__Lynx__) || defined(__APPLE__)
#define unix
#endif

#if defined(__hpux) || defined(__osf__)       /* W.Karig@gsi.de */
#ifndef unix
#define unix
#endif
#endif

#ifdef unix
#include <unistd.h>
#if !defined(__osf__) && !defined(_AIX)       /* W.Karig@gsi.de */
#include <sys/ioctl.h>
#endif

#if defined(__linux__) && defined(__powerpc__)
#   define R__MKLINUX       // = linux on PowerMac
#endif
#if defined(__linux__) && defined(__alpha__)
#   define R__ALPHALINUX    // = linux on Alpha
#endif

#if defined(TIOCGETP) && !defined(__sgi) && !defined(R__MKLINUX) && \
   !defined(R__ALPHALINUX)  /* use BSD interface if possible */
#include <sgtty.h>
static struct sgttyb   new_tty, old_tty;
static struct tchars   tch;
static struct ltchars  ltch;
#else
#ifdef SIGTSTP          /* need POSIX interface to handle SUSP */
#include <termios.h>
#if defined(__sun) || defined(__sgi) || defined(R__MKLINUX) || \
    defined(R__ALPHALINUX)
#undef TIOCGETP         /* Solaris and SGI define TIOCGETP in <termios.h> */
#undef TIOCSETP
#endif
static struct termios  new_termios, old_termios;
#else                   /* use SYSV interface */
#include <termio.h>
static struct termio   new_termio, old_termio;
#endif
#endif
#endif  /* unix */

#ifdef VMS
#include <descrip.h>
#include <ttdef.h>
#include <iodef.h>
#include <starlet.h>
#include <unistd.h>
#include unixio

static int   setbuff[2];             /* buffer to set terminal attributes */
static short chan = -1;              /* channel to terminal */
struct dsc$descriptor_s descrip;     /* VMS descriptor */
#endif

static void
sigar_getline_config(const char *which, int value)
{
   if (strcmp(which, "noecho") == 0)
      gl_no_echo = value;
   else if (strcmp(which, "erase") == 0)
      gl_erase_line = value;
   else
      printf("gl_config: %s ?\n", which);
}

static void
gl_char_init()                  /* turn off input echo */
{
    if (gl_notty) return;
#ifdef unix
#ifdef TIOCGETP                 /* BSD */
    ioctl(0, TIOCGETC, &tch);
    ioctl(0, TIOCGLTC, &ltch);
    gl_intrc = tch.t_intrc;
    gl_quitc = tch.t_quitc;
    gl_suspc = ltch.t_suspc;
    gl_dsuspc = ltch.t_dsuspc;
    ioctl(0, TIOCGETP, &old_tty);
    new_tty = old_tty;
    new_tty.sg_flags |= RAW;
    new_tty.sg_flags &= ~ECHO;
    ioctl(0, TIOCSETP, &new_tty);
#else
#ifdef SIGTSTP                  /* POSIX */
    tcgetattr(0, &old_termios);
    gl_intrc = old_termios.c_cc[VINTR];
    gl_quitc = old_termios.c_cc[VQUIT];
#ifdef VSUSP
    gl_suspc = old_termios.c_cc[VSUSP];
#endif
#ifdef VDSUSP
    gl_dsuspc = old_termios.c_cc[VDSUSP];
#endif
    new_termios = old_termios;
    new_termios.c_iflag &= ~(BRKINT|ISTRIP|IXON|IXOFF);
    new_termios.c_iflag |= (IGNBRK|IGNPAR);
    new_termios.c_lflag &= ~(ICANON|ISIG|IEXTEN|ECHO);
    new_termios.c_cc[VMIN] = 1;
    new_termios.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &new_termios);
#else                           /* SYSV */
    ioctl(0, TCGETA, &old_termio);
    gl_intrc = old_termio.c_cc[VINTR];
    gl_quitc = old_termio.c_cc[VQUIT];
    new_termio = old_termio;
    new_termio.c_iflag &= ~(BRKINT|ISTRIP|IXON|IXOFF);
    new_termio.c_iflag |= (IGNBRK|IGNPAR);
    new_termio.c_lflag &= ~(ICANON|ISIG|ECHO);
    new_termio.c_cc[VMIN] = 1;
    new_termio.c_cc[VTIME] = 0;
    ioctl(0, TCSETA, &new_termio);
#endif
#endif
#endif /* unix */

#ifdef MSDOS
    gl_intrc = 'C' - '@';
    gl_quitc = 'Q' - '@';
//    gl_suspc = ltch.t_suspc;
#endif /* MSDOS */

#ifdef R__MWERKS
    gl_intrc = 'C' - '@';
    gl_quitc = 'Q' - '@';
#endif

#ifdef vms
    descrip.dsc$w_length  = strlen("tt:");
    descrip.dsc$b_dtype   = DSC$K_DTYPE_T;
    descrip.dsc$b_class   = DSC$K_CLASS_S;
    descrip.dsc$a_pointer = "tt:";
    (void)sys$assign(&descrip,&chan,0,0);
    (void)sys$qiow(0,chan,IO$_SENSEMODE,0,0,0,setbuff,8,0,0,0,0);
    setbuff[1] |= TT$M_NOECHO;
    (void)sys$qiow(0,chan,IO$_SETMODE,0,0,0,setbuff,8,0,0,0,0);
#endif /* vms */
}

static void
gl_char_cleanup()               /* undo effects of gl_char_init */
{
    if (gl_notty) return;
#ifdef unix
#ifdef TIOCSETP         /* BSD */
    ioctl(0, TIOCSETP, &old_tty);
#else
#ifdef SIGTSTP          /* POSIX */
    tcsetattr(0, TCSANOW, &old_termios);
#else                   /* SYSV */
    ioctl(0, TCSETA, &old_termio);
#endif
#endif
#endif /* unix */

#ifdef vms
    setbuff[1] &= ~TT$M_NOECHO;
    (void)sys$qiow(0,chan,IO$_SETMODE,0,0,0,setbuff,8,0,0,0,0);
    sys$dassgn(chan);
    chan = -1;
#endif
}

#if defined(MSDOS) && !defined(WIN32)
// +DECK, PAUSE, T=XCC, IF=WINNT. (from KERNDOS.CAR )
#  include <conio.h>
   int pause_()
   {
      int first_char;
        first_char = _getch();
        if (first_char == 0 || first_char == 0xE0) first_char = -_getch();
        return first_char;
   }
#endif

#if defined(MSDOS) && defined(WIN32)
//______________________________________________________________________________
int pause_()
{
 static HANDLE hConsoleInput = NULL;
 static iCharCount = 0;
 static int chLastChar = 0;

 DWORD cRead;

 INPUT_RECORD pirBuffer;
 KEY_EVENT_RECORD *KeyEvent= (KEY_EVENT_RECORD *)&(pirBuffer.Event);

 if (!hConsoleInput) hConsoleInput = GetStdHandle(STD_INPUT_HANDLE);

 if (iCharCount) iCharCount--;      // Whether several symbols had been read
 else {
   chLastChar = 0;
   while (chLastChar == 0) {
     if (!ReadConsoleInput(hConsoleInput,       // handle of a console input buffer
                           &pirBuffer,          // address of the buffer for read data
                           1,                   // number of records to read
                           &cRead               // address of number of records read
        )) return 0;

     if (pirBuffer.EventType == KEY_EVENT  && KeyEvent->bKeyDown == TRUE){
         iCharCount = KeyEvent->wRepeatCount - 1;
         chLastChar = ((int) (KeyEvent->uChar).AsciiChar & 0xffff);
         if (chLastChar)
              OemToCharBuff((char const *)&chLastChar,(char *)&chLastChar,1);
         else
              chLastChar = - (KeyEvent->wVirtualScanCode);
//            chLastChar = - (KeyEvent->wVirtualKeyCode);
     }
   }
 }
 return chLastChar;

}
#endif

static int
gl_getc()
/* get a character without echoing it to screen */
{
#ifdef MSDOS
# define k_ctrl_C   3
# define k_ctrl_Z  26
# define k_ctrl_Q  17
# define k_ctrl_K  11
# define k_rt_arr -77
# define k_lt_arr -75
# define k_up_arr -72
# define k_dn_arr -80
# define k_PGUP   -73
# define k_PGDW   -81
# define k_HOME   -71
# define k_END    -79
# define k_INS    -82
# define k_DEL    -83
# define k_ENTER   13
# define k_CR      13
# define k_BS       8
# define k_ESC     27
# define k_alt_H  -35
# define k_beep     7
# ifndef WIN32
    int get_cursor__(int *,int *);
    int display_off__(int *);
    int display_on__();
    int locate_(int *,int *);
    int ixc, iyc;
# endif
    int pause_();
#endif

    int c;

#if defined(unix)
    unsigned char ch;
    while ((c = (read(0, &ch, 1) > 0) ? ch : -1) == -1 && errno == EINTR)
       errno = 0;
#endif

#if defined(R__MWERKS)
    c = getchar();
#endif

#ifdef MSDOS
    c = pause_();
     if (c < 0) {
         switch (c) {
           case k_up_arr: c =  'P' - '@';   /* up -> ^P = 16 */
             break;
           case k_dn_arr: c =  'N' - '@';   /* down -> ^N = 14 */
             break;
           case k_lt_arr: c =  'B' - '@';   /* left -> ^B =2 */
             break;
           case k_rt_arr: c =  'F' - '@';   /* right -> ^F = 6*/
             break;
           case k_INS:    c =  'O' - '@';   /* right -> ^O  = 15*/
             break;
           case k_DEL:    c =  'D' - '@';   /* Delete character under cursor = 4*/
             break;
           case k_END:    c =  'E' - '@';   /* Moves cursor to end of line * = 5 */
             break;
           case k_HOME:   c =  'A' - '@';   /* Moves cursor to beginning of line = 1*/
             break;
           default: c = 0;    /* make it garbage */
         }
     }
     else {
       switch(c) {
           case k_ESC:    c =  'U' - '@'; /* Clear full line  -> ^U */
             break;
           default:
             break;
         }
     }
#endif

#ifdef vms
    if(chan < 0) {
       c='\0';
    }
    (void)sys$qiow(0,chan,IO$_TTYREADALL,0,0,0,&c,1,0,0,0,0);
    c &= 0177;                        /* get a char */
#endif
    return c;
}

static void
gl_putc(int c)
{
    char   ch = c;

    if (gl_notty) return;

    if ( !gl_passwd || !isgraph(c))
    {
#ifdef WIN32
       CharToOemBuff((char const *)&c,&ch,1);
#endif

       sigar_write(1, &ch, 1);
    }
#if defined(unix) || defined(MSDOS) || defined(WIN32) || defined(R__MWERKS)
#ifdef TIOCSETP         /* BSD in RAW mode, map NL to NL,CR */
    if (ch == '\n') {
        ch = '\r';
        sigar_write(1, &ch, 1);
    }
#endif
#endif
}

/******************** fairly portable part *********************************/

static void
gl_puts(char *buf)
{
    int len = strlen(buf);

    if (gl_notty) return;
#ifdef WIN32
    {
     char *OemBuf = (char *)malloc(2*len);
     CharToOemBuff(buf,OemBuf,len);
     sigar_write(1, OemBuf, len);
     free(OemBuf);
    }
#else
    sigar_write(1, buf, len);
#endif
}

static void
gl_error(char *buf)
{
    int len = strlen(buf);

    gl_cleanup();
#ifdef WIN32
    {
      char *OemBuf = (char *)malloc(2*len);
      CharToOemBuff(buf,OemBuf,len);
      sigar_write(2, OemBuf, len);
      free(OemBuf);
    }
#else
    sigar_write(2, buf, len);
#endif
    exit(1);
}

static void
gl_init()
/* set up variables and terminal */
{
    if (gl_init_done < 0) {             /* -1 only on startup */
        hist_init();
    }
    if (sigar_isatty(0) == 0 || sigar_isatty(1) == 0)
        gl_notty = 1;
    gl_char_init();
    gl_init_done = 1;
}

static void
gl_bell()
{
    if (gl_bell_enabled) {
        gl_putc('\007');
    }
}

static void
gl_cleanup()
/* undo effects of gl_init, as necessary */
{
    if (gl_init_done > 0)
        gl_char_cleanup();
    gl_init_done = 0;
}

SIGAR_DECLARE(void)
sigar_getline_setwidth(int w)
{
    if (w > 20) {
        gl_termw = w;
        gl_scroll = w / 3;
    } else {
        gl_error("\n*** Error: minimum screen width is 21\n");
    }
}

SIGAR_DECLARE(void)
sigar_getline_windowchanged()
{
#ifdef TIOCGWINSZ
   if (sigar_isatty(0)) {
      static char lenv[32], cenv[32];
      struct winsize wins;
      ioctl(0, TIOCGWINSZ, &wins);

      if (wins.ws_col == 0) wins.ws_col = 80;
      if (wins.ws_row == 0) wins.ws_row = 24;

      sigar_getline_setwidth(wins.ws_col);

      sprintf(lenv, "LINES=%d", wins.ws_row);
      putenv(lenv);
      sprintf(cenv, "COLUMNS=%d", wins.ws_col);
      putenv(cenv);
   }
#endif
}

/* -1 = init, 0 = line mode, 1 = one char at a time mode, 2 = cleanup */

static char *
sigar_getlinem(int mode, char *prompt)
{
    int c, loc, tmp;
    int sig;

    if (mode == 2) {
       gl_cleanup();
       return NULL;
    }

    if (mode < 1) {
       if (mode == -1) {
           sigar_getline_config("noecho", 0);
           sigar_getline_config("erase", 0);
       }
       gl_init();
       gl_prompt = (prompt)? prompt : (char*)"";
       gl_buf[0] = 0;
       if (gl_in_hook)
           gl_in_hook(gl_buf);
       gl_fixup(gl_prompt, -2, BUF_SIZE);
       if (mode == -1) return NULL;
    }
    while ((c = gl_getc()) >= 0) {
        gl_extent = 0;          /* reset to full extent */
#ifndef WIN32
        if (isprint(c)) {
#else
        if (c >= ' ') {
#endif
            if (gl_search_mode)
               search_addchar(c);
            else
               gl_addchar(c);
        } else {
            if (gl_search_mode) {
                if (c == '\033' || c == '\016' || c == '\020') {
                    search_term();
                    c = 0;              /* ignore the character */
                } else if (c == '\010' || c == '\177') {
                    search_addchar(-1); /* unwind search string */
                    c = 0;
                } else if (c != '\022' && c != '\023') {
                    search_term();      /* terminate and handle char */
                }
            }
            /* NOTE:
             * sometimes M-x turns on bit 8               ( M-x --> 'x' + 128  )
             * sometimes M-x prepends an escape character ( M-x --> '\033','x' )
             * both cases are handled ...
             */
            switch (c)
            {
            case 'b'+128:                           /* M-b */
            case 'B'+128:                           /* M-B */
                 gl_back_1_word();
                 break;
            case 'd'+128:                           /* M-d */
            case 'D'+128:                           /* M-D */
                 gl_kill_1_word();
                 break;
            case 'f'+128:                           /* M-f */
            case 'F'+128:                           /* M-F */
                 gl_fwd_1_word();
                 break;
            case '\000':                           /* ^SPC */
                 gl_set_mark();
                 break;
            case '\027':                           /* ^W  */
                 gl_wipe();
                 break;
            case '\030':                           /* ^X  */
                 gl_exch();
                 break;
            case '\n':                             /* newline */
            case '\r':
                 gl_newline();
                 gl_cleanup();
                 return gl_buf;
                 /*NOTREACHED*/
                 break;
            case '\001': gl_fixup(gl_prompt, -1, 0);          /* ^A */
                 break;
            case '\002': gl_fixup(gl_prompt, -1, gl_pos-1);   /* ^B */
                 break;
            case '\004':                                      /* ^D */
                 if (gl_cnt == 0) {
                      gl_buf[0] = 0;
                      gl_cleanup();
                      gl_putc('\n');
                      return gl_buf;
                 } else {
                      gl_del(0);
                 }
                 break;
            case '\005': gl_fixup(gl_prompt, -1, gl_cnt);     /* ^E */
                 break;
            case '\006': gl_fixup(gl_prompt, -1, gl_pos+1);   /* ^F */
                 break;
            case '\010': case '\177': gl_del(-1);     /* ^H and DEL */
                 break;
            case '\t':                                        /* TAB */
                 if (gl_tab_hook) {
                      tmp = gl_pos;
                      loc = gl_tab_hook(gl_buf, strlen(gl_prompt), &tmp);
                      if (loc >= 0 || tmp != gl_pos || loc == -2)
                           gl_fixup(gl_prompt, loc, tmp);
                 }
                 break;
            case '\013': gl_kill();                           /* ^K */
                 break;
            case '\014': sigar_getline_clear_screen();        /* ^L */
                 break;
            case '\016':                                      /* ^N */
                 strcpy(gl_buf, hist_next());
                 if (gl_in_hook)
                      gl_in_hook(gl_buf);
                 gl_fixup(gl_prompt, 0, BUF_SIZE);
                 break;
            case '\017': gl_overwrite = !gl_overwrite;        /* ^O */
                 break;
            case '\020':                                      /* ^P */
                 strcpy(gl_buf, hist_prev());
                 if (gl_in_hook)
                      gl_in_hook(gl_buf);
                 gl_fixup(gl_prompt, 0, BUF_SIZE);
                 break;
            case '\022': search_back(1);                      /* ^R */
                 break;
            case '\023': search_forw(1);                      /* ^S */
                 break;
            case '\024': gl_transpose();                      /* ^T */
                 break;
            case '\025': gl_fixup(gl_prompt,-1,0); gl_kill(); /* ^U */
                 break;
            case '\031': gl_yank();                           /* ^Y */
                 break;
            case '\033':
                 switch(c = gl_getc())
                 {
                 case 'b':                           /* M-b */
                 case 'B':                           /* M-B */
                      gl_back_1_word();
                      break;
                 case 'd':                           /* M-d */
                 case 'D':                           /* M-D */
                      gl_kill_1_word();
                      break;
                 case 'f':                           /* M-f */
                 case 'F':                           /* M-F */
                      gl_fwd_1_word();
                      break;
                 case '[':                                /* ansi arrow keys */
                 case 'O':                                /* xterm arrow keys */
                      switch(c = gl_getc())
                      {
                      case 'A':                           /* up */
                           strcpy(gl_buf, hist_prev());
                           if (gl_in_hook)
                                gl_in_hook(gl_buf);
                           gl_fixup(gl_prompt, 0, BUF_SIZE);
                           break;
                      case 'B':                          /* down */
                           strcpy(gl_buf, hist_next());
                           if (gl_in_hook)
                                gl_in_hook(gl_buf);
                           gl_fixup(gl_prompt, 0, BUF_SIZE);
                           break;
                      case 'C': gl_fixup(gl_prompt, -1, gl_pos+1);  /* right */
                           break;
                      case 'D': gl_fixup(gl_prompt, -1, gl_pos-1);  /* left */
                           break;
                      default:                                 /* who knows */
                           gl_bell();
                           break;
                      }
                      break;
                 default:
                      gl_bell();
                 }
                 break;
            default:          /* check for a terminal signal */

#if defined(unix) || defined(WIN32) || defined(R__MWERKS)
                if (c > 0) {    /* ignore 0 (reset above) */
                    sig = 0;
#ifdef SIGINT
                    if (c == gl_intrc)
                        sig = SIGINT;
#endif
#ifdef SIGQUIT
                    if (c == gl_quitc)
                        sig = SIGQUIT;
#endif
#ifdef SIGTSTP
                    if (c == gl_suspc || c == gl_dsuspc)
                        sig = SIGTSTP;
#endif
                    if (sig != 0) {
                        gl_cleanup();
#if !defined(WIN32)
                        raise(sig);
#endif
#ifdef WIN32
                        if (sig == SIGINT) GenerateConsoleCtrlEvent(CTRL_C_EVENT,0);
                        else raise(sig);
#endif
                        gl_init();
                        sigar_getline_redraw();
                        c = 0;
                    }
                }
#endif /* unix */
                if (c > 0)
                    gl_bell();
                break;
            }
        }
        if (mode == 1) return NULL;
    }
    if (c == -1 && gl_notty)
       gl_eof = 1;
    else
       gl_eof = 0;

    gl_cleanup();
    gl_buf[0] = 0;
    return gl_buf;
}

SIGAR_DECLARE(int)
sigar_getline_eof()
{
   return gl_eof;
}

SIGAR_DECLARE(char *)
sigar_getline(char *prompt)
{
   return sigar_getlinem(0, prompt);
}

static void
gl_addchar(int c)
/* adds the character c to the input buffer at current location */
{
    int  i;

    if (gl_cnt >= BUF_SIZE - 1)
        gl_error("\n*** Error: sigar_getline(): input buffer overflow\n");
    if (gl_overwrite == 0 || gl_pos == gl_cnt) {
        for (i=gl_cnt; i >= gl_pos; i--)
            gl_buf[i+1] = gl_buf[i];
        gl_buf[gl_pos] = c;
        gl_fixup(gl_prompt, gl_pos, gl_pos+1);
    } else {
        gl_buf[gl_pos] = c;
        gl_extent = 1;
        gl_fixup(gl_prompt, gl_pos, gl_pos+1);
    }
}

static void
gl_yank()
/* adds the kill buffer to the input buffer at current location */
{
    int  i, len;

    len = strlen(gl_killbuf);
    if (len > 0) {
        gl_mark = gl_pos;
        if (gl_overwrite == 0) {
            if (gl_cnt + len >= BUF_SIZE - 1)
                gl_error("\n*** Error: sigar_getline(): input buffer overflow\n");
            for (i=gl_cnt; i >= gl_pos; i--)
                gl_buf[i+len] = gl_buf[i];
            for (i=0; i < len; i++)
                gl_buf[gl_pos+i] = gl_killbuf[i];
            gl_fixup(gl_prompt, gl_pos, gl_pos+len);
        } else {
            if (gl_pos + len > gl_cnt) {
                if (gl_pos + len >= BUF_SIZE - 1)
                    gl_error("\n*** Error: sigar_getline(): input buffer overflow\n");
                gl_buf[gl_pos + len] = 0;
            }
            for (i=0; i < len; i++)
                gl_buf[gl_pos+i] = gl_killbuf[i];
            gl_extent = len;
            gl_fixup(gl_prompt, gl_pos, gl_pos+len);
        }
    } else
        gl_bell();
}

static void
gl_transpose()
/* switch character under cursor and to left of cursor */
{
    int    c;

    if (gl_pos > 0 && gl_cnt > gl_pos) {
        c = gl_buf[gl_pos-1];
        gl_buf[gl_pos-1] = gl_buf[gl_pos];
        gl_buf[gl_pos] = c;
        gl_extent = 2;
        gl_fixup(gl_prompt, gl_pos-1, gl_pos);
    } else
        gl_bell();
}

static void
gl_newline()
/*
 * Cleans up entire line before returning to caller. A \n is appended.
 * If line longer than screen, we redraw starting at beginning
 */
{
    int change = gl_cnt;
    int len = gl_cnt;
    int loc = gl_width - 5;     /* shifts line back to start position */

    if (gl_cnt >= BUF_SIZE - 1)
        gl_error("\n*** Error: sigar_getline(): input buffer overflow\n");
    if (gl_out_hook) {
        change = gl_out_hook(gl_buf);
        len = strlen(gl_buf);
    }
    if (gl_erase_line) {
        char gl_buf0 = gl_buf[0];
        gl_buf[0] = '\0';
        gl_fixup("", 0, 0);
        gl_buf[0] = gl_buf0;
    }
    else {
        if (loc > len)
            loc = len;
        gl_fixup(gl_prompt, change, loc);   /* must do this before appending \n */
        gl_putc('\n');
    }
#if 0
    gl_buf[len] = '\n';
    gl_buf[len+1] = '\0';
#endif
    gl_mark = -1;
}

static void
gl_del(int loc)
/*
 * Delete a character.  The loc variable can be:
 *    -1 : delete character to left of cursor
 *     0 : delete character under cursor
 */
{
    int i;

    if ((loc == -1 && gl_pos > 0) || (loc == 0 && gl_pos < gl_cnt)) {
        for (i=gl_pos+loc; i < gl_cnt; i++)
            gl_buf[i] = gl_buf[i+1];
        gl_fixup(gl_prompt, gl_pos+loc, gl_pos+loc);
    } else
        gl_bell();
}

static void
gl_kill()
/* delete from current position to the end of line */
{
    if (gl_pos < gl_cnt) {
        strcpy(gl_killbuf, gl_buf + gl_pos);
        gl_buf[gl_pos] = '\0';
        gl_fixup(gl_prompt, gl_pos, gl_pos);
    } else
        gl_bell();
}

SIGAR_DECLARE(void) sigar_getline_redraw(void)
/* emit a newline, reset and redraw prompt and current input line */
{
    if (gl_init_done > 0) {
        gl_putc('\n');
        gl_fixup(gl_prompt, -2, gl_pos);
    }
}

#define CLEAR_SCREEN "\033[2J"

static void sigar_getline_clear_screen(void)
{
    if (gl_init_done > 0) {
        gl_putc('\n');
        /* XXX what to do for non-ansi term? */
        gl_puts(CLEAR_SCREEN);
        gl_fixup(gl_prompt, -2, gl_pos);
    }
}

SIGAR_DECLARE(void) sigar_getline_reset(void)
{
    gl_fixup(gl_prompt,-1,0);
    gl_kill();
}

static void
gl_fixup(char *prompt, int change, int cursor)
/*
 * This function is used both for redrawing when input changes or for
 * moving within the input line.  The parameters are:
 *   prompt:  compared to last_prompt[] for changes;
 *   change : the index of the start of changes in the input buffer,
 *            with -1 indicating no changes, -2 indicating we're on
 *            a new line, redraw everything.
 *   cursor : the desired location of the cursor after the call.
 *            A value of BUF_SIZE can be used  to indicate the cursor should
 *            move just past the end of the input line.
 */
{
    static int   gl_shift;      /* index of first on screen character */
    static int   off_right;     /* true if more text right of screen */
    static int   off_left;      /* true if more text left of screen */
    static char  last_prompt[BUF_SIZE] = "";
    int          left = 0, right = -1;          /* bounds for redraw */
    int          padl;          /* how much to erase at end of line */
    int          backup;        /* how far to backup before fixing */
    int          new_shift;     /* value of shift based on cursor */
    int          extra;         /* adjusts when shift (scroll) happens */
    int          i;
    int          new_right = -1; /* alternate right bound, using gl_extent */
    int          l1, l2;

    if (change == -2) {   /* reset */
        gl_pos = gl_cnt = gl_shift = off_right = off_left = 0;
        gl_passwd = 0;
        gl_puts(prompt);
        gl_passwd = gl_no_echo;
        strcpy(last_prompt, prompt);
        change = 0;
        gl_width = gl_termw - strlen(prompt);
    } else if (strcmp(prompt, last_prompt) != 0) {
        l1 = strlen(last_prompt);
        l2 = strlen(prompt);
        gl_cnt = gl_cnt + l1 - l2;
        strcpy(last_prompt, prompt);
        backup = gl_pos - gl_shift + l1;
        for (i=0; i < backup; i++)
            gl_putc('\b');
        gl_passwd = 0;
        gl_puts(prompt);
        gl_passwd = gl_no_echo;
        gl_pos = gl_shift;
        gl_width = gl_termw - l2;
        change = 0;
    }
    padl = (off_right)? gl_width - 1 : gl_cnt - gl_shift;   /* old length */
    backup = gl_pos - gl_shift;
    if (change >= 0) {
        gl_cnt = strlen(gl_buf);
        if (change > gl_cnt)
            change = gl_cnt;
    }
    if (cursor > gl_cnt) {
        if (cursor != BUF_SIZE)         /* BUF_SIZE means end of line */
            gl_bell();
        cursor = gl_cnt;
    }
    if (cursor < 0) {
        gl_bell();
        cursor = 0;
    }
    if (off_right || (off_left && cursor < gl_shift + gl_width - gl_scroll / 2))
        extra = 2;                      /* shift the scrolling boundary */
    else
        extra = 0;
    new_shift = cursor + extra + gl_scroll - gl_width;
    if (new_shift > 0) {
        new_shift /= gl_scroll;
        new_shift *= gl_scroll;
    } else
        new_shift = 0;
    if (new_shift != gl_shift) {        /* scroll occurs */
        gl_shift = new_shift;
        off_left = (gl_shift)? 1 : 0;
        off_right = (gl_cnt > gl_shift + gl_width - 1)? 1 : 0;
        left = gl_shift;
        new_right = right = (off_right)? gl_shift + gl_width - 2 : gl_cnt;
    } else if (change >= 0) {           /* no scroll, but text changed */
        if (change < gl_shift + off_left) {
            left = gl_shift;
        } else {
            left = change;
            backup = gl_pos - change;
        }
        off_right = (gl_cnt > gl_shift + gl_width - 1)? 1 : 0;
        right = (off_right)? gl_shift + gl_width - 2 : gl_cnt;
        new_right = (gl_extent && (right > left + gl_extent))?
                     left + gl_extent : right;
    }
    padl -= (off_right)? gl_width - 1 : gl_cnt - gl_shift;
    padl = (padl < 0)? 0 : padl;
    if (left <= right) {                /* clean up screen */
        for (i=0; i < backup; i++)
            gl_putc('\b');
        if (left == gl_shift && off_left) {
            gl_putc('$');
            left++;
        }
        for (i=left; i < new_right; i++)
            gl_putc(gl_buf[i]);
        gl_pos = new_right;
        if (off_right && new_right == right) {
            gl_putc('$');
            gl_pos++;
        } else {
            for (i=0; i < padl; i++)     /* erase remains of prev line */
                gl_putc(' ');
            gl_pos += padl;
        }
    }
    i = gl_pos - cursor;                /* move to final cursor location */
    if (i > 0) {
        while (i--)
           gl_putc('\b');
    } else {
        for (i=gl_pos; i < cursor; i++)
            gl_putc(gl_buf[i]);
    }
    gl_pos = cursor;
}

static int
gl_tab(char *buf, int offset, int *loc)
/* default tab handler, acts like tabstops every 8 cols */
{
    int i, count, len;

    len = strlen(buf);
    count = 8 - (offset + *loc) % 8;
    for (i=len; i >= *loc; i--)
        buf[i+count] = buf[i];
    for (i=0; i < count; i++)
        buf[*loc+i] = ' ';
    i = *loc;
    *loc = i + count;
    return i;
}

/******************* History stuff **************************************/

#ifndef HIST_SIZE
#define HIST_SIZE 100
#endif

static int      hist_pos = 0, hist_last = 0;
static char    *hist_buf[HIST_SIZE];

static void
hist_init()
{
    int i;

    if (gl_savehist) return;

    hist_buf[0] = "";
    for (i=1; i < HIST_SIZE; i++)
        hist_buf[i] = (char *)0;
}

SIGAR_DECLARE(void) sigar_getline_completer_set(sigar_getline_completer_t func)
{
    if (func) {
        gl_tab_hook = func;
    }
    else {
        gl_tab_hook = gl_tab;
    }
}

SIGAR_DECLARE(void)
sigar_getline_histinit(char *file)
{
   char line[256];
   FILE *fp;
   int   nline = 1;   /* prevent from becoming 0 */

   gl_savehist = 0;

   hist_init();

   if (!strcmp(file, "-")) return;

   sprintf(gl_histfile, "%s", file);

   fp = fopen(gl_histfile, "r");
   if (fp)
      while (fgets(line, 256, fp)) {
         nline++;
         sigar_getline_histadd(line);
      }
   else
      fp = fopen(gl_histfile, "w");

   if (fp) fclose(fp);

   gl_savehist = nline;
}

SIGAR_DECLARE(void)
sigar_getline_histadd(char *buf)
{
    static char *prev = 0;
    char *p = buf;
    int len;

    while (*p == ' ' || *p == '\t' || *p == '\n')
        p++;
    if (*p) {
        len = strlen(buf);
        if (strchr(p, '\n'))    /* previously line already has NL stripped */
            len--;
        if (prev == 0 || strlen(prev) != len ||
                            strncmp(prev, buf, len) != 0) {
            hist_buf[hist_last] = hist_save(buf);
            prev = hist_buf[hist_last];
            hist_last = (hist_last + 1) % HIST_SIZE;
            if (hist_buf[hist_last] && *hist_buf[hist_last]) {
                free(hist_buf[hist_last]);
            }
            hist_buf[hist_last] = "";

            /* append command to history file */
            if (gl_savehist) {
               FILE *fp;
               fp = fopen(gl_histfile, "a+");
               if (fp) {
                   fprintf(fp, "%s\n", prev);
                   gl_savehist++;
                   fclose(fp);
               }

               /* if more than HIST_SIZE lines, safe last 60 command and delete rest */
               if (gl_savehist > HIST_SIZE) {
                  FILE *ftmp;
                  char tname[L_tmpnam];
                  char line[BUFSIZ];

                  fp = fopen(gl_histfile, "r");
                  tmpnam(tname);
                  ftmp = fopen(tname, "w");
                  if (fp && ftmp) {
                     int nline = 0;
                     while (fgets(line, BUFSIZ, fp)) {
                        nline++;
                        gl_savehist = 1;  /* prevent from becoming 0 */
                        if (nline > HIST_SIZE-60) {
                           gl_savehist++;
                           fprintf(ftmp, "%s", line);
                        }
                     }
                  }
                  if (fp)   fclose(fp);
                  if (ftmp) fclose(ftmp);

                  /* copy back to history file */
                  fp   = fopen(gl_histfile, "w");
                  ftmp = fopen(tname, "r");
                  if (fp && ftmp)
                     while (fgets(line, BUFSIZ, ftmp))
                        fprintf(fp, "%s", line);

                  if (fp)   fclose(fp);
                  if (ftmp) fclose(ftmp);
                  remove(tname);
               }
            }
        }
    }
    hist_pos = hist_last;
}

static char *
hist_prev()
/* loads previous hist entry into input buffer, sticks on first */
{
    char *p = 0;
    int   next = (hist_pos - 1 + HIST_SIZE) % HIST_SIZE;

    if (hist_buf[hist_pos] != 0 && next != hist_last) {
        hist_pos = next;
        p = hist_buf[hist_pos];
    }
    if (p == 0) {
        p = "";
        gl_bell();
    }
    return p;
}

static char *
hist_next()
/* loads next hist entry into input buffer, clears on last */
{
    char *p = 0;

    if (hist_pos != hist_last) {
        hist_pos = (hist_pos+1) % HIST_SIZE;
        p = hist_buf[hist_pos];
    }
    if (p == 0) {
        p = "";
        gl_bell();
    }
    return p;
}

static char *
hist_save(char *p)
/* makes a copy of the string */
{
    char *s = 0;
    int   len = strlen(p);
    char *nl = strchr(p, '\n');

    if (nl) {
        if ((s = (char *)malloc(len)) != 0) {
            strncpy(s, p, len-1);
            s[len-1] = 0;
        }
    } else {
        if ((s = (char *)malloc(len+1)) != 0) {
            strcpy(s, p);
        }
    }
    if (s == 0)
        gl_error("\n*** Error: hist_save() failed on malloc\n");
    return s;
}

/******************* Search stuff **************************************/

static char  search_prompt[101];  /* prompt includes search string */
static char  search_string[100];
static int   search_pos = 0;      /* current location in search_string */
static int   search_forw_flg = 0; /* search direction flag */
static int   search_last = 0;     /* last match found */

static void
search_update(int c)
{
    if (c == 0) {
        search_pos = 0;
        search_string[0] = 0;
        search_prompt[0] = '?';
        search_prompt[1] = ' ';
        search_prompt[2] = 0;
    } else if (c > 0) {
        search_string[search_pos] = c;
        search_string[search_pos+1] = 0;
        search_prompt[search_pos] = c;
        search_prompt[search_pos+1] = '?';
        search_prompt[search_pos+2] = ' ';
        search_prompt[search_pos+3] = 0;
        search_pos++;
    } else {
        if (search_pos > 0) {
            search_pos--;
            search_string[search_pos] = 0;
            search_prompt[search_pos] = '?';
            search_prompt[search_pos+1] = ' ';
            search_prompt[search_pos+2] = 0;
        } else {
            gl_bell();
            hist_pos = hist_last;
        }
    }
}

static void
search_addchar(int c)
{
    char *loc;

    search_update(c);
    if (c < 0) {
        if (search_pos > 0) {
            hist_pos = search_last;
        } else {
            gl_buf[0] = 0;
            hist_pos = hist_last;
        }
        strcpy(gl_buf, hist_buf[hist_pos]);
    }
    if ((loc = strstr(gl_buf, search_string)) != 0) {
        gl_fixup(search_prompt, 0, loc - gl_buf);
    } else if (search_pos > 0) {
        if (search_forw_flg) {
            search_forw(0);
        } else {
            search_back(0);
        }
    } else {
        gl_fixup(search_prompt, 0, 0);
    }
}

static void
search_term()
{
    gl_search_mode = 0;
    if (gl_buf[0] == 0)         /* not found, reset hist list */
        hist_pos = hist_last;
    if (gl_in_hook)
        gl_in_hook(gl_buf);
    gl_fixup(gl_prompt, 0, gl_pos);
}

static void
search_back(int new_search)
{
    int    found = 0;
    char  *p, *loc;

    search_forw_flg = 0;
    if (gl_search_mode == 0) {
        search_last = hist_pos = hist_last;
        search_update(0);
        gl_search_mode = 1;
        gl_buf[0] = 0;
        gl_fixup(search_prompt, 0, 0);
    } else if (search_pos > 0) {
        while (!found) {
            p = hist_prev();
            if (*p == 0) {              /* not found, done looking */
               gl_buf[0] = 0;
               gl_fixup(search_prompt, 0, 0);
               found = 1;
            } else if ((loc = strstr(p, search_string)) != 0) {
               strcpy(gl_buf, p);
               gl_fixup(search_prompt, 0, loc - p);
               if (new_search)
                   search_last = hist_pos;
               found = 1;
            }
        }
    } else {
        gl_bell();
    }
}

static void
search_forw(int new_search)
{
    int    found = 0;
    char  *p, *loc;

    search_forw_flg = 1;
    if (gl_search_mode == 0) {
        search_last = hist_pos = hist_last;
        search_update(0);
        gl_search_mode = 1;
        gl_buf[0] = 0;
        gl_fixup(search_prompt, 0, 0);
    } else if (search_pos > 0) {
        while (!found) {
            p = hist_next();
            if (*p == 0) {              /* not found, done looking */
               gl_buf[0] = 0;
               gl_fixup(search_prompt, 0, 0);
               found = 1;
            } else if ((loc = strstr(p, search_string)) != 0) {
               strcpy(gl_buf, p);
               gl_fixup(search_prompt, 0, loc - p);
               if (new_search)
                   search_last = hist_pos;
               found = 1;
            }
        }
    } else {
        gl_bell();
    }
}
#if 0
/***********************************************************************
 *                                                                     *
 *   Strip blanks from both sides of a string. Space for the new       *
 *   string is allocated and a pointer to it is returned.              *
 *                                                                     *
 ***********************************************************************/
char *strip(char *s)
{
   char *r, *t1, *t2;
   int   l;

   l = strlen(s);
   r = (char *)calloc(l+1, 1);

   if (l == 0) {
      *r = '\0';
      return r;
   }

   /* get rid of leading blanks */
   t1 = s;
   while (*t1 == ' ')
      t1++;

   t2 = s + l - 1;
   while (*t2 == ' ' && t2 > s)
      t2--;

   if (t1 > t2) {
      *r = '\0';
      return r;
   }
   strncpy(r, t1, (size_t) (t2-t1+1));

   return r;
}
#endif
/*****************************************************************************/
/* Extra routine provided by Christian Lacunza <lacunza@cdfsg5.lbl.gov>      */
/*****************************************************************************/

/* move cursor back to beginning of _current_ word */
/* unless it's already at the beginning,           */
/* in which case it moves back to the beginning    */
/* of the _previous_ word.                         */
static void gl_back_1_word( void )
{
     int i = gl_pos;

     /* if we're at the beginning of a word,     */
     /* slip back into the preceeding whitespace */
     if( i>0 && is_whitespace(gl_buf[i-1]) ) {
          i-=1;
     }

     /* now move back over all consecutive whitespace */
     while( i>0 && is_whitespace(gl_buf[i]) ) {
          i-=1;
     }

     /* now keep moving back over all consecutive non-whitespace */
     /* until we find the beginning of this word.                */
     /* ie. stop just before more whitespace shows up.           */
     while( i>0 && !is_whitespace(gl_buf[i-1]) ) {
          i-=1;
     }

     /* move the cursor here */
     gl_fixup(gl_prompt, -1, i);
}

/* kills from current position to end of word */
static void gl_kill_1_word( void )
{
     int i = gl_pos;
     int j = gl_pos;

/* delete this: */
#if 0
     /* not sure what to do with "punctuation" */
     if( is_whitespace(gl_buf[j]) && gl_buf[j]!=' ' ) {
          return;
     }
     /* first find a word */
     while( j<gl_cnt && gl_buf[j]==' ' ) {
          j+=1;
     }
#endif

     /* first find a word */
     while( j<gl_cnt && is_whitespace(gl_buf[j]) ) {
          j+=1;
     }

     /* next, find the end of this word. */
     while( j<gl_cnt && !is_whitespace(gl_buf[j+1]) ) {
          j+=1;
     }

     /* kill */
     gl_kill_region( i, j );

     /* fixup */
     gl_fixup(gl_prompt, gl_pos, gl_pos);
}

static void gl_kill_region( int i, int j )
{
     /* copy to kill buffer */
     strncpy( gl_killbuf, gl_buf+i, j-i+1 );
     gl_killbuf[j-i+1]='\0';

     /* remove from gl_buf */
     while( j<gl_cnt ) {
          gl_buf[i]=gl_buf[j+1];
          i+=1;
          j+=1;
     }
     gl_buf[i]='\0';
}

/* move cursor forward to the beginning of the next word. */
static void gl_fwd_1_word( void )
{
     int i = gl_pos;

     /* move past all non-whitespace into the whitespace between words. */
     while( i<gl_cnt && !is_whitespace(gl_buf[i]) ) {
          i+=1;
     }

     /* move past this whitespace to the beginning of the next word. */
     while( i<gl_cnt && is_whitespace(gl_buf[i]) ) {
          i+=1;
     }

     /* move the cursor here. */
     gl_fixup(gl_prompt, -1, i);
}

/* NOTE: "whitespace" is very loosely defined. */
static int is_whitespace( char c )
{
     int decent_character;

     decent_character = sigar_isalpha(c) || sigar_isdigit(c) || c=='_';

     return !decent_character;
}

/* sets mark to be at point */
static void gl_set_mark( void )
{
     gl_mark = gl_pos;
}

/* kills from mark to point */
static void gl_wipe( void )
{
     int left, right;

     if( gl_mark  <   0    ) return;
     if( gl_mark == gl_pos ) return;

     if( gl_mark < gl_pos ) {
          left  = gl_mark;
          right = gl_pos;
     }
     else {
          left  = gl_pos;
          right = gl_mark;
     }

     gl_kill_region( left, right-1 );
     gl_fixup( gl_prompt, left, left );
}

/* echanges point and mark */
static void gl_exch( void )
{
     int tmp;

     /* make sure mark is set */
     if( gl_mark < 0 ) return;

     tmp = gl_pos;
     gl_fixup( gl_prompt, -1, gl_mark );
     gl_mark = tmp;
}
