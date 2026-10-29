param(
    [string]$Compiler = '',
    [string]$ResourceRoot = '',
    [string]$Destination = ''
)
$ErrorActionPreference = 'Stop'
$nativeSource = $PSScriptRoot
$workspaceRoot = Split-Path (Split-Path $nativeSource -Parent) -Parent
if (-not $ResourceRoot) { $ResourceRoot = Join-Path $workspaceRoot 'outputs\SekiroModAssistant' }
if (-not $Destination) { $Destination = Join-Path $workspaceRoot 'outputs\SekiModAssistant-Native-v0.3' }
if (-not $Compiler) {
    foreach ($candidate in @('clang++.exe','g++.exe','cl.exe')) {
        $tool = Get-Command $candidate -ErrorAction SilentlyContinue
        if ($tool) { $Compiler = $tool.Source; break }
    }
}
if (-not $Compiler -or -not (Test-Path -LiteralPath $Compiler)) {
    throw 'C++ compiler unavailable. Use -Compiler <LLVM-MinGW bin\clang++.exe>, or run from an x64 Visual Studio Native Tools shell.'
}
$Compiler = (Resolve-Path -LiteralPath $Compiler).Path
$nativeDestination = [IO.Path]::GetFullPath($Destination)
New-Item -ItemType Directory -Force $nativeDestination | Out-Null
$resources = @('assets','tools','licenses','preset-downloads.json','README.md')
foreach ($item in $resources) {
    $source = Join-Path $ResourceRoot $item
    if (-not (Test-Path -LiteralPath $source)) { throw "Required resource missing: $source" }
    if (Test-Path -LiteralPath $source -PathType Container) {
        $targetFolder = Join-Path $nativeDestination $item
        New-Item -ItemType Directory -Force $targetFolder | Out-Null
        Get-ChildItem -LiteralPath $source -Force | ForEach-Object { Copy-Item -LiteralPath $_.FullName -Destination $targetFolder -Recurse -Force }
    } else { Copy-Item -LiteralPath $source -Destination $nativeDestination -Force }
}
$libraries = @('bcrypt','shell32','ole32','uuid','advapi32','comctl32','wininet','user32','gdi32','gdiplus','dwmapi','uxtheme')
foreach ($resourceName in @('app','setup')) {
$resourceObject=Join-Path $nativeDestination "$resourceName.res"
if((Split-Path $Compiler -Leaf) -eq 'cl.exe'){
 $rc=(Get-Command rc.exe -ErrorAction Stop).Source
 Push-Location $nativeSource
 try { & $rc /nologo "/fo$resourceObject" "$resourceName.rc"; if($LASTEXITCODE -ne 0){throw 'Resource compile failed'} } finally {Pop-Location}
}else{
 $windres=Join-Path (Split-Path $Compiler -Parent) 'x86_64-w64-mingw32-windres.exe'
 Push-Location $nativeSource
 try { & $windres -i "$resourceName.rc" -o $resourceObject -O coff; if($LASTEXITCODE -ne 0){throw 'Resource compile failed'} } finally {Pop-Location}
}
}
foreach ($unit in @(@('main.cpp','SekiModAssistant.exe'),@('installer.cpp','SekiModSetupStub.exe'))) {
    $resourceObject=Join-Path $nativeDestination $(if($unit[0] -eq 'main.cpp'){'app.res'}else{'setup.res'})
    $inputSource = Join-Path $nativeSource $unit[0]
    $outputExe = Join-Path $nativeDestination $unit[1]
    if ((Split-Path $Compiler -Leaf) -eq 'cl.exe') {
        $arguments = @('/nologo','/std:c++17','/O2','/MT','/EHsc','/W4','/utf-8','/DUNICODE','/D_UNICODE','/DWINVER=0x0A00','/D_WIN32_WINNT=0x0A00',"/Fe:$outputExe",("/Fo:"+(Join-Path $nativeDestination ($unit[0]+'.obj'))),$inputSource,$resourceObject,'/link','/SUBSYSTEM:WINDOWS')
        $arguments += $libraries | ForEach-Object { "$_.lib" }
    } else {
        $arguments = @('-std=c++17','-O2','-Wall','-Wextra','-municode','-mwindows','-static','-s','-static-libgcc','-static-libstdc++','-DUNICODE','-D_UNICODE','-DWINVER=0x0A00','-D_WIN32_WINNT=0x0A00',$inputSource,$resourceObject,'-o',$outputExe)
        $arguments += $libraries | ForEach-Object { "-l$_" }
    }
    & $Compiler @arguments
    if ($LASTEXITCODE -ne 0) { throw "Compile failed: $($unit[0])" }
}
Copy-Item -LiteralPath (Join-Path $nativeSource 'APP-README.md') -Destination (Join-Path $nativeDestination 'README.md') -Force
Write-Output "Compiled native application and installer stub: $nativeDestination"
Write-Output 'Run SekiModAssistant.exe --self-test before distributing. Then run package-native.ps1.'
