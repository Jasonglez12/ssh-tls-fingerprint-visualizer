FROM python:3.9-slim

# Install system dependencies (ssh-keyscan is required)
RUN apt-get update && apt-get install -y \
    openssh-client \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Install Python dependencies
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copy source code
COPY . .

# CRITICAL: Set python path so imports work in tests
ENV PYTHONPATH=/app

# Create artifacts directory inside container
RUN mkdir -p artifacts/release

# Default command
CMD ["python", "src/main.py"]