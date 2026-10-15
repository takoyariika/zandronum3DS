param(
    [Parameter(Position=0)][string]$Archive,
    [string]$Output
)
$ErrorActionPreference = 'Stop'
if (-not $Archive) { $Archive = Read-Host 'WAD/PK3/ZIP path (drag the file here, then Enter)' }
$Archive = [IO.Path]::GetFullPath($Archive.Trim().Trim('"'))
# WADs use the engine's lump-indexed resource cache, not the ZIP PNG format.
# Detect by signature so uppercase extensions and renamed archives work too.
$probe = [IO.File]::OpenRead($Archive)
try {
    $magic = New-Object byte[] 4
    $read = $probe.Read($magic,0,4)
} finally { $probe.Dispose() }
if ($read -eq 4 -and [Text.Encoding]::ASCII.GetString($magic) -in @('IWAD','PWAD')) {
    if ($Output -and -not $Output.EndsWith('.3ds-resources',[StringComparison]::OrdinalIgnoreCase)) {
        throw 'WAD output must end in .3ds-resources. Omit -Output to choose the correct name automatically.'
    }
    Write-Host 'WAD detected: preparing all lump resources (textures, sounds, maps and scripts).'
    & "$PSScriptRoot/Build-ResourceCache.ps1" -Archive $Archive -Output $Output
    return
}
if (-not $Output) { $Output = $Archive + '.3ds-cache' }
$Output = [IO.Path]::GetFullPath($Output)
if ($Archive -eq $Output) { throw 'Output must differ from the source archive.' }
if (-not $Output.EndsWith('.3ds-cache',[StringComparison]::OrdinalIgnoreCase)) { throw 'Output must end in .3ds-cache; original archives are never overwritten.' }
$file = [IO.File]::Open($Archive, 'Open', 'Read', 'Read')
$reader = [IO.BinaryReader]::new($file, [Text.Encoding]::UTF8, $true)
$payload = [IO.MemoryStream]::new()
$writer = [IO.BinaryWriter]::new($payload)
$md5 = [Security.Cryptography.MD5]::Create()
try {
    if ($file.Length -gt [uint32]::MaxValue) { throw 'ZIP64 is not supported by this engine.' }
    $tailSize = [int][Math]::Min(65557, $file.Length)
    $file.Position = $file.Length - $tailSize
    $tail = $reader.ReadBytes($tailSize)
    $eocd = -1
    for ($i=$tail.Length-22; $i -ge 0; $i--) {
        if ([BitConverter]::ToUInt32($tail,$i) -eq 0x06054b50 -and
            $i+22+[BitConverter]::ToUInt16($tail,$i+20) -eq $tail.Length) { $eocd=$i; break }
    }
    if ($eocd -lt 0) { throw 'ZIP end directory not found.' }
    if ([BitConverter]::ToUInt16($tail,$eocd+4) -ne 0 -or [BitConverter]::ToUInt16($tail,$eocd+6) -ne 0) { throw 'Multi-disk ZIP is unsupported.' }
    $entryCount = [BitConverter]::ToUInt16($tail,$eocd+10)
    if ($entryCount -eq 65535) { throw 'ZIP64 is unsupported.' }
    $directorySize = [BitConverter]::ToUInt32($tail,$eocd+12)
    $directoryOffset = [BitConverter]::ToUInt32($tail,$eocd+16)
    $file.Position = $directoryOffset
    $directory = $reader.ReadBytes($directorySize)
    if ($directory.Length -ne $directorySize) { throw 'Truncated directory.' }
    $records = [Collections.Generic.List[object]]::new()
    $pos=0
    for ($i=0; $i -lt $entryCount; $i++) {
        if ($pos+46 -gt $directory.Length -or [BitConverter]::ToUInt32($directory,$pos) -ne 0x02014b50) { throw 'Invalid central directory.' }
        $nameLength = [BitConverter]::ToUInt16($directory,$pos+28)
        $extraLength = [BitConverter]::ToUInt16($directory,$pos+30)
        $commentLength = [BitConverter]::ToUInt16($directory,$pos+32)
        $name = [Text.Encoding]::UTF8.GetString($directory,$pos+46,$nameLength)
        $records.Add(@{
            offset=[BitConverter]::ToUInt32($directory,$pos+42)
            size=[BitConverter]::ToUInt32($directory,$pos+24)
            compressed=[BitConverter]::ToUInt32($directory,$pos+20)
            crc=[BitConverter]::ToUInt32($directory,$pos+16)
            flags=[BitConverter]::ToUInt16($directory,$pos+8)
            method=[BitConverter]::ToUInt16($directory,$pos+10)
            name=$name
        })
        $pos += 46+$nameLength+$extraLength+$commentLength
    }
    $count=0; $processed=0
    foreach ($entry in ($records | Sort-Object -Property @{Expression={ [uint64]$_['offset'] }})) {
        $processed++
        if ($processed % 1000 -eq 0) { Write-Progress -Activity 'Preparing PNG texture headers' -Status "$processed / $entryCount" -PercentComplete (100*$processed/$entryCount) }
        if ($entry.size -lt 41 -or ($entry.flags -band 1) -or $entry.method -notin @(0,8)) { continue }
        $file.Position = $entry.offset
        $local = $reader.ReadBytes(30)
        if ($local.Length -ne 30 -or [BitConverter]::ToUInt32($local,0) -ne 0x04034b50) { throw 'Invalid local header.' }
        $file.Position += [BitConverter]::ToUInt16($local,26)+[BitConverter]::ToUInt16($local,28)
        $stream = $file
        if ($entry.method -eq 8) { $stream = [IO.Compression.DeflateStream]::new($file,[IO.Compression.CompressionMode]::Decompress,$true) }
        $input = [IO.BinaryReader]::new($stream,[Text.Encoding]::UTF8,$true)
        $prefix = [IO.MemoryStream]::new()
        try {
            $signature = $input.ReadBytes(8)
            if ([BitConverter]::ToString($signature) -ne '89-50-4E-47-0D-0A-1A-0A') { continue }
            $prefix.Write($signature,0,8)
            $valid=$false
            while ($prefix.Length -lt 65536) {
                $chunk=$input.ReadBytes(8)
                if ($chunk.Length -ne 8) { break }
                $length = [uint32]($chunk[0]*16777216L+$chunk[1]*65536L+$chunk[2]*256L+$chunk[3])
                $type = [Text.Encoding]::ASCII.GetString($chunk,4,4)
                $prefix.Write($chunk,0,8)
                if ($type -eq 'IDAT') { $valid=$true; break }
                if ($type -eq 'IEND' -or $length -gt 65536 -or $prefix.Length+$length+4 -gt 65536) { break }
                $body=$input.ReadBytes($length+4)
                if ($body.Length -ne $length+4) { break }
                $prefix.Write($body,0,$body.Length)
            }
            if (-not $valid) { continue }
            $bytes=$prefix.ToArray()
            if ($bytes.Length -lt 41 -or [Text.Encoding]::ASCII.GetString($bytes,12,4) -ne 'IHDR' -or
                $bytes[24] -notin @(1,2,4,8) -or $bytes[25] -notin @(0,2,3,4,6) -or
                $bytes[26] -ne 0 -or $bytes[27] -ne 0 -or $bytes[28] -gt 1) { continue }
            $writer.Write([uint32]$entry.offset); $writer.Write([uint32]$entry.size)
            $writer.Write([uint32]$entry.compressed); $writer.Write([uint32]$entry.crc)
            $writer.Write([uint32]$bytes.Length); $writer.Write($bytes)
            $count++
            if ($payload.Length+48 -gt 16*1024*1024) { throw 'Header cache exceeds the engine 16 MiB limit.' }
        } finally {
            $input.Dispose(); $prefix.Dispose()
            if ($entry.method -eq 8) { $stream.Dispose() }
        }
    }
    $writer.Flush()
    $body=$payload.ToArray()
    $result=[IO.MemoryStream]::new()
    $outWriter=[IO.BinaryWriter]::new($result)
    try {
        $outWriter.Write([Text.Encoding]::ASCII.GetBytes('Z3DTH003'))
        $outWriter.Write($md5.ComputeHash($directory))
        $outWriter.Write([uint32]$file.Length); $outWriter.Write([uint32]$count)
        $outWriter.Write($md5.ComputeHash($body)); $outWriter.Write($body)
        $outWriter.Flush()
        [IO.File]::WriteAllBytes($Output,$result.ToArray())
    } finally { $outWriter.Dispose(); $result.Dispose() }
    Write-Progress -Activity 'Preparing PNG texture headers' -Completed
    Write-Host "Prepared $count PNG headers: $Output ($($body.Length+48) bytes)"
    Write-Host 'Copy this file beside the unchanged original PK3 on the 3DS SD card.'
} finally { $writer.Dispose(); $payload.Dispose(); $reader.Dispose(); $file.Dispose(); $md5.Dispose() }
