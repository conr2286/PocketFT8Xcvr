#include "AWidget.h"

#include "AGUI.h"
#include "NODEBUG.h"
// #include "ft8_font.h"

// Initialize the head of the unordered list of all AWidget objects.
// The processTouch() class method uses the list to find the selected widget.
AWidget* AWidget::headOfWidgets = NULL;

/**
 * @brief Default constructor for the AWidget base class
 *
 * Our primary responsibilities here are:
 *  + Link this new widget into the list of all widgets
 *  + Initialize the member variables
 */
AWidget::AWidget() {
    // if (!Serial) Serial.begin(9600);
    // DPRINTF("AWidget()=0x%x\n", this);
    // Link this new widget into the unordered list of all widgets
    this->next = headOfWidgets;
    headOfWidgets = this;

    // Setup default colors for new widget using the application's defaults
    // Each widget can change any of these colors if they wish, but initializing
    // them here promotes consistency throughout the application.
    this->bgColor = AGUI::bgColor;  // Background color
    this->fgColor = AGUI::fgColor;  // Foreground color
    this->bdColor = AGUI::bdColor;  // Border color
    this->spColor = AGUI::spColor;  // Special color (e.g. selected item color)

    // Initially config the widget font to the application's default
    DPRINTF("txtFont=%p\n", AGUI::appFont);
    this->font = AGUI::appFont;  // Widget's font

    // TODO:  Refactor such that radius==0 ==> squared corners rather than rounded
    this->radius = 7;  // Radius of rounded corners in this widget
}

/**
 * @brief Copy constructor for AWidget
 * @param existing Reference to an existing AWidget
 *
 * @note New object placed at head of list and members, except next, copied from existing object.
 */
AWidget::AWidget(const AWidget& existing) {
    // Link this new widget at the head of the unordered list of all widgets
    this->next = headOfWidgets;  // We link to the old head
    headOfWidgets = this;        // We become the new head

    // Copy fonts
    this->bgColor = existing.bgColor;  // Background color
    this->fgColor = existing.fgColor;  // Foreground color
    this->bdColor = existing.bdColor;  // Border color
    this->spColor = existing.spColor;  // Special color (e.g. selected item color)

    // Copy font
    this->font = existing.font;  // This widget's font

    // Copy radius
    this->radius = existing.radius;  // Radius of rounded corners in this widget
}  // Copy Constructor

/**
 * @brief AWidget assignment operator
 * @param that Reference to object whose members will be assigned to this
 * @return Reference to target object
 *
 * @note Assignment copies all members except link to next AWidget
 */
AWidget& AWidget::operator=(const AWidget& that) {
    // Check for self-assignment
    if (this == &that) return *this;

    // Copy member variables except the link to the next widget which must remain unmodified
    this->bgColor = that.bgColor;  // Background color
    this->fgColor = that.fgColor;  // Foreground color
    this->bdColor = that.bdColor;  // Border color
    this->spColor = that.spColor;  // Special color (e.g. selected item color)
    this->font = that.font;        // This widget's font
    this->radius = that.radius;    // Radius of rounded corners in this widget

    return *this;

}  // Assignment Operator

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
    if (headOfWidgets == this) {
        // Unlink this widget from head of list
        headOfWidgets = this->next;

        // Paranoia for dangling pointers
        this->next = NULL;

        // Finished
        return;
    }

    // Unlink this widget from somewhere else in the list of all widgets
    for (AWidget* scannedWidget = headOfWidgets; scannedWidget != NULL; scannedWidget = scannedWidget->next) {
        // Does scannedWidget precede this widget?
        if (scannedWidget->next == this) {
            // Yes, unlink this widget from list
            scannedWidget->next = this->next;

            // Paranoia for dangling pointers
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
    for (AWidget* scannedWidget = headOfWidgets; scannedWidget != NULL; scannedWidget = scannedWidget->next) {
        // DTRACE();
        //  If touch coords lie within scanned widget and call its notification method if provided
        if (scannedWidget->boundary.isWithin(xCoord, yCoord)) {
            // DTRACE();
            scannedWidget->onTouchWidget(xCoord, yCoord);
        }
    }
    // delay(50);  //Cheap debounce
}

/**
 * @brief Determine if a widget has a drawn border
 * @return true if has a border, else false
 *
 * A widget has a border if its border color != its background color
 */
bool AWidget::hasBorder() {
    return bdColor != bgColor;
}