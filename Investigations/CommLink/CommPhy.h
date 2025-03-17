#pragma once

class CommPhy {
   public:
    void putch(int c);
    int getch(void);
    void begin(void);
    int available(void);
    int availableForWrite(void);
};