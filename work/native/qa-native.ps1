$ErrorActionPreference="Stop"
$workspace=Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$app=Join-Path $workspace "outputs/SekiModAssistant-Native-v0.3"
$qa=Join-Path $workspace ("work/native-qa-"+(Get-Date -Format "HHmmss"))
New-Item -ItemType Directory -Force $qa | Out-Null
$env:SMA_DATA=Join-Path $qa "userdata"
$native=Join-Path $app "SekiModAssistant.exe"
$test=Start-Process $native -ArgumentList "--self-test" -WindowStyle Hidden -PassThru -Wait
if($test.ExitCode -ne 0){throw (Get-Content (Join-Path $app "native-test-report.txt") -Raw)}
Get-Content (Join-Path $app "native-test-report.txt")
[Reflection.Assembly]::LoadFrom((Join-Path $workspace "outputs/SekiroModAssistant/SekiroModAssistant.dll")) | Out-Null
$game=Join-Path $qa "cross-version-game"
$inputRoot=Join-Path $qa "input"
New-Item -ItemType Directory -Force "$game/mods/parts","$inputRoot/chr" | Out-Null
[IO.File]::WriteAllText("$game/sekiro.exe",'fixture, never executed')
[IO.File]::WriteAllText("$game/dinput8.dll",'original dll')
[IO.File]::WriteAllText("$game/modengine.ini",'original ini')
[IO.File]::WriteAllText("$game/mods/parts/原始.bin",'original mod')
[IO.File]::WriteAllText("$game/.sma-state.json",'{"PauseEngine":false,"Name":"原始模组","Version":"1","Extra":"保留"}')
[IO.File]::WriteAllText("$inputRoot/chr/new.bin",'new mod')
[SekiroAssistant.GameService]::Install($game,(Join-Path $app "assets"),$inputRoot,'Fixture','1',$null,$true)
function NativeOp($op){
  $p=Start-Process $native -ArgumentList @('--fixture-op',('"'+$game+'"'),$op) -WindowStyle Hidden -PassThru -Wait
  if($p.ExitCode -ne 0){throw "Native fixture operation failed: $op"}
}
NativeOp 'snapshot'
NativeOp 'restore'
if([IO.File]::ReadAllText("$game/dinput8.dll") -ne 'original dll' -or [IO.File]::ReadAllText("$game/mods/parts/原始.bin") -ne 'original mod' -or (Test-Path "$game/mods/chr/new.bin")){throw 'Native failed to restore managed snapshot'}
[SekiroAssistant.GameService]::Restore($game,$null)
if([IO.File]::ReadAllText("$game/mods/chr/new.bin") -ne 'new mod' -or -not [SekiroAssistant.GameService]::EngineMatches($game,(Join-Path $app "assets"))){throw 'Managed failed to restore native snapshot'}
NativeOp 'restore'
if([IO.File]::ReadAllText("$game/dinput8.dll") -ne 'original dll'){throw 'Repeated cross-version undo failed'}
$report=@('PASS native reads .NET snapshot and Unicode paths','PASS .NET reads native snapshot','PASS cross-version undo restore') -join [Environment]::NewLine
[IO.File]::WriteAllText((Join-Path $workspace "outputs/v0.3-Native-CrossVersion-Tests.txt"),$report)
Write-Output $report
Write-Output "QA directory: $qa"
