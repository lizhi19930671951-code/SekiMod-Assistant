$ErrorActionPreference="Stop"
$workspace=Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$qa=Join-Path $workspace "work/v03-ui"
New-Item -ItemType Directory -Force "$qa/userdata","$qa/game/mods/parts" | Out-Null
[IO.File]::WriteAllText("$qa/game/sekiro.exe","fixture, never executed")
[IO.File]::WriteAllText("$qa/game/mods/parts/example.dcx","fixture")
$env:SMA_DATA="$qa/userdata"
@{Language="zh";Game="$qa/game"} | ConvertTo-Json | Set-Content "$qa/userdata/settings.json" -Encoding utf8NoBOM
$p=Start-Process (Join-Path $workspace "outputs/SekiModAssistant-Native-v0.3/SekiModAssistant.exe") -ArgumentList "--qa-ui" -WindowStyle Hidden -PassThru
if(-not $p.WaitForExit(30000)){Stop-Process -Id $p.Id -ErrorAction SilentlyContinue;throw "UI diagnostic timed out"}
if($p.ExitCode -ne 0){throw "UI diagnostic failed"}
if(-not(Test-Path "$qa/game/mods/parts/example.dcx")){throw "No must preserve installed mods"}
Add-Type -AssemblyName System.Drawing
Get-ChildItem "$qa/userdata/ui-captures" -Filter "*.bmp" | ForEach-Object {
  $picture=[Drawing.Image]::FromFile($_.FullName)
  try{$picture.Save((Join-Path $workspace ("outputs/v0.3-Native-"+$_.BaseName+".png")),[Drawing.Imaging.ImageFormat]::Png)}finally{$picture.Dispose()}
}
Copy-Item "$qa/userdata/ui-captures/result.txt" (Join-Path $workspace "outputs/v0.3-Native-UI-Tests.txt")
