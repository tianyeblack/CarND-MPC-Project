
# Overall

Similar to PID controller, once MPC starts swinging from side to side, it's hard for the model to recover. This could be the solver's capacity is not high enough to find a good solution. Or at high speed, it's generally harder to recover from mistakes.

# The Model

The model has 6 states: x and y coordinates (with vehicle's current position as origin and current velocity as the x-axis), angle relative to the vehicle's current direction, speed of the vehicle and errors of distance from ideal track and difference from desired angle.

There are two actuators, acceleration and angular acceleration, that will be determined based on cost function.

Only the x, y coordinates, velocity and direction are updated from the simulator measurments. The equations are (from time t to t+1):

$$ x_{t+1} = x_{t} + v_{t}cos(\psi_{t}) * dt $$
$$ y_{t+1} = y_{t} + v_{t}sin(\psi_{t}) * dt $$
$$ \psi_{t+1} = \psi_{t} + \frac{v_{t}}{L_{f}} * \delta_{t} * dt $$
$$ v_{t+1} = v_{t} + a_{t} * dt $$

# Model-parameter Tuning

Tried several combination of N and dt. Each has its pros and cons. Eventually I settled down with N=10 and dt=0.1.

## Large N (25) and small dt (0.05)

This enables the vehicle to look further with more granularity in planning future steps. In theory, the curve planned would be smoother and closer to the reference track. However, the computation power required by this leads to sub-optimal results and gradually increased error to a degree the vehicle swings too much and runs off track. Also, intuitively, looking further means the cost would be higher, which leads the solver to use larger values in acuators to keep the costs down.

## Small N (8) and large dt (0.16)

This looks even further into the future (1.28s instead 1.25s) but is less computationally intensive. One curious phenomenon is the vehicle sticks to the inner curve. This is likely due to the large cost multiplier I used to prevent delta and acceleration from changing too much. It was able to complete the track even sometimes running really close to the edge.

## Small N (6) and small dt (0.075)

This combination is especially short sighted. It delays turning until very close to entering the turn. It works okay in terms of driving in the simulator but likely won't do very well with a lot of curves (in the mountains). Besides, the experience is not as smooth as the settings above.

## Medium N (10) and medium dt (0.1)

This is the final selected combination. It has enough foresight to plan actions, which provides smooth experience. But it does not incur significant amount of computations (the first setting) or large cost per step (the second setting), which prevents it from getting an optimal or sensible result.

# Waypoints

The waypoints are passed back from simulator in global coordinates. Before fitting them to a polynomial, I translated them into local coordinate system with the vehicle's position as the origin. The vehicle's velocity becomes the positive x-axis. Every waypoint is considered rotated counter-clockwise to the global x-axis with the angle between local and global x-axis.

# Latency

This is handled before converting waypoints to local coordinate system. (See comment in main.cpp about handling latency) It's accomplished via updating the state the same way as the model does with observed vehicle velocity and accelerations.

Accounting for latency in x, y coordinates gets the vehicle through the first two turns and even the third with wild swing. Additionally, when speed is also accounted for, the vehicle is already capable of making through the whole track safely majority of the time (2 out of 3 laps).

After adding corrections for vehicle's directions and making the cost of changing accelerations larger, the vehicle stably makes through the whole track at 40mph.

# Constraints Tuning

Multiply the lower and upper bound by Lf or not and then divide steering angle by Lf makes the turning more smooth and prevents wild swings at low speed. This works because it's sufficient to get the cost of changing direction (delta) large so it's not changed that much. Later, I removed the Lf from constraint but multiple the cost with 200 and was able to get the vehicle to move faster.

I also multiply the variation of acceleration by a large number to avoid swinging too much at the beginning when speed is low. This also prevents the vehicle from suddenly braking.