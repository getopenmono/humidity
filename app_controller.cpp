#include "app_controller.h"
#include <dht.h>
#include <Fonts/FreeSans24pt7b.h>
#include <cmath>
using mono::geo::Rect;
using mono::IApplicationContext;
using mono::String;
using mono::Timer;
using mono::ui::TextLabelView;
using namespace mono::sensor::dht;

#define HUMIDITY_HUMIDITY_LABEL "humidity"
#define HUMIDITY_TEMPERATURE_LABEL "temperature"

AppController::AppController ()
:
    sleeper(120*1000),
    measure(3*1000),
    toucher(this),
    sensor(J_RING2,50,27,70,50+80+5*8*(50+70)),
    button(J_RING1,PullUp),
    metric(true),
    humidityLabel(Rect(0,10,176,20),HUMIDITY_HUMIDITY_LABEL),
    humidityValueLabel(Rect(0,35,176,42),"--.--"),
    temperatureLabel(Rect(0,95,176,20),HUMIDITY_TEMPERATURE_LABEL),
    temperatureValueLabel(Rect(0,120,176,42),"--.--"),
    statusLabel(Rect(0,190,176,20),"starting...")
{
}

void AppController::monoWakeFromReset ()
{
    monoWakeFromSleep();
}

void AppController::setJackTipTo3V3 ()
{
    mono::io::DigitalOut(VAUX_EN,1);
    mono::io::DigitalOut(VAUX_SEL,1);
    mono::io::DigitalOut(JPO_nEN,0);
}

void AppController::setUpButtonHandler ()
{
    button.setDebouncing(true);
    button.rise(this,&AppController::handleClick);
    button.setInterruptsSleep(true);
}

void AppController::setupUi ()
{
    humidityLabel.setAlignment(TextLabelView::ALIGN_CENTER);
    humidityLabel.setText(TurquoiseColor);
    humidityValueLabel.setAlignment(TextLabelView::ALIGN_CENTER);
    humidityValueLabel.setFont(FreeSans24pt7b);
    humidityValueLabel.setText(AlizarinColor);
    temperatureLabel.setAlignment(TextLabelView::ALIGN_CENTER);
    temperatureLabel.setText(TurquoiseColor);
    temperatureLabel.setAlignment(TextLabelView::ALIGN_CENTER);
    temperatureValueLabel.setAlignment(TextLabelView::ALIGN_CENTER);
    temperatureValueLabel.setFont(FreeSans24pt7b);
    temperatureValueLabel.setText(AlizarinColor);
    statusLabel.setAlignment(TextLabelView::ALIGN_CENTER);
    humidityLabel.show();
    humidityValueLabel.show();
    temperatureLabel.show();
    temperatureValueLabel.show();
    statusLabel.show();
}

void AppController::setupTimersAndHandler ()
{
    sleeper.setCallback(IApplicationContext::EnterSleepMode);
    sleeper.start();
    measure.setCallback<AppController>(this,&AppController::requestSensorReading);
    measure.start();
}

void AppController::monoWillGotoSleep ()
{
    debugLine("ZZzzzzz...");
    turnOffJackTipVcc();
    stopTimers();
}

void AppController::monoWakeFromSleep ()
{
    setupTimersAndHandler();
    setJackTipTo3V3();
    setUpButtonHandler();
    setupUi();
}

void AppController::turnOffJackTipVcc ()
{
    DigitalOut(JPO_nEN,1);
}

void AppController::stopTimers ()
{
    sleeper.stop();
    measure.stop();
}

void AppController::requestSensorReading ()
{
    sensor.send(18*1000,2,buffer,6,this,&AppController::handleReading);
}

void AppController::handleReading ()
{
    debugLine(String::Format("%02X %02X %02X %02X",buffer[0],buffer[1],buffer[2],buffer[3]));
    SensorType type = guessSensorType(buffer,5);
    float temperature = NAN;
    float humidity = NAN;
    switch (type)
    {
        default:
        case Unknown:
            statusLabel.setText("no reading");
            break;
        case DHT22:
            statusLabel.setText("DHT22");
            temperature = dht22::getTemperatureC(buffer,5);
            humidity = dht22::getHumidity(buffer,5);
            break;
        case DHT11:
            statusLabel.setText("DHT11");
            temperature = dht11::getTemperatureC(buffer,5);
            humidity = dht11::getHumidity(buffer,5);
            break;
    }
    updateUi(temperature,humidity);
}

void AppController::updateUi (float temperatureC, float humidity)
{
    if (std::isnan(temperatureC))
        temperatureValueLabel.setText("--.--");
    else
    {
        if (metric)
        {
            int tempWhole = temperatureC;
            unsigned tempDecimal = (temperatureC - tempWhole) * 10.0;
            temperatureValueLabel.setText(String::Format("%d.%01uC",tempWhole,tempDecimal));
        }
        else
        {
            float temperature = temperatureC * 9.0 / 5.0 + 32.0;
            int tempWhole = temperature;
            unsigned tempDecimal = (temperature - tempWhole) * 10.0;
            temperatureValueLabel.setText(String::Format("%d.%01uF",tempWhole,tempDecimal));
        }
    }
    if (std::isnan(humidity))
        humidityValueLabel.setText("--.--");
    else
    {
        unsigned humiWhole = humidity;
        unsigned humiDecimal = (humidity - humiWhole) * 10.0;
        humidityValueLabel.setText(String::Format("%u.%01u%%",humiWhole,humiDecimal));;
    }
    temperatureValueLabel.scheduleRepaint();
    humidityValueLabel.scheduleRepaint();
    statusLabel.scheduleRepaint();
}

void AppController::handleTouch ()
{
    handleClick();
}

void AppController::handleClick ()
{
    metric = !metric;
    async(this,&AppController::handleReading);
}

void AppController::debugLine (String msg)
{
    printf(String::Format("%s\r\n",msg())());
}

void AppController::error (String shortMsg, String longMsg)
{
    debugLine(longMsg);
    statusLabel.setText(shortMsg());
}
