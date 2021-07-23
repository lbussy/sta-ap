#pragma once
#ifndef _SETSERIAL_H
#define _SETSERIAL_H

#include <Arduino.h>
#include <ArduinoLog.h>

void printTimestamp(Print *_logOutput);
void printLogLevel(Print *_logOutput, int logLevel);
void printPrefix(Print *_logOutput, int logLevel);
void printSuffix(Print *_logOutput, int logLevel);
void setSerial();

#endif // _SETSERIAL_H
