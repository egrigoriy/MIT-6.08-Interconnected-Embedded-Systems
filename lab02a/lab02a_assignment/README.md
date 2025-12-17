## Lab2a: Final Assignment: Step Counter

An easy first thing to do is to treat the three acceleration values as depicting a acceleration vector in three-dimensional space and to then derive the magnitude of that vector. (If you don't remember how to determine the magnitude of a vector, that's ok; look it up online). Once we have a magnitude of the 3D acceleration we only have one value to worry about which makes processing it easier. Then, one very simple step counting algorithm is to find peaks in the magnitude of the acceleration experienced by the device. Each isolated peak corresponds to a step. The goal then becomes to process our signal to first find peaks, and then count those peaks as steps. This is graphically shown below:

![Progressing from Magnitude of Acceleration to Peaks to Steps](https://github.com/egrigoriy/MIT-6.08-Interconnected-Embedded-Systems/blob/main/lab02a/lab02a_assignment/plot.svg)

The result can be seen here: (https://github.com/egrigoriy/MIT-6.08-Interconnected-Embedded-Systems/blob/main/lab02a/lab02a_assignment/lab02a_video.mp4).

![Setup](https://github.com/egrigoriy/MIT-6.08-Interconnected-Embedded-Systems/blob/main/lab02a/lab02a_assignment/lab02a_setup.jpg)
