param([string]$Application = '',[string]$Output = '',[string]$DiagnosticApplication = '')
$ErrorActionPreference='Stop'
$workspaceRoot=Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
if(-not $Application){$Application=Join-Path $workspaceRoot 'outputs\SekiModAssistant-Native-v0.3'}
if(-not $Output){$Output=Join-Path $workspaceRoot 'outputs'}
$Application=[IO.Path]::GetFullPath($Application)
$Output=[IO.Path]::GetFullPath($Output)
New-Item -ItemType Directory -Force $Output | Out-Null
$main=Join-Path $Application 'SekiModAssistant.exe'
$stub=Join-Path $Application 'SekiModSetupStub.exe'
foreach($path in @($main,$stub,(Join-Path $Application 'tools\7z.exe'),(Join-Path $Application 'tools\7z.dll'))){if(-not(Test-Path -LiteralPath $path)){throw "Missing compiled file: $path"}}
. (Join-Path $PSScriptRoot 'verify-binary.ps1')
if((Get-NativeManifest $main) -notmatch 'level="requireAdministrator"'){throw 'Release must request administrator on startup'}
$selfTestMain=$main
if($DiagnosticApplication){
    $selfTestMain=Join-Path $DiagnosticApplication 'SekiModAssistant.exe'
    Assert-NativeDiagnosticMatches $main $selfTestMain
    Write-Warning 'Testing matching code with diagnostic manifest; release UAC and Explorer cross-integrity drag need desktop verification.'
}
$verification=Start-Process -FilePath $selfTestMain -ArgumentList '--self-test' -WindowStyle Hidden -PassThru -Wait
if($verification.ExitCode -ne 0){throw 'Native self-test failed; package was not created'}
$portable=Join-Path $Output 'SekiModAssistant-v0.3-Native-portable.zip'
if(Test-Path -LiteralPath $portable){throw 'Portable archive already exists; rename it before packaging to avoid retaining stale entries.'}
$seven=Join-Path $Application 'tools\7z.exe'
& $seven a -tzip -mx=7 $portable "$Application\*" '-xr!native-test-results' '-x!native-test-report.txt' '-x!SekiModSetupStub.exe' '-xr!*.obj' '-xr!*.res'
if($LASTEXITCODE -ne 0){throw 'Archive failed'}
$installer=Join-Path $Output 'SekiModAssistant-v0.3-Native-Setup.exe'
$parts=@($stub,$seven,(Join-Path $Application 'tools\7z.dll'),$portable)
$stream=[IO.File]::Create($installer)
try{
    foreach($path in $parts){$inputStream=[IO.File]::OpenRead($path);try{$inputStream.CopyTo($stream)}finally{$inputStream.Dispose()}}
    foreach($path in $parts){$bytes=[BitConverter]::GetBytes([uint64](Get-Item -LiteralPath $path).Length);$stream.Write($bytes,0,8)}
    $magic=[Text.Encoding]::ASCII.GetBytes('SMANAT02');$stream.Write($magic,0,8)
}finally{$stream.Dispose()}
$report=Join-Path $Output 'v0.3-Native-Installer-Verification.txt'
$tail=[IO.File]::OpenRead($installer)
try{$tail.Seek(-40,[IO.SeekOrigin]::End)|Out-Null;$bytes=New-Object byte[] 40;if($tail.Read($bytes,0,40)-ne 40){throw "Installer trailer unreadable"};$magic=[Text.Encoding]::ASCII.GetString($bytes,32,8);if($magic -ne "SMANAT02"){throw "Installer trailer invalid"};$sizes=0..3|ForEach-Object{[BitConverter]::ToUInt64($bytes,$_ * 8)};if((40+($sizes|Measure-Object -Sum).Sum)-ne (Get-Item $installer).Length){throw "Installer payload length invalid"}}finally{$tail.Dispose()}
Set-Content -LiteralPath $report -Value "PASS installer trailer and payload lengths verified" -Encoding utf8
$previousData=$env:SMA_DATA
try {
    $env:SMA_DATA=Join-Path $workspaceRoot 'work/package-payload-verification'
    $check=Start-Process -FilePath $installer -ArgumentList @('--verify-payload',('"'+$report+'"')) -WindowStyle Hidden -PassThru -Wait
} finally {$env:SMA_DATA=$previousData}
if($check.ExitCode -ne 0){throw 'Installer payload verification failed'}
$bytes=(Get-ChildItem -LiteralPath $Application -Recurse -File | Where-Object { $_.FullName -notlike '*\native-test-results\*' -and $_.Name -notin @('native-test-report.txt','SekiModSetupStub.exe') -and $_.Extension -notin @('.obj','.res') } | Measure-Object Length -Sum).Sum
Write-Output ('Application size: {0:N2} MiB' -f ($bytes/1MB))
if($bytes -gt 15MB){Write-Warning 'Application exceeds the 15 MiB target; inspect before delivery.'}

