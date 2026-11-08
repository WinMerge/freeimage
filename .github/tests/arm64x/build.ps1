param(
    [ValidateSet('Debug', 'Release')][string]$Configuration = 'Release',
    [Parameter(Mandatory)][string]$BuildRoot,
    [Parameter(Mandatory)][string]$OutputRoot
)
$ErrorActionPreference = 'Stop'
$source = $PSScriptRoot
$vswhere = "${env:ProgramFiles(x86)}/Microsoft Visual Studio/Installer/vswhere.exe"
$instances = @(& $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.ARM64 -format json | ConvertFrom-Json)
if ($LASTEXITCODE -or !$instances.Count) { throw 'No Visual Studio instance with ARM64 tools was found' }
$instance = $instances[0]
$major = ([version]$instance.installationVersion).Major
$generator = switch ($major) {
    17 { 'Visual Studio 17 2022' }
    18 { 'Visual Studio 18 2026' }
    default { throw "Unsupported Visual Studio major version: $major" }
}
$msvcRoot = Join-Path $instance.installationPath 'VC/Tools/MSVC'
$toolset = Get-ChildItem $msvcRoot -Directory | Sort-Object { [version]$_.Name } -Descending | Select-Object -First 1
$dumpbin = Join-Path $toolset.FullName 'bin/Hostx64/x64/dumpbin.exe'
if (!(Test-Path $dumpbin)) { throw 'Cannot find the x64-hosted dumpbin tool' }
$env:CMAKE_BUILD_PARALLEL_LEVEL = [Environment]::ProcessorCount
New-Item -ItemType Directory -Force $BuildRoot, $OutputRoot | Out-Null
$BuildRoot = (Resolve-Path $BuildRoot).Path
$OutputRoot = (Resolve-Path $OutputRoot).Path

function Invoke-CMake([string[]]$Arguments) {
    & cmake @Arguments
    if ($LASTEXITCODE) { throw "CMake failed with exit code $LASTEXITCODE" }
}
function Configure([string]$Folder, [string]$Architecture, [string]$Stage, [string[]]$Extra = @()) {
    Invoke-CMake (@('-S', $source, '-B', $Folder, '-G', $generator, '-A', $Architecture,
        "-DCMAKE_GENERATOR_INSTANCE=$($instance.installationPath)", "-DARM64X_STAGE=$Stage") + $Extra)
}
function Build([string]$Folder, [string[]]$Targets) {
    Invoke-CMake (@('--build', $Folder, '--config', $Configuration, '--target') + $Targets)
}
function Stage([string]$Folder, [string]$Destination) {
    $destinationPath = Join-Path $OutputRoot $Destination
    New-Item -ItemType Directory -Force $destinationPath | Out-Null
    Copy-Item (Join-Path $Folder "bin/$Configuration/FreeImage.dll") $destinationPath
}
$native = Join-Path $BuildRoot 'native'
$compatible = Join-Path $BuildRoot 'compatible'
$consumer = Join-Path $BuildRoot 'x64-consumer'
$probes = Join-Path $OutputRoot 'probes'
New-Item -ItemType Directory -Force $probes | Out-Null

Configure $native ARM64 Native
Build $native @('FreeImage', 'ARM64XSmoke')
Stage $native native
Copy-Item (Join-Path $native "bin/$Configuration/ARM64XSmoke.exe") (Join-Path $probes 'arm64.exe')

Configure $compatible ARM64EC Compatible
Build $compatible @('FreeImage', 'ARM64XSmoke')
Stage $compatible compatible
Copy-Item (Join-Path $compatible "bin/$Configuration/ARM64XSmoke.exe") (Join-Path $probes 'arm64ec.exe')

# Reconfigure and relink the EC objects with the actual native ARM64 inputs.
Configure $compatible ARM64EC Hybrid @("-DARM64X_NATIVE_RSP=$native/native.rsp")
Copy-Item (Join-Path $native 'native.rsp') (Join-Path $OutputRoot 'native-link-inputs.rsp')
# Log both the mapped TargetMachine and AdditionalOptions. CMake's default
# /machine:ARM64EC must not override the hybrid link's /MACHINE:ARM64X.
Write-Host '::group::Hybrid FreeImage.vcxproj link machine settings'
Select-String -Path (Join-Path $compatible 'freeimage/FreeImage.vcxproj') -Pattern '<TargetMachine>|machine:' |
    ForEach-Object { $line = $_.Line.Trim(); if ($line.Length -gt 300) { $line = $line.Substring(0, 300) + '...' }; "$($_.LineNumber): $line" } |
    Write-Host
Write-Host '::endgroup::'
# Relink with normal MSBuild verbosity so the link command line is logged,
# then print only its /MACHINE switches and any linker diagnostics.
$hybridLog = Join-Path $OutputRoot 'hybrid-link.log'
& cmake --build $compatible --config $Configuration --target FreeImage -- /v:n *>&1 | Set-Content $hybridLog
$linkExit = $LASTEXITCODE
$machineSwitches = @(Select-String -Path $hybridLog -Pattern '(?i)/machine:\S+' -AllMatches |
    ForEach-Object { $_.Matches.Value } | Sort-Object -Unique)
"Hybrid link /MACHINE switches: $($machineSwitches -join ' ')" | Write-Host
Select-String -Path $hybridLog -Pattern 'LNK\d{4}' | ForEach-Object { $_.Line.Trim() } | Write-Host
if ($linkExit) { Get-Content $hybridLog -Tail 40 | Write-Host; throw "Hybrid link failed with exit code $linkExit" }
$unexpectedMachines = @($machineSwitches | Where-Object { $_ -ine '/MACHINE:ARM64X' })
if ($unexpectedMachines.Count -or $machineSwitches -notcontains '/MACHINE:ARM64X') {
    throw "Hybrid link must use only /MACHINE:ARM64X; found: $($machineSwitches -join ' ')"
}
Stage $compatible hybrid

Configure $consumer x64 Consumer
Build $consumer @('ARM64XSmoke')
Copy-Item (Join-Path $consumer "bin/$Configuration/ARM64XSmoke.exe") (Join-Path $probes 'x64.exe')

foreach ($kind in @('native', 'compatible', 'hybrid')) {
    $dll = Join-Path $OutputRoot "$kind/FreeImage.dll"
    $headers = & $dumpbin /headers $dll
    if ($LASTEXITCODE) { throw "dumpbin failed for $kind" }
    $headers | Set-Content (Join-Path $OutputRoot "$kind-headers.txt")
    $machine = @($headers | Select-String -SimpleMatch ' machine ' | ForEach-Object { $_.Line.Trim() })
    "${kind}: $($machine -join ' | ')" | Write-Host
    & $dumpbin /loadconfig $dll | Set-Content (Join-Path $OutputRoot "$kind-loadconfig.txt")
    if ($LASTEXITCODE) { throw "dumpbin /loadconfig failed for $kind" }
    # dumpbin also prints "(ARM64X)" for plain ARM64EC images, so check for
    # the ARM64X dynamic relocations that only a merged image carries.
    & python (Join-Path $source 'check-arm64x.py') $dll
    if ($LASTEXITCODE) { throw "check-arm64x.py failed for $kind" }
    if ($kind -eq 'hybrid') {
        & python (Join-Path $source 'check-arm64x.py') --require-arm64x $dll | Out-Null
        if ($LASTEXITCODE) { throw 'The merged DLL has no ARM64X dynamic relocations, so it is not an ARM64X image' }
    }
    & $dumpbin /exports $dll | Set-Content (Join-Path $OutputRoot "$kind-exports.txt")
    if ($LASTEXITCODE) { throw "Cannot inspect $kind exports" }
}
Get-ChildItem $OutputRoot -Recurse -File | Where-Object { $_.Extension -in '.dll', '.exe' } | ForEach-Object {
    '{0}  {1}' -f (Get-FileHash $_.FullName -Algorithm SHA256).Hash, [IO.Path]::GetRelativePath($OutputRoot, $_.FullName)
} | Set-Content (Join-Path $OutputRoot 'SHA256SUMS.txt')
"Built ARM64X $Configuration artifacts in $OutputRoot" | Write-Host
