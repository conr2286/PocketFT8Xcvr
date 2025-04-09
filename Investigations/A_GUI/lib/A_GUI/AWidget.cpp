#include "AWidget.h"

#include "AGUI.h"
#include "DEBUG.h"
#include "ft8_font.h"


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
    if (!Serial) Serial.begin(9600);
    DPRINTF("AWidget()=0x%x\n", this);
    // Link this new widget into the unordered list of all widgets
    this->next = allWidgets;
    allWidgets = this;

    // Setup default colors for new widget using the application's defaults
    // Each widget can change any of these colors if they wish, but initializing
    // them here promotes consistency throughout the application.
    this->bgColor = AGUI::bgColor;  // Background color
    this->fgColor = AGUI::fgColor;  // Foreground color
    this->bdColor = AGUI::bdColor;  // Border color
    this->spColor = AGUI::spColor;  // Special color (e.g. selected item color)

    // Initially config the widget font to the application's default
    DPRINTF("txtFont=%p\n", AGUI::appFont);
    this->defaultFont = AGUI::appFont;
}

/**
 * @brief Destructor for the AWidget base class
 *
 * The destructor's primary responsibility is to unlink this widget from the
 * unordered list of all widgets, stomp some dangling pointers, and erase
 * the widget from the display.
 *
 * If someday AGUI needs to deal with overlapping widgets, this is where we
 * would notify widgets uncovered by this vanishing widget.
 */
AWidget::~AWidget() {
    DPRINTF("~AWidget()=0x%x\n", this);

    // Erase this widget from the display
    // DPRINTF("x1=%d y1=%d, x2=%d y2=%d\n", boundary.x1, boundary.y1, boundary.x2, boundary.y2);
    AGUI::gfx->setClipRect(boundary.x1, boundary.y1, boundary.x2 - boundary.x1, boundary.y2 - boundary.y1);
    AGUI::gfx->fillRect(boundary.x1, boundary.y1, boundary.x2 - boundary.x1, boundary.y2 - boundary.y1, bgColor);

    // Unlink this widget if at the head of the list of all widgets
    if (allWidgets == this) {
        // Unlink this widget from head of list
        allWidgets = this->next;

        // Paranoia for dangling pointers
        this->next = NULL;

        // Finished
        return;
    }

    // Unlink this widget from somewhere else in the list of all widgets
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
}  //~AWidget()

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
        //DTRACE();
        // If touch coords lie within scanned widget and call its notification method if provided
        if (scannedWidget->boundary.isWithin(xCoord, yCoord)) {
            //DTRACE();
            scannedWidget->doTouchWidget(xCoord, yCoord);
        }
    }
}
