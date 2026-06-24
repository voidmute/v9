$ErrorActionPreference = "Stop"
$dir = Split-Path -Parent $MyInvocation.MyCommand.Path
$candidates = @(
    (Join-Path $dir "v9_ui.dll"),
    (Join-Path $dir "v9.dll")
) | Where-Object { Test-Path $_ } | Sort-Object { (Get-Item $_).LastWriteTime } -Descending
if (-not $candidates) { Write-Host "v9.dll not found. Run v9\build.bat"; exit 1 }
$dll = $candidates[0]
$injector = Join-Path $dir "v9injector.exe"
$process = "PenguinHotel-Win64-Shipping.exe"
if (-not (Test-Path $dll)) { Write-Host "v9.dll not found. Run v9\build.bat"; exit 1 }
if (-not (Get-Process -Name ($process -replace '\.exe$','') -ErrorAction SilentlyContinue)) {
    Write-Host "Start the game first."; exit 1
}
Write-Host "Injecting v9 ESP from $(Split-Path -Leaf $dll)..."
& $injector $process $dll
if ($LASTEXITCODE -eq 0) { Write-Host "OK. INSERT = menu, END = unload" } else { exit $LASTEXITCODE }