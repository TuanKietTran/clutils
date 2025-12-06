Write-Host "Setting up clutils on Windows..." -ForegroundColor Green

# Smart vcpkg handling
if (-Not (Test-Path "vcpkg")) {
    Write-Host "Cloning vcpkg..."
    git clone --depth 1 https://github.com/microsoft/vcpkg.git
} else {
    Write-Host "vcpkg exists → updating..."
    Set-Location vcpkg
    git pull --depth 1
    Set-Location ..
}

# Bootstrap only if needed
if (-Not (Test-Path "vcpkg/vcpkg.exe")) {
    Write-Host "Bootstrapping vcpkg..."
    Set-Location vcpkg
    .\bootstrap-vcpkg.bat
    Set-Location ..
} else {
    Write-Host "vcpkg already bootstrapped"
}

Write-Host "Installing OpenSSL..."
.\vcpkg\vcpkg install openssl

Write-Host "Building with Ninja (fastest)..."
cmake -S . -B build -G Ninja `
  -DCMAKE_TOOLCHAIN_FILE="$PWD/vcpkg/scripts/buildsystems/vcpkg.cmake" `
  -DCMAKE_BUILD_TYPE=Release

cmake --build build --config Release

Write-Host "Done! → build\clutils.exe uuid" -ForegroundColor Green