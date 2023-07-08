cmake_minimum_required(VERSION 3.25)
project(c_aunisoma)

set(CMAKE_CXX_STANDARD 98)

add_executable(c_aunisoma main.cpp Panel.cpp Panel.h Color.cpp Color.h Maths.h Gradient.cpp Gradient.h Clock.cpp Clock.h Cycle.cpp Cycle.h Range.cpp Range.h Config.cpp Config.h Interaction.cpp Interaction.h PanelReverberation.cpp PanelReverberation.h PanelContext.h ReverberationDistanceSource.h Aunisoma.cpp Aunisoma.h TransitionAnimation.cpp TransitionAnimation.h Sensor.cpp Sensor.h TransitionAnimationCallback.h Arduino.h Aunisoma-Sketch.h Arduino.cpp)
