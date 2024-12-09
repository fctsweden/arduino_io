
#define  READ_PWM_INTERVAL      1000       // Interval in milliseconds
#define  SEC_2_MICROSEC         1000000
#define  DUTY_2_RAW(X)          ((X * 255)/100)


// HW configuration 
const int ledPin = 13;      // select the pin for the LED
const int pwmOutPin = 9;    // Select a pin capable of PWM (e.g., 3, 5, 6, 9, 10, 11 on Arduino Uno)
const int pwmInPin  = 2;    // measure FAN Speed

// PWM parameters 
volatile unsigned long highTime = 0;
volatile unsigned long lowTime  = 0;
volatile unsigned long lastTime = 0;
volatile unsigned long count    = 0;
unsigned long pwmIn_f = 0;
unsigned long previousMillis = 0;       // Stores the last time the task ran

/* --------------------------------------------------------  */

// Function to split a string by a delimiter
int splitString(String str, char delimiter, String* output, int maxParts) {
  int index = 0;
  int start = 0;

  while (index < maxParts) {
    int delimIndex = str.indexOf(delimiter, start);

    // If no more delimiters, add the last part
    if (delimIndex == -1) {
      output[index++] = str.substring(start);
      break;
    }

    // Extract part and update start position
    output[index++] = str.substring(start, delimIndex);
    start = delimIndex + 1;
  }

  return index; // Return the number of parts
}

int adcCmdHandler(String command)
{
    int result = 0;

    // Check Read ADC Command e.g. ADC 1
    if (command.startsWith("adc"))
    { 
      // Extract the pin number after "ADC" e.g "ADC 1"
      int pin = command.substring(3).toInt(); 
      // Validate that the pin number is between 0 and 5
      if (pin >= 0 && pin <= 5) 
      { 
        int adcValue = analogRead(pin); // Read the ADC value for the selected pin
        Serial.print("ADC");
        Serial.print(pin);
        Serial.print(": ");
        Serial.println(adcValue); // Send the ADC value back
        result = 1;
      } 
      else 
      {
         Serial.println("Error: Invalid ADC Channel!");
         result = 1; // adc command, invalid channel though
      }
    }    
    return result; 
}


int pwmCmdHandler(String command)
{
    int result = 0;
    if (command.startsWith("pwm"))
    {     
        // Check Set PWM command e.g. PWM 9, 50 - set pwm pin 9 50%
        String parts[2]; // Array to store the split parts
        int count = splitString(command, ',', parts, 2);
        if(count == 2)
        {
            // Extract the pin number and duty cycle
            int pin = parts[0].substring(3).toInt();
            int duty = parts[1].toInt();

            if(pin == pwmOutPin)
            {
                // Set PWM output 
                if(duty > 100) duty = 100;
                analogWrite(pin, DUTY_2_RAW(duty));
                Serial.print("Set:");
                Serial.println(command);
                result = 1;              
            }
            else
            {
               result = 1; // pwm command but invalid pin 
               Serial.println("Error: Invalid PWM Pin! Supported: PWM 9, <Duty>");
            }
        }
        else if(count == 1)
        {
            // Extract the pin number and duty cycle
            int pin = parts[0].substring(3).toInt();
            if(pin == pwmInPin)
            {
                // Get PWM2 
                Serial.print("PWM2:");
                Serial.println(pwmIn_f);
                result = 1;              
            }
            else
            {
               result = 1; // pwm command but invalid pin 
               Serial.println("Error: Invalid PWM Pin! Supported: PWM 2");
            }          
        }
        else
        {
            result = 1;  // pwm command but invalid format
            Serial.println("Error: Invalid Command! Try: PWM 9, <Duty>");
        }
    }
    return result;
}


void pwmInterrupt() {
    unsigned long now = micros();
    if (digitalRead(pwmInPin) == HIGH) {
        lowTime = now - lastTime;
    
    } else {
        highTime = now - lastTime;
    }
    lastTime = now;
}

void readPwmIn()
{
    static unsigned long last_period = 0;
    unsigned long period = highTime + lowTime;
    if (period > 0) {
      pwmIn_f = SEC_2_MICROSEC/(period);
      //Serial.print("pwm F(hz):");
      //Serial.println(pwmIn_f);

      // reset old values
      lowTime = 0;
      highTime = 0;
      last_period = period;
    }
    else if(last_period != period)
    {
      pwmIn_f = 0;
      last_period = period;
      //Serial.println("pwm F(hz):0");
    } 
}

void setup() 
{
    // Initialize Serial
    Serial.begin(115200); 

    // declare the ledPin as an OUTPUT:
    pinMode(ledPin, OUTPUT);
  
    // Init PWM 
    pinMode(pwmOutPin, OUTPUT);
    analogWrite(pwmOutPin, 0);
    pinMode(pwmInPin, INPUT_PULLUP); 
    attachInterrupt(digitalPinToInterrupt(pwmInPin), pwmInterrupt, CHANGE);
}

void loop() 
{
    
    // LED Control
    //digitalWrite(ledPin, HIGH);
    //delay(250); // Wait 1 second
    //digitalWrite(ledPin, LOW);
    //delay(250); // Wait 1 second
    
    if (Serial.available() > 0) 
    {
        int result = 0;
        String command = Serial.readStringUntil('\n'); // Read the incoming command
        if(command != "")
        {
            command.trim(); // Remove any leading or trailing whitespace
            command.toLowerCase(); // using low case only
            
            // Handle ADC commands 
            result = adcCmdHandler(command);
    
            // Handle PWM command 
            if( result == 0) result = pwmCmdHandler(command);
    
            // Invalid command 
            if( result == 0)  Serial.println("Error: Invalid Command!");         
        }
    }

    // Read PWM In every 1s
    unsigned long currentMillis = millis(); // Get current time
    if (currentMillis - previousMillis >= READ_PWM_INTERVAL)
    {
        previousMillis = currentMillis; // Update the last execution time
        readPwmIn();
    }    
}
