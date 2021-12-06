#include <Arduino.h>

#ifdef BOARD_STM32
#define MY_RF24_CE_PIN (PB0)
#define MY_RF24_CS_PIN (PB2)
#define MY_RF24_IRQ_PIN (PA15)
#define MY_DEFAULT_RX_LED_PIN (PA1)
#define MY_SIGNING_SOFT_RANDOMSEED_PIN PA12
#endif

#ifdef BOARD_RFNANO
#define MY_RF24_CE_PIN 9 
#define MY_RF24_CS_PIN 10 
#endif

#include "config.h"
#include <MySensors.h>

void setup()
{
    Serial.println("Hello!");
}

void loop()
{
    Serial.println("Hello!");
}