## Lab1A: Final Assignment

Your assignment for this lab is to build a number-fact looker-upper. The number requested will be based on the number of button presses a user provides as input in quick succession. To be more specific:

1. Your system will start at rest.
2. Upon pressing and releasing your button, the system will wait for up to one second for another press and release. If the press-release sequence occurs, the system will restart its timeout and wait a second for another press-release, all the while tallying the number of times the button has been pressed and then released.
3. Following at least one press-release sequence, if the button remains unpressed for more than one second, the system is to look up trivia on the number of presses that occured and display it on the LCD, before returning to rest.
4. 
The system should be able to handle long presses by a user and only count them as one push.

The result can be shawn here: (https://github.com/egrigoriy/MIT-6.08-Interconnected-Embedded-Systems/blob/main/lab01b/lab01b_assignment/lab01b_video.mp4).

![Setup](https://github.com/egrigoriy/MIT-6.08-Interconnected-Embedded-Systems/blob/main/lab01b/lab01b_assignment/lab01b.png)
