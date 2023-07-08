#!/bin/bash

for f in `find src -type f -d 1 | cut -f 2 -d / | sort | grep -v Aunisoma-Sketch.h | grep -v main.cpp | grep -v Arduino.h | grep -v Arduino.cpp | grep -v Adafruit_DotStar.h | grep -v SPI.h | grep -v CMakeLists.txt`; { 
	cp src/$f Aunisoma-Sketch/$f 
}

vimdiff src/Aunisoma-Sketch.h Aunisoma-Sketch/Aunisoma-Sketch.ino
