#ifndef app_controller_h
#define app_controller_h

#include <mono.h>
#include "toucher.hpp"
#include "onewire.hpp"

class AppController
:
    public mono::IApplication
{
    mono::Timer sleeper;
    mono::Timer measure;
    Toucher toucher;
    uint8_t buffer[5];
    OneWire sensor;
    mono::QueueInterrupt button;
    bool metric;
    mono::ui::TextLabelView humidityLabel;
    mono::ui::TextLabelView humidityValueLabel;
    mono::ui::TextLabelView temperatureLabel;
    mono::ui::TextLabelView temperatureValueLabel;
    mono::ui::TextLabelView statusLabel;
    void setJackTipTo3V3 ();
    void setUpButtonHandler ();
    void turnOffJackTipVcc ();
    void stopTimers ();
    void setupUi ();
    void setupTimersAndHandler ();
    void requestSensorReading ();
    void handleReading ();
    void updateUi (float temperatureC, float humidity);
    void error (mono::String shortMsg, mono::String longMsg);
    void debugLine (mono::String msg);
public:
    AppController ();
    void monoWakeFromReset ();
    void monoWillGotoSleep ();
    void monoWakeFromSleep ();
    void handleTouch ();
    void handleClick ();
};

#endif /* app_controller_h */
