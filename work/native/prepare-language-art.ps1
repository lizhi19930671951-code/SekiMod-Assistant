param([string]$Source='C:/Users/lizhi/Pictures/只狼mod安装工具素材/Screenshot_20251005_161730_tv_danmaku_bili_UnitedBizDetailsActivity_edit_714212166161922.jpg')
$ErrorActionPreference='Stop'
Add-Type -AssemblyName System.Drawing
$original=[Drawing.Bitmap]::new($Source)
$cut=[Drawing.Bitmap]::new(220,42,[Drawing.Imaging.PixelFormat]::Format32bppArgb)
try {
 for($y=0;$y -lt 42;$y++){for($x=0;$x -lt 220;$x++){
  $color=$original.GetPixel($x+7,$y+13)
  $value=[Math]::Max($color.R,[Math]::Max($color.G,$color.B))
  $alpha=[int][Math]::Clamp(($value-26)*255/35,0,255)
  if($y -lt 3 -or $y -gt 38){$alpha=0}
  $cut.SetPixel($x,$y,[Drawing.Color]::FromArgb($alpha,$color.R,$color.G,$color.B))
 }}
 $cut.Save((Join-Path $PSScriptRoot 'art/language-strip.png'),[Drawing.Imaging.ImageFormat]::Png)
} finally {$cut.Dispose();$original.Dispose()}
