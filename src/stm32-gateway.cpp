/**
 * Necromancer's MySensors gateway based on cheapest STM32 dev board.
 * (c) Andrew 'Necromant' Andrianov
 * 
 * This gateway also provides you with a handful of stats. The code lacks polish
 * and has a few things hardcoded. I know that. Feel free to clean it up.
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#define MY_RF24_CE_PIN (PB0)
#define MY_RF24_CS_PIN (PB2)
#define MY_RF24_IRQ_PIN (PA15)
#define MY_DEFAULT_RX_LED_PIN (PA1)

/* 

IRQs aren't around yet. 
Shame, we have a crapload of RAM for the buffer

#define SPI_HAS_TRANSACTION
#define MY_RX_MESSAGE_BUFFER_FEATURE
#define MY_RX_MESSAGE_BUFFER_SIZE 128

*/

#include "config.h"
#include <MySensors.h>

class GatewayStats
{
private:
    bool first_loop = true;
    uint8_t offset = 0;
    const char *prefix;

    uint32_t total = 0;
    uint32_t session;
    uint32_t delta_max;
    uint32_t delta_min;
    uint32_t delta_avg;

    /* Time */
    uint32_t last_packet;
    uint32_t last_session = 0;
    MyMessage msg[5]; /* We have a crapload of RAM */

public:
    GatewayStats(uint8_t _offset, char *_prefix)
    {
        offset = _offset;
        prefix = _prefix;
        int i;
        for (i = 0; i < 5; i++)
        {
            msg[i] = MyMessage(offset + i, V_LEVEL);
        }
    };

    void report(){};

    void update_stats()
    {
        if (delta_avg > 0)
        {
            delta_avg = delta_avg / session;
        }
        msg[0].set(total);
        msg[1].set(session);
        msg[2].set((delta_min == UINT32_MAX) ? 0 : delta_min);
        msg[3].set(delta_max);
        msg[4].set(delta_avg);
        for (int i = 0; i < 5; i++)
        {
            send(msg[i]);
        }

        session = 0;
        delta_max = 0;
        delta_min = -1;
        delta_avg = 0;
    }

    void present_one(int id, mysensors_sensor_t tp, const char *val)
    {
        char buf[64];
        snprintf(buf, 64, "%s %s", val, prefix);
        present(id, tp, buf);
    }

    void present_self()
    {
        present_one(offset + 0, S_SOUND, "Overall");
        present_one(offset + 1, S_SOUND, "10 Min");
        present_one(offset + 2, S_SOUND, "Min. packet interval");
        present_one(offset + 3, S_SOUND, "Max. packet interval");
        present_one(offset + 4, S_SOUND, "Avg. packet interval");
    };

    void count(const MyMessage &msg)
    {
        total++;
        session++;

        uint32_t cur = millis();
        uint32_t delta = last_packet - cur;
        if (delta > delta_max)
        {
            delta_max = delta;
        }
        if (delta < delta_min)
        {
            delta_min = delta;
        }
        delta_avg += delta;
        last_packet = millis();
    };

    void loop()
    {
        if (first_loop)
        {
            first_loop = false;
            update_stats(); /* Send initial shit */
        }

        if ((millis() - last_session) > (1 * 60 * 1000))
        {
            update_stats();
            last_session = millis();
        } /* 10 Min */
    };
};

class Rebooter
{
    int offset;
    bool first_loop = true;
    MyMessage rpt;
    uint32_t lastbeat = 0;

public:
    Rebooter(int _offset)
    {
        offset = _offset;
        rpt = MyMessage(offset, V_STATUS);
        rpt.set((bool)false);
    }

    void present_self()
    {
        present(offset, S_BINARY, "Reboot to bootloader");
        present(offset+1, S_BINARY, "Reboot Gateway");
    }

    void loop()
    {
        if (first_loop)
        {
            first_loop = false;
            rpt.setSensor(offset);
            send(rpt);
            rpt.setSensor(offset+1);
            send(rpt);
        }
        if (millis() - lastbeat > 1 * 60 * 1000)
        {
            sendHeartbeat();
            lastbeat = millis();
        }
    }



#define SCB_AIRCR ((volatile uint32*) (0xE000ED00 + 0x0C))
#define SCB_AIRCR_SYSRESETREQ (1 << 2)
#define SCB_AIRCR_RESET ((0x05FA0000) | SCB_AIRCR_SYSRESETREQ)

    void receive(const MyMessage &message)
    {
        if ((message.getCommand() == C_SET) &&
            (message.getDestination() == 0) &&
            (message.getInt() == 1))
        {
            /* Drive the pin low, as if the button is pressed */
            if (message.getSensor() == offset) {
                /* reboot to bootloader */
                pinMode(PA8, OUTPUT);
                digitalWrite(PA8, LOW);
                wait(500);
                *(SCB_AIRCR) = SCB_AIRCR_RESET;
            }

            if (message.getSensor() == offset + 1) {
                /* reboot to application */
                pinMode(PA8, OUTPUT);
                digitalWrite(PA8, HIGH);
                wait(500);
                *(SCB_AIRCR) = SCB_AIRCR_RESET;
            }
            
        }
    }
};

void preHwInit()
{
    disableDebugPorts();
    pinMode(PA1, OUTPUT);
    pinMode(PB0, OUTPUT);
    pinMode(PB2, OUTPUT);
}

void before()
{
}

void setup()
{
    sendSketchInfo("The Ancient Gateway", "1.0");
}

GatewayStats stats_rx(0, (char *)"RX");
GatewayStats stats_tx(5, (char *)"TX");
Rebooter rbt(10);

void receive(const MyMessage &message)
{
    if (message.getSender() != 0)
    {
        stats_rx.count(message);
    } else {
        stats_tx.count(message);
    }

    rbt.receive(message);
}

void presentation()
{
    stats_rx.present_self();
    stats_tx.present_self();
    rbt.present_self();
}

void loop()
{
    stats_rx.loop();
    stats_tx.loop();
    rbt.loop();
}
