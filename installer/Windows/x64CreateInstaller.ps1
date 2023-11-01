# Check whether the build directory exists, if not create it
if (!(Test-Path -Path "build" -PathType Container)) {
    New-Item -ItemType Directory -Path "build"
}

# Build the installer for x64-Windows architecture
wix build -o build/NikmanInstaller.msi -src Folders.wxs -src GameComponents.wxs -src Package.wxs -culture en-US -loc Package.en-us.wxl -arch x64
