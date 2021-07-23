#include "setserial.h"

void printTimestamp(Print *_logOutput)
{

    // Division constants
    const unsigned long MSECS_PER_SEC = 1000;
    const unsigned long SECS_PER_MIN = 60;
    const unsigned long SECS_PER_HOUR = 3600;
    const unsigned long SECS_PER_DAY = 86400;

    // Total time
    const unsigned long msecs = millis();
    const unsigned long secs = msecs / MSECS_PER_SEC;

    // Time in components
    const unsigned long MilliSeconds = msecs % MSECS_PER_SEC;
    const unsigned long Seconds = secs % SECS_PER_MIN;
    const unsigned long Minutes = (secs / SECS_PER_MIN) % SECS_PER_MIN;
    const unsigned long Hours = (secs % SECS_PER_DAY) / SECS_PER_HOUR;

    // Time as string
    char timestamp[20];
    sprintf(timestamp, "%02d:%02d:%02d.%03d ", (int)Hours, (int)Minutes, (int)Seconds, (int)MilliSeconds);
    _logOutput->print(timestamp);
}

void printLogLevel(Print *_logOutput, int logLevel)
{
    /// Show log description based on log level
    switch (logLevel)
    {
    default:
    case 0:
        _logOutput->print("SILENT\t");
        break;
    case 1:
        _logOutput->print("FATAL\t");
        break;
    case 2:
        _logOutput->print("ERROR\t");
        break;
    case 3:
        _logOutput->print("WARNING\t");
        break;
    case 4:
        _logOutput->print("INFO\t");
        break;
    case 5:
        _logOutput->print("TRACE\t");
        break;
    case 6:
        _logOutput->print("VERBOSE\t");
        break;
    }
}

void printPrefix(Print *_logOutput, int logLevel)
{
    printTimestamp(_logOutput);
    printLogLevel(_logOutput, logLevel);
}

void printSuffix(Print *_logOutput, int logLevel)
{
    _logOutput->print("");
}

void setSerial()
{
    Serial.begin(BAUD);
    Serial.flush();
    Serial.println();
    Log.begin(LOG_LEVEL_VERBOSE, &Serial, false);
    Log.setPrefix(printPrefix); // Set log prefix similar to NLog
    Log.setSuffix(printSuffix); // Set log suffix
    Log.notice(F("Serial logging started at %l." CR), BAUD);
}
