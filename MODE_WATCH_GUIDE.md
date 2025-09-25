# Guide d'utilisation du Mode Watch
## Convertisseur Automatique Insta360 pour Synology NAS

### 🎯 Résumé
Le système propose **2 modes d'utilisation** :
1. **Mode Single Run** : Traitement unique de tous les fichiers existants
2. **Mode Watch** : Surveillance continue pour nouveaux fichiers

---

## 📁 Configuration des Dossiers

### Dossiers par défaut pour le mode Watch :
- **Entrée** : `C:\Users\stf039\Pictures\insta360` *(déposer vos fichiers .insp ici)*
- **Sortie** : `C:\Users\stf039\Pictures\output` *(images 360° converties)*
- **Config** : `C:\Users\stf039\Pictures\config` *(configuration automatique)*

---

## 🚀 Utilisation

### Mode Single Run (traitement unique)
```powershell
wsl docker-compose -f docker-compose-wsl.yml up
```
- Traite tous les fichiers du dossier `input/` une fois
- Se ferme automatiquement après traitement
- Idéal pour conversion en lot

### Mode Watch (surveillance continue)
```powershell
wsl docker run -it --rm \
  -v /mnt/c/Users/stf039/Pictures/insta360:/data/input \
  -v /mnt/c/Users/stf039/Pictures/output:/data/output \
  -v /mnt/c/Users/stf039/Pictures/config:/data/config \
  -e DISPLAY=:99 \
  --entrypoint=/bin/bash insta360-auto-converter:latest \
  -c "Xvfb :99 -screen 0 1024x768x24 -ac +extension GLX +render -noreset & sleep 2 && echo 'Starting WATCH mode on Pictures/insta360...' && /app/build/insta360_batch_processor /data/input /data/output /data/config/config.json --watch"
```
- Surveille continuellement `C:\Users\stf039\Pictures\insta360`
- Convertit automatiquement les nouveaux fichiers
- Idéal pour synchronisation Synology

---

## 🔄 Test du Mode Watch

### Étape 1 : Lancer le mode watch
```powershell
# Créer les dossiers si nécessaire
New-Item -ItemType Directory -Force -Path "C:\Users\stf039\Pictures\insta360", "C:\Users\stf039\Pictures\output", "C:\Users\stf039\Pictures\config"

# Lancer la surveillance
wsl docker run -it --rm -v /mnt/c/Users/stf039/Pictures/insta360:/data/input -v /mnt/c/Users/stf039/Pictures/output:/data/output -v /mnt/c/Users/stf039/Pictures/config:/data/config -e DISPLAY=:99 --entrypoint=/bin/bash insta360-auto-converter:latest -c "Xvfb :99 -screen 0 1024x768x24 -ac +extension GLX +render -noreset & sleep 2 && /app/build/insta360_batch_processor /data/input /data/output /data/config/config.json --watch"
```

### Étape 2 : Test de détection automatique
1. **Déposer un fichier** `.insp` dans `C:\Users\stf039\Pictures\insta360`
2. Le système détecte automatiquement le nouveau fichier
3. Conversion automatique en 360° dans `C:\Users\stf039\Pictures\output`

---

## ✅ Fonctionnalités Garanties

### Qualité Native
- **Résolution** : 11904x5952 pixels (70.9 MP)
- **Format** : JPEG optimisé pour 360°
- **Métadonnées** : Toutes préservées + tags Google Photo Sphere

### Compatibilité 360°
- ✅ **Synology Photos** : Reconnu comme image 360°
- ✅ **Google Photos** : Navigation sphérique
- ✅ **Facebook/Meta** : Affichage immersif
- ✅ **Autres** : Standard Google Photo Sphere XMP

### Détection Intelligente
- Compare les dossiers input/output
- Ne retraite pas les fichiers déjà convertis
- Détection temps réel des nouveaux fichiers

---

## 🛠️ Dépannage

### Problème : "Le fichier n'est pas détecté"
**Solution** : Vérifier les dossiers
```powershell
# Vérifier le contenu
dir "C:\Users\stf039\Pictures\insta360"
dir "C:\Users\stf039\Pictures\output"
```

### Problème : "Conversion échoue"
**Solution** : Vérifier les logs Docker
```powershell
# Voir les logs en temps réel
docker logs <container_id> -f
```

### Problème : "Pas d'affichage 360°"
**Solution** : Les métadonnées sont automatiquement ajoutées
- Vérifier avec `exiv2 -pa image.jpg`
- Standard Google Photo Sphere XMP intégré

---

## 📊 Performance

### Temps de traitement (par image)
- **Small** (4-8 MB) : ~30-60 secondes
- **Large** (15-20 MB) : ~2-3 minutes
- **Résolution native** : Toujours 11904x5952

### Ressources système
- **CPU** : 1 core recommandé
- **RAM** : 2GB limite Docker
- **Stockage** : ~3x taille originale pour output

---

## 🔧 Configuration Avancée

### Modifier les dossiers surveillés
Éditer la commande Docker et changer les volumes :
```bash
-v /mnt/c/VotreDossier/input:/data/input \
-v /mnt/c/VotreDossier/output:/data/output \
```

### Paramètres de qualité
Le fichier `config.json` est auto-généré avec :
- Résolution native maximale
- Qualité optimale
- Métadonnées 360° activées

---

## 📱 Utilisation Synology

### Configuration idéale
1. **Mode Watch** en fonctionnement continu
2. **Synology Drive** synchronise `Pictures/insta360`
3. Conversion automatique vers `Pictures/output`
4. **Synology Photos** affiche les 360° automatiquement

### Workflow recommandé
1. Prendre photo avec Insta360 X4
2. Synchronisation automatique vers PC/NAS
3. Conversion automatique via Mode Watch
4. Visualisation 360° dans Synology Photos

---

*Dernière mise à jour : 24 septembre 2025*
*Version : Native Resolution + Dual Mode + Auto-Watch*