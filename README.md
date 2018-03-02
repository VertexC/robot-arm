# robot-arm
use basic transformation and model view in opengl


#### build up
```bash
make
# or
make robot
```
#### run
```bash
# old for initial position of ball, new for the end position of ball
# -tv: top view
# -sv: side view
# the robot arm fetches the ball and moves it to the end positon, and then go back to initial state
./myrobot old_x old_y old_z new_x new_y new_z <-tv|-sv>
```
#### keyboard control
- view selector
    - 1: top view
    - 2: front view
    - 3: right(side) view
- rotate selector, currently disabled
    - z: base
    - x: lower arm
    - c: upper arm
    - left key: decrease angle
    - right key: increase angle
- other
    - q: quit

#### REFERENCE
** render the sphere **
https://stackoverflow.com/questions/7687148/drawing-sphere-in-opengl-without-using-glusphere
