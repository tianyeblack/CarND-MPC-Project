
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

# Waypoints

The waypoints are passed back from simulator in global coordinates. Before fitting them to a polynomial, I translated them into local coordinate system with the vehicle's position as the origin. The vehicle's velocity becomes the positive x-axis. Every waypoint is considered rotated counter-clockwise to the global x-axis with the angle between local and global x-axis.

# Latency

Latency is taken into consideration by updating the state with the same equations as the model updates and observed vehicle velocity and accelerations.

Accounting for latency in x, y coordinates gets the vehicle through the first two turns and even the third with wild swing. Additionally, when speed is also accounted for, the vehicle is already capable of making through the whole track safely majority of the time (2 out of 3 laps).

After adding corrections for vehicle's directions and making the cost of changing accelerations larger, the vehicle stably makes through the whole track at 36mph.

# Constraints Tuning

Multiply the lower and upper bound by Lf or not and then divide steering angle by Lf makes the turning more smooth and prevents wild swings at low speed. This works because it's sufficient to get the cost of changing direction (delta) large so it's not changed that much. Later, I removed the Lf from constraint but multiple the cost with 200 and was able to get the vehicle to move faster.

This likely applies to other costs as well. To encourage certain behaviors, making it less costly will allow the solver to optimize for that.