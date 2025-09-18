FROM nvidia/cuda:12.1.1-devel-ubuntu22.04

# Installer dépendances de base
RUN apt-get update && apt-get install -y \
    build-essential cmake git \
    libtiff5 libegl1 libxext6 libvulkan1 freeglut3-dev vulkan-tools libdc1394-25 \
    && rm -rf /var/lib/apt/lists/*

# Définir les chemins
WORKDIR /app

# Copier le code C++
COPY app /app

# Copier le SDK (l’utilisateur doit fournir sdk/ localement)
COPY sdk /sdk

# Construire le projet
RUN cmake -S /app -B /app/build && cmake --build /app/build --config Release

# Point d’entrée
CMD ["/app/build/insta360_converter", "/data/input/sample.insv", "/data/output"]
