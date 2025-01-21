ARG VARIANT
FROM mcr.microsoft.com/vscode/devcontainers/cpp:${VARIANT}

# tzdata confirmation override
ENV DEBIAN_FRONTEND=noninteractive 

RUN apt-get update && \
    apt-get install -y libboost-all-dev && \
    rm -rf /var/lib/apt/lists/*

# RUN git clone --recurse-submodules https://github.com/uNetworking/uWebSockets && \
#     cd uWebSockets && \
#     make && \
#     make install && \
#     rm -rf /uWebSockets
