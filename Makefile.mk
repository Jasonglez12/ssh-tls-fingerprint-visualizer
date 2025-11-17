SHELL := /bin/bash
VENV := .venv

.PHONY: bootstrap up down clean

bootstrap:
	@python3 -m venv $(VENV) || true
	@. $(VENV)/bin/activate && pip install --upgrade pip && pip install -r requirements.txt || true
	@# Bring up dev container if Docker Compose is available; otherwise continue with local venv
	@if command -v docker compose >/dev/null 2>&1; then docker compose up -d; \
	elif command -v docker-compose >/dev/null 2>&1; then docker-compose up -d; \
	else echo "Docker Compose not installed; using local venv only."; fi

up:
	@if command -v docker compose >/dev/null 2>&1; then docker compose up -d; \
	elif command -v docker-compose >/dev/null 2>&1; then docker-compose up -d; \
	else echo "Docker Compose not installed."; fi

down:
	@if command -v docker compose >/dev/null 2>&1; then docker compose down; \
	elif command -v docker-compose >/dev/null 2>&1; then docker-compose down; \
	else echo "Docker Compose not installed."; fi

clean:
	rm -rf $(VENV) __pycache__ .pytest_cache build dist
