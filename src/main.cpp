#include <math.h>
#include <uWS/uWS.h>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include "Eigen-3.3/Eigen/Core"
#include "Eigen-3.3/Eigen/QR"
#include "MPC.h"
#include "helpers.h"
#include "json.hpp"

// for convenience
using nlohmann::json;
using std::string;
using std::vector;

// For converting back and forth between radians and degrees.
constexpr double pi() { return M_PI; }
double deg2rad(double x) { return x * pi() / 180; }
double rad2deg(double x) { return x * 180 / pi(); }

Eigen::VectorXd stdToEigenVector(const vector<double> vals) {
  Eigen::VectorXd result(vals.size());
  for (size_t i = 0; i < vals.size(); i++) result[i] = vals[i];
  return result;
}

double normalizeToPi(double psi) {
  while (psi >= pi()) psi -= 2 * pi();
  while (psi <= -pi()) psi += 2 * pi();
  return psi;
}

int main() {
  uWS::Hub h;

  // MPC is initialized here!
  MPC mpc;

  h.onMessage([&mpc, &Lf, &N](uWS::WebSocket<uWS::SERVER> ws, char *data, size_t length, uWS::OpCode opCode) {
    // "42" at the start of the message means there's a websocket message event.
    // The 4 signifies a websocket message
    // The 2 signifies a websocket event
    string sdata = string(data).substr(0, length);
    std::cout << sdata << std::endl;
    if (sdata.size() > 2 && sdata[0] == '4' && sdata[1] == '2') {
      string s = hasData(sdata);
      if (s != "") {
        auto j = json::parse(s);
        string event = j[0].get<string>();
        if (event == "telemetry") {
          // j[1] is the data JSON object
          vector<double> ptsx = j[1]["ptsx"];
          vector<double> ptsy = j[1]["ptsy"];

          double px = j[1]["x"];
          double py = j[1]["y"];
          double psi = j[1]["psi"];
          double v = j[1]["speed"];
          double steer_value = (nullptr == j[1]["steering_value"] ? 0.0 : (double)j[1]["steering_value"]);
          double throttle_value = (nullptr == j[1]["throttle"] ? 0.0 : (double)j[1]["throttle"]);
          double latency = 0.1;
          /**
           * Handling latency
           * Since the action will be applied 100ms after we do the calculation, we need to consider where the vehicle is 100ms later
           * Only px, py, psi and v are updated as cte and epsi depends on the fit polynomial
           */
          px += v * cos(psi) * latency;
          py += v * sin(psi) * latency;
          psi -= v * steer_value * deg2rad(25) * latency / Lf;
          psi = normalizeToPi(psi);
          v += throttle_value * latency;
          for (size_t i = 0; i < ptsx.size(); i++) {
            double shift_x = ptsx[i] - px;
            double shift_y = ptsy[i] - py;
            ptsx[i] = shift_x * cos(psi) + shift_y * sin(psi);
            ptsy[i] = -shift_x * sin(psi) + shift_y * cos(psi);
          }
          auto coeffs = polyfit(stdToEigenVector(ptsx), stdToEigenVector(ptsy), 3);
          double cte = coeffs[0];
          double epsi = -atan(coeffs[1]);

          VectorXd state(6);
          state << 0, 0, 0, v, cte, epsi;

          auto vars = mpc.Solve(state, coeffs);
          /**
           * Calculate steering angle and throttle using MPC. Both are in between [-1, 1].
           * Divide by deg2rad(25) before sending the steering value back.
           * Otherwise the values will be in between [-deg2rad(25), deg2rad(25)] instead of [-1, 1].
           */
          steer_value = vars[0] / deg2rad(25);
          throttle_value = vars[1];

          json msgJson;
          msgJson["steering_angle"] = steer_value;
          msgJson["throttle"] = throttle_value;

          // Display the MPC predicted trajectory
          vector<double> mpc_x_vals;
          vector<double> mpc_y_vals;

          /**
           * Add (x,y) points to the predicted trajectory list, points are in reference to the vehicle's coordinate system the points in the simulator are
           * connected by a Green line
           */
          for (size_t t = 1; t < N; t++) {
            mpc_x_vals.push_back(vars[t * 2]);
            mpc_y_vals.push_back(vars[t * 2 + 1]);
          }

          msgJson["mpc_x"] = mpc_x_vals;
          msgJson["mpc_y"] = mpc_y_vals;

          // Display the waypoints/reference line
          vector<double> next_x_vals;
          vector<double> next_y_vals;

          /**
           * Add (x,y) points to list, points are in reference to the vehicle's coordinate system the points in the simulator are
           * connected by a Yellow line
           */
          for (size_t i = 1; i < 25; i++) {
            next_x_vals.push_back(2.5 * i);
            next_y_vals.push_back(polyeval(coeffs, 2.5 * i));
          }

          msgJson["next_x"] = next_x_vals;
          msgJson["next_y"] = next_y_vals;

          auto msg = "42[\"steer\"," + msgJson.dump() + "]";
          std::cout << msg << std::endl;
          // Latency
          // The purpose is to mimic real driving conditions where
          //   the car does actuate the commands instantly.
          //
          // Feel free to play around with this value but should be to drive
          //   around the track with 100ms latency.
          //
          // NOTE: REMEMBER TO SET THIS TO 100 MILLISECONDS BEFORE SUBMITTING.
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
          ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
        }  // end "telemetry" if
      } else {
        // Manual driving
        std::string msg = "42[\"manual\",{}]";
        ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
      }
    }  // end websocket if
  });  // end h.onMessage

  h.onConnection([&h](uWS::WebSocket<uWS::SERVER> ws, uWS::HttpRequest req) { std::cout << "Connected!!!" << std::endl; });

  h.onDisconnection([&h](uWS::WebSocket<uWS::SERVER> ws, int code, char *message, size_t length) {
    ws.close();
    std::cout << "Disconnected" << std::endl;
  });

  int port = 4567;
  if (h.listen(port)) {
    std::cout << "Listening to port " << port << std::endl;
  } else {
    std::cerr << "Failed to listen to port" << std::endl;
    return -1;
  }

  h.run();
}