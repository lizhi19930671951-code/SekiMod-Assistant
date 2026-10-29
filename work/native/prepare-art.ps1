$ErrorActionPreference="Stop"
Add-Type -AssemblyName System.Drawing
$art=Join-Path $PSScriptRoot "art"
$original="C:/Users/lizhi/Pictures/只狼mod安装工具素材/MEITU_20261028_025543562.jpg"
$image=[Drawing.Image]::FromFile($original)
try {
 $width=1920;$height=[int][Math]::Round($image.Height*$width/$image.Width)
 $canvas=[Drawing.Bitmap]::new($width,$height);$g=[Drawing.Graphics]::FromImage($canvas)
 $g.InterpolationMode=[Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
 $g.DrawImage($image,0,0,$width,$height);$g.Dispose()
 $codec=[Drawing.Imaging.ImageCodecInfo]::GetImageEncoders() | Where-Object MimeType -eq "image/jpeg"
 $parameters=[Drawing.Imaging.EncoderParameters]::new(1);$parameters.Param[0]=[Drawing.Imaging.EncoderParameter]::new([Drawing.Imaging.Encoder]::Quality,[long]88)
 $canvas.Save((Join-Path $art "background.jpg"),$codec,$parameters);$canvas.Dispose();$parameters.Dispose()
}finally{$image.Dispose()}
$blade=[Drawing.Image]::FromFile((Join-Path $art "blade.png"))
$sizes=@(16,20,24,32,40,48,64,128,256);$chunks=@()
try{foreach($size in $sizes){
 $bitmap=[Drawing.Bitmap]::new($size,$size,[Drawing.Imaging.PixelFormat]::Format32bppArgb);$g=[Drawing.Graphics]::FromImage($bitmap);$g.Clear([Drawing.Color]::Transparent);$g.InterpolationMode=[Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
 $ratio=[Math]::Min(($size-2)/$blade.Width,($size-2)/$blade.Height);$w=[int]($blade.Width*$ratio);$h=[int]($blade.Height*$ratio);$g.DrawImage($blade,[int](($size-$w)/2),[int](($size-$h)/2),$w,$h);$g.Dispose()
 $memory=[IO.MemoryStream]::new();$bitmap.Save($memory,[Drawing.Imaging.ImageFormat]::Png);$chunks+=,@($size,$memory.ToArray());$memory.Dispose();$bitmap.Dispose()
}}finally{$blade.Dispose()}
$stream=[IO.File]::Create((Join-Path $art "app.ico"));$writer=[IO.BinaryWriter]::new($stream)
try{$writer.Write([uint16]0);$writer.Write([uint16]1);$writer.Write([uint16]$sizes.Count);$offset=6+16*$sizes.Count
 foreach($chunk in $chunks){$edge=if($chunk[0] -eq 256){0}else{$chunk[0]};$writer.Write([byte]$edge);$writer.Write([byte]$edge);$writer.Write([uint16]0);$writer.Write([uint16]1);$writer.Write([uint16]32);$writer.Write([uint32]$chunk[1].Length);$writer.Write([uint32]$offset);$offset+=$chunk[1].Length}
 foreach($chunk in $chunks){$writer.Write([byte[]]$chunk[1])}
}finally{$writer.Dispose()}
