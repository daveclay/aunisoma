in the real project, green and teal were picked almost all the time. Yellow once in a while, magenta once in a while.
Never orange, and never red. 

Instead of trying to keep the same color for adjacent active panels, the first panel should pick a random target 
color (any random color but must be 100% saturated) and if the neighbor becomes active, move it's target color 
around the color wheel by 1/20 amount. A panel that becomes active without any neighbors should pick a random target 
color. Do not bother to try to "avoid" similar colors in that case - random colors should be sufficient.