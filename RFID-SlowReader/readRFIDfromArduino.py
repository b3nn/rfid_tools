#!/usr/bin/env python

import time
import RPi.GPIO as GPIO

DATA0 = 18
DATA1 = 17
ACKPIN = 22

ACKDELAY = 0.005     # Time we need for pin transition


GPIO.cleanup()
GPIO.setmode(GPIO.BCM)

GPIO.setup(ACKPIN, GPIO.OUT)
GPIO.output(ACKPIN, GPIO.LOW)

GPIO.setup(DATA0, GPIO.IN)
GPIO.setup(DATA1, GPIO.IN)

RFID_CARD = ""

while True:
       	D0 = GPIO.input(DATA0)
       	D1 = GPIO.input(DATA1)

       	if (D0 or D1) != 0:
       	   GPIO.output(ACKPIN, GPIO.HIGH)
       	   time.sleep(ACKDELAY)
       	   GPIO.output(ACKPIN, GPIO.LOW)
       	   time.sleep(ACKDELAY)
       	   if D0:
       		RFID_CARD += '0'
       	   if D1:
       		RFID_CARD += '1'
        else:
          if len(RFID_CARD) > 20:
             print "Card is: " + RFID_CARD
             # Convert the string of bits to an int (removes leading zeros)
             card_int = int(RFID_CARD[7:-1],2)
             print card_int
             RFID_CARD = ""


GPIO.cleanup()
