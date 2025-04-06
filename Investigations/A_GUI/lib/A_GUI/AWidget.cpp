#include "AWidget.h"

#include "AGraphicsDriver.h"
#include "DEBUG.h"

// Initialize the head of the unordered list of all AWidget objects.
// The processTouch() class method uses the list to find the selected widget.
AWidget* AWidget::allWidgets = NULL;

/**
 * @brief Default constructor for the AWidget base class
 *
 * Our primary responsibilities here are:
 *  + Link this new widget into the list of all widgets
 *  + Initialize the member variables
 */
AWidget::AWidget() {
    DTRACE();
    // Link this new widget into the unordered list of all widgets
    this->next = allWidgets;
    allWidgets = this;

    // Setup default colors for new widget
    this->bgColor = DEFAULT_BACKGROUND_COLOR;
    this->fgColor = DEFAULT_FOREGROUND_COLOR;
    this->bdColor = DEFAULT_BORDER_COLOR;

    // For now... we are leaving the boundary box uninitialized in the base class

    // Reset font to default (at least for now)
    gfx->setFont();
}

/**
 * @brief Destructor for the AWidget base class
 *
 * The destructor's primary responsibility is to unlink this widget from the
 * unordered list of all widgets.  It also stomps some dangling pointers.
 *
 * If someday AGUI needs to deal with overlapping widgets, this is where we
 * would notify widgets uncovered by this vanishing widget.
 */
AWidget::~AWidget() {
    DTRACE();
    // Special case handles this widget at the head of the list
    if (allWidgets == this) {
        // Unlink this widget from head of list
        allWidgets = this->next;

        // Paranoia for dangling pointers
        this->next = NULL;

        // Finished
        return;
    }

    // Unlink this widget from the list of all widgets
    for (AWidget* scannedWidget = allWidgets; scannedWidget != NULL; scannedWidget = scannedWidget->next) {
        // Does scannedWidget precede this widget?
        if (scannedWidget->next == this) {
            // Yes, unlink this widget from list
            scannedWidget->next = this->next;

            // Paranoia for stale pointers
            this->next = NULL;

            break;  // Finished
        }
    }
}

/**
 * @brief Process touch/click at xCoord/yCoord
 * @param xCoord Touch screen x-coordinate
 * @param yCoord Touch screen y-coordinate
 *
 * This is where all widget touch notifications begin in AGUI.
 *
 * Scans the list of all widgets to determine which, if any, were touched.  When
 * the app's loop() function detects a touch event, it should pass the touch coordinates
 * to this static class method.
 *
 * Note:  We are not, at least for now, handling overlapping widgets on the
 * screen.  If overlapping widgets have been created and touched, we notify
 * only the first in the list.
 */
void AWidget::processTouch(uint16_t xCoord, uint16_t yCoord) {
    DTRACE();
    for (AWidget* scannedWidget = allWidgets; scannedWidget != NULL; scannedWidget = scannedWidget->next) {
        DTRACE();
        // If touch coords lie within scanned widget and call its notification method if provided
        if (scannedWidget->boundary.isWithin(xCoord, yCoord)) {
            DTRACE();
            scannedWidget->doTouchWidget(xCoord, yCoord);
        }
    }
}
