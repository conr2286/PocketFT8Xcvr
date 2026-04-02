/**
 * TEST:
 *  + This test exercises the Adafruit 3.5" TFT 320x480 touchscreen display breakout board
 *
 * EXERCISED:
 *  + MCU connectivity with the Adafruit 3.5" Product 2050 display
 *  + MCU connectivity with the Adafruit 3.5" Product 2050 touchscreen
 *  + Display orientation
 *  + Display coordinate system
 *
 * USAGE:
 *  pio test -vvv -f "hw/test_05"       // Execute just this test
 *  pio test -vvv -f "hw/*"             // Execute all hw tests
 *
 * PREREQUISITES:
 *  + Version 4+ hardware (or:  XDM jumpered to A16, and YDP jumpered to A17)
 *  + The display needs to be working to prompt the operator
 *
 * NOTES:
 *  + Touchscreen (TS) uses 10-bit ADC conversions with a ~3.3V reference voltage.
 *  The resulting unsigned readings vary from 0 (0 Volts) to 1023 (~3.3 Volts).
 *  + The display coordinate system is 480X320 pixels addressed from 0..479 and 0..319
 *  + The textbook approach to read a resistive touchscreen's x-Axis is to ground one X pin,
 *  drive the other X pin with Vcc, and read xTouch from the floating y-Axis.  When
 *  the screen is touched, the touch pressure connects the x and y-Axis linear
 *  resistances at the touchpoint.
 *  + However, Teensy's 4.1 ADC has a very high input impedance incapable of suppressing
 *  noise on a floating axis.  When the screen isn't touched, the readings on a floating
 *  axis produce wildly inaccurate results indistinguishable from an actual touch.
 *  + We workaround the noise problem by lowering the impedance of a floating axis
 *  by grounding one side through a 510 Ohm resistor, greatly reducing the induced noise.
 *  + Pocket FT8 rotates the display in landscape mode, so the hardware's X-Axis becomes
 *  the Y-Axis.  When we refer to a "hardware axis" signal we are discussing the signal
 *  names (e.g. x-) silkscreened onto the display and schematic nodes (e.g. XDM).  We
 *  refer to coordinates on the rotated (landscape) screen as "screen" coordinates.
 *
 * SCHEMATIC:
 *                     YDM
 *                      *
 *                      |                           RY1 HW Y-Axis resistance above touchpoint
 *                      |                           RY2 HW Y-Axis resistance below touchpoint
 *                     RY1                            T Touchpoint "shorts" the Y-Axis to X-Axis
 *                      |                           RX1 HW X-Axis resistance left of touchpoint
 *         XDP          |          XDM              RX2 HW X-Axis resistance right of touchpoint
 *          *---RX1-----T-----RX2---*---510---* XR
 *                      |                           510 Ohm Resistors
 *                      |                           XDP Touchpad's X+ pin
 *                     RY2                          XDM Touchpad's X- pin
 *                      |                            XM Drives X-Axis from MCU
 *                      |                           YDM Touchpad's Y- pin
 *                      * YDP                       YDP Touchpad's Y+ pin
 *                      |                            YP Drives Y-Axis from MCU
 *                     510
 *                      |
 *                      *
 *                      YR
 *
 *
 *
 * GPIO:
 *  + 36 YDM (TS Y-)
 *  + 41 (A17) YDP (TS Y+)
 *  + 38 YP
 *  + 39 XDP (TS X+)
 *  + 40 (A16) XDM (TS X-)
 *  + 37 XM
 *
 */

#include <Arduino.h>
#include <Wire.h>
#include <unity.h>
#include "DEBUG.h"
#include "hwdefs.h"
#include "HX8357_t3n.h"       //WARNING:  #include HX8357_t3n following Adafruit_GFX
#include "TouchScreen_I2C.h"  //MCP342X interface to Adafruit's 2050 touchscreen

// Define touchscreen parameters
#define NOISE_LEVEL 50        // Tolerable analog noise (ADC reading)
#define X_ADC_MIN 20          // Min x adc reading
#define X_ADC_MAX 451         // Max x adc reading
#define Y_ADC_MIN 17          // Min y adc reading
#define Y_ADC_MAX 451         // Max y adc reading
#define X_SCREEN_MAX 480      // Max x screen coord
#define Y_SCREEN_MAX 320      // Max y screen coord
static const int delta = 20;  // Size of target in screen pixels

// Build the drivers for the Adafruit display and touchscreen
HX8357_t3n tft = HX8357_t3n(PIN_CS, PIN_DC, PIN_DRST, PIN_MOSI, PIN_DCLK, PIN_MISO);  // Teensy 4.1 pins
// TouchScreen ts = TouchScreen(PIN_XP, PIN_YR, PIN_XR, PIN_YM, 282);                    // The 282 ohms is the measured x-Axis resistance of 3.5" Adafruit touchscreen in 2024

/**
 * @brief Determine if a is approximately equal to b
 * @param a value1
 * @param b value2
 * @param delta allowed difference
 * @return true if a lies within the allowed difference of b
 */
bool isNear(unsigned a, unsigned b, unsigned delta) {
    if ((a >= b - delta) && (a <= b + delta)) return true;
    return false;
}  // isNear()

/**
 * @brief Helper function to configure GPIO pin to float
 * @param pin GPIO pin number
 *
 * @note A floating pin is allowed to float with analog noise at high impedance
 */
void floatPin(unsigned pin) {
    pinMode(pin, INPUT_DISABLE);  // Floats a Teensy 4.1 GPIO pin
    delayMicroseconds(20);        // Allow pin to settle
}  // floatPin()

/**
 * @brief Helper function to "ground" a pin
 * @param pin GPIO pin number
 *
 * @note The specified pin is brought to logic level zero, close to but not
 * truly ground.
 */
void groundPin(unsigned pin) {
    pinMode(pin, OUTPUT);    // Configure the pin for digital output
    digitalWrite(pin, LOW);  // And a logic zero
    delayMicroseconds(20);   // Allow pin to settle
}  // groundPin()

/**
 * @brief Helper function to supply Vcc to a pin
 * @param pin GPIO pin number
 *
 * @note The specified pin is actually supplied with logic level one, close to but
 * not truly Vcc.
 */
void vccPin(unsigned pin) {
    pinMode(pin, OUTPUT);     // Configure pin for digital output
    digitalWrite(pin, HIGH);  // Supply logic one to pin
    delayMicroseconds(20);    // Allow pin to settle
}  // vccPin()

/**
 * @brief Non-blocking read of touchscreen coordinates
 * @return TouchPoint coordinates in the rotated (landscape) screen system
 *
 * NOTES:
 *  + The returned TSPoint.z=0 if the x/y result is invalid
 *  + The returned TSPoint.z>0 if the x/y values are valid coordinates
 *  + No, we are not returning a resistive touchscreen pressure measurement
 */
static int xMin = 9999, xMax = 0;
static int yMin = 9999, yMax = 0;
static int xMx = 0, yMx = 0;
TSPoint getPoint(void) {
    TSPoint result;
    int adcX, adcY, adcZ;  // The raw (unmapped, unrotated) ADC readings
    bool touching;         // true ==> valid touch event

    // Begin setup to determine if we have a valid touch event (i.e. operator
    // is actually touching the pad) by driving the X-Axis to ~Vcc.  During
    // a valid touch event, the X and Y axis are connected by operator's
    // pressure on the pad.
    floatPin(PIN_XP);  // Float X+
    floatPin(PIN_XM);  // Also X-
    vccPin(PIN_XR);    // Drive the X-Axis to Vcc through its 510 Ohm resistor

    // Clear the noise from High-Z Y-Axis prior to analog reading below
    floatPin(PIN_YM);   // Float Y-
    floatPin(PIN_YR);   // Float YR
    groundPin(PIN_YP);  // Briefly ground Y+ to discharge noise from Y-Axis
    floatPin(PIN_YP);   // Prepare to read the Y+ signal by floating that pin

    // Vcc will flow through driven X-Axis connection to floating Y-Axis iff we have a touch event
    adcZ = analogRead(PIN_YP);                   // Read signal from floating Y-Axis
    floatPin(PIN_XR);                            // Remove Vcc to conserve battery
    touching = isNear(adcZ, 1023, NOISE_LEVEL);  // They're touching if Y-Axis is near VCC
    // DPRINTF("adcZ=%u touching=%u\n", adcZ, touching);

    // If we have a valid touchpoint then proceed to read the X and Y-Axis coordinates.  Because
    // the X and Y-Axis connect thru touchpoint, we have low-Z signals relatively free of noise.
    if (touching) {
        // Setup touchpad to read the hardware X-Axis signal from the Y-Axis
        floatPin(PIN_YM);  // Float touchpad Y-
        floatPin(PIN_YR);
        // groundPin(PIN_YR);  // Ground Y-Axis resistor (for noise immunity)
        // floatPin(PIN_XR);   // Float X-Axis resistor
        groundPin(PIN_XP);  // Ground touchpad X+
        floatPin(PIN_XM);
        // vccPin(PIN_XM);     // Drive the XM to Vcc
        vccPin(PIN_XR);

        // Read the touchpoint hardware X-Coordinate signal from the Y-Axis
        adcX = analogRead(PIN_YP);  //
        floatPin(PIN_XM);           // Remove Vcc to conserve battery

        // Setup touchpad to read the hardware Y-Axis signal from the floating X-Axis
        floatPin(PIN_XP);  // Float touchpad X+
        // groundPin(PIN_XR);  // Ground X-Axis resistor (for noise immunity)
        floatPin(PIN_XR);
        // floatPin(PIN_YR);   // Float Y-Axis resistor
        floatPin(PIN_XM);
        groundPin(PIN_YM);  // Ground Y-
        // vccPin(PIN_YP);     // Drive the Y+ to Vcc
        vccPin(PIN_YR);

        // Read the touchpoint's Y-Axis coordinate signal from the floating X-Axis
        adcY = analogRead(PIN_XM);  // Raw ADC value ranges 0..1023
        floatPin(PIN_YP);           // Remove Vcc to save battery

        // Record max and min
        DPRINTF("xMin=%d xMax=%d yMin=%d yMax=%d\n", xMin, xMax, yMin, yMax);
        if (adcX < xMin) xMin = adcX;
        if (adcX > xMax) xMax = adcX;
        if (adcY < yMin) yMin = adcY;
        if (adcY > yMax) yMax = adcY;

        // Return valid result in the rotated screen coordinate system
        if (touching) {
            result.y = map(adcX, 28, 382, 0, Y_SCREEN_MAX);  // adcX is floating x-Axis signal for Y-Axis coordinate
            result.x = map(adcY, 30, 540, 0, X_SCREEN_MAX);  // adcY is floating y-Axis signal for X-Axis coordinate
            result.z = adcZ;
            if (result.x > xMx) xMx = result.x;
            if (result.y > yMx) yMx = result.y;
        }
    } else {
        result.x = result.y = result.z = 0;  // Operator is not touching the pad
    }
    // DPRINTF("adcY= %u result.x=%u adcX=%u result.y=%u result.z=%u xMx=%u yMx=%u\n", adcY, result.x, adcX, result.y, result.z, xMx, yMx);
    //  DPRINTF("xMx=%u yMx=%u\n", xMx, yMx);

    return result;
}

/**
 * @brief Helper function to poll the touchscreen
 *
 * @return TSPoint Screen coordinate of touchpoint
 *
 * @note Returns TSPoint.z==0 if error or no timely touch
 *
 * The touchscreen is polled for a valid touch upto 5 seconds
 *
 */
TSPoint pollTouchscreen() {
    TSPoint p1;               // Touchpoint's screen coordinates
    unsigned dt = 100;        // Milliseconds between retries
    unsigned timeout = 5000;  // Milliseconds until timeout

    // Loop polls touchscreen until touch event or timeout
    for (unsigned retries = 0; retries < timeout; retries += dt) {
        p1 = getPoint();  // Read the screen

        // Touch?
        if (p1.z > MINPRESSURE) {
            // Serial.printf("ADC = (%d,%d,%d)\n", p1.x, p1.y, p1.z);
            return p1;  // Return touchpoint
        } else {
            // DTRACE();
            p1.z = 0;  // Invalid reading (not touched)
        }

        // Wait before polling again
        delay(dt);
    }
    return p1;  // Timeout error (no touch)
}

void test_etchSketch(void) {
    int dt = 10;         // Milliseconds
    int finish = 60000;  // Milliseconds
    int minX = 9999, maxX = 0;
    int minY = 9999, maxY = 0;
    for (int t = 0; t < finish; t += dt) {
        TSPoint p = getPoint();
        if (p.z) {
            if (p.x < minX) minX = p.x;
            if (p.x > maxX) maxX = p.x;
            if (p.y < minY) minY = p.y;
            if (p.y > maxY) maxY = p.y;
            tft.drawPixel(p.x, p.y, HX8357_YELLOW);
        }
        delay(dt);
    }
    DPRINTF("minX=%u maxX=%u minY=%u maxY=%u\n", minX, maxX, minY, maxY);
}  // test_etchSketch

/**
 * @brief Display target and prompt operator to touch it
 * @param x Screen X-Coordinate
 * @param y Screen Y-Coordinate
 */
void drawTarget(int x, int y) {
    int x1, y1, x2, y2;  // Screen coords of target lines
    int x3, y3;          // Screen coords of prompt

    // Draw target's horizontal line
    x1 = constrain(x - delta, 0, X_SCREEN_MAX);
    y1 = constrain(y, 0, Y_SCREEN_MAX);
    x2 = constrain(x + delta, 0, X_SCREEN_MAX);
    y2 = y1;
    tft.drawLine(x1, y1, x2, y2, HX8357_WHITE);

    // Draw target's vertical line
    x1 = constrain(x, 0, X_SCREEN_MAX);
    y1 = constrain(y - delta, 0, Y_SCREEN_MAX);
    x2 = x1;
    y2 = constrain(y + delta, 0, Y_SCREEN_MAX);
    tft.drawLine(x1, y1, x2, y2, HX8357_WHITE);

    // Prompt operator to touch the screen
    x3 = constrain(x - delta, 0, X_SCREEN_MAX - 2 * delta);
    y3 = constrain(y - delta, 0, Y_SCREEN_MAX - 2 * delta);
    tft.drawString("Touch", x3, y3);

}  // drawTarget()

// Unity function invoked prior to each test
void setUp(void) {
}

// Unity function invoked following each test
void tearDown(void) {
}  // tearDown()

/**
 * @brief Draw a rectangle and label the origin and lower-right corner
 *
 * @note There are not sensible assertions here because the HX8357 code
 * doesn't inform us when something fails, so we rely on the operator
 * observing a yellow rectangle on the display's boundaries, and
 * two text strings labeling the upper-left and lower-right corners'
 * coordinates.
 *
 * @note Sadly, we have no ready way to determine if the display worked
 */
void test_rect(void) {
    tft.drawRect(0, 0, 480, 320, HX8357_YELLOW);
    tft.drawPixel(0, 0, HX8357_WHITE);
    tft.drawString("(0,0)", 5, 8);
    tft.drawPixel(479, 319, HX8357_WHITE);
    tft.drawString("(479,319)", 340, 300);
}  // test_config()

/**
 * @brief Check the touchscreen wiring
 *
 * @note Be aware that the Teensy 4.1 ADC has a very high input impedance and readily
 * measures the induced noise (and there's plenty noise;) on a floating signal.  We
 * ground the free (not connected to the touchscreen) end of a 510 Ohm resistor
 * to reduce the circuit impendance of the axis under measurement.
 *
 * USAGE:
 *  This test should execute without the operator touching the panel
 *
 *
 * EXERCISED:
 *  + Verify the voltage on the grounded y-Axis is near logic 0
 *  + Verify the voltage on the grounded x-Axis is near logic 0
 *  + Confirm, sans touch, the x-Axis is disconnected from the y-Axis
 *
 */
void test_wiring(void) {
    int xdm, ydp;

    // Verify the voltage on a grounded y-Axis is at least near ground
    floatPin(PIN_XP);                     // Float all the x-Axis pins...
    floatPin(PIN_XM);                     // to ensure their mistakes...
    floatPin(PIN_XR);                     // don't interfere with y-Axis.
    floatPin(PIN_YM);                     // Float the Y- end of the y-Axis
    groundPin(PIN_YR);                    // Ground the y-Axis 510 Ohm resistor
    ydp = analogRead(PIN_YP);             // Read voltage on Y+
    TEST_ASSERT(abs(ydp) < NOISE_LEVEL);  // Y+ should be near logic zero

    // Verify the voltage on a grounded x-Axis is at least near ground
    floatPin(PIN_YM);                     // Float all the y-Axis pins...
    floatPin(PIN_YP);                     // to ensure their mistakes...
    floatPin(PIN_YR);                     // don't interfere with x-Axis.
    floatPin(PIN_XP);                     // Float the X+ end of the x-Axis
    groundPin(PIN_XR);                    // Ground the x-Axis 510 Ohm resistor
    xdm = analogRead(PIN_XM);             // Read voltage on X-
    TEST_ASSERT(abs(xdm) < NOISE_LEVEL);  // X- should be near logic zero

    // Check for an inadvertent connection between untouched x and y-Axis
    floatPin(PIN_XP);                     // Float X+
    groundPin(PIN_XR);                    // Ground the x-Axis resistor to reduce noise
    floatPin(PIN_YM);                     // Float Y-
    floatPin(PIN_YP);                     // Float Y+
    vccPin(PIN_YR);                       // Supply Vcc to y-Axis resistor
    xdm = analogRead(PIN_XM);             // Read voltage on X-
    TEST_ASSERT(abs(xdm) < NOISE_LEVEL);  // x-Axis should be near logic zero (no connection to charged y-Axis)

    // Cleanup driven pins to ensure programming mistakes don't create smoke
    floatPin(PIN_YR);  // Remove Vcc from Y+
}  // test_wiring()

/**
 * @brief Exercise the operator actually touching the center of the touchscreen
 *
 * @note The test draws targets on the screen and requests the operator
 * touch the screen at that location.
 *
 * @note The test will fail if the operator doesn't touch the screen in a timely
 * manner, or if the operator touches the screen in the wrong location.
 *
 * EXERCISED
 *  + Display
 *  + Touch event
 *  + Touchscreen's calibration (are we seeing the expected x/y coordinates)
 */
void test_touchCalibration(void) {
    TSPoint result;  // Touchpoint coordinates
    unsigned x, y;   // Target X/Y landscape (rotated) coordinates

    // Check a touchpoint near the center of the rotated screen
    x = 480 / 2;       // Target X landscape coordinate
    y = 320 / 2;       // Target Y landscape coordinate
    drawTarget(x, y);  // Draws the target and prompts opeator to touch

    // Read and check the touchpoint's screen coordinates
    // for (int i = 0; i < 100; i++) {
    TSPoint p = pollTouchscreen();  // Returns screen coords or z==0 if error
    DPRINTF("p.x=%u p.y=%u p.z=%u\n", p.x, p.y, p.z);
    TEST_ASSERT_NOT_EQUAL(0, p.z);       // Error?
    TEST_ASSERT(isNear(p.x, x, delta));  // p.x should be near x
    TEST_ASSERT(isNear(p.y, y, delta));  // p.y should be near y
    // delay(100);
    // }

}  // test_touchCalibration()

/*
 * @brief Run all tests in their prescribed order
 * @return Failure/Success indication
 */
int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_wiring);  // Checks touchscreen wiring
    // RUN_TEST(test_rect);              // Exercises the display
    // RUN_TEST(test_touchCalibration);  // Checks touchscreen calibration
    RUN_TEST(test_etchSketch);
    DPRINTF("xMin=%u xMax=%u yMin=%u yMax=%u\n", xMin, xMax, yMin, yMax);

    return UNITY_END();
}  // runUnityTests()

void setup() {
    delay(10);           // Wait for reliable comm with host computer when debugging
    Serial.begin(9600);  // Test message output device
    Serial.printf("Starting...\n");

    // Initialize the display
    tft.begin(30000000UL, 2000000UL);  // Configure SPI clock speeds
    tft.setRotation(3);                // Configure landscape screen rotation
    tft.fillScreen(HX8357_BLACK);      // Erase the display
    tft.setTextSize(2);

    // Run the tests
    runUnityTests();
}  // setup()

void loop() {
}  // loop()