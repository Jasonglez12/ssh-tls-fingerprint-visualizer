SHELL := /bin/bash
BUILD_DIR := build

.PHONY: all build clean configure install test help

help:
	@echo "Available targets:"
	@echo "  configure   - Configure CMake build"
	@echo "  build       - Build the project"
	@echo "  clean       - Clean build directory"
	@echo "  install     - Install executables"
	@echo "  test        - Run basic tests"
	@echo "  all         - Configure and build"

all: configure build

configure:
	@echo "Configuring CMake..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake ..

build: configure
	@echo "Building..."
	@cd $(BUILD_DIR) && cmake --build .

clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)

install: build
	@cd $(BUILD_DIR) && cmake --install .

test: build
	@echo "Running basic tests..."
	@cd $(BUILD_DIR) && ./fingerprint_tls github.com:443 || echo "Test requires network connectivity"
	@cd $(BUILD_DIR) && ./fingerprint_ssh github.com || echo "Test requires network connectivity and ssh-keyscan"

# Docker targets (optional)
up:
	@if command -v docker compose >/dev/null 2>&1; then docker compose up -d; \
	elif command -v docker-compose >/dev/null 2>&1; then docker-compose up -d; \
	else echo "Docker Compose not installed."; fi

down:
	@if command -v docker compose >/dev/null 2>&1; then docker compose down; \
	elif command -v docker-compose >/dev/null 2>&1; then docker-compose down; \
	else echo "Docker Compose not installed."; fi
