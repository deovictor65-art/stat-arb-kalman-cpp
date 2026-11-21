# Adaptive Statistical Arbitrage & Dynamic Kalman Engine (`stat-arb-kalman-cpp`)

A high-performance C++20 quantitative pairs trading and statistical arbitrage engine leveraging recursive state-space Kalman Filtering for real-time dynamic hedge ratio ($\beta_t$) estimation, Ornstein-Uhlenbeck mean-reversion modeling, and adaptive Bollinger Z-score signal execution.

## Key Features

- **Dynamic Kalman State-Space Tracking**: Updates rolling intercept ($\alpha_t$) and hedge ratio ($\beta_t$) on every price tick, dynamically adapting across regime shifts.
- **Microsecond Latency Engine**: Pure header-only state machine designed without heap allocations in the critical trading loop.
- **Ornstein-Uhlenbeck Parameter Estimation**: Real-time analytical computation of mean-reversion speed ($\theta$) and expected half-life ($\tau = \frac{\ln 2}{\theta}$).
- **Signal & Risk Guard**: Configurable entry ($\pm 2.0\sigma$), mean-reversion exit ($\pm 0.5\sigma$), and volatility-expanded trailing stop-loss guards ($\pm 3.5\sigma$).

## Mathematical Formulation

The dynamic cointegration relation is modeled in state-space form:

$$y_t = \alpha_t + \beta_t x_t + \epsilon_t, \quad \epsilon_t \sim \mathcal{N}(0, R)$$

State transition with process covariance $Q$:

$$\theta_t = \theta_{t-1} + w_t, \quad \theta_t = [\alpha_t, \beta_t]^T, \quad w_t \sim \mathcal{N}(0, Q)$$

Kalman gain and innovation updates:

$$S_t = H_t P_{t|t-1} H_t^T + R, \quad K_t = P_{t|t-1} H_t^T S_t^{-1}$$

$$\theta_t = \theta_{t-1} + K_t (y_t - H_t \theta_{t-1})$$

## Quick Build & Run

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --output-on-failure
./build/statarb_sim
```

## License

MIT License. Authored by deovictor65-art.
