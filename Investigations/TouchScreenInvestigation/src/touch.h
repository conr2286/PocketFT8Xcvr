struct TouchSample {
    int16_t  x;      // 0..ADC_MAX (raw, uncalibrated)
    int16_t  y;      // 0..ADC_MAX (raw, uncalibrated)
    uint16_t z;      // pressure proxy
    bool     valid;  // true if stable + above Z threshold
};

void touchInit();
TouchSample touchRead();
