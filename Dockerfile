FROM ubuntu:24.04

RUN apt update && \
    apt -y install cmake g++ git && \
    rm -rf /var/lib/apt/lists/* 

COPY ./. .
RUN cmake -B build && \
    cmake --build build && \
    cmake --install build --prefix ./ && \
    rm -rf /brion_pipelines/build

CMD ./bin/pipeline_parser --help


