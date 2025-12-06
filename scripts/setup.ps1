# scripts/setup.ps1
# Fully cache-aware, works with or without vcpkg.json

Write-Host "Setting up clutils on Windows..." -ForegroundColor Green

# === Smart vcpkg handling (respects GitHub Actions cache) ===
if (-Not (Test-Path "vcpkg")) {
    Write-Host "Cloning vcpkg (first time or cache miss)..." -ForegroundColor Cyan
    git clone --depth 1 https://github.com/microsoft/vcpkg.git
} else {
    Write-Host "vcpkg directory exists → updating to latest..." -ForegroundColor Cyan
    Set-Location vcpkg
    git pull --depth 1
    Set-Location ..
}

# === Bootstrap only if needed ===
if (-Not (Test-Path "vcpkg/vcpkg.exe")) {
    Write-Host "Bootstrapping vcpkg..." -ForegroundColor Cyan
    Set-Location vcpkg
    .\bootstrap-vcpkg.bat
    Set-Location ..
} else {
    Write-Host "vcpkg already bootstrapped" -ForegroundColor Gray
}

# === Install OpenSSL (smart: works in classic or manifest mode) ===
Write-Host "Installing OpenSSL via vcpkg..." -ForegroundColor Cyan

if (Test-Path "vcpkg.json") {
    Write-Host "Detected vcpkg.json → using manifest mode"
    .\vcpkg\vcpkg install
} else {
    Write-Host "Using classic mode (no vcpkg.json)"
    .\vcpkg\vcpkg install openssl
}

# === Build with Ninja (fastest on Windows) ===
Write-Host "Configuring and building clutils with Ninja..." -ForegroundColor Cyan

cmake -S . -B build -G Ninja `
    -DCMAKE_TOOLCHAIN_FILE="$PWD/vcpkg/scripts/buildsystems/vcpkg.cmake" `
    -DCMAKE_BUILD_TYPE=Release

cmake --build build --config Release --parallel

Write-Host "`nSUCCESS! clutils is ready!" -ForegroundColor Green
Write-Host "Run: ./build/clutils.exe uuid" -ForegroundColor Yellow
Write-Host "Or add to PATH: copy build\clutils.exe C:\bin\clutils.exe`n"