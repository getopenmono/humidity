#include "app_controller.h"
#include "lib/dht.hpp"
#include <Fonts/FreeSans24pt7b.h>
#include <cmath>
using mono::geo::Rect;
using mono::IApplicationContext;
using mono::io::DigitalOut;
using mono::String;
using mono::Timer;
using mono::ui::TextLabelView;

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
    humidityValueLabel(Rect(0,30,176,42),"--.--"),
    temperatureLabel(Rect(0,80,176,20),HUMIDITY_TEMPERATURE_LABEL),
    temperatureValueLabel(Rect(0,100,176,42),"--.--"),
    statusLabel(Rect(0,190,176,20),"starting...")
{
}

void AppController::monoWakeFromReset ()
{
    monoWakeFromSleep();
}

void AppController::setJackTipTo3V3 ()
{
    DigitalOut(VAUX_EN,1);
    DigitalOut(VAUX_SEL,1);
    DigitalOut(JPO_nEN,0);
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
    humidityLabel.setTextColor(TurquoiseColor);
    humidityValueLabel.setAlignment(TextLabelView::ALIGN_CENTER);
    humidityValueLabel.setFont(FreeSans24pt7b);
    humidityValueLabel.setTextColor(AlizarinColor);
    temperatureLabel.setAlignment(TextLabelView::ALIGN_CENTER);
    temperatureLabel.setTextColor(TurquoiseColor);
    temperatureLabel.setAlignment(TextLabelView::ALIGN_CENTER);
    temperatureValueLabel.setAlignment(TextLabelView::ALIGN_CENTER);
    temperatureValueLabel.setFont(FreeSans24pt7b);
    temperatureValueLabel.setTextColor(AlizarinColor);
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
    sleeper.Start();
    measure.setCallback<AppController>(this,&AppController::requestSensorReading);
    measure.Start();
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
    sleeper.Stop();
    measure.Stop();
}

void AppController::requestSensorReading ()
{
    sensor.send(18*1000,2,buffer,6,this,&AppController::handleReading);
}

void AppController::handleReading ()
{
    debugLine(String::Format("%02X %02X %02X %02X",buffer[0],buffer[1],buffer[2],buffer[3]));
    dht::SensorType type = dht::guessSensorType(buffer,5);
    float temperature = NAN;
    float humidity = NAN;
    switch (type)
    {
        default:
        case dht::Unknown:
            statusLabel.setText("no reading");
            break;
        case dht::DHT22:
            statusLabel.setText("DHT22");
            temperature = dht::dht22::getTemperatureC(buffer,5);
            humidity = dht::dht22::getHumidity(buffer,5);
            break;
        case dht::DHT11:
            statusLabel.setText("DHT11");
            temperature = dht::dht11::getTemperatureC(buffer,5);
            humidity = dht::dht11::getHumidity(buffer,5);
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
