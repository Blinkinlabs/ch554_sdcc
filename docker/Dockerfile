FROM docker.io/library/debian:11 as base

RUN apt-get -qq update -y

RUN DEBIAN_FRONTEND=noninteractive \
       apt-get install -y --no-install-recommends \
            build-essential \
            libusb-1.0.0-dev \
            git \
            sdcc \
            ca-certificates

# Cleanup cached apt stuff
RUN rm -rf /var/lib/apt/lists/*

FROM base as toolsbuilder

# Tools that needs to be built from source

RUN git clone https://github.com/ole00/chprog.git /src
WORKDIR /src
RUN ./build_linux.sh
RUN cp ./chprog /usr/local/bin

From base
COPY --from=toolsbuilder /usr/local/ /usr/local

RUN apt-get -qq update -y

# Needed for git --describe to work
RUN git config --global --add safe.directory /build