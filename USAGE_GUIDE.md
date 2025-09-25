# Guide d'Utilisation Complet
## Insta360 Auto Converter - Dual Mode System

### 🎯 Vue d'Ensemble
Système de conversion automatique pour Insta360 X4 avec **2 modes d'utilisation** :
- **Single Run** : Traitement ponctuel de tous les fichiers existants
- **Watch Mode** : Surveillance continue pour nouveaux fichiers (idéal Synology NAS)

---

## 🚀 Utilisation Rapide

### **Bash/WSL (Recommandé)**
```bash
# Mode Single Run (défaut)
./start-batch-wsl.sh

# Mode Watch (surveillance continue)  
./start-batch-wsl.sh -w
./start-batch-wsl.sh --watch

# Aide
./start-batch-wsl.sh --help
```

### **PowerShell**
```powershell
# Mode Single Run (défaut)
.\start-batch.ps1

# Mode Watch (surveillance continue)
.\start-batch.ps1 -Watch
.\start-batch.ps1 -w

# Aide
.\start-batch.ps1 -Help
```

### **Batch (.bat)**
```cmd
REM Mode Single Run (défaut)
start-batch.bat

REM Mode Watch (surveillance continue)
start-batch.bat -w
start-batch.bat --watch

REM Aide  
start-batch.bat --help
```

---

## 📁 Configuration des Dossiers

### **Dossiers surveillés** :
- **Entrée** : `C:\Users\stf039\Pictures\insta360` 
- **Sortie** : `C:\Users\stf039\Pictures\output`
- **Config** : `C:\Users\stf039\Pictures\config`

### **Préparation automatique** :
Les scripts créent automatiquement les dossiers nécessaires s'ils n'existent pas.

---

## 🔄 Fonctionnement

### **Mode Single Run**
1. Lance le script sans paramètre : `./start-batch-wsl.sh`
2. Construit l'image Docker si nécessaire 
3. Traite **tous les fichiers existants** dans le dossier d'entrée
4. **Se ferme automatiquement** après traitement complet
5. Idéal pour : Conversion ponctuelle, traitement en lot

### **Mode Watch** 
1. Lance le script avec `-w` : `./start-batch-wsl.sh -w`
2. Construit l'image Docker si nécessaire
3. **Surveille continuellement** le dossier d'entrée
4. Traite automatiquement les **nouveaux fichiers** ajoutés
5. **Reste actif** jusqu'à Ctrl+C
6. Idéal pour : Synology NAS, synchronisation automatique

---

## 🎨 Fonctionnalités Garanties

### **Qualité Native Maximum**
- **Résolution** : 11904x5952 pixels (70.9 MP)
- **Format** : JPEG optimisé pour 360°
- **Compression** : Qualité élevée préservée

### **Métadonnées 360° Automatiques**  
- **Préservation** : Toutes métadonnées originales (Make, Model, DateTime, EXIF, etc.)
- **Ajout** : Google Photo Sphere XMP tags pour reconnaissance 360°
- **Compatibilité** : Synology Photos, Google Photos, Facebook, etc.

### **Détection Intelligente**
- **No Duplicates** : Ne retraite jamais les fichiers déjà convertis
- **Comparison** : Détection basée sur comparaison input/output directories  
- **Performance** : Skip automatique des fichiers existants

---

## 📊 Performance & Monitoring

### **Temps de Traitement** (par image)
- **Petite** (4-8 MB) : ~30-60 secondes
- **Grande** (15-20 MB) : ~2-3 minutes  
- **Très grande** (20+ MB) : ~3-5 minutes

### **Logs en Temps Réel**
```
🔄 Insta360 Auto Converter - WATCH MODE
Starting batch processor in WATCH MODE (continuous monitoring)...
File already converted: "IMG_001.insp" -> "IMG_001.jpg"
Added to queue: "IMG_002.insp" (.insp)
Processing image: "IMG_002.insp"
Image conversion completed: "IMG_002.jpg"
Successfully added 360° EXIF metadata
```

### **Messages de Statut**
- ✅ **Success** : "Image conversion completed" + "Successfully added 360° EXIF metadata"
- ⚠️ **Skip** : "File already converted"
- ❌ **Error** : "ErrorCode:11; ErrorDescr: The input material does not meet the requirements"

---

## 🛠️ Dépannage

### **Problème : "Docker build failed"**
**Solution** : Le script retry automatiquement avec `--no-cache`
```bash
# Si persistant, rebuild manuel :
wsl docker-compose -f docker-compose-wsl.yml build --no-cache
```

### **Problème : "Container name conflict"**
**Solution** : Le script nettoie automatiquement les containers existants
```bash
# Nettoyage manuel si nécessaire :
wsl docker rm -f insta360-watch-mode
```

### **Problème : "Input directory not found"**
**Solution** : 
```bash
# Créer les dossiers manuellement :
mkdir -p /mnt/c/Users/stf039/Pictures/insta360
mkdir -p /mnt/c/Users/stf039/Pictures/output  
mkdir -p /mnt/c/Users/stf039/Pictures/config
```

### **Problème : "File not detected in Watch Mode"**
**Vérification** :
1. Fichier bien dans `C:\Users\stf039\Pictures\insta360` ?
2. Extension `.insp` ou `.insv` ?  
3. Pas déjà converti dans `output/` ?

---

## 🏆 Workflow Synology NAS Recommandé

### **Configuration Idéale**
1. **Synology Drive Client** synchronise `Pictures/insta360` 
2. **Watch Mode** actif en permanence : `./start-batch-wsl.sh -w`
3. Conversion automatique vers `Pictures/output`
4. **Synology Photos** indexe automatiquement les 360°

### **Processus Automatisé**
1. 📸 **Prendre photo** avec Insta360 X4
2. 📱 **Synchronisation** automatique vers PC/NAS via Synology Drive
3. 🔄 **Détection** automatique par Watch Mode  
4. ⚡ **Conversion** automatique native 11904x5952
5. 🏷️ **Métadonnées 360°** ajoutées automatiquement
6. 🖼️ **Synology Photos** affiche en 360° natif

### **Résultat Final**
- ✅ **Zero intervention** requise
- ✅ **Qualité maximale** préservée (70.9 MP)
- ✅ **360° natif** dans Synology Photos
- ✅ **Métadonnées complètes** préservées

---

## ⚙️ Configuration Avancée

### **Modifier les dossiers surveillés**
Éditer `start-batch-wsl.sh` lignes 50-52 :
```bash
INPUT_DIR="/mnt/c/VotreDossier/input"
OUTPUT_DIR="/mnt/c/VotreDossier/output"  
CONFIG_DIR="/mnt/c/VotreDossier/config"
```

### **Paramètres de qualité**
Le fichier `config.json` est auto-généré avec paramètres optimaux :
- Résolution native maximale (11904x5952)
- Qualité JPEG élevée
- Métadonnées 360° activées

---

**💡 Conseil Pro** : Utilisez le **Mode Watch** pour une expérience totalement automatisée avec votre Synology NAS !

*Dernière mise à jour : 25 septembre 2025*  
*Version : Dual Mode + Script Automation + Error Handling*