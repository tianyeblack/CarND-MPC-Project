Overall
Once it starts swinging from side to side, it's hard for the model to recover as the solver's capacity is not high enough to find a good solution (Or at high speed, it's generally harder to recover from mistakes)
Constraint of psi
Multiply the lower and upper bound by Lf or not and then divide steering angle by Lf makes the turning more smooth and prevents wild swings
Latency
Accounting for latency in x, y coordinates get the vehicle through the first two turns and even the third with wild swing.
When v is also accounted for, the vehicle is already capable of making through the whole track safely majority of the time (2 out of 3 laps)
