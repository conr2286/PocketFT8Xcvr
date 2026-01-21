/**
 * AWidget is the base class for nearly all AGUI controls
 *
 * Notes:
 *  + AGUI controls have limited support for repainting an underlying control
 *  when an overlying control is deleted.
 *  + AGUI widgets repaint by recalculating what appears within them; AGUI does
 *  not store a shadow pixel map of the display as do some GUIs.  Widgets always
 *  repaint their entire display, they are not aware when only a portion actually
 *  needs to be repainted.
 *  + Widgets are said to be "stacked" such that a "higher" widget can be displayed
 *  on top of a "lower" widget.  The lowest widget is called the "bottom" of the
 *  stack while the higest is the "top."  The oldest widget is the bottom and the
 *  newest widget becomes the top, covering any lower widgets it overlaps.
 *  + All remaining widgets are repainted in stacking order, lowest (oldest) first,
 *  top (newest) last, when a widget is deleted.  Thus, higher widget(s) may cover
 *  the displayed image(s) of lower widget(s).
 */

#include "AWidget.h"

#include "AGUI.h"
#include "DEBUG.h"
// #include "ft8_font.h"

// Initialize the head of the list of all AWidget objects. The list
// is arranged in stacking order, with the lowest (oldest) widget
// at the head and the newest at the tail.
AWidget* AWidget::allWidgets = NULL;

/**
 * @brief Default constructor for the AWidget base class
 *
 * Our primary responsibilities here are:
 *  + Link this new widget into the list of all widgets
 *  + Initialize the member variables
 *
 * @note AGUI derived classes depend upon initialized AWidget member variables.  If you
 * add a new constructor, ensure everything below gets initialized.
 */
AWidget::AWidget() {
    // if (!Serial) Serial.begin(9600);
    DPRINTF("%p.AWidget()\n", this);

    insert();  // Insert this new widget at the end of the list of all widgets

    // Setup default colors for new widget using the application's defaults
    // Each widget can change any of these colors if they wish, but initializing
    // them here promotes consistency throughout the application.
    this->bgColor = AGUI::bgColor;  // Background color
    this->fgColor = AGUI::fgColor;  // Foreground color
    this->bdColor = AGUI::bdColor;  // Border color
    this->spColor = AGUI::spColor;  // Special color (e.g. selected item color)

    // Initially config the widget font to the application's default font
    DPRINTF("txtFont=%p\n", AGUI::appFont);
    this->font = AGUI::appFont;  // Widget's font

    // TODO:  Refactor such that radius==0 ==> squared corners rather than rounded
    this->radius = 7;  // Hardwired radius of rounded corners in this widget
}

/**
 * @brief Copy constructor for AWidget
 * @param existing Reference to an existing AWidget
 *
 * @note New object placed at head of list and members, except next, copied from existing object.
 */
AWidget::AWidget(const AWidget& existing) {
    DTRACE();
    insert();  // Insert this new widget at the end of the list of all widgets

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
 * @brief Insert this widget into the list of all widgets
 *
 * @note The list of all widgets is maintained in "stacked" order with the lowest
 * (oldest) displayed widget at the head, and the highest (newest) widget at the tail.
 * The repaintAll() method uses the ordered list to repaint the oldest (bottom) widget
 * first followed by those displayed above in stacked order.
 */
void AWidget::insert(void) {
    DPRINTF("%p.insert()\n", this);
    this->next = NULL;  // This widget always becomes the end of the list

    // Deal with a now empty list (if this widget will become the first and only in the list)
    if (allWidgets == NULL) {
        allWidgets = this;  // This widget becomes the head and the only widget in the list
    } else {
        // Insert this widget at the end of an existing list
        AWidget* thatWidget = allWidgets;                                // First (oldest) widget at the bottom of the displayed stack
        while (thatWidget->next != NULL) thatWidget = thatWidget->next;  // Find the last (newest) widget in the existing list
        thatWidget->next = this;                                         // This (even newer) widget becomes the tail of the list
    }
}  // insert()

/**
 * @brief Repaint all widgets in stacked, displayed order
 *
 * @note Repaints all widgets starting with the lowest (oldest) in the stack and
 * finishing with the highest (newest) in the stack.  Thus, overlapping widgets near
 * the top of the stack will display over those below.
 *
 * @note This implementation depends upon the list of all widgets maintained in
 * stacking order with the lowest (oldest) widget at the head.
 *
 * @note Yes, a sophisticated approach would be to repaint only those widgets that
 * actually need repainted (as when a widget stacked above another is deleted).
 * But our widget data structures are not that sophisticated.
 */
void AWidget::repaintAll(void) {
    DTRACE();

    // Repaint all widgets whether they need it or not
    for (AWidget* thatWidget = allWidgets; thatWidget != NULL; thatWidget = thatWidget->next) {
        thatWidget->repaint();  // Repaint that widget
    }
}

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
 * There is some limited support for uncovering an underlying widget
 * when this widget is destroyed.
 */
AWidget::~AWidget() {
    DPRINTF("~AWidget()=0x%x\n", this);

    // Erase this widget from the display
    // DPRINTF("x1=%d y1=%d, x2=%d y2=%d\n", boundary.x1, boundary.y1, boundary.x2, boundary.y2);
    AGUI::gfx->setClipRect(boundary.x1, boundary.y1, boundary.x2 - boundary.x1, boundary.y2 - boundary.y1);
    AGUI::gfx->fillRect(boundary.x1, boundary.y1, boundary.x2 - boundary.x1, boundary.y2 - boundary.y1, bgColor);

    // Unlink this widget if it resides at the head of the list of all widgets
    if (allWidgets == this) {
        // Unlink this widget from head of list
        allWidgets = this->next;

        // Paranoia for dangling pointers
        this->next = NULL;

    } else {
        // Unlink this widget from somewhere in the midst of all widgets in the list
        for (AWidget* scannedWidget = allWidgets; scannedWidget != NULL; scannedWidget = scannedWidget->next) {
            // Does scannedWidget precede this widget?
            if (scannedWidget->next == this) {
                // Yes, unlink this widget from list
                scannedWidget->next = this->next;

                // Paranoia for dangling pointers
                this->next = NULL;

                break;  // Finished
            }
        }
    }

    // Note:  This widget object still remains but has been removed from the list of all widgets
    repaintAll();  // Repaint all widgets remaining in the list of all widgets

}  //~AWidget()

/**
 * @brief Process touch/click at xCoord/yCoord
 * @param xCoord Touch screen x-coordinate
 * @param yCoord Touch screen y-coordinate
 *
 * This is where all widget touch notifications begin in AGUI
 *
 * We scan the list of all widgets to determine which, if any, were touched.  When
 * the app's loop() function detects a touch event, it should pass the touch coordinates
 * to this static class method.
 *
 * Note:  We are not, at least for now, handling overlapping widgets on the
 * screen.  If overlapping widgets have been created and touched, we notify
 * only the first in the list.  Perhaps this will change someday???
 */
void AWidget::processTouch(uint16_t xCoord, uint16_t yCoord) {
    DTRACE();
    for (AWidget* scannedWidget = allWidgets; scannedWidget != NULL; scannedWidget = scannedWidget->next) {
        // DTRACE();
        //  If touch coords lie within scanned widget and call its notification method if provided
        if (scannedWidget->boundary.isWithin(xCoord, yCoord)) {
            // DTRACE();
            scannedWidget->onTouchWidget(xCoord, yCoord);
        }
    }
}  // processTouch()

/**
 * @brief Determine if a widget has a drawn border
 * @return true if has a border, else false
 *
 * A widget has a border if its border color != its background color
 */
bool AWidget::hasBorder() {
    return bdColor != bgColor;
}

/**
 * @brief Determine if someWidget overlaps this widget
 * @param someWidget The other widget which might overlap this widget
 * @return true if the two widgets' screen areas overlap
 *
 * Two widgets overlap if any point on one, including their borders, overlaps any point on the other
 */
bool AWidget::overlaps(AWidget* someWidget) {
    // Does one widget lie entirely to the left of the other?
    if (someWidget->boundary.x2 < this->boundary.x1 || someWidget->boundary.x1 > this->boundary.x2) return false;

    // Does one widget lie entirely below the other?
    if (someWidget->boundary.y2 < this->boundary.y1 || someWidget->boundary.y1 > this->boundary.y2) return false;

    // This and someWidget must overlap somehow
    return true;
}