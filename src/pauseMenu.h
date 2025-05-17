#ifndef PAUSE_MENU_H
#define PAUSE_MENU_H

#define PAUSE_CONTINUE     0
#define PAUSE_ACHIEVEMENTS 1
#define PAUSE_OPTIONS      2
#define PAUSE_USB          3
#define PAUSE_QUIT         4
#define PAUSE_TOTAL        5

extern int pause_selected;

int runPauseMenu(void); // Now replaces previous update/draw split

#endif