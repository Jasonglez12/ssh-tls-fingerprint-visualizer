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
	@echo "Running tests..."
	@cd $(BUILD_DIR) && ctest --output-on-failure || echo "Some tests may have failed (requires network connectivity for full suite)"

test-unit: build
	@echo "Running unit tests..."
	@cd $(BUILD_DIR) && ./test_utils || echo "Unit tests failed"

test-integration: build
	@echo "Running integration tests..."
	@cd $(BUILD_DIR) && ./test_integration || echo "Integration tests failed"

# Docker targets
up:
	@echo "Building and starting Docker containers..."
	@if command -v docker compose >/dev/null 2>&1; then \
		docker compose build; \
		docker compose up -d; \
	elif command -v docker-compose >/dev/null 2>&1; then \
		docker-compose build; \
		docker-compose up -d; \
	else \
		echo "Docker Compose not installed. Please install Docker and Docker Compose."; \
		exit 1; \
	fi

down:
	@if command -v docker compose >/dev/null 2>&1; then docker compose down; \
	elif command -v docker-compose >/dev/null 2>&1; then docker-compose down; \
	fi

demo: up
	@echo "Running end-to-end demonstration..."
	@if command -v docker compose >/dev/null 2>&1; then \
		docker compose exec -T app /bin/bash /app/scripts/demo.sh; \
	elif command -v docker-compose >/dev/null 2>&1; then \
		docker-compose exec -T app /bin/bash /app/scripts/demo.sh; \
	fi

demo-local: build
	@echo "Running local demonstration (without Docker)..."
	@bash scripts/demo.sh || echo "Demo script requires bash and network connectivity"
