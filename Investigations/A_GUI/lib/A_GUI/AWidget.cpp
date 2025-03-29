#include "AWidget.h"

#include <Adafruit_GFX.h>

#include "HX8357_t3n.h"

// Initialize the head of the unordered list of all AWidget objects.
// The processTouch() class method uses the list to find the selected widget.
AWidget* AWidget::allWidgets = NULL;

/**
 * @brief Constructor for the AWidget base class
 */
AWidget::AWidget() {
    // Link this new widget into the unordered list of all widgets
    this->next = allWidgets;
    allWidgets = this;

    // Setup default colors for new widget
    this->bgColor = DEFAULT_BACKGROUND_COLOR;
    this->fgColor = DEFAULT_FOREGROUND_COLOR;
    this->bdColor = DEFAULT_BORDER_COLOR;

    // New widgets initially have no selection notification (callback) function
    this->doSelection = NULL;

    // For now... we are leaving the boundary box uninitialized in the base class
}

/**
 * @brief Destructor for the AWidget base class
 *
 * The destructor's primary responsibility is to unlink this widget from the
 * unordered list of all widgets.  It also stomps some dangling pointers.
 */
AWidget::~AWidget() {
    // Special case handles this widget at the head of the list
    if (allWidgets == this) {
        // Unlink this widget from head of list
        allWidgets = this->next;

        // Paranoia for dangling pointers
        this->next = NULL;
        this->doSelection = NULL;

        // Finished
        return;
    }

    // Scan the list searching for the widget preceding this widget
    for (AWidget* scannedWidget = allWidgets; scannedWidget != NULL; scannedWidget = scannedWidget->next) {
        // Does scannedWidget precede this widget?
        if (scannedWidget->next == this) {
            // Yes, unlink this widget from list
            scannedWidget->next = this->next;

            // Paranoia for stale pointers
            this->next = NULL;
            this->doSelection = NULL;

            break;  // Finished
        }
    }
}

/**
 * @brief Process touch/click at xCoord/yCoord
 * @param xCoord Touch screen x-coordinate
 * @param yCoord Touch screen y-coordinate
 *
 * This is where all widget touch notifications begin in A_GUI.
 *
 * Scans the list of all widgets to determine which, if any, were touched
 * and calls their notification method, if supplied.
 *
 * Note:  We are not, at least for now, handling overlapping widgets on the
 * screen.  If overlapping widgets have been created and touched, we notify
 * only the first in the list.
 */
void AWidget::processTouch(uint16_t xCoord, uint16_t yCoord) {
    for (AWidget* scannedWidget = allWidgets; scannedWidget != NULL; scannedWidget = scannedWidget->next) {
        // If touch coords lie within scanned widget then call its notification method if provided
        if (scannedWidget->boundary.isWithin(xCoord, yCoord) && (scannedWidget->doSelection != NULL)) scannedWidget->doSelection(xCoord, yCoord);
    }
}
