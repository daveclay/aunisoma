cmake_minimum_required(VERSION 3.25)
project(c_aunisoma)

set(CMAKE_CXX_STANDARD 98)

add_executable(c_aunisoma main.cpp Panel.cpp Panel.h Color.cpp Color.h Maths.h Gradient.cpp Gradient.h Clock.cpp Clock.h Cycle.cpp Cycle.h Range.cpp Range.h Config.cpp Config.h Reverberation.cpp Reverberation.h Aunisoma.cpp Aunisoma.h TransitionAnimation.cpp TransitionAnimation.h Sensor.cpp Sensor.h Arduino.h Aunisoma-Sketch.h Arduino.cpp Adafruit_DotStar.h SPI.h MaxInteractionAnimation.cpp MaxInteractionAnimation.h wiring_private.h
        Pulse.h
        Pulse.cpp
        ColorManager.h
        ColorManager.cpp
        Debounce.h
        Debounce.cpp
        ValueSmoothingFn.h
        ValueSmoothingFn.cpp)
