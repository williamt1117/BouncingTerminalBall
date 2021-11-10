# Terminal Bouncing Ball

First published project. Creates a ball of set height and width and collides with the edges of the screen. This runs for a set amount of frames at a set window height and width and redraws via escape characters moving the cursor to the top of the terminal. Overriding width and height of the ball can be done by manually defining 'WIDTH' and 'HEIGHT'. If this is done the program performs best if height and width are odd numbers. The physics is ran at the same intervals as the screen updates so lowering FPS under 20 may have undefined behavior with collision and bouncing.

![image](https://user-images.githubusercontent.com/92940760/141096057-76c4e818-8cd3-4be4-b902-9470e6a5e022.png)
