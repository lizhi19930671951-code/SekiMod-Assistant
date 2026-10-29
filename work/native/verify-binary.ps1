function Get-NativeSections([string]$Path) {
    $bytes=[IO.File]::ReadAllBytes($Path)
    $pe=[BitConverter]::ToInt32($bytes,0x3c)
    if([Text.Encoding]::ASCII.GetString($bytes,$pe,4) -ne "PE`0`0"){throw 'Not a PE image'}
    $count=[BitConverter]::ToUInt16($bytes,$pe+6)
    $table=$pe+24+[BitConverter]::ToUInt16($bytes,$pe+20)
    $result=@{}
    for($i=0;$i -lt $count;$i++) {
        $at=$table+$i*40
        $name=[Text.Encoding]::ASCII.GetString($bytes,$at,8).TrimEnd([char]0)
        $length=[BitConverter]::ToUInt32($bytes,$at+16)
        $offset=[BitConverter]::ToUInt32($bytes,$at+20)
        $sha=[Security.Cryptography.SHA256]::Create()
        try{$result[$name]=[Convert]::ToHexString($sha.ComputeHash($bytes,$offset,$length))}finally{$sha.Dispose()}
    }
    return $result
}
function Assert-NativeDiagnosticMatches([string]$Release,[string]$Diagnostic) {
    $releaseSections=Get-NativeSections $Release
    $diagnosticSections=Get-NativeSections $Diagnostic
    foreach($section in $releaseSections.Keys | Where-Object {$_ -ne '.rsrc'}) {
        if($releaseSections[$section] -ne $diagnosticSections[$section]){throw "Diagnostic does not match release section $section"}
    }
    if(-not $releaseSections.ContainsKey('.text')){throw 'Missing executable code section'}
}
function Get-NativeManifest([string]$Path) {
    if(-not ('SmaResourceReader' -as [type])) {
        Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
public static class SmaResourceReader {
 [DllImport("kernel32.dll", CharSet=CharSet.Unicode, SetLastError=true)] public static extern IntPtr LoadLibraryEx(string path,IntPtr file,uint flags);
 [DllImport("kernel32.dll", SetLastError=true)] public static extern IntPtr FindResource(IntPtr module,IntPtr name,IntPtr type);
 [DllImport("kernel32.dll")] public static extern IntPtr LoadResource(IntPtr module,IntPtr resource);
 [DllImport("kernel32.dll")] public static extern IntPtr LockResource(IntPtr resource);
 [DllImport("kernel32.dll")] public static extern uint SizeofResource(IntPtr module,IntPtr resource);
 [DllImport("kernel32.dll")] public static extern bool FreeLibrary(IntPtr module);
}
'@
    }
    $module=[SmaResourceReader]::LoadLibraryEx([IO.Path]::GetFullPath($Path),[IntPtr]::Zero,0x22)
    if($module -eq [IntPtr]::Zero){throw 'Cannot read executable resources'}
    try {
        $resource=[SmaResourceReader]::FindResource($module,[IntPtr]1,[IntPtr]24)
        if($resource -eq [IntPtr]::Zero){throw 'Missing embedded manifest'}
        $length=[SmaResourceReader]::SizeofResource($module,$resource)
        $pointer=[SmaResourceReader]::LockResource([SmaResourceReader]::LoadResource($module,$resource))
        $bytes=New-Object byte[] $length
        [Runtime.InteropServices.Marshal]::Copy($pointer,$bytes,0,$length)
        return [Text.Encoding]::UTF8.GetString($bytes).TrimStart([char]0xfeff)
    } finally {[SmaResourceReader]::FreeLibrary($module)|Out-Null}
}
