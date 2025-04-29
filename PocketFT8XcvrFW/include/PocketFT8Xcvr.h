#pragma once

#include <Arduino.h>

#include "Config.h"
#include "Station.h"
#include "UserInterface.h"

// Global objects widely accessed
extern Station thisStation;
extern ConfigType config;
extern UserInterface ui;
