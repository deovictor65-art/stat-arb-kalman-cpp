#include "statarb/kalman_filter.hpp"
#include "statarb/strategy_engine.hpp"
#include <cassert>
#include <iostream>

void test_kalman_convergence() {
    statarb::AdaptiveKalmanFilter kf(1e-4, 1e-3);
    double true_beta = 1.75;
    double true_alpha = 5.0;

    for (int i = 1; i <= 200; ++i) {
        double x = 50.0 + (i % 20);
        double y = true_alpha + true_beta * x;
        kf.update(x, y);
    }

    assert(std::abs(kf.beta() - true_beta) < 0.05);
    assert(std::abs(kf.alpha() - true_alpha) < 1.0);
    std::cout << "[PASS] Kalman filter convergence on linear relation\n";
}

void test_pairs_signal_generation() {
    statarb::PairsTradingEngine engine(30, 2.0, 0.5, 4.0);

    // Warm up
    for (int i = 0; i < 50; ++i) {
        engine.process_tick(100.0, 150.0);
    }

    // Huge spike in Y should trigger ShortSpread
    auto sig = engine.process_tick(100.0, 180.0);
    assert(sig.action == statarb::PositionState::ShortSpread);
    assert(sig.z_score > 2.0);
    std::cout << "[PASS] Statistical Arbitrage Signal Generation\n";
}

int main() {
    std::cout << "Running Kalman Stat-Arb Unit Tests...\n";
    test_kalman_convergence();
    test_pairs_signal_generation();
    std::cout << "\nAll Unit Tests Passed Successfully!\n";
    return 0;
}
