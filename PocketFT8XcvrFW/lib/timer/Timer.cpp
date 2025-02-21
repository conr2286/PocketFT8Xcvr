/**
 * SYNOPSIS
 *  Timer implements a callback timer service for the Arduino world
 *
 * USAGE
 *  Timer::buildTimer()      More or less a Timer factory
 *  Timer->start()           Start a Timer
 *  Timer::serviceTimers()   Called from loop() to service the Timer inventory
 *  callback(Timer*)         User-supplied callback function invoked when a Timer expires
 *
 * NOTES
 *  The current implementation maintains the Timer inventory in an unsorted list.
 *  If a need develops for many Timers, a sorted list might perform better.
 *
 *  In the current implementation, Timers can be created but never destroyed.  The
 *  best you can do is neglect to start those you need not.
 */

#include "Timer.h"

#include <Arduino.h>

#include "DEBUG.h"

/**
 * @brief Private constructor initializes a new Timer object
 *
 * Note:  This constructor is unnecessary as buildTimer() initializes
 * everything.
 */
Timer::Timer() {
    this->next = NULL;
    this->running = false;
    this->callback = NULL;
}

/**
 * @brief Build a new Timer object and add it into the listOfAllTimers
 * @param milliSeconds
 * @param callback
 * @return Reference to the new Timer object or NULL if error
 *
 * A Timer is either expired or running.  A new Timer is initially expired.
 * Starting a Timer sets it running. A running Timer's callback function
 * is invoked when it expires.
 *
 * Note:  buildTimer() is a lazy Timer factory.
 */
Timer* Timer::inventory = NULL;
Timer* Timer::buildTimer(unsigned long milliSeconds, void (*callback)(Timer*)) {
    // Validate parameters.  Note:  OK for callback to be NULL.
    if (milliSeconds == 0) return NULL;

    // Build and initialize a new Timer
    Timer* newTimer = new (Timer);    // Build the new Timer object
    newTimer->period = milliSeconds;  // New Timer's period in milliseconds
    newTimer->callback = callback;    // New Timer's callback function
    newTimer->running = false;        // New Timer won't run until started

    // Insert new Timer at head of list
    newTimer->next = inventory;  // The existing Timer inventory list or NULL
    inventory = newTimer;        // Add new Timer at head of inventory list
    // DPRINTF("buildTimer()=%lu\n", newTimer);
    return newTimer;
}  // buildTimer()

/**
 * @brief Start a Timer running
 *
 * Notes:  The started Timer will expire in period milliseconds. The started
 * Timer will be "running" until it expires.  The serviceTimer() determines
 * when a Timer expires.
 */
void Timer::start() {
    // DPRINTF("this Timer = %lu\n", this);
    this->running = true;                          // This Timer is now running
    this->timeExpiring = millis() + this->period;  // Record when this Timer will expire
    DTRACE();
}

/**
 * @brief Stop a running Timer
 *
 * A stopped timer is not running and will not expire (i.e. a stopped Timer's
 * callback function will not be invoked).  Stopping a Timer does not destroy
 * the Timer object; it can be [re]started if desired.  Stopped Timers remain
 * in the Timer inventory.
 */
void Timer::stop() {
    this->running = false;
}  // stop()

/**
 * @brief Service Timer inventory, invoking callback functions of expiring Timers
 *
 * The Arduino's loop() function should invoke serviceTimers() during each pass
 * to walk the inventory list of Timer objects and invoke the callback function
 * of those expiring.
 */
void Timer::serviceTimers() {
    unsigned long now = millis();

    // Examine each Timer in the inventory list
    for (Timer* thisTimer = inventory; thisTimer != NULL; thisTimer = thisTimer->next) {
        // Service running Timers that have expired
        if (thisTimer->running && (now >= thisTimer->timeExpiring)) {
            thisTimer->running = false;                                          // This Timer has expired
            if (thisTimer->callback != NULL) (*thisTimer->callback)(thisTimer);  // Notify callback function
        }
    }

}  // serviceTimers()