FROM debian:trixie-slim AS deps

# Install wxWidgets (with STC support), CMake, and minimal build tools
RUN apt-get update && apt-get install -y \
    g++ make cmake pkg-config \
    libwxgtk3.2-dev \
    clangd \
    wget \
    bear \
    && rm -rf /var/lib/apt/lists/*

RUN wget https://github.com/helix-editor/helix/releases/download/25.07.1/helix_25.7.1-1_amd64.deb \
    && dpkg -i ./helix_25.7.1-1_amd64.deb \
    && rm helix_25.7.1-1_amd64.deb

FROM deps AS build

WORKDIR /app

# Copy the entire project source, including the CMakeLists.txt
COPY --link . .

# --- Build using CMake ---
# Create a build directory to keep the source tree clean
RUN mkdir build
WORKDIR /app/build

# Configure the project with CMake
RUN cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the project
RUN cmake --build .

# Clean up source and build artifacts for a smaller runtime image
WORKDIR /app
RUN rm -rf src include build CMakeLists.txt

FROM build AS runtime

WORKDIR /app

# The executable is already built and copied implicitly by the build stage.
# If you need to specify a command, it would be:
# CMD ["/app/wxstc-editor"]

ENTRYPOINT ["/bin/bash"]
