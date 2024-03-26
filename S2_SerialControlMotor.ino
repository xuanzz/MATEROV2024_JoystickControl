#include <Servo.h>

Servo motor[8];    // Array to hold the motor objects
Servo gripper;  // Add 1 servo for gripper
String readString; // String to hold incoming command from serial port
#define indicator 13 // Indicator LED pin
#define gripperOpen 150 //gripper open value
#define gripperClose 30 //gripper close value
#define gripperStop 93 //gripper stop value

void setup()
{
  Serial.begin(115200);  // Start serial communication at 115200 baud rate
  Serial2.begin(115200); // Communication with the Arduino on shore
  pinMode(indicator, OUTPUT); // Set the indicator LED pin as output
  digitalWrite(indicator, LOW); // Turn off the indicator LED
  gripper.attach(10);
  gripper.write(gripperStop);
  // for (int i = 0; i < 8; i++)
  // {
  //   motor[i].attach(i + 2); // Attach the motors to pins 2 through 9
  // }
}

void loop()
{
  if (Serial2.available())
  {
    char c = Serial2.read(); // Get one byte from serial buffer
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

void processCommand(String command)
{
  char functionType = command.charAt(0); // Get the function type (M or F)
  command = command.substring(2);        // Remove the function type and comma from the command
  int commaIndex = command.indexOf(','); // Find the index of the first comma
  // Serial.println("Function type: " + functionType);
  if (functionType == 'M')
  {e
    // Motor control function
    Serial.println("Motor speed: " + command);
    int i = 0;
    while (commaIndex != -1 && i < 8)
    {
      int value = command.substring(0, commaIndex).toInt(); // Get the value before the comma
      motor[i].write(value);                                // Control the motor with the value
      command = command.substring(commaIndex + 1);          // Remove the processed part from the command
      commaIndex = command.indexOf(',');                    // Find the next comma
      i++;
    }
    // Process the last value if there are less than 8 motors
    if (i < 8)
    {
      int lastValue = command.toInt();
      motor[i].write(lastValue); // Control the motor with the last value
    }
  }
  else if (functionType == 'F')
  {
    // Toggle function
    int functionNumber = command.substring(0, commaIndex).toInt(); // Get the function number
    int functionState = command.substring(commaIndex + 1).toInt(); // Get the function state
    // Call your toggle function here with the function number and state
    toggleFunction(functionNumber, functionState);
  }
  else if (functionType == 'S')
  {
    // Stop all motors
    for (int i = 0; i < 8; i++)
    {
      motor[i].write(90); // Stop the motor
    }
    Serial.println("All motors stopped"); 
  }
  else if (functionType == 'H') //trigger when hat button is pressed
  {
  }
  else if (functionType == 'P')
  {
    // Ping function
    Serial2.println("P");
  }
  else
  {
    Serial.println("Invalid function type");
  }
}

// Function to toggle a specific function based on the function number and state
void toggleFunction(int functionNumber, int functionState)
{
  // Implement your toggle function here
  Serial.print("Function " + (String)functionNumber + " toggled to state " + (String)functionState);
  switch (functionNumber)
  {
  case 1:
    // Button X, Blue
    break;
  case 2:
    // Button A, Green
    break;
  case 3:
    // Button B, Red
    break;
  case 4:
    // Button Y, Yellow
    break;
  case 5:
    // Button L1, Left bumper
    break;
  case 6:
    // Button R1, Right bumper
    if (functionState == 1)
    {
      gripper.write(gripperStop + 60); //open the gripper
    }
    else
    {
      gripper.write(gripperStop); //stop the gripper
    }
    break;
  case 7:
    // Button L2, Left trigger
    break;
  case 8:
    // Button R2, Right trigger
    if (functionState == 1)
    {
      gripper.write(gripperStop - 60); //close the gripper
    }
    else
    {
      gripper.write(gripperStop); //stop the gripper
    }
    break;
  case 9:
    // Button Back, detach all motors
    for (int i = 0; i < 8; i++)
    {
      motor[i].detach();
    }
    digitalWrite(indicator, LOW); // Turn off the indicator LED
    Serial.println("Disarmed!");
    break;
  case 10:
    // Button Start, attach all motors and set to 90
    for (int i = 0; i < 8; i++)
    {
      motor[i].attach(i + 2);
      motor[i].write(90);
    }    
    delay(1000); //wait for 1 second
    digitalWrite(indicator, HIGH); // Turn on the indicator LED
    Serial.println("Armed and ready!");
    break;
  case 11:
    // Button L3, Left stick button
    break;
  case 12:
    // Button R3, Right stick button
    break;
  default:
    // Invalid function number
    Serial.println("Invalid function number");
    break;
  }

}