# Initial setup script for Godot GDExtension (PowerShell)
# Dumps extension API, builds godot-cpp, then builds the GDExtension

# Check if Godot is available
if (-not (Get-Command godot -ErrorAction SilentlyContinue)) {
	Write-Error "Godot executable is not installed or not in PATH. Please install Godot before running this script."
	exit 1
}

# Check if scons is available
if (-not (Get-Command scons -ErrorAction SilentlyContinue)) {
	Write-Error "scons is not installed or not in PATH. Please install scons before running this script."
	exit 1
}

# Step 0: Dump extension API
Write-Host "Dumping Godot extension API..."
godot --dump-extension-api

# Step 1: Build godot-cpp
Write-Host "Building godot-cpp..."
Push-Location "godot-cpp"
scons platform=windows custom_api_file=../extension_api.json
Pop-Location

# Step 2: Build GDExtension
Write-Host "Building GodotItchExtension..."
scons platform=windows

Write-Host "Setup complete."
