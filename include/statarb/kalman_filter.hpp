#pragma once

#include <cmath>
#include <array>
#include <algorithm>

namespace statarb {

struct KalmanState {
    double alpha{0.0};  // Intercept
    double beta{1.0};   // Dynamic hedge ratio
    double spread{0.0}; // Residual error: y - (alpha + beta * x)
    double variance{0.0}; // Innovation variance
};

class AdaptiveKalmanFilter {
public:
    // delta: transition covariance tuning parameter
    // R: measurement noise variance
    explicit AdaptiveKalmanFilter(double delta = 1e-4, double R = 1e-3)
        : delta_(delta), R_(R) {
        // Initial state covariance matrix P = diag([1.0, 1.0])
        P_[0][0] = 1.0; P_[0][1] = 0.0;
        P_[1][0] = 0.0; P_[1][1] = 1.0;
        
        // Process noise covariance Q
        double q_val = delta_ / (1.0 - delta_);
        Q_[0][0] = q_val; Q_[0][1] = 0.0;
        Q_[1][0] = 0.0;   Q_[1][1] = q_val;
    }

    KalmanState update(double x, double y) {
        // Measurement matrix H = [1.0, x]
        // 1. Time Update (Predict)
        P_[0][0] += Q_[0][0];
        P_[0][1] += Q_[0][1];
        P_[1][0] += Q_[1][0];
        P_[1][1] += Q_[1][1];

        // 2. Innovation (Measurement Pre-fit Residual)
        double y_hat = alpha_ + beta_ * x;
        double error = y - y_hat;

        // Innovation covariance S = H * P * H^T + R
        // H * P = [P00 + x*P10, P01 + x*P11]
        double hp0 = P_[0][0] + x * P_[1][0];
        double hp1 = P_[0][1] + x * P_[1][1];
        double S = hp0 * 1.0 + hp1 * x + R_;

        // Kalman Gain K = P * H^T / S
        double K0 = (P_[0][0] + P_[0][1] * x) / S;
        double K1 = (P_[1][0] + P_[1][1] * x) / S;

        // State Update
        alpha_ += K0 * error;
        beta_ += K1 * error;

        // Covariance Update P = (I - K * H) * P
        // KH = [[K0, K0*x], [K1, K1*x]]
        std::array<std::array<double, 2>, 2> newP;
        newP[0][0] = P_[0][0] - K0 * hp0;
        newP[0][1] = P_[0][1] - K0 * hp1;
        newP[1][0] = P_[1][0] - K1 * hp0;
        newP[1][1] = P_[1][1] - K1 * hp1;
        P_ = newP;

        // Post-fit spread
        double current_spread = y - (alpha_ + beta_ * x);

        return KalmanState{
            .alpha = alpha_,
            .beta = beta_,
            .spread = current_spread,
            .variance = S
        };
    }

    double alpha() const noexcept { return alpha_; }
    double beta() const noexcept { return beta_; }

private:
    double delta_{1e-4};
    double R_{1e-3};
    double alpha_{0.0};
    double beta_{1.0};
    std::array<std::array<double, 2>, 2> P_{};
    std::array<std::array<double, 2>, 2> Q_{};
};

} // namespace statarb
