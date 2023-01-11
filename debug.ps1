Param(
    [string]$FilePath = "$((Get-Location).Path)/build/debug/asteroids-emu.exe",
    [string]$InitialCommand = 'bp asteroids_emu!main; g',
    [switch]$NoDebugger = $false
)

function Show-Error {
    Param([Parameter(mandatory = $true, position = 0)][string]$message)
    Write-Host -ForegroundColor Red -NoNewline 'Error:'
    Write-Host " $message"
}

function Get-WinDbgX {
    Param(
        [String] $AppxPackage = 'Microsoft.WinDbg',
        [String] $Executable = 'DbgX.Shell.exe'
    )

    $pkg = Get-AppxPackage -Name $AppxPackage
    if (-not $pkg) {
        throw 'Unknown AppX Package: ' + $AppxPackage
    }
    $xml = [xml](Get-Content (Join-Path $pkg.InstallLocation 'AppxManifest.xml'))

    $pfn = $pkg.PackageFamilyName

    $appid = ($xml.Package.Applications.Application | Where-Object Executable -eq $Executable).id
    if (-not $appid) {
        throw 'Unknown Executable: ' + $Executable
    }

    "shell:appsFolder\$pfn!$appid"
}

if (-not (Test-Path $FilePath -PathType Leaf)) {
    Show-Error "Cannot find $FilePath"
    exit 1
}

"Launching $FilePath . . ."

if ($NoDebugger) {
    & $FilePath
}
else {
    try {
        $windbgx = Get-WinDbgX
    }
    catch {
        Show-Error "$_"
        'To install WinDbg use: winget install --accept-package-agreements ''WinDbg Preview'''
        exit 1
    }

    cmd.exe /c start $windbgx -c $InitialCommand $FilePath $args
}

exit 0
