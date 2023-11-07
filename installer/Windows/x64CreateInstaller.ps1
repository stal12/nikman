# Check whether the build directory exists, if not create it
if (!(Test-Path -Path "build" -PathType Container)) {
    New-Item -ItemType Directory -Path "build"
}

# Before creating the installer we need to generate the "Resources" and "Shaders" components
# This is done by running the "scripts/CreateResourcesWIXComponents.ps1" script
Write-Host "[INFO] => Generating the WIX components from the `"resourcers` folder..." -NoNewLine
.\scripts\CreateWIXComponents.ps1 -dir "..\\..\\resources" -groupName "Resources" -outputDir "generated"
echo "DONE"

Write-Host "[INFO] => Generating the WIX components from the `"shaders`" folder..." -NoNewLine
.\scripts\CreateWIXComponents.ps1 -dir "..\\..\\shaders" -groupName "Shaders" -outputDir "generated"
echo "DONE"

# Build the installer for x64-Windows architecture
Write-Host "[INFO] => Building the installer for x64-Windows architecture..." -NoNewLine
wix build -o build/NikmanInstaller.msi -src Folders.wxs -src GameComponents.wxs -src Package.wxs -src generated\\Resources.wxs -src generated\\Shaders.wxs -culture en-US -loc Package.en-us.wxl -arch x64 -ext WixToolset.UI.wixext
echo "DONE"
echo "[INFO] => The installer is located in the `"build`" folder and is named `"NikmanInstaller.msi`""
