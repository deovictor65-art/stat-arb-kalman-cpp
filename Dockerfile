FROM ubuntu:22.04 AS builder
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y build-essential cmake clang-15 && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY . .
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++-15
RUN cmake --build build --config Release -j$(nproc)
RUN ctest --test-dir build --output-on-failure

FROM ubuntu:22.04 AS runner
WORKDIR /app
COPY --from=builder /app/build/statarb_sim /app/statarb_sim
COPY --from=builder /app/build/statarb_tests /app/statarb_tests
CMD ["./statarb_sim"]
