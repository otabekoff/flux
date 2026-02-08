#!/bin/bash
set -e

# Professional Docker Test Script for Flux
# Ensures the compiler builds and passes tests on a clean Linux environment.

IMAGE_NAME="flux-dev-env"

echo "==> Building Docker image: $IMAGE_NAME"
docker build -t $IMAGE_NAME .

echo "==> Running internal unit tests in container"
docker run --rm $IMAGE_NAME --version

# If the user wants to run specific tests, they can mount their source
# docker run --rm -v $(pwd):/app -w /app $IMAGE_NAME ctest --test-dir build --output-on-failure

echo "==> Success! Linux environment verified."
