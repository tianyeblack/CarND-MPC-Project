#ifndef MPC_H
#define MPC_H

#include <vector>
#include "Eigen-3.3/Eigen/Core"

/**
 * Set the timestep length and duration
 */
static const size_t N = 10;
static const double dt = 0.1;

// This value assumes the model presented in the classroom is used.
//
// It was obtained by measuring the radius formed by running the vehicle in the
//   simulator around in a circle with a constant steering angle and velocity on
//   a flat terrain.
//
// Lf was tuned until the the radius formed by the simulating the model
//   presented in the classroom matched the previous radius.
//
// This is the length from front to CoG that has a similar radius.
static const double Lf = 2.67;

// NOTE: feel free to play around with this or do something completely different
static const double ref_v = 40;

// The solver takes all the state variables and actuator
// variables in a singular vector. Thus, we should to establish
// when one variable starts and another ends to make our lifes easier.
static const size_t x_start = 0;
static const size_t y_start = x_start + N;
static const size_t psi_start = y_start + N;
static const size_t v_start = psi_start + N;
static const size_t cte_start = v_start + N;
static const size_t epsi_start = cte_start + N;
static const size_t delta_start = epsi_start + N;
static const size_t a_start = delta_start + N - 1;

class MPC {
 public:
  MPC();

  virtual ~MPC();

  // Solve the model given an initial state and polynomial coefficients.
  // Return the first actuations.
  std::vector<double> Solve(const Eigen::VectorXd &state, const Eigen::VectorXd &coeffs);
};

#endif  // MPC_H
