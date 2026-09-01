param(
    [Parameter(Mandatory = $true)]
    [string]$Robot,
    [string]$RemoteRoot = "/home/ucar/PetCargo",
    [string]$Underlay = "/home/ucar/2026-xunfei-race/devel/setup.bash",
    [string]$Workspace = "/home/ucar/petcargo_ws"
)

$ErrorActionPreference = "Stop"
$repoRoot = Split-Path -Parent $PSScriptRoot
if (-not $RemoteRoot.StartsWith("/home/ucar/")) {
    throw "RemoteRoot must stay below /home/ucar/."
}
if (-not $Workspace.StartsWith("/home/ucar/")) {
    throw "Workspace must stay below /home/ucar/."
}

Write-Host "[1/4] Checking SSH connection to $Robot"
& ssh $Robot "test -d /home/ucar && mkdir -p '$RemoteRoot'"
if ($LASTEXITCODE -ne 0) { throw "SSH check failed." }

Write-Host "[2/4] Uploading PetCargo source without touching the competition repository"
$items = @("README.md", "firmware", "voice", "ros", "dashboard", "tools", "docs")
foreach ($item in $items) {
    $source = Join-Path $repoRoot $item
    if (Test-Path -LiteralPath $source) {
        & scp -r $source "${Robot}:${RemoteRoot}/"
        if ($LASTEXITCODE -ne 0) { throw "Upload failed: $item" }
    }
}

Write-Host "[3/4] Building the isolated catkin workspace"
& ssh $Robot "bash '$RemoteRoot/tools/install_on_robot.sh' '$RemoteRoot' '$Underlay' '$Workspace'"
if ($LASTEXITCODE -ne 0) { throw "Remote build failed." }

Write-Host "[4/4] Deployment complete"
Write-Host "Install the udev rule once if needed:"
Write-Host "  ssh $Robot sudo bash $RemoteRoot/tools/install_udev_rule.sh $RemoteRoot"
Write-Host "Start PetCargo:"
Write-Host "  ssh -t $Robot $RemoteRoot/tools/run_robot.sh $RemoteRoot $Underlay $Workspace"

