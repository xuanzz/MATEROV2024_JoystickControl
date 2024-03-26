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
String readString; // String to hold incoming command from serial port
// uint8_t buf[sizeof(USB_DEVICE_DESCRIPTOR)];
USB_DEVICE_DESCRIPTOR buf;

USB Usb;
USBHub Hub(&Usb);
HIDUniversal Hid(&Usb);
JoystickEvents JoyEvents;
JoystickReportParser Joy(&JoyEvents);

void setup()
{
        Serial.begin(115200);
        // Initialize the LCD
        lcd.init();
        lcd.backlight();
        printToScreen("USB HID Joystick", "Initializing...");
#if !defined(__MIPSEL__)
        while (!Serial)
                ; // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
        Serial.println("Start");

        if (Usb.Init() == -1)
                Serial.println("OSC did not start.");

        delay(200);

        if (!Hid.SetReportParser(0, &Joy))
                ErrorMessage<uint8_t>(PSTR("SetReportParser"), 1);
}

void loop()
{
        Usb.Task();
        usbState();
        if (Serial.available())
        {
                char c = Serial.read(); // Get one byte from serial buffer
                if (c == '\r')
                {
                        processCommand(readString); // Process the command
                        readString = "";            // Clear the string for new input
                }
                else if (c == '\n')
                {
                        // Do nothing
                }
                else
                {
                        readString += c; // Build the string
                }
        }
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

void processCommand(String command)
{
        char functionType = command.charAt(0); // Get the function type (M or F)
        command = command.substring(2);        // Remove the function type and comma from the command
        int commaIndex = command.indexOf(','); // Find the index of the first comma
        // Serial.println("Function type: " + functionType);
        if (functionType == 'T')
        {
                // Motor control function
                Serial.println("Temperature: " + command);
                printToScreen("Temp: " + String(command[0])+String(command[1])+"."+String(command[2])+String(command[3])+"C","");
        }
        else
        {
                Serial.println("Invalid function type");
        }
}