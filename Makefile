# ============================================================================
# Flux Compiler — Makefile Wrapper
# ============================================================================
# This Makefile wraps CMake + Ninja for convenience.
#
# Usage:
#   make              Build debug (default)
#   make release      Build release
#   make test         Run all tests
#   make clean        Clean build artifacts
#   make configure    Re-run CMake configure
#   make reconfigure  Wipe build dir and reconfigure
#   make run          Run flux compiler with ARGS
#   make install      Install (to CMAKE_INSTALL_PREFIX)
#
# Variables:
#   PRESET   CMake configure preset  (default: default)
#   ARGS     Arguments for `make run` (default: empty)
#   JOBS     Parallel build jobs      (default: auto)
# ============================================================================

# Configuration
PRESET       ?= default
BUILD_DIR    := build
FLUX_EXE     := $(BUILD_DIR)/tools/flux/flux.exe
ARGS         ?=

# Detect Environment
ifeq ($(OS),Windows_NT)
    # Detect if we are in MSYS or similar (which provides a real rm)
    ifneq ($(findstring MSYS,$(shell uname -s 2>/dev/null)),)
        SHELL    := /usr/bin/bash
        RM_RF    = rm -rf $(1)
        IS_MSYS  := 1
        PRESET   ?= msys2-clang64
    else
        SHELL    := cmd.exe
        RM_RF    = if exist "$(1)" rmdir /s /q "$(1)"
        PRESET   ?= default
    endif
    NINJA    := ninja
else
    RM_RF    = rm -rf $(1)
    NINJA    := ninja
    PRESET   ?= default
endif

# Default target
.PHONY: all
all: build

# ── Configure ──────────────────────────────────────────────────────────────
.PHONY: configure
configure:
	@# Detect if building for a different environment than before
	@if [ -f $(BUILD_DIR)/CMakeCache.txt ]; then \
		if [ "$(IS_MSYS)" = "1" ] && ! grep -q "MSYSTEM" $(BUILD_DIR)/CMakeCache.txt; then \
			echo "Detected environment change to MSYS. Wiping build directory..."; \
			$(call RM_RF,$(BUILD_DIR)); \
		fi \
	fi
	cmake --preset $(PRESET)

# ── Build ──────────────────────────────────────────────────────────────────
.PHONY: build
build: configure
	cmake --build $(BUILD_DIR)

.PHONY: release
release:
	cmake --preset release
	cmake --build $(BUILD_DIR)

# ── Test ───────────────────────────────────────────────────────────────────
.PHONY: test
test: build
	cd $(BUILD_DIR) && ctest --output-on-failure

# ── Run ────────────────────────────────────────────────────────────────────
.PHONY: run
run: build
	$(FLUX_EXE) $(ARGS)

# ── Install ────────────────────────────────────────────────────────────────
.PHONY: install
install: build
	cmake --install $(BUILD_DIR)

# ── Clean ──────────────────────────────────────────────────────────────────
.PHONY: clean
clean:
	$(call RM_RF,$(BUILD_DIR))

# ── Reconfigure ────────────────────────────────────────────────────────────
.PHONY: reconfigure
reconfigure: clean configure

# ── Helpers ────────────────────────────────────────────────────────────────
.PHONY: dump-tokens
dump-tokens: build
	$(FLUX_EXE) --dump-tokens $(ARGS)

.PHONY: dump-ast
dump-ast: build
	$(FLUX_EXE) --dump-ast $(ARGS)

.PHONY: help
help:
	@echo "Flux Compiler Build Targets:"
	@echo "  make              Build (debug, default)"
	@echo "  make release      Build (release)"
	@echo "  make test         Run all tests"
	@echo "  make run ARGS=f   Run flux compiler"
	@echo "  make clean        Remove build directory"
	@echo "  make reconfigure  Clean + reconfigure"
	@echo "  make configure    Run CMake configure"
	@echo "  make install      Install to prefix"
	@echo "  make dump-tokens  Run --dump-tokens"
	@echo "  make dump-ast     Run --dump-ast"
	@echo "  make help         Show this help"
