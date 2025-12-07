
SHELL := /bin/bash
BUILD_DIR := build
DATA_DIR := data
ARTIFACT_DIR := release/artifacts
HOSTS_FILE := $(DATA_DIR)/demo_hosts.txt
DEMO_TIMESTAMP ?= 2024-01-01T00:00:00.000Z
DEMO_SEED ?= 1337
DEMO_TIMEOUT ?= 5

.PHONY: all build clean configure install test help demo demo_prepare demo_run demo_artifacts demo_hashes clean-data

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
	@rm -rf $(BUILD_DIR) $(ARTIFACT_DIR)
	@rm -f $(DATA_DIR)/*_fingerprints.csv $(DATA_DIR)/*_fingerprints.json $(DATA_DIR)/baseline.json $(DATA_DIR)/eval_metadata.json

clean-data:
	@echo "Removing generated data..."
	@rm -f $(DATA_DIR)/*_fingerprints.csv $(DATA_DIR)/*_fingerprints.json $(DATA_DIR)/baseline.json $(DATA_DIR)/eval_metadata.json

install: build
	@cd $(BUILD_DIR) && cmake --install .

test: build
	@echo "Running basic tests..."
	@cd $(BUILD_DIR) && ./fingerprint_tls github.com:443 || echo "Test requires network connectivity"
	@cd $(BUILD_DIR) && ./fingerprint_ssh github.com || echo "Test requires network connectivity and ssh-keyscan"

# Docker targets (optional)
up:
	@if command -v docker compose >/dev/null 2>&1; then docker compose up -d --build; \
	elif command -v docker-compose >/dev/null 2>&1; then docker-compose up -d --build; \
	else echo "Docker Compose not installed."; fi

down:
	@if command -v docker compose >/dev/null 2>&1; then docker compose down; \
	elif command -v docker-compose >/dev/null 2>&1; then docker-compose down; \
	else echo "Docker Compose not installed."; fi

demo: configure build demo_prepare demo_run demo_artifacts demo_hashes

demo_prepare:
	@echo "Preparing reproducible demo directories..."
	@mkdir -p $(DATA_DIR) $(ARTIFACT_DIR)

demo_run:
	@echo "Running reproducible demo with fixed timestamp $(DEMO_TIMESTAMP) and seed $(DEMO_SEED)..."
	@bash -lc "cd $(BUILD_DIR) && set -o pipefail && ./generate_eval_set --data-dir ../$(DATA_DIR) --hosts-file ../$(HOSTS_FILE) --timeout $(DEMO_TIMEOUT) --seed $(DEMO_SEED) --timestamp "$(DEMO_TIMESTAMP)" --allow-failures 2>&1 | tee ../$(ARTIFACT_DIR)/demo_run.log"
	@bash -lc "cd $(BUILD_DIR) && set -o pipefail && ./baseline_diff create --data-dir ../$(DATA_DIR) --baseline ../$(DATA_DIR)/baseline.json --timestamp "$(DEMO_TIMESTAMP)" 2>&1 | tee -a ../$(ARTIFACT_DIR)/demo_run.log"

demo_artifacts:
	@echo "Collecting artifacts into $(ARTIFACT_DIR)..."
	@mkdir -p $(ARTIFACT_DIR)
	@for f in tls_fingerprints.csv tls_fingerprints.json ssh_fingerprints.csv ssh_fingerprints.json; do \
		if [ -f $(DATA_DIR)/$$f ]; then \
			cp $(DATA_DIR)/$$f $(ARTIFACT_DIR)/$$f; \
		elif [[ $$f == *.csv ]]; then \
			echo "timestamp,type,host,port,fingerprint" > $(ARTIFACT_DIR)/$$f; \
		else \
			echo "[]" > $(ARTIFACT_DIR)/$$f; \
		fi; \
	done
	@if [ -f $(DATA_DIR)/eval_metadata.json ]; then cp $(DATA_DIR)/eval_metadata.json $(ARTIFACT_DIR)/eval_metadata.json; else echo "{}" > $(ARTIFACT_DIR)/eval_metadata.json; fi
	@if [ -f $(DATA_DIR)/baseline.json ]; then cp $(DATA_DIR)/baseline.json $(ARTIFACT_DIR)/baseline.json; else echo "[]" > $(ARTIFACT_DIR)/baseline.json; fi

demo_hashes:
	@echo "Generating SHA-256 hashes for artifacts..."
	@if [ -d $(ARTIFACT_DIR) ]; then cd $(ARTIFACT_DIR) && ls -1 | sort | xargs -r sha256sum > SHA256SUMS.txt; fi
