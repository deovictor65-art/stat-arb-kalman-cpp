#pragma once

#include "kalman_filter.hpp"
#include <vector>
#include <numeric>
#include <cmath>
#include <deque>

namespace statarb {

enum class PositionState {
    Flat = 0,
    LongSpread = 1,  // Long Y, Short Beta*X
    ShortSpread = -1 // Short Y, Long Beta*X
};

struct Signal {
    PositionState action{PositionState::Flat};
    double z_score{0.0};
    double spread{0.0};
    double hedge_ratio{1.0};
    double estimated_half_life{0.0};
};

class PairsTradingEngine {
public:
    PairsTradingEngine(size_t rolling_window = 60, double entry_z = 2.0, double exit_z = 0.5, double stop_z = 3.5)
        : rolling_window_(rolling_window), entry_z_(entry_z), exit_z_(exit_z), stop_z_(stop_z) {}

    Signal process_tick(double x, double y) {
        auto kstate = kf_.update(x, y);
        spread_history_.push_back(kstate.spread);
        if (spread_history_.size() > rolling_window_) {
            spread_history_.pop_front();
        }

        if (spread_history_.size() < 10) {
            return Signal{PositionState::Flat, 0.0, kstate.spread, kstate.beta, 0.0};
        }

        // Compute mean and standard deviation of spread
        double sum = std::accumulate(spread_history_.begin(), spread_history_.end(), 0.0);
        double mean = sum / spread_history_.size();

        double sq_sum = 0.0;
        for (double s : spread_history_) {
            sq_sum += (s - mean) * (s - mean);
        }
        double std_dev = std::sqrt(sq_sum / spread_history_.size());
        if (std_dev < 1e-6) std_dev = 1e-6;

        double z_score = (kstate.spread - mean) / std_dev;
        double half_life = compute_half_life();

        // Signal state machine
        PositionState new_action = current_position_;

        if (current_position_ == PositionState::Flat) {
            if (z_score <= -entry_z_) {
                new_action = PositionState::LongSpread;
            } else if (z_score >= entry_z_) {
                new_action = PositionState::ShortSpread;
            }
        } else if (current_position_ == PositionState::LongSpread) {
            if (z_score >= -exit_z_ || z_score <= -stop_z_) {
                new_action = PositionState::Flat;
            }
        } else if (current_position_ == PositionState::ShortSpread) {
            if (z_score <= exit_z_ || z_score >= stop_z_) {
                new_action = PositionState::Flat;
            }
        }

        current_position_ = new_action;

        return Signal{
            .action = current_position_,
            .z_score = z_score,
            .spread = kstate.spread,
            .hedge_ratio = kstate.beta,
            .estimated_half_life = half_life
        };
    }

    PositionState current_position() const noexcept { return current_position_; }

private:
    double compute_half_life() const {
        if (spread_history_.size() < 20) return 0.0;
        // Estimate AR(1): delta_s_t = phi * s_{t-1} + error
        double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_xx = 0.0;
        size_t n = spread_history_.size() - 1;

        for (size_t i = 0; i < n; ++i) {
            double x_val = spread_history_[i];
            double y_val = spread_history_[i + 1] - spread_history_[i];
            sum_x += x_val;
            sum_y += y_val;
            sum_xy += x_val * y_val;
            sum_xx += x_val * x_val;
        }

        double denominator = (n * sum_xx - sum_x * sum_x);
        if (std::abs(denominator) < 1e-9) return 0.0;

        double phi = (n * sum_xy - sum_x * sum_y) / denominator;
        if (phi >= 0.0) return 999.0; // Non-mean-reverting

        // Half life = -ln(2) / phi
        return -std::log(2.0) / phi;
    }

    size_t rolling_window_;
    double entry_z_;
    double exit_z_;
    double stop_z_;
    AdaptiveKalmanFilter kf_;
    std::deque<double> spread_history_;
    PositionState current_position_{PositionState::Flat};
};

} // namespace statarb
