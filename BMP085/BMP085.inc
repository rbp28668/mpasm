;**********************************************************
; BMP085 routine and data definitions
; Interfaces to the BMP085 pressure sensor.
; Sensor must be initialised first using BMP085Init
; To read pressure must call BMP085GetUT and BMP085GetUP
; first to get uncalibrated temperature and pressure.
; Then BMPgetTemp and BMPgetPressure can be called (in
; that order) to initialise the T and Pa variables which
; return temperature in units of 0.1C and pressure in Pa.
; Note 1 hPa = 100 Pa
;**********************************************************

        extern BMP085Init
        extern BMP085getUT
        extern BMP085getUP
        extern BMP085getTemp
        extern BMP085getPressure
        extern T
        extern Pa
        extern UP
        extern UT
