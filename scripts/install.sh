#!/bin/bash
set -e

# Flux One-Liner Installer (Unix)
# Usage: curl -fsSL https://raw.githubusercontent.com/otabekoff/flux/main/scripts/install.sh | bash

REPO="otabekoff/flux"
INSTALL_DIR="/usr/local/bin"

echo "==> Flux Installer"

# 1. Detect OS and Arch
OS="$(uname -s | tr '[:upper:]' '[:lower:]')"
ARCH="$(uname -m)"

case "$ARCH" in
    x86_64)  ARCH="amd64" ;;
    aarch64|arm64) ARCH="arm64" ;;
    *) echo "Error: Unsupported architecture $ARCH"; exit 1 ;;
esac

case "$OS" in
    linux) OS="ubuntu-latest" ;; # CI build label
    darwin) OS="macos-latest" ;; # Use macos-latest for universal/amd64
    *) echo "Error: Unsupported OS $OS"; exit 1 ;;
esac

echo "==> Detected: $OS $ARCH"

# 2. Get latest release from GitHub API
LATEST_RELEASE=$(curl -s https://api.github.com/repos/$REPO/releases/latest | grep "tag_name" | cut -d '"' -f 4)
if [ -z "$LATEST_RELEASE" ]; then
    echo "Error: Could not retrieve latest release version."
    exit 1
fi

echo "==> Downloading Flux $LATEST_RELEASE..."

# 3. Download appropriate artifact
# Note: In CI we name them flux-macos-latest-amd64 etc.
ARTIFACT_NAME="flux-$OS-$ARCH"
DOWNLOAD_URL="https://github.com/$REPO/releases/download/$LATEST_RELEASE/$ARTIFACT_NAME"

# Download to temp
TMP_DIR=$(mktemp -d)
curl -L "$DOWNLOAD_URL" -o "$TMP_DIR/flux"
chmod +x "$TMP_DIR/flux"

# 4. Install
echo "==> Installing to $INSTALL_DIR (requires sudo)..."
sudo mv "$TMP_DIR/flux" "$INSTALL_DIR/flux"

# 5. Verify
flux --version
echo "==> Flux $LATEST_RELEASE successfully installed to $INSTALL_DIR"
rm -rf "$TMP_DIR"
