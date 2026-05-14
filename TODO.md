- replace ticks with ms so we're not so contingent on processing speed
  - timers and cycles run using ticks for math calculations... and so we can't really know how long they'll take. 
    And how long they take depends on the speed of processing while calculating the algorithm... so this is probably 
    impossible to implement? Or do we interpolate based on ms? 

- add length of time active to trigger gradient change in addition to number of interactions
- explore per-interaction gradient - if distance between interaction is greater than 4 panels, that reverberation 
  uses a different gradient. 
  - need to prevent color from hitting white (limit power usage)