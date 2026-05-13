The aunisoma code lives in the arduino folder.
arduino/Aunisoma-Sketch folder is the arduino sketch code that runs on an arduino.
arduino/src folder is a copy of the project that compiles and runs on the desktop used for testing.
arduino/src/Arduino.cpp mocks out the arduino. It writes a script to a file that can be run in a mock html page that shows the colors and interactions from the script.
arduino/src/Aunisoma-Sketch.h is basically the same as [Aunisoma-Sketch.ino](arduino/Aunisoma-Sketch/Aunisoma-Sketch.
ino). arduino/src/main.cpp runs the mock sketch to call `loop` a number of times to produce the output script.

I don't want to duplicate the code in these two folders. I want to be able to write the code once and have it run 
both on the arduino and in the mock environment. Options to implement this?


should this project use `uv` for executing python?
update README.md with required dependencies to be installed (assume mac os), and build / mock run instructions