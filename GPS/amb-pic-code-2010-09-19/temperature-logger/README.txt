                        Temperature Logger
                        ------------------

1) Programming PIC must be done without any power connected, it uses the PIC
   programmer power.

2) The thermistor connects between the "5V" and "Ana" terminals.

3) If the thermistor is not connected then a value of 0xff is stored which is
   ignored by the data processing and shows as a break in the graph.

4) The thermal self-heating effect in the thermistor is significant and can
   change the result by several degrees if the voltage is not specified
   correctly.  For a 5kOhm thermistor, 4k resistor and 5V supply at 20 degrees
   the error is 2.5 degrees (reading is too low) if this factor is ignored.
   For a 50kOhm thermistor and 100k resistor the error is only 0.1 degrees.

5) Power consumption

   No thermistor        14 mV / 2kOhm =  7 uA.
   with thermistor      65 mV / 2kOhm = 33 uA.

   Active period of PIC is 1 ms in every 1 second (worse case).

   Active power is estimated at 1 mA for 1 ms = 1 uA continuous.
   
   Total power consumption < 50 uA => ~ 1 mAh per day.

6) Measurement Duration

   Interval    Duration
      1s            9h
      10s       3d 17h
      20s       7d  9h
      30s      11d  2h
      60s      22d  4h
