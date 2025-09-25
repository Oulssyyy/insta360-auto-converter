#!/bin/bash

# 🎯 Insta360 Auto Converter - Script Unifié pour WSL/Ubuntu
# Modes: Single Run ou Watch Continu avec jonctions optimisées

set -e

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INPUT_DIR="/mnt/c/Users/stf039/Pictures/insta360"
OUTPUT_DIR="/mnt/c/Users/stf039/Pictures/output"
CONFIG_DIR="/mnt/c/Users/stf039/Pictures/config"

# Variables de contrôle
WATCH_MODE=false
SHOW_HELP=false

# Fonction d'aide
show_help() {
    cat << EOF
🎯 Insta360 Auto Converter - Script Unifié Ubuntu/WSL
=====================================================

✨ NOUVEAU: Optimisation des jonctions avec algorithme OPTFLOW
   Élimine le flou aux extrémités pour une qualité identique à Insta360 Studio

USAGE: $0 [OPTIONS]

OPTIONS:
  (aucun arg)   Mode Single - Traite tous les fichiers une fois et s'arrête
  -w, --watch   Mode Watch - Surveillance continue pour nouveaux fichiers
  -h, --help    Affiche cette aide

EXEMPLES:
  $0                # Traitement batch unique
  $0 --watch        # Surveillance continue
  $0 -w             # Surveillance continue (forme courte)

QUALITÉ:
  • Résolution native: 11904x5952 (70.9MP)
  • Algorithme OPTFLOW pour jonctions parfaites
  • Métadonnées 360° préservées pour Synology Photos
  • EnableStitchFusion pour éliminer le flou aux bords
EOF
}

# Parse des arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -w|--watch)
            WATCH_MODE=true
            shift
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            echo "❌ Argument inconnu: $1"
            echo "Utilisez -h ou --help pour l'aide"
            exit 1
            ;;
    esac
done

# Affichage du mode
echo "🚀 Insta360 Auto Converter - $([ "$WATCH_MODE" = true ] && echo "MODE WATCH" || echo "MODE SINGLE")"
echo "=================================================================="
if [ "$WATCH_MODE" = true ]; then
    echo "🔄 Surveillance continue activée - détection automatique des nouveaux fichiers"
else
    echo "🎯 Traitement batch unique - traite tous les fichiers et s'arrête"
fi
echo ""

# Création des répertoires
echo "📁 Création des répertoires..."
mkdir -p "$OUTPUT_DIR" "$CONFIG_DIR"

# Configuration par défaut
if [ ! -f "$CONFIG_DIR/config.json" ]; then
    echo "📋 Création de la configuration par défaut..."
    cp "$SCRIPT_DIR/config.json" "$CONFIG_DIR/"
fi

echo "📂 Répertoires configurés:"
echo "   Input:  $INPUT_DIR"
echo "   Output: $OUTPUT_DIR"
echo "   Config: $CONFIG_DIR"
echo ""

# Vérification du répertoire d'entrée
if [ ! -d "$INPUT_DIR" ]; then
    echo "❌ Répertoire d'entrée introuvable: $INPUT_DIR"
    echo "💡 Créez le répertoire ou modifiez INPUT_DIR dans le script"
    exit 1
fi

# Fonction de nettoyage
cleanup() {
    echo ""
    echo "🧹 Nettoyage des conteneurs..."
    if [ "$WATCH_MODE" = true ]; then
        docker stop insta360-watch-mode 2>/dev/null || true
        docker rm -f insta360-watch-mode 2>/dev/null || true
    else
        docker stop insta360-single-run 2>/dev/null || true
        docker rm -f insta360-single-run 2>/dev/null || true
    fi
    echo "✅ Nettoyage terminé"
}

# Trap pour nettoyage automatique
trap cleanup EXIT INT TERM

# Build de l'image Docker si nécessaire
echo "🔧 Vérification de l'image Docker..."
if ! docker image inspect insta360-auto-converter:latest >/dev/null 2>&1; then
    echo "🏗️ Construction de l'image Docker..."
    cd "$SCRIPT_DIR"
    docker-compose build
else
    echo "✅ Image Docker prête"
fi

# Configuration Docker commune
DOCKER_VOLUMES=(
    -v "$INPUT_DIR:/data/input:ro"
    -v "$OUTPUT_DIR:/data/output"
    -v "$CONFIG_DIR:/data/config"
)

DOCKER_ENV=(
    -e DISPLAY=:99
    -e LIBGL_ALWAYS_INDIRECT=1
    -e MESA_GL_VERSION_OVERRIDE=4.5
)

# Lancement selon le mode
if [ "$WATCH_MODE" = true ]; then
    echo "🔄 Démarrage du mode WATCH..."
    echo "   Pressez Ctrl+C pour arrêter"
    echo ""
    
    docker run --rm -it \
        --name insta360-watch-mode \
        "${DOCKER_VOLUMES[@]}" \
        "${DOCKER_ENV[@]}" \
        --memory=4g \
        --cpus=2 \
        insta360-auto-converter:latest \
        bash -c "Xvfb :99 -screen 0 1024x768x24 -ac +extension GLX +render -noreset & sleep 2 && /app/build/insta360_batch_processor /data/input /data/output /data/config/config.json --watch; kill \$! 2>/dev/null || true"
else
    echo "🎯 Démarrage du mode SINGLE..."
    echo ""
    
    docker run --rm \
        --name insta360-single-run \
        "${DOCKER_VOLUMES[@]}" \
        "${DOCKER_ENV[@]}" \
        --memory=4g \
        --cpus=2 \
        insta360-auto-converter:latest \
        bash -c "Xvfb :99 -screen 0 1024x768x24 -ac +extension GLX +render -noreset & sleep 2 && /app/build/insta360_batch_processor /data/input /data/output /data/config/config.json; kill \$! 2>/dev/null || true"
    
    echo ""
    echo "✅ Traitement terminé!"
    echo "📁 Fichiers convertis disponibles dans: $OUTPUT_DIR"
fi