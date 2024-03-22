#include <usbhid.h>
#include <hiduniversal.h>
#include <usbhub.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Satisfy IDE, which only needs to see the include statment in the ino.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

#include "hidjoystickrptparser.h"

// Initialize the LiquidCrystal_I2C object
// Parameters: (I2C address, Number of columns, Number of rows)
LiquidCrystal_I2C lcd(0x27, 16, 2);

/* variables */
uint8_t rcode;
uint8_t usbstate;
uint8_t laststate;
//uint8_t buf[sizeof(USB_DEVICE_DESCRIPTOR)];
USB_DEVICE_DESCRIPTOR buf;

USB Usb;
USBHub Hub(&Usb);
HIDUniversal Hid(&Usb);
JoystickEvents JoyEvents;
JoystickReportParser Joy(&JoyEvents);

// Joystick values
int* X1, *Y1, *X2, *Y2; // Pointers to Joysticks (X and Y)
bool* button[11]; // Button array
int* hatSwitch; // Hat switch, 0-7 for direction or 8 for no press.
bool joystickConnected = false; // Joystick connected flag

void setup() {
        Serial.begin(115200);
        // Initialize the LCD
        lcd.init();
        lcd.backlight();
        printToScreen("USB HID Joystick", "Initializing...");
#if !defined(__MIPSEL__)
        while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
        Serial.println("Start");

        if (Usb.Init() == -1)
                Serial.println("OSC did not start.");

        delay(200);

        if (!Hid.SetReportParser(0, &Joy))
                ErrorMessage<uint8_t > (PSTR("SetReportParser"), 1);
}

void loop() {
        Usb.Task();
        usbState();
        printJoystickState();
}

void usbState()
{
        usbstate = Usb.getUsbTaskState();
        if (usbstate != laststate)
        {
                laststate = usbstate;
                /**/
                switch (usbstate)
                {
                case (USB_DETACHED_SUBSTATE_WAIT_FOR_DEVICE):
                        E_Notify(PSTR("\r\nWaiting for device..."), 0x80);
                        printToScreen("Waiting for", "device...");
                        break;
                case (USB_ATTACHED_SUBSTATE_RESET_DEVICE):
                        E_Notify(PSTR("\r\nDevice connected. Resetting..."), 0x80);
                        break;
                case (USB_ATTACHED_SUBSTATE_WAIT_SOF):
                        E_Notify(PSTR("\r\nReset complete. Waiting for the first SOF..."), 0x80);
                        break;
                case (USB_ATTACHED_SUBSTATE_GET_DEVICE_DESCRIPTOR_SIZE):
                        E_Notify(PSTR("\r\nSOF generation started. Enumerating device..."), 0x80);
                        break;
                case (USB_STATE_ADDRESSING):
                        E_Notify(PSTR("\r\nSetting device address..."), 0x80);
                        break;
                case (USB_STATE_RUNNING):
                        E_Notify(PSTR("\r\nGetting device descriptor"), 0x80);
                        rcode = Usb.getDevDescr(1, 0, sizeof(USB_DEVICE_DESCRIPTOR), (uint8_t *)&buf);

                        if (rcode)
                        {
                                E_Notify(PSTR("\r\nError reading device descriptor. Error code "), 0x80);
                                print_hex(rcode, 8);
                        }
                        else
                        {
                                /**/
                                E_Notify(PSTR("\r\nVendor  ID:\t\t"), 0x80);
                                print_hex(buf.idVendor, 16);
                                E_Notify(PSTR("\r\nProduct ID:\t\t"), 0x80);
                                print_hex(buf.idProduct, 16);
                                if (String(buf.idProduct, HEX) == "c219")
                                {
                                        printToScreen("F710 Joystick", "Connected");
                                }
                                else
                                {
                                        printToScreen("F710 Joystick", "Not Connected");
                                }
                                E_Notify(PSTR("\r\n"), 0x80);
                                /**/
                        }
                        break;
                case (USB_STATE_ERROR):
                        E_Notify(PSTR("\r\nUSB state machine reached error state"), 0x80);
                        break;

                default:
                        break;
                } // switch( usbstate...
        }
}

/* prints hex numbers with leading zeroes */
void print_hex(int v, int num_places)
{
        int mask = 0, n, num_nibbles, digit;

        for (n = 1; n <= num_places; n++)
        {
                mask = (mask << 1) | 0x0001;
        }
        v = v & mask; // truncate v to specified number of places

        num_nibbles = num_places / 4;
        if ((num_places % 4) != 0)
        {
                ++num_nibbles;
        }
        do
        {
                digit = ((v >> (num_nibbles - 1) * 4)) & 0x0f;
                Serial.print(digit, HEX);
        } while (--num_nibbles);
}

void printToScreen(String line1, String line2)
{
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(line1);
        lcd.setCursor(0, 1);
        lcd.print(line2);
}

void printJoystickState() {
        int lxa = JoyEvents.lx - 128;
        int lya = 127 - JoyEvents.ly;
        int rxa = JoyEvents.rx - 128;
        int rya = 127 - JoyEvents.ry;

        int buttons = (Joy.blue ? 0x001 : 0) |
                (Joy.green ? 0x002 : 0) |
                (Joy.red ? 0x004 : 0) |
                (Joy.yellow ? 0x008 : 0) |
                (Joy.lb ? 0x010 : 0) |
                (Joy.rb ? 0x020 : 0) |
                (Joy.lt ? 0x040 : 0) |
                (Joy.rt ? 0x080 : 0) |
                (Joy.bk ? 0x100 : 0) |
                (Joy.st ? 0x200 : 0) |
                (Joy.jl ? 0x400 : 0) |
                (Joy.jr ? 0x800 : 0);

        if (buttons == 0) buttons = 0x000;

        Serial.print("Joy ");
        Serial.print(buttons & 0x00F, HEX);
        Serial.print(" ");
        Serial.print((buttons >> 4) & 0x0F, HEX);
        Serial.print(" ");
        Serial.print(buttons >> 8, HEX);
        Serial.print(" ");
        Serial.print(JoyEvents.ht);
        Serial.print(" ");
        Serial.print(lxa < 0 ? "-" : "+");
        Serial.print(abs(lxa) < 100 ? "0" : String(abs(lxa) / 100));
        Serial.print(abs(lxa) < 10 ? "0" : String((abs(lxa) / 10) % 10));
        Serial.print(abs(lxa) % 10);
        Serial.print(" ");
        Serial.print(lya < 0 ? "-" : "+");
        Serial.print(abs(lya) < 100 ? "0" : String(abs(lya) / 100));
        Serial.print(abs(lya) < 10 ? "0" : String((abs(lya) / 10) % 10));
        Serial.print(abs(lya) % 10);
        Serial.print(" ");
        Serial.print(rxa < 0 ? "-" : "+");
        Serial.print(abs(rxa) < 100 ? "0" : String(abs(rxa) / 100));
        Serial.print(abs(rxa) < 10 ? "0" : String((abs(rxa) / 10) % 10));
        Serial.print(abs(rxa) % 10);
        Serial.print(" ");
        Serial.print(rya < 0 ? "-" : "+");
        Serial.print(abs(rya) < 100 ? "0" : String(abs(rya) / 100));
        Serial.print(abs(rya) < 10 ? "0" : String((abs(rya) / 10) % 10));
        Serial.print(abs(rya) % 10);
        Serial.println();
}