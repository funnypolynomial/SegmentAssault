# SegmentAssault
A simple invaders-style game played on a 10-digit 16-segment display, treating the segments like *pixels*.  Rotary encoder with push-button to aim and fire.

![image](https://github.com/user-attachments/assets/2899ab44-3802-401b-8047-26f918a795d3)

This is loosely inspired by the Casio calculator game "Digital Invaders".
The left-most character is the "phaser", waves of aliens scroll in from the right. _Shoot them_.  
Turning the rotary encoder moves the "row" the phaser is aiming at, pressing the integrated button fires the phaser.
Rather than digits, the aliens are groups of random segments on a particular character.
Individual alien segments are killed by firing the phaser on the same row.  
Points are accrued for each alien destroyed: 10 points for destroying one on the 10th (right-most) character, 9 points for one on the 9th character, etc

At the start of each level, the game waits for Fire to be pressed. Turning the encoder at this time adjusts the display brightness.
At each level, the delay between alien groups gets shorter and the number of segments per group increases.  The number of groups in a wave decreases to compensate, a little.

[Video](https://youtu.be/kccyrMVi98g)

Note:  
Holding the Fire button when powering on resets the high score and brightness.  
Some game details are controlled by defines in CFG.h  
There is a simple schematic in PINS.h  

