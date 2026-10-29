param([string]$DiagnosticApplication='')
$ErrorActionPreference="Stop"
. (Join-Path $PSScriptRoot 'verify-binary.ps1')
$workspace=Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$fixture=Join-Path $workspace "work/v03-setup-fixture"
New-Item -ItemType Directory -Force $fixture | Out-Null
[IO.File]::WriteAllText((Join-Path $fixture "setup-fixture.txt"),"fixture, never executed")
$env:SMA_DATA=Join-Path $fixture "userdata"
$installer=Join-Path $workspace "outputs/SekiModAssistant-v0.3-Native-Setup.exe"
function InstallFixture([bool]$fail){
 $argsList=@("--fixture-install",('"'+$fixture+'"'));if($fail){$argsList+="fail"}
 $p=Start-Process $installer -ArgumentList $argsList -WindowStyle Hidden -PassThru -Wait
 if(($p.ExitCode -eq 0) -eq $fail){throw "Unexpected installer exit $($p.ExitCode)"}
}
InstallFixture $false
$target=Join-Path $fixture "LocalAppData/Programs/SekiModAssistantNative"
if(-not(Test-Path "$target/Uninstall.exe") -or -not(Test-Path "$fixture/Desktop/SekiMod Assistant Native.lnk")){throw "Incomplete fixture install"}
[IO.File]::WriteAllText("$target/upgrade-marker.txt","old install")
InstallFixture $true
if([IO.File]::ReadAllText("$target/upgrade-marker.txt") -ne "old install"){throw "Setup failure did not restore old installation"}
InstallFixture $false
if(Test-Path "$target/upgrade-marker.txt"){throw "Upgrade retained obsolete file"}
$installedMain="$target/SekiModAssistant.exe"
if((Get-FileHash $installedMain).Hash -ne (Get-FileHash (Join-Path $workspace 'outputs/SekiModAssistant-Native-v0.3/SekiModAssistant.exe')).Hash){throw 'Installed main differs from release'}
if((Get-NativeManifest $installedMain) -notmatch 'level="requireAdministrator"'){throw 'Installed app missing administrator manifest'}
$testMain=$installedMain
if($DiagnosticApplication){$testMain=Join-Path $DiagnosticApplication 'SekiModAssistant.exe';Assert-NativeDiagnosticMatches $installedMain $testMain}
$p=Start-Process $testMain -ArgumentList "--self-test" -WindowStyle Hidden -PassThru -Wait
if($p.ExitCode -ne 0){throw "Installed application self-test failed"}
$customParent=Join-Path $fixture '用户选择的父目录'
$customTarget=Join-Path $customParent 'SekiMod Assistant'
New-Item -ItemType Directory -Force $customParent | Out-Null
[IO.File]::WriteAllText((Join-Path $customParent 'keep-parent.txt'),'parent file must remain')
function InstallCustomFixture([bool]$fail){
 $argsList=@('--fixture-install',('"'+$fixture+'"'),'custom');if($fail){$argsList+='fail'}
 $p=Start-Process $installer -ArgumentList $argsList -WindowStyle Hidden -PassThru -Wait
 if(($p.ExitCode -eq 0) -eq $fail){throw "Unexpected custom install exit $($p.ExitCode)"}
}
InstallCustomFixture $false
if(-not(Test-Path "$customTarget/SekiModAssistant.exe") -or (Test-Path "$customParent/SekiModAssistant.exe")){throw 'Custom install did not use application child directory'}
if((Get-Content "$customParent/keep-parent.txt" -Raw) -ne 'parent file must remain'){throw 'Parent directory changed'}
$receipt=Get-Content "$customTarget/.sma-install.json" -Raw | ConvertFrom-Json
if([IO.Path]::GetFullPath($receipt.InstallRoot) -ne [IO.Path]::GetFullPath($customTarget)){throw 'Custom uninstall target receipt invalid'}
$shell=New-Object -ComObject WScript.Shell
$shortcut=$shell.CreateShortcut((Join-Path $fixture 'Desktop/SekiMod Assistant Native.lnk'))
if($shortcut.TargetPath -ne (Join-Path $customTarget 'SekiModAssistant.exe')){throw 'Shortcut does not target child directory'}
[IO.File]::WriteAllText("$customTarget/upgrade-marker.txt",'old custom install')
InstallCustomFixture $true
if((Get-Content "$customTarget/upgrade-marker.txt" -Raw) -ne 'old custom install'){throw 'Custom install rollback failed'}
InstallCustomFixture $false
if(Test-Path "$customTarget/upgrade-marker.txt"){throw 'Custom upgrade retained obsolete file'}
if((Get-FileHash "$customTarget/SekiModAssistant.exe").Hash -ne (Get-FileHash $installedMain).Hash){throw 'Custom installed executable differs'}
if((Get-Content "$customParent/keep-parent.txt" -Raw) -ne 'parent file must remain'){throw 'Custom upgrade touched parent files'}
Write-Output 'PASS custom parent selection installs in SekiMod Assistant child directory'
Write-Output 'PASS custom shortcut and uninstall receipt point to child directory'
Write-Output 'PASS custom upgrade and failure rollback preserve parent files'
$report=@("PASS isolated installation and shortcut creation","PASS injected setup failure restores previous application","PASS upgrade replaces obsolete files","PASS installed application matches release hash and administrator manifest","PASS application code self-test (diagnostic build if specified)","NOTE registry writes simulated; actual uninstall registration/UAC not tested") -join [Environment]::NewLine
[IO.File]::WriteAllText((Join-Path $workspace "outputs/v0.3-Native-Setup-Tests.txt"),$report)
Write-Output $report
