# 📦 Insta360 Auto Converter for Synology NAS

Complete setup guide for automated Insta360 file conversion on your Synology NAS.

---

## 📋 Prerequisites

### Synology NAS Requirements
- **DSM 7.0+** (for better Docker support)
- **Docker Package** installed from Package Center
- **Docker Compose** (optional but recommended)
- **Minimum 2GB RAM** (4GB+ recommended for video processing)
- **ARM64 or x64 CPU** (Intel/AMD models recommended for better performance)

### Recommended Synology Models
- **DS918+, DS920+, DS1520+** (Intel Celeron - good performance)
- **DS220+, DS420+** (Intel Celeron - basic performance)
- **DS1621+, DS1821+** (AMD Ryzen - excellent performance)

---

## 🚀 Quick Setup

### 1. Prepare Directory Structure

Create the following folder structure on your NAS:

```
/volume1/homes/[your_username]/insta360/
├── input/          # Put your .insv/.insp files here
├── output/         # Converted files will appear here
├── processed/      # Tracking files (don't touch)
├── config/         # Configuration files
└── sdk/            # Insta360 SDK (you provide this)
```

---

### 2. Install Insta360 SDK

1. Download the **Insta360 Media SDK** from the official Insta360 developer portal  
2. Extract the SDK and copy its contents to:

```
/volume1/homes/[your_username]/insta360/sdk/
├── include/
├── lib/
└── bin/
```

---

### 3. Clone and Build

SSH into your Synology NAS and run:

```bash
cd /volume1/homes/[your_username]/

# Clone the repository
git clone https://github.com/your-repo/insta360-auto-converter.git
cd insta360-auto-converter

# Copy your SDK into the project
cp -r ../insta360/sdk/ ./

# Build the Docker image
sudo docker build -t insta360-auto-converter .
```

---

### 4. Configure Docker Compose

Edit the `docker-compose.yml` file to match your paths:

```yaml
volumes:
  # Update these paths with your actual username
  - /volume1/homes/YOUR_USERNAME/insta360/input:/data/input:ro
  - /volume1/homes/YOUR_USERNAME/insta360/output:/data/output
  - /volume1/homes/YOUR_USERNAME/insta360/processed:/data/processed
  - /volume1/homes/YOUR_USERNAME/insta360/config:/data/config
```

---

### 5. Start the Service

```bash
docker-compose up -d
docker-compose logs -f
```

---

## ⚙️ Configuration

The system will automatically create a configuration file at `/data/config/config.json`:

```json
{
  "enableGPU": false,
  "outputWidth": 5760,
  "outputHeight": 2880,
  "bitrate": 50000000,
  "maxConcurrentJobs": 1,
  "watchInterval": 30,
  "comment": "Insta360 Batch Processor Configuration"
}
```

### Configuration Options

| Option              | Description                  | Recommended Value              |
|---------------------|------------------------------|--------------------------------|
| `enableGPU`         | Use GPU acceleration         | `false` (for compatibility)    |
| `outputWidth`       | Output video/image width     | `5760` (4K) or `3840` (standard) |
| `outputHeight`      | Output video/image height    | `2880` (4K) or `1920` (standard) |
| `bitrate`           | Video bitrate in bps         | `50000000` (50 Mbps)           |
| `maxConcurrentJobs` | Concurrent processing jobs   | `1`                            |
| `watchInterval`     | Directory scan interval (s)  | `30`                           |

---

## 📁 Usage

### Automated Processing

1. Copy files into the input directory:

```
/volume1/homes/[your_username]/insta360/input/
├── vacation_001.insv
├── vacation_002.insp
└── party_video.insv
```

2. Wait for processing (default: scans every 30s).  
3. Converted files will appear in:

```
/volume1/homes/[your_username]/insta360/output/
├── vacation_001.mp4
├── vacation_002.jpg
└── party_video.mp4
```

---

### Manual Single File Processing

```bash
# Convert a single video file
docker run --rm \
  -v /volume1/homes/[username]/insta360/input:/data/input:ro \
  -v /volume1/homes/[username]/insta360/output:/data/output \
  insta360-auto-converter \
  /data/input/your_file.insv /data/output

# Convert a single photo file
docker run --rm \
  -v /volume1/homes/[username]/insta360/input:/data/input:ro \
  -v /volume1/homes/[username]/insta360/output:/data/output \
  insta360-auto-converter \
  /data/input/your_photo.insp /data/output
```

---

## 🔧 DSM Integration

### Using Docker GUI
1. Open **Docker** in DSM  
2. Go to **Registry** → search `insta360-auto-converter`  
3. Download the image  
4. Go to **Container** → **Create**  
5. Configure volumes:  
   - `/volume1/homes/[username]/insta360/input` → `/data/input`  
   - `/volume1/homes/[username]/insta360/output` → `/data/output`  
   - `/volume1/homes/[username]/insta360/processed` → `/data/processed`  
   - `/volume1/homes/[username]/insta360/config` → `/data/config`  

---

### Task Scheduler Integration

Create a scheduled task to auto-restart:

```bash
#!/bin/bash
cd /volume1/homes/[username]/insta360-auto-converter
docker-compose up -d
```

---

## 📊 Monitoring

```bash
docker ps                  # view running containers
docker logs insta360-batch-processor   # check logs
docker stats insta360-batch-processor  # monitor resources
```

Logs are rotated automatically:
- Max size: 10MB per file  
- Max files: 3  
- Location: `/var/lib/docker/containers/[container-id]/`  

---

## 🚨 Troubleshooting

### Common Issues

- **"SDK directory not found"** → Ensure SDK is copied into `sdk/`  
- **"OpenCV error: !_src.empty()"** → Xvfb is required, already included  
- **"Permission denied"** → Fix with:
  ```bash
  sudo chown -R [username]:users /volume1/homes/[username]/insta360/
  sudo chmod -R 755 /volume1/homes/[username]/insta360/
  ```
- **"Container keeps restarting"**  
  1. Check available memory (≥2GB)  
  2. Verify SDK completeness  
  3. Inspect logs  

---

## ⚡ Performance Tips

- Increase memory in `docker-compose.yml`  
- Store input/output on SSDs  
- Run multiple containers for parallel queues  

To reduce load:  
- Lower resolution (`outputWidth/Height`)  
- Reduce bitrate  
- Increase `watchInterval`  

---

## 🔄 Updates

```bash
docker-compose down
git pull
sudo docker build -t insta360-auto-converter .
docker-compose up -d
```

---

## 📱 Mobile Access

- **DS File** app → browse output  
- **Synology Drive** → sync folders  
- **DS Video** → index videos  
- **QuickConnect/VPN** → remote access  

---

## 🔐 Security

- Keep SDK files private  
- Run containers as non-root  
- Limit network access (container doesn’t need internet)  
- Backup config + processed folder  

---

## 📈 Advanced Usage

### Multiple Queues

```bash
# High priority
docker run -d --name insta360-priority \
  -v /volume1/priority:/data/input:ro \
  -v /volume1/output:/data/output \
  insta360-auto-converter

# Regular queue
docker run -d --name insta360-regular \
  -v /volume1/regular:/data/input:ro \
  -v /volume1/output:/data/output \
  insta360-auto-converter
```

### Integration
- **Plex/Jellyfin** → point to `output/`  
- **Surveillance Station** → process camera footage  
- **Cloud Sync** → upload automatically  
- **DSM notifications** → alerts when done  

---

## 📞 Support

1. Check logs: `docker logs insta360-batch-processor`  
2. Verify SDK installation  
3. Test single file before batch  
4. Monitor RAM/CPU usage  
5. Report issues on GitHub  

---

✨ **Happy converting!** 📹 → 🎬
