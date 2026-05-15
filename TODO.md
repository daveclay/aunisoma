- add length of time active to trigger gradient change in addition to number of interactions

- explore per-interaction gradient - if distance between interaction is greater than 4 panels, that reverberation 
  uses a different gradient. 
  - need to prevent color from hitting white (limit power usage)

# plan to implement new color interaction algorith.
Idle state is still the idle red color #030000
When a sensor goes from inactive to active:
- the associated panel picks a target primary color away from the idle color: red, green, blue, cyan (R+G), magenta 
  (R+B), yellow (G+B).
- start an animated color wave, starting at the associated panel from the idle color and transitioning to the target color over a period of 
  time (e.g., 3 seconds). 
- after a short delay, the left and right neighbor panels animate their color to the target color, with decreasing 
  intensity as the distance from the original panel increases.
- Each subsequent neighbor participates in the wave after subsequent delays until the last panel, at which the wave 
  halts.

When additional sensors are activated:
- pick a new target color for the associated panel, as above.
- where the wave colors overlap, pick a new complimentary target color, transitioning to that color instead of 
  either wave's previous color.


The wave animation should go from idle color to target color, then back to idle color. The 2 neighbor panels should 
do the same animation, slightly delayed from the source panel, and reduced intensity. It currently only goes from 
the idle color directly to the target color. Animation from the idle color to target color should be 600ms and 
return to idle color in 600ms


when a panel is activated, pick a single target color for the wave pulse, and keep that target color while the 
panel is active. keep the wave pulse animation out from the source panel, but diminish after 8 panels distance. Where 
two or more wave pulses overlap, don't sum the color. instead, calculate the distance between the two actual panel 
colors on a rainbow spectrum gradient (taking into account brightness) and use that color. So, at the "front" of the 
wave pulse, the target color has low brightness, and where it meets the "front" of the other wave pulse, the two 
colors would blend into each other, but stay at the same brightness.

Yes. But I need a tool to help figure out color interaction. create a simple html file. It should have a 200px high and 
1200px wide rainbow gradient, with a smooth transition between colors. I want to click on a part of the gradient, 
and have that color displayed in a box below the gradient line, representing one of the target colors. I also need 
to be able to remove a box. 