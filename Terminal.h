#ifndef TERMINAL_H
#define TERMINAL_H

class Terminal {
public:
    static void enableRawMode();
    static void disableRawMode();
    static int kbhit();
};

#endif // TERMINAL_H
