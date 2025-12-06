# Multi-stage Dockerfile for SSH/TLS Fingerprint Visualizer
# Stage 1: Build environment
FROM ubuntu:22.04 AS builder

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libssl-dev \
    libpcap-dev \
    pkg-config \
    git \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /build

# Copy source files
COPY CMakeLists.txt ./
COPY include/ ./include/
COPY src/ ./src/
COPY Makefile.mk ./

# Configure and build
RUN mkdir -p build && \
    cd build && \
    cmake .. && \
    cmake --build . -j$(nproc)

# Stage 2: Runtime environment
FROM ubuntu:22.04

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libssl3 \
    libpcap0.8 \
    ca-certificates \
    openssh-client \
    && rm -rf /var/lib/apt/lists/*

# Create non-root user for least-privilege execution
RUN useradd -m -u 1000 appuser && \
    mkdir -p /app/data /app/artifacts /app/scripts && \
    chown -R appuser:appuser /app

# Copy executables from builder
COPY --from=builder /build/build/fingerprint_tls /app/
COPY --from=builder /build/build/fingerprint_ssh /app/
COPY --from=builder /build/build/baseline_diff /app/
COPY --from=builder /build/build/visualize /app/
COPY --from=builder /build/build/generate_eval_set /app/
COPY --from=builder /build/build/fingerprint_pcap /app/

# Copy scripts directory
COPY scripts/ /app/scripts/
RUN chmod +x /app/scripts/*.sh 2>/dev/null || true

# Set working directory
WORKDIR /app

# Switch to non-root user
USER appuser

# Set PATH
ENV PATH="/app:$PATH"

# Default command
CMD ["/bin/bash"]

