Add-Type -AssemblyName System.Drawing

Add-Type @"
using System;
using System.Runtime.InteropServices;
public class Win32 {
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr hWnd, out RECT lpRect);
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
    [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left, Top, Right, Bottom; }
}
"@

$proc = Start-Process -FilePath $args[0] -PassThru
Start-Sleep -Seconds 4
$proc.Refresh()

$h = $proc.MainWindowHandle
if ($h -eq [IntPtr]::Zero) { Write-Error "no window handle"; $proc.Kill(); exit 1 }

[void][Win32]::SetForegroundWindow($h)
Start-Sleep -Milliseconds 700

$r = New-Object Win32+RECT
[void][Win32]::GetWindowRect($h, [ref]$r)

$w = $r.Right - $r.Left
$hgt = $r.Bottom - $r.Top
Write-Output "window $w x $hgt at $($r.Left),$($r.Top)"

$bmp = New-Object System.Drawing.Bitmap $w, $hgt
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.CopyFromScreen($r.Left, $r.Top, 0, 0, $bmp.Size)
$bmp.Save($args[1], [System.Drawing.Imaging.ImageFormat]::Png)
$g.Dispose(); $bmp.Dispose()

$proc.CloseMainWindow() | Out-Null
Start-Sleep -Milliseconds 500
if (-not $proc.HasExited) { $proc.Kill() }
Write-Output "saved $($args[1])"
