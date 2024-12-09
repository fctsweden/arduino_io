# arduino_io
-------------------------------------------------------------------------------------------
Functionality is same as pi pico IO but using arduino boards. Advantage is it has 6 ADC in channels
------------------------------------------------------------------------------------------
Suppoted Commands:
- ADC <pin>                # Read ADC channels 0-5 
- PWM <pin>, <duty>        # e.g. PWM 9, 50 : Set PWM channel 9 with duty 50%  (set fan speed) 
- PWM <pin>                # e.g. PWM 2     : Read PWM channel 2 (check fan speed) 

