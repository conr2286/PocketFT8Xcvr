#pragma once

#include <Arduino.h>

class Timer {
   public:
    static Timer* buildTimer(unsigned long milliSeconds, void (*callback)(Timer*));  // Build a new Timer object
    static void serviceTimers(void);                                                 // Service Timer inventory
    void start(void);                                                                // Start this Timer running
    void stop();                                                                     // Stop a running Timer

   private:
    Timer();                        // Private constructor for new Timer object
    ~Timer() = delete;              // Delete the built-in destructor
    static Timer* inventory;        // Head of the Timer inventory list
    Timer* next;                    // Next timer in inventory list
    unsigned long period;           // This Timer's period in milliseconds
    unsigned long timeExpiring;     // This Timer's millis() at expiration
    bool running;                   // True if this Timer is not expired
    void (*callback)(Timer*);  // This Timer's callback function
};