param([string]$Compiler='D:\Tools\llvm-mingw-20261020-msvcrt-i686\bin\x86_64-w64-mingw32-clang++.exe')
$ErrorActionPreference='Stop'
$workspace=Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$release=Join-Path $workspace 'outputs/SekiModAssistant-Native-v0.3'
$diagnostic=Join-Path $workspace 'work/v03-elevation-drop-diagnostics'
$qa=Join-Path $workspace 'work/v03-elevation-drop-ui'
. (Join-Path $PSScriptRoot 'verify-binary.ps1')
New-Item -ItemType Directory -Force $diagnostic,"$qa/userdata","$qa/game/mods/parts" | Out-Null
foreach($item in @('assets','tools','licenses','preset-downloads.json','README.md')){Copy-Item -LiteralPath (Join-Path $release $item) -Destination $diagnostic -Recurse -Force}
$arguments=@('-std=c++17','-O2','-Wall','-Wextra','-municode','-mwindows','-static','-s','-static-libgcc','-static-libstdc++','-DUNICODE','-D_UNICODE','-DWINVER=0x0A00','-D_WIN32_WINNT=0x0A00',(Join-Path $PSScriptRoot 'main.cpp'),(Join-Path $release 'setup.res'),'-o',"$diagnostic/SekiModAssistant.exe")
$arguments+=@('bcrypt','shell32','ole32','uuid','advapi32','comctl32','wininet','user32','gdi32','gdiplus','dwmapi') | ForEach-Object {"-l$_"}
& $Compiler @arguments
if($LASTEXITCODE -ne 0){throw 'Diagnostic compile failed'}
Assert-NativeDiagnosticMatches "$release/SekiModAssistant.exe" "$diagnostic/SekiModAssistant.exe"
if((Get-NativeManifest "$release/SekiModAssistant.exe") -notmatch 'level="requireAdministrator"'){throw 'Missing release administrator manifest'}
if((Get-NativeManifest "$release/SekiModSetupStub.exe") -notmatch 'level="asInvoker"'){throw 'Installer per-user manifest changed'}
$p=Start-Process "$diagnostic/SekiModAssistant.exe" -ArgumentList '--self-test' -WindowStyle Hidden -PassThru -Wait
if($p.ExitCode -ne 0){throw 'Diagnostic self-test failed'}
[IO.File]::WriteAllText("$qa/game/sekiro.exe",'fixture, never executed')
[IO.File]::WriteAllText("$qa/game/mods/parts/example.dcx",'fixture')
$previousData=$env:SMA_DATA
try {
    $env:SMA_DATA="$qa/userdata"
    @{Language='zh';Game="$qa/game"} | ConvertTo-Json | Set-Content "$qa/userdata/settings.json" -Encoding utf8NoBOM
    foreach($argument in @('--qa-ui','--qa-ui-96')) {
        $p=Start-Process "$diagnostic/SekiModAssistant.exe" -ArgumentList $argument -WindowStyle Hidden -PassThru
        if(-not $p.WaitForExit(30000)){Stop-Process -Id $p.Id;throw 'UI diagnostics timed out'}
        if($p.ExitCode -ne 0){Get-Content "$qa/userdata/ui-captures/failure.txt" -ErrorAction SilentlyContinue;throw 'UI diagnostics failed'}
        Copy-Item "$qa/userdata/ui-captures/result.txt" (Join-Path $workspace "outputs/v0.3-Elevation-Drop-$($argument.TrimStart('-')).txt") -Force
    }
} finally {$env:SMA_DATA=$previousData}
Copy-Item "$diagnostic/native-test-report.txt" (Join-Path $workspace 'outputs/v0.3-Elevation-Drop-SelfTests.txt') -Force
Write-Output 'PASS matching executable code, release/admin and setup/per-user manifests, self-test and two UI scales'
Write-Warning 'Diagnostic runs use asInvoker. Release UAC and Explorer -> elevated window drag were not executed in this environment.'
