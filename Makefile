.PHONY: build up test run visualize demo clean

IMAGE_NAME = fingerprint-viz
ARTIFACTS_DIR = $(PWD)/artifacts/release

# Ensure artifacts dir exists on host so Docker doesn't create it as root
init:
	mkdir -p "$(ARTIFACTS_DIR)"
	touch "$(ARTIFACTS_DIR)/.gitkeep"

build: init
	docker build -t $(IMAGE_NAME) .

up: build
	@echo "Docker environment ready."

test: init
	docker run --rm $(IMAGE_NAME) pytest tests/ --cov=src --cov-report=term-missing

# Quotes added below to handle spaces in folder names like "CECS478 Project"
run: init
	docker run --rm -v "$(ARTIFACTS_DIR):/app/artifacts/release" $(IMAGE_NAME) python src/main.py

visualize: init
	docker run --rm -v "$(ARTIFACTS_DIR):/app/artifacts/release" $(IMAGE_NAME) python src/visualizer.py

# The "One Command" sequence required by the professor
demo: test run visualize
	@echo "---------------------------------------------------"
	@echo "DEMO COMPLETE: Vertical slice (Test -> Run -> Viz) finished."
	@echo "Artifacts generated in artifacts/release/"
	@echo "---------------------------------------------------"

clean:
	rm -rf artifacts/release/*
	touch artifacts/release/.gitkeep
	find . -name "__pycache__" -type d -exec rm -rf {} +