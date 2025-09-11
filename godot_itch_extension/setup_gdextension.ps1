# Initial setup script for Godot GDExtension (PowerShell)
# Dumps extension API, builds godot-cpp, then builds the GDExtension

# Check if Godot is available
if (-not (Get-Command godot -ErrorAction SilentlyContinue)) {
	Write-Error "Godot executable is not installed or not in PATH. Please install Godot before running this script."
	exit 1
}

# Check if scons is available
if (-not (Get-Command scons -ErrorAction SilentlyContinue)) {
    Write-Error "scons not in PATH."; exit 1
}

# Ensure godot-cpp is initialized
$cppPath = Join-Path $PSScriptRoot 'godot-cpp'
$helper = Join-Path $PSScriptRoot 'setup_godot_cpp_module.ps1'

# Run the helper if godot-cpp is missing or empty
if (-not (Test-Path $cppPath) -or -not (Get-ChildItem -Path $cppPath -File -Recurse -ErrorAction SilentlyContinue)) {
    if (-not (Test-Path $helper)) { Write-Error "Helper script missing."; exit 1 }
    Write-Host "Running helper: $helper"
    & $helper
    if (-not $? -or -not (Test-Path $cppPath)) { Write-Error "Helper failed."; exit 1 }
}

# Step 1: Dump the extension API
Write-Host "Dumping extension API..."
godot --dump-extension-api

# Step 2: Build godot-cpp
Write-Host "Building godot-cpp..."
Push-Location "godot-cpp"
scons platform=windows custom_api_file=../extension_api.json
Pop-Location

# Step 3: Build GDExtension
Write-Host "Building GodotItchExtension..."
scons platform=windows

Write-Host "Setup complete."
