param(
    [string]$TargetRoot = "C:\Users\flyin\OneDrive\Desktop\qwen tts\QwensTtsBuddy"
)

$ErrorActionPreference = "Stop"
$SourceRoot = Split-Path -Parent $MyInvocation.MyCommand.Path

New-Item -ItemType Directory -Force -Path $TargetRoot | Out-Null

$files = @(
    "QwensTtsBuddy.exe",
    "main.cpp",
    "QwensTtsBuddy.sln",
    "QwensTtsBuddy.vcxproj",
    "BUILD_RELEASE.bat",
    "README.md",
    ".gitignore"
)

foreach ($file in $files) {
    $src = Join-Path $SourceRoot $file
    if (Test-Path $src) {
        Copy-Item -LiteralPath $src -Destination (Join-Path $TargetRoot $file) -Force
    }
}

Write-Host "Installed Qwens Tts Buddy to: $TargetRoot"
Write-Host "Your Qwen models, embedded Python, and generated audio were not removed or changed."

