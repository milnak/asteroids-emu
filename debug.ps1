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

"Launching $FilePath . . ."

if ($NoDebugger) {
    & $FilePath
}
else {
    if (-not (Get-Command 'WinDbgX.exe' -CommandType Application))
    {
        Show-Error "$_"
        'To install WinDbg use: winget install --accept-package-agreements ''WinDbg Preview'''
        exit 1
    }

    WinDbgX.exe -c $InitialCommand $FilePath $args
}

exit 0
