param(
    [string]$V9Root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path,
    [string]$OutDir = ""
)

$ErrorActionPreference = "Stop"

if (-not $OutDir) {
    $OutDir = Join-Path $V9Root "built"
}
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
$ObjDir = Join-Path $V9Root ".build\obj"
New-Item -ItemType Directory -Force -Path $ObjDir | Out-Null

function Quote-CmdArg([string]$Value) {
    if ($Value -match '^[A-Za-z0-9_./:=+\-\\]+$') { return $Value }
    return '"' + ($Value -replace '"', '\"') + '"'
}

function Get-VsDevCmd {
    $VsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $VsWhere)) { return "" }
    $VsInstall = & $VsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (-not $VsInstall) { return "" }
    $VsDevCmd = Join-Path $VsInstall "Common7\Tools\VsDevCmd.bat"
    if (Test-Path $VsDevCmd) { return $VsDevCmd }
    return ""
}

function Invoke-VsToolCommand {
    param(
        [Parameter(Mandatory = $true)][string]$ToolName,
        [Parameter(Mandatory = $true)][string[]]$ToolArgs
    )
    if (Get-Command $ToolName -ErrorAction SilentlyContinue) {
        & $ToolName @ToolArgs
        if ($LASTEXITCODE -ne 0) { throw "$ToolName failed with exit code $LASTEXITCODE" }
        return
    }
    $VsDevCmd = Get-VsDevCmd
    if (-not $VsDevCmd) {
        throw "$ToolName was not found. Install Visual Studio 2022 Build Tools."
    }
    $ArgText = ($ToolArgs | ForEach-Object { Quote-CmdArg $_ }) -join " "
    $CommandLine = "$(Quote-CmdArg $VsDevCmd) -arch=x64 -host_arch=x64 >nul && $ToolName $ArgText"
    cmd /d /c $CommandLine
    if ($LASTEXITCODE -ne 0) { throw "$ToolName failed with exit code $LASTEXITCODE" }
}

$BridgeSource = Join-Path $V9Root "runtime\src\v9_bridge.cpp"
$InjectorSource = Join-Path $V9Root "runtime\src\v9_injector.cpp"
$CamouflageSource = Join-Path $V9Root "runtime\src\v9_camouflage.cpp"
$EspProject = Join-Path $V9Root "v9\v9.vcxproj"

foreach ($source in @($BridgeSource, $InjectorSource, $CamouflageSource)) {
    if (-not (Test-Path $source)) { throw "Source not found: $source" }
}
if (-not (Test-Path $EspProject)) { throw "ESP project not found: $EspProject" }

$BridgeOutput = Join-Path $OutDir "v9bridge.dll"
$InjectorOutput = Join-Path $OutDir "v9injector.exe"
$CamouflageOutput = Join-Path $OutDir "v9camouflage.exe"
$EspOutput = Join-Path $V9Root "v9\x64\Release\v9.dll"

Write-Host "Building v9bridge.dll..."
Invoke-VsToolCommand -ToolName "cl.exe" -ToolArgs @(
    "/nologo", "/std:c++17", "/EHsc", "/O2", "/LD", $BridgeSource,
    "/Fo:$(Join-Path $ObjDir 'v9_bridge.obj')",
    "/Fe:$BridgeOutput",
    "Ws2_32.lib",
    "User32.lib"
)

Write-Host "Building v9injector.exe..."
Invoke-VsToolCommand -ToolName "cl.exe" -ToolArgs @(
    "/nologo", "/EHsc", "/O2", $InjectorSource,
    "/Fo:$(Join-Path $ObjDir 'v9_injector.obj')",
    "/Fe:$InjectorOutput"
)

if (-not (Test-Path $BridgeOutput)) { throw "Bridge DLL was not produced: $BridgeOutput" }

$ResourceRc = Join-Path $ObjDir "v9_camouflage.rc"
$ResourceRes = Join-Path $ObjDir "v9_camouflage.res"
$BridgeResourcePath = ((Resolve-Path $BridgeOutput).Path -replace '\\', '\\')
Set-Content -Encoding ASCII -Path $ResourceRc -Value "101 RCDATA `"$BridgeResourcePath`"`r`n"
Invoke-VsToolCommand -ToolName "rc.exe" -ToolArgs @("/nologo", "/fo", $ResourceRes, $ResourceRc)

Write-Host "Building v9camouflage.exe..."
Invoke-VsToolCommand -ToolName "cl.exe" -ToolArgs @(
    "/nologo", "/std:c++17", "/EHsc", "/O2", $CamouflageSource, $ResourceRes,
    "/Fo:$(Join-Path $ObjDir 'v9_camouflage.obj')",
    "/Fe:$CamouflageOutput",
    "Ws2_32.lib",
    "User32.lib"
)

Write-Host "Building v9.dll (ESP)..."
$msbuild = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath 2>$null
if ($msbuild) { $msbuild = Join-Path $msbuild "MSBuild\Current\Bin\MSBuild.exe" }
if (-not $msbuild -or -not (Test-Path $msbuild)) { throw "MSBuild not found" }
& $msbuild $EspProject /t:Build /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v143 /m /nologo /v:minimal
if ($LASTEXITCODE -ne 0) { throw "v9.dll build failed" }

Copy-Item -Force $EspOutput (Join-Path $OutDir "v9.dll")

Write-Host ""
Write-Host "v9 build complete:"
Write-Host "  $CamouflageOutput"
Write-Host "  $BridgeOutput"
Write-Host "  $InjectorOutput"
Write-Host "  $(Join-Path $OutDir 'v9.dll')"
