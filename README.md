# Tier
S-D Tier Ranking Interface

An introductory C project for yours truly, relies on GLFW, Cococry's 'Leif' GitHub, and installing 'xcb' from terminal command line:
    https://www.glfw.org/download.html 
    https://github.com/cococry/leif


Unfortunately has a bug where pressing "x" when interacting with the item string input field causes program to crash.
I have left this unfixed as my main intention with this project was getting an understanding of C programming conventions and capabilities.
    Making an awesome tier ranking interace was a side effect.

Compilation: 
    gcc -o tier tier.c -lglfw -lGL -lleif -lclipboard -lm -lxcb
