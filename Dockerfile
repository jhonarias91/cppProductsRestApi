# Usamos como base la imagen de Ubuntu
FROM ubuntu:latest

# Se instalan las dependencias
RUN apt-get update && apt-get install -y \
    g++ \
    libboost-all-dev \
    libssl-dev \
    libcpprest-dev \
    && rm -rf /var/lib/apt/lists/*

# Añade un argumento para el número de build, con un valor por defecto
ARG BUILD_NUMBER=locV0.0.1

# Establece la variable de entorno APP_VERSION con el valor de BUILD_NUMBER
ENV APP_VERSION=$BUILD_NUMBER

# Se copian los archivos del proyecto al contenedor
COPY . /app

# Se Establece el directorio de trabajo
WORKDIR /app

# Compilar la aplicación
RUN g++ -std=c++11 -o productsapp ./src/main.cpp ./src/functions.cpp -lcpprest -lboost_system -lssl -lcrypto

# Exponemos el puerto en el que corre la app productos api.
EXPOSE 4300

# Se le otorgan permisos para que se ejecute sin inconvenientes.
RUN chmod +x ./productsapp

# Comando para ejecutar la aplicación
CMD ["./productsapp"]
