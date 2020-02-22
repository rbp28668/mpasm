                                GPS data logger
                                ---------------

1) GPS is WD-G-ZX4120 device, operating at 3.3V.

2) Disconnect GPS and SD card and remove PIC power jumper before programming
   PIC.  Programming uses programmer 5V supply.

3) Power consumption

   Active:   40-45 mA (measured on 5V side of 3.3V regulator)

   Sleeping: 0.6 mA (measured on 5V side of 3.3V regulator)

4) Battery lifetime (based on 2000 mAh for NiH AA battery)

   Active:   2 days

   Sleeping: 4 months

5) NMEA messages, maximum lengths

   GGA  87 bytes  time, fix, lat, long, altitude, horizontal dilution of precision
       (78 bytes without DGPS information)   
   RMC  78 bytes  lat, long, speed, heading
   GLL  52 bytes  lat, long, time
   GSA  69 bytes  fix (none, 2D, 3D), satellites used, dilution of precision
   GSV  74 bytes  satellite information (4 messages)
   RMC  78 bytes  time, lat, long, speed, course, date
   VTG  51 bytes  heading, speed
   ZDA  40 bytes  time, day, month, year, timezone

   GSA + GGA + RMC + 4*GSV = 530 bytes
                           = 521 bytes excluding DGPS information in GGA.
                           = 514 bytes excluding ^M characters.

   Chance of reaching 514 bytes is very small, would require 16 satellites in
   view.

6) Card writing address is reset if card has empty first block or if on button
   is held down for 10 seconds.
