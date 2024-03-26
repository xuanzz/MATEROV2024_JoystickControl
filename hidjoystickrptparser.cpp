#include "Arduino.h"
#include "hidjoystickrptparser.h"
int motorMinVal = 70;
int motorMaxVal = 110;

JoystickReportParser::JoystickReportParser(JoystickEvents *evt) : joyEvents(evt),
                                                                  oldHat(0xDE),
                                                                  oldButtons(0)
{
        for (uint8_t i = 0; i < RPT_GEMEPAD_LEN; i++)
                oldPad[i] = 0xD;
}

void JoystickReportParser::Parse(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf)
{
        bool match = true;

        // Checking if there are changes in report since the method was last called
        for (uint8_t i = 0; i < RPT_GEMEPAD_LEN; i++)
                if (buf[i] != oldPad[i])
                {
                        match = false;
                        break;
                }

        // Calling Game Pad event handler
        if (!match && joyEvents)
        {
                joyEvents->OnGamePadChanged((const GamePadEventData *)buf);

                for (uint8_t i = 0; i < RPT_GEMEPAD_LEN; i++)
                        oldPad[i] = buf[i];
        }

        uint8_t hat = (buf[5] & 0xF);

        // Calling Hat Switch event handler
        if (hat != oldHat && joyEvents)
        {
                joyEvents->OnHatSwitch(hat);
                oldHat = hat;
        }

        uint16_t buttons = (0x0000 | buf[6]);
        buttons <<= 4;
        buttons |= (buf[5] >> 4);
        uint16_t changes = (buttons ^ oldButtons);

        // Calling Button Event Handler for every button changed
        if (changes)
        {
                for (uint8_t i = 0; i < 0x0C; i++)
                {
                        uint16_t mask = (0x0001 << i);

                        if (((mask & changes) > 0) && joyEvents)
                        {
                                if ((buttons & mask) > 0)
                                        joyEvents->OnButtonDn(i + 1);
                                else
                                        joyEvents->OnButtonUp(i + 1);
                        }
                }
                oldButtons = buttons;
        }
}

void JoystickEvents::OnGamePadChanged(const GamePadEventData *evt)
{
        int x1, y1, x2, y2;
        int m1Val,m2Val,m3Val,m4Val,m5Val,m6Val,m7Val,m8Val;
        x1 = map(evt->Y, 0, 255, motorMinVal, motorMaxVal);  // map the value of X1
        y1 = map(evt->Z1, 0, 255, motorMinVal, motorMaxVal); // map the val of Y1    
        y2 = map(evt->Rz, 0, 255, motorMinVal, motorMaxVal); //map the val of y2
        m1Val = constrain((x1 + y1),motorMinVal,motorMaxVal); //limit the value of left motor
        m2Val = constrain((y1 - x1),motorMinVal,motorMaxVal); //limit the value of right motor
        m3Val = constrain(y2 ,motorMinVal, motorMaxVal);
        m4Val = constrain(y2 ,motorMinVal,motorMaxVal);
        Serial.println("M," + String(m1Val) + "," + String(m2Val) + "," + String(m3Val) + "," + String(m4Val));
        
        // Serial.print("X1: ");
        // PrintHex<uint8_t > (evt->Y, 0x80);
        // Serial.print("\tY1: ");
        // PrintHex<uint8_t > (evt->Z1, 0x80);
        // Serial.print("\tX2: ");
        // PrintHex<uint8_t > (evt->Z2, 0x80);
        // Serial.print("\tY2: ");
        // PrintHex<uint8_t > (evt->Rz, 0x80);
        // Serial.println("");
}

void JoystickEvents::OnHatSwitch(uint8_t hat)
{
        Serial.print("H,");
        Serial.print(hat, DEC);
        Serial.println("");
}

void JoystickEvents::OnButtonUp(uint8_t but_id)
{
        Serial.print("F,");
        Serial.print(but_id, DEC);
        Serial.println(",0");
}

void JoystickEvents::OnButtonDn(uint8_t but_id)
{
        Serial.print("F,");
        Serial.print(but_id, DEC);
        Serial.println(",1");
}
