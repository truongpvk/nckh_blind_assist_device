#ifndef NAVIGATION_MANAGER_H
#define NAVIGATION_MANAGER_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <Audio.h>
#include "config.h"

int CreateRoute(const GPSData& currentLocation, const GPSData& destination);
String GetNavigationInstruction(const GPSData& currentLocation);

#endif