# Build stage
FROM ubuntu:24.04 AS builder

# Prevent interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    cmake \
    ninja-build \
    llvm-18-dev \
    clang-18 \
    libxml2-dev \
    git \
    && rm -rf /var/lib/apt/lists/*

# Set clang-18 as default
ENV CC=clang-18
ENV CXX=clang++-18

WORKDIR /app
COPY . .

# Build Flux
RUN cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_DIR=/usr/lib/llvm-18/lib/cmake/llvm
RUN cmake --build build --target flux runtime

# Final stage
FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/local/bin
COPY --from=builder /app/build/tools/flux/flux .
COPY --from=builder /app/build/runtime/libFluxRuntime.a /usr/local/lib/

# Set up runner environment
ENV PATH="/usr/local/bin:${PATH}"

ENTRYPOINT ["flux"]
CMD ["--help"]
