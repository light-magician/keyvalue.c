FROM ubuntu:latest

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    build-essential \
    gcc \
    gdb \
    make \
    python3 \
    python3-pip

WORKDIR /code

CMD ["/bin/bash"]
