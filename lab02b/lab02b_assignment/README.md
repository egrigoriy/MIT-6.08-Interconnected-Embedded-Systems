## Lab2b: Final Assignment: Step Counter with web logging

Тhe goal is to develop a system which should:
1. Re-use your step-counting algorithm that you developed in Lab02A in a single callable function which utilizes global variables,
2. Have a second state machine that, upon pressing the button connected to Pin 45, will POST your users current tally of steps to the database and re-zero the current tally.
3. Use a second button connected to Pin 39 to decide to either display the current local tally of steps or the global tally of steps you've taken (if you are doing this before a POST request has been made, and therefore before it knows its total steps, it can just list the current local tally). This should be a stateless design; something like: leave the button unpushed to get current tally, hold the button down to get the total tally.

The result can be seen here: (https://github.com/egrigoriy/MIT-6.08-Interconnected-Embedded-Systems/blob/main/lab02b/lab02b_assignment/lab02b_video.mp4).

![Setup1](https://github.com/egrigoriy/MIT-6.08-Interconnected-Embedded-Systems/blob/main/lab02b/lab02b_assignment/lab02b_setup1.jpg)
![Setup2](https://github.com/egrigoriy/MIT-6.08-Interconnected-Embedded-Systems/blob/main/lab02b/lab02b_assignment/lab02b_setup2.jpg)
![Setup3](https://github.com/egrigoriy/MIT-6.08-Interconnected-Embedded-Systems/blob/main/lab02b/lab02b_assignment/lab02b_setup3.jpg)