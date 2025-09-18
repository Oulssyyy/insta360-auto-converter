FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential cmake git \
    && rm -rf /var/lib/apt/lists/*

COPY Insta360MediaSDK /opt/Insta360MediaSDK

WORKDIR /app
COPY . /app

RUN cmake -S . -B build && cmake --build build --config Release

CMD ["/app/build/insta360_watcher"]
