# Setup script for building SSH/TLS Fingerprint Visualizer on Windows
# Run this script before building: .\setup_build.ps1

Write-Host "Setting up build environment..." -ForegroundColor Cyan

# Check for OpenSSL
$opensslPaths = @(
    "C:\Program Files\OpenSSL-Win64",
    "C:\OpenSSL-Win64",
    "C:\msys64\mingw64"
)

$opensslRoot = $null
foreach ($path in $opensslPaths) {
    if (Test-Path $path) {
        # Check if it has include directory (development version)
        if (Test-Path "$path\include\openssl") {
            $opensslRoot = $path
            Write-Host "Found OpenSSL development headers at: $path" -ForegroundColor Green
            break
        }
    }
}

if (-not $opensslRoot) {
    Write-Host "OpenSSL development headers not found!" -ForegroundColor Yellow
    Write-Host "Please install OpenSSL development version from:" -ForegroundColor Yellow
    Write-Host "  https://slproweb.com/products/Win32OpenSSL.html" -ForegroundColor Yellow
    Write-Host "  Or use vcpkg: vcpkg install openssl:x64-windows" -ForegroundColor Yellow
    exit 1
}

# Clean build directory
if (Test-Path "build") {
    Write-Host "Cleaning build directory..." -ForegroundColor Cyan
    Remove-Item -Recurse -Force build
}

# Create build directory
New-Item -ItemType Directory -Path build | Out-Null
Set-Location build

# Configure with CMake
Write-Host "Configuring CMake..." -ForegroundColor Cyan

# Try Visual Studio first
$vsVersions = @("Visual Studio 17 2022", "Visual Studio 16 2022", "Visual Studio 15 2019")
$configured = $false

foreach ($generator in $vsVersions) {
    Write-Host "Trying generator: $generator" -ForegroundColor Cyan
    $result = cmake .. -G "$generator" -DOPENSSL_ROOT_DIR="$opensslRoot" 2>&1
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Successfully configured with $generator!" -ForegroundColor Green
        Write-Host "Now run: cmake --build . --config Release" -ForegroundColor Green
        $configured = $true
        break
    } else {
        Write-Host "Failed with $generator, trying next..." -ForegroundColor Yellow
    }
}

if (-not $configured) {
    Write-Host "Visual Studio generators failed. Trying MinGW..." -ForegroundColor Yellow
    
    # Try MinGW with MSYS2
    if (Test-Path "C:\msys64\mingw64\bin\g++.exe") {
        $env:PATH = "C:\msys64\mingw64\bin;" + $env:PATH
        $result = cmake .. -G "MinGW Makefiles" -DOPENSSL_ROOT_DIR="C:\msys64\mingw64" 2>&1
        if ($LASTEXITCODE -eq 0) {
            Write-Host "Successfully configured with MinGW!" -ForegroundColor Green
            Write-Host "Now run: cmake --build ." -ForegroundColor Green
            $configured = $true
        }
    }
}

if (-not $configured) {
    Write-Host "Failed to configure. Please check:" -ForegroundColor Red
    Write-Host "  1. CMake is installed and in PATH" -ForegroundColor Red
    Write-Host "  2. Visual Studio Build Tools or MinGW is installed" -ForegroundColor Red
    Write-Host "  3. OpenSSL development headers are available" -ForegroundColor Red
    exit 1
}

Set-Location ..

