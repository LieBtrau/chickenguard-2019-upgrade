#pragma once

#include <Arduino.h>

void setupWebserver();
void loopWebserver();
void notifyClients(String key, String status);