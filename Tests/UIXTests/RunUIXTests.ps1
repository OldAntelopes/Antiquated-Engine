# RunUIXTests.ps1
#
# Builds the UIX test harness (Debug, Win32) and runs it. Prints the harness's verdict line,
# "UIXTests: ALL PASSED (...)" or "UIXTests: FAILED (...)", and exits 0 only if everything passed.
#
#   powershell -ExecutionPolicy Bypass -File Tests\UIXTests\RunUIXTests.ps1
#   ... -UpdateRefs        write the reference traces and images instead of comparing (then run again to check)
#   ... -Only Dropdown     run the tests whose names contain this text
#   ... -BuildLibs         build Pub\Libraries.sln (Debug) first, so the harness links the current Pub code
#   ... -NoBuild           run what's already built
#   ... -Verbose           list every check
#
# Failed snapshots leave the frame and a diff image (red = differing pixels) in Out\Results.
# See GameCommon\UIX\UIX.md.

param(
	[switch]$UpdateRefs,
	[string]$Only = "",
	[switch]$BuildLibs,
	[switch]$NoBuild,
	[switch]$Verbose
)

$ErrorActionPreference = "Stop"
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$root = Resolve-Path (Join-Path $here "..\..")
$msbuild = "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
$exe = Join-Path $here "Out\UIXTestsD.exe"

if (-not $NoBuild) {
	if (-not (Test-Path $msbuild)) {
		Write-Output "UIXTests: FAILED (MSBuild not found at $msbuild)"
		exit 2
	}
	if ($BuildLibs) {
		& $msbuild (Join-Path $root "Pub\Libraries.sln") -p:Configuration=Debug -p:Platform=Win32 -p:PlatformToolset=v143 -m -v:minimal -clp:ErrorsOnly -nologo
		if ($LASTEXITCODE -ne 0) {
			Write-Output "UIXTests: FAILED (Pub\Libraries.sln didn't build)"
			exit 2
		}
	}
	& $msbuild (Join-Path $here "UIXTests.vcxproj") -p:Configuration=Debug -p:Platform=Win32 -m -v:minimal -clp:ErrorsOnly -nologo
	if ($LASTEXITCODE -ne 0) {
		Write-Output "UIXTests: FAILED (the harness didn't build)"
		exit 2
	}
}

if (-not (Test-Path $exe)) {
	Write-Output "UIXTests: FAILED ($exe isn't built)"
	exit 2
}

$arguments = @()
if ($UpdateRefs) { $arguments += "--update-refs" }
if ($Only -ne "") { $arguments += "--only"; $arguments += $Only }
if ($Verbose) { $arguments += "--verbose" }

# The harness prints its own verdict line; Interface's debug prints are left out
& $exe @arguments | Where-Object { $_ -notmatch '^\[\d\d-\d\d\]' }
$code = $LASTEXITCODE
if ($code -ne 0 -and $code -ne 1) {
	Write-Output "UIXTests: FAILED (the harness stopped with exit code $code; it may have crashed)"
}
exit $code
