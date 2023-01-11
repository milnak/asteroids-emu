<#
.SYNOPSIS
Convert asteroids roms to C arrays

.DESCRIPTION
Verify original ROMs, and convert to C arrays for use in C code.

.EXAMPLE
.\generate-romcode.ps1 -File 'romcode.h'
Include 'romcode.h' in your code.
Encoding ascii is used, as utf8 adds a BOM.
#>
Param(
    [Parameter(Mandatory = $true)][string]$Output
)

Function Get-FileBytes {
    Param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)][string]$ExpectedSHA1Hash
    )

    $resolvedPath = Resolve-Path -LiteralPath $Path

    $sha1 = New-Object Security.Cryptography.SHA1CryptoServiceProvider

    $fs = New-Object IO.FileStream($resolvedPath, [IO.FileMode]::Open, [IO.FileAccess]::Read)
    $fr = New-Object IO.BinaryReader($fs)
    $length = $fr.BaseStream.Length
    [byte[]]$bytes = $fr.ReadBytes($length)
    $hash = [BitConverter]::ToString($sha1.ComputeHash($bytes)) -replace '-', ''
    $fr.Dispose()
    $fs.Dispose()

    if ($hash -ine $ExpectedSHA1Hash) {
        throw "File $Path has invalid hash.`nExpected: $ExpectedSHA1Hash`nFound: $hash"
    }

    $bytes
}

Function Generate-Code {
    Param(
        [Parameter(Mandatory = $true)][string]$Identifier,
        [Parameter(Mandatory = $true)][byte[]]$Bytes
    )

    $bytes_per_row = 16
    $idx = 0

    'const uint8_t {0}[{1}] = {{' -f $Identifier, $Bytes.Length

    for ($row = 0; $row -lt [Math]::Ceiling($Bytes.Length / $bytes_per_row); $row++) {
        $str = $null
        for ($col = 0; $col -lt $bytes_per_row; $col++) {
            $str += ('0x{0:X2}, ' -f $Bytes[$idx])
            $idx++
        }
        "    $str"
    }

    '};'
    ''
}

Function Write-SectionHeader {
    Param([Parameter(Mandatory = $true)][string]$Text)

    "// {0}`n// {1}`n// {0}" -f ('-' * 76), $Text
}

if (Test-Path -LiteralPath $Output -PathType Leaf) {
    Remove-Item -LiteralPath $Output
}

# Vector ROM
# ROM_LOAD( "035127-02.np3",  0x5000, 0x0800, CRC(8b71fd9e) SHA1(8cd5005e531eafa361d6b7e9eed159d164776c70) )

$bytes = Get-FileBytes -Path '035127-02.np3' -ExpectedSHA1Hash '8cd5005e531eafa361d6b7e9eed159d164776c70'
Write-SectionHeader -Text '2K vector ROM (5000:57FF)' | Out-File -FilePath $Output -Encoding ascii -Append
Generate-Code -Identifier 'dvg_rom' -Bytes $bytes | Out-File -FilePath $Output -Encoding ascii -Append

# Game ROM
# ROM_LOAD( "035145-04e.ef2", 0x6800, 0x0800, CRC(b503eaf7) SHA1(5369dcfe01c0b9e48b15a96a0de8d23ee8ef9145) )
# ROM_LOAD( "035144-04e.h2",  0x7000, 0x0800, CRC(25233192) SHA1(51b2865fa897cdaa84ac6500c4b4833a80827019) )
# ROM_LOAD( "035143-02.j2",   0x7800, 0x0800, CRC(312caa02) SHA1(1ce2eac1ab90b972e3f1fc3d250908f26328d6cb) )

$bytes = Get-FileBytes -Path '035145-04e.ef2' -ExpectedSHA1Hash '5369dcfe01c0b9e48b15a96a0de8d23ee8ef9145'
$bytes += Get-FileBytes -Path '035144-04e.h2' -ExpectedSHA1Hash '51b2865fa897cdaa84ac6500c4b4833a80827019'
$bytes += Get-FileBytes -Path '035143-02.j2' -ExpectedSHA1Hash '1ce2eac1ab90b972e3f1fc3d250908f26328d6cb'

Write-SectionHeader -Text '6K game ROM (6800:7FFF)' | Out-File -FilePath $Output -Encoding ascii -Append
Generate-Code -Identifier 'game_rom' -Bytes $bytes | Out-File -FilePath $Output -Encoding ascii -Append

# DVG PROM
# ROM_REGION( 0x100, "dvg:prom", 0 )
# ROM_LOAD( "034602-01.c8",   0x0000, 0x0100, CRC(97953db8) SHA1(8cbded64d1dd35b18c4d5cece00f77e7b2cab2ad) )

# $bytes = Get-FileBytes -Path '034602-01.c8' -ExpectedSHA1Hash '8cbded64d1dd35b18c4d5cece00f77e7b2cab2ad'
# Generate-Code -Identifier 'dvg_prom' -Bytes $bytes
