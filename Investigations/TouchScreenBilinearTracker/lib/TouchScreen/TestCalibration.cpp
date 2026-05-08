
#include <TouchScreen.h>
#include <AGUI.h>
#include <Arduino.h>
#include <AToggleButton.h>

bool TouchScreen::testCalibration(AGUI& agui) {
    // Display buttons on UI
    AToggleButton okButton = AToggleButton("OK", 100, 100);
    AToggleButton errButton = AToggleButton("ERR", 100 + 50 + 30, 100);

    
}
