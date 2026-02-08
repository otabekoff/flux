# -- Stage 1: Build --
FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y \
    cmake \
    ninja-build \
    clang-18 \
    llvm-18-dev \
    libxml2-dev \
    git \
    && rm -rf /var/lib/apt/lists/*

# Set clang-18 as default compiler
ENV CC=clang-18
ENV CXX=clang++-18

WORKDIR /app
COPY . .

# Configure and Build
RUN cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DFLUX_ENABLE_TESTS=ON \
    -DLLVM_DIR=/usr/lib/llvm-18/lib/cmake/llvm
RUN cmake --build build

# -- Stage 2: Runtime & Test --
FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
    libllvm18 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /app/build/tools/flux/flux /usr/local/bin/
COPY --from=builder /app/build/runtime/libFluxRuntime.a /usr/local/lib/
COPY --from=builder /app/include /usr/local/include/flux

# Verify installation
RUN flux --version

ENTRYPOINT ["flux"]
