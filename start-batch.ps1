# Insta360 Auto Converter - PowerShell Launcher
# Dual Mode: Single Run or Continuous Watch

param(
    [switch]$Watch,
    [switch]$w,
    [switch]$Help,
    [switch]$h
)

# Help display
if ($Help -or $h) {
    Write-Host "🎯 Insta360 Auto Converter - Dual Mode" -ForegroundColor Cyan
    Write-Host "======================================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Usage: .\start-batch.ps1 [OPTIONS]" -ForegroundColor White
    Write-Host ""
    Write-Host "OPTIONS:" -ForegroundColor Yellow
    Write-Host "  (no params)   Single Run Mode - Process all files once and exit"
    Write-Host "  -Watch, -w    Watch Mode - Continuous monitoring for new files"
    Write-Host "  -Help, -h     Show this help message"
    Write-Host ""
    Write-Host "EXAMPLES:" -ForegroundColor Green
    Write-Host "  .\start-batch.ps1           # Single batch processing"
    Write-Host "  .\start-batch.ps1 -Watch    # Continuous monitoring"
    Write-Host "  .\start-batch.ps1 -w        # Continuous monitoring (short form)"
    Write-Host ""
    exit 0
}

# Determine mode
$WatchMode = $Watch -or $w

# Display mode and launch
if ($WatchMode) {
    Write-Host "🔄 Launching WATCH MODE..." -ForegroundColor Green
    Write-Host "Continuous file monitoring enabled" -ForegroundColor White
    Write-Host ""
    
    # Change to project directory and run watch mode
    Set-Location "C:\Users\stf039\Documents\GitHub\insta360-auto-converter"
    wsl bash -c "./start-batch-wsl.sh --watch"
} else {
    Write-Host "🎯 Launching SINGLE RUN MODE..." -ForegroundColor Blue  
    Write-Host "Single batch processing" -ForegroundColor White
    Write-Host ""
    
    # Change to project directory and run single mode
    Set-Location "C:\Users\stf039\Documents\GitHub\insta360-auto-converter"
    wsl bash -c "./start-batch-wsl.sh"
}

Write-Host ""
Write-Host "✅ Processing completed" -ForegroundColor Green