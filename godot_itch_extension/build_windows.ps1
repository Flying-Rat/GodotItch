# Build Windows script for Godot Itch Extension
# Cleans the build and builds all Windows variants (debug/release for x86_32/x86_64)

Write-Host "Cleaning build..."
scons -c
if ($LASTEXITCODE -ne 0) {
    Write-Error "Clean failed with exit code $LASTEXITCODE"
    exit $LASTEXITCODE
}

Write-Host "Building Windows debug x86_32..."
scons platform=windows target=template_debug arch=x86_32
if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed for debug x86_32 with exit code $LASTEXITCODE"
    exit $LASTEXITCODE
}

Write-Host "Building Windows release x86_32..."
scons platform=windows target=template_release arch=x86_32
if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed for release x86_32 with exit code $LASTEXITCODE"
    exit $LASTEXITCODE
}

Write-Host "Building Windows debug x86_64..."
scons platform=windows target=template_debug arch=x86_64
if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed for debug x86_64 with exit code $LASTEXITCODE"
    exit $LASTEXITCODE
}

Write-Host "Building Windows release x86_64..."
scons platform=windows target=template_release arch=x86_64
if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed for release x86_64 with exit code $LASTEXITCODE"
    exit $LASTEXITCODE
}

Write-Host "All Windows builds complete successfully."
