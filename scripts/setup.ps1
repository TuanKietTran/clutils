# scripts/setup.ps1
Write-Host "Setting up clutils on Windows..." -ForegroundColor Green

# Clean & fresh vcpkg
if (Test-Path vcpkg) { Remove-Item -Recurse -Force vcpkg }
git clone --depth 1 https://github.com/microsoft/vcpkg.git

Set-Location vcpkg
.\bootstrap-vcpkg.bat
Set-Location ..

# Build
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_TOOLCHAIN_FILE="$PWD/vcpkg/scripts/buildsystems/vcpkg.cmake" `
  -DCMAKE_BUILD_TYPE=Release

cmake --build build --config Release --parallel

Write-Host "Done! → build\Release\clutils.exe uuid" -ForegroundColor Green