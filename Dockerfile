FROM ubuntu:22.04

RUN apt-get update && apt-get install -y cmake build-essential iproute2 libboost-all-dev \ 
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . /app
RUN chmod +x test.sh
CMD ["./test.sh"]
