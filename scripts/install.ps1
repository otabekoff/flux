# Flux One-Liner Installer (Windows)
# Usage: iwr -useb https://raw.githubusercontent.com/otabekoff/flux/main/scripts/install.ps1 | iex

$Repo = "otabekoff/flux"
$InstallDir = "$HOME\.flux\bin"

Write-Host "==> Flux Installer" -ForegroundColor Cyan

# 1. Ensure folder exists
if (!(Test-Path $InstallDir)) {
    New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null
}

# 2. Get latest release version
$LatestRelease = (Invoke-RestMethod -Uri "https://api.github.com/repos/$Repo/releases/latest").tag_name
if (!$LatestRelease) {
    Write-Error "Could not retrieve latest release."
    return
}

Write-Host "==> Downloading Flux $LatestRelease..."

# 3. Determine URL
$ArtifactName = "flux-windows-latest-amd64" # Standard build
$DownloadUrl = "https://github.com/$Repo/releases/download/$LatestRelease/$ArtifactName"

# 4. Download
Invoke-WebRequest -Uri $DownloadUrl -OutFile "$InstallDir\flux.exe"

# 5. Update PATH if needed
$UserPath = [Environment]::GetEnvironmentVariable("Path", "User")
if ($UserPath -notlike "*$InstallDir*") {
    Write-Host "==> Adding $InstallDir to User PATH..."
    [Environment]::SetEnvironmentVariable("Path", "$UserPath;$InstallDir", "User")
}

Write-Host "==> Flux $LatestRelease successfully installed to $InstallDir" -ForegroundColor Green
Write-Host "==> Please restart your terminal to use 'flux'."
