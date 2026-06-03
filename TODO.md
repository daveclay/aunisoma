Change WaveColorAlgorithm. Goal is to have overlapping waves result in more colorful animations.

New interaction on panel should still pick target color as it does today. But, when interaction waves overlap and new
color is calculated, instead of maxing out saturation and clamping, if max saturation is reached, move 
around the color wheel based on how many waves are overlapping.

Might be better to calculate wave overlap using vectors instead of RGB colors so that values beyond 255 can be 
handled and translated to color wheel. Open to suggestions though.

Write to plan.md