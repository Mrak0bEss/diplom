FROM ubuntu:22.04

RUN apt-get update && \
    apt-get install -y g++ cmake libboost-all-dev uuid-dev git curl && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Установка Crow
RUN git clone https://github.com/CrowCpp/Crow.git && \
    cp -r Crow/include/crow /usr/local/include/

# Копируем файлы
COPY . .

# Сборка
RUN mkdir build && cd build && \
    cmake .. && \
    make

EXPOSE 8080

CMD ["./build/cache_server"]
