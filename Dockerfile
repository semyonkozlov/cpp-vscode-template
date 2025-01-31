ARG VARIANT
FROM mcr.microsoft.com/vscode/devcontainers/cpp:${VARIANT}

# tzdata confirmation override
ENV DEBIAN_FRONTEND=noninteractive 

ARG BOOST_VERSION=1.87.0

RUN cd /opt && \
    BOOST_VERSION_MOD=$(echo $BOOST_VERSION | tr . _) && \
    wget https://archives.boost.io/release/${BOOST_VERSION}/source/boost_${BOOST_VERSION_MOD}.tar.bz2 && \
    tar --bzip2 -xf boost_${BOOST_VERSION_MOD}.tar.bz2 && \
    cd boost_${BOOST_VERSION_MOD} && \
    ./bootstrap.sh --with-toolset=clang --prefix=/usr/local && \
    ./b2 cxxflags="-std=c++20" install

# RUN git clone --recurse-submodules https://github.com/uNetworking/uWebSockets && \
#     cd uWebSockets && \
#     make && \
#     make install && \
#     rm -rf /uWebSockets
