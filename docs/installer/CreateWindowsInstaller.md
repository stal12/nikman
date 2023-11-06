# Create a Windows installer

In order to create a Windows installer, you need to have a Windows machine with the following software installed:
- [.NET SDK](https://dotnet.microsoft.com/download): you can download it following the instructions in the website;
- [WiX Toolset](https://wixtoolset.org/): you can download it following the instructions in the website;

## Tools configuration

To create the installer, you need to configure the `WiX Toolset` as it's used to build the installer. `WiX` can be used as a command line tool or as a Visual Studio extension. In this guide, we will use the command line tool.

The `WiX Toolset` can be installed via the `dotnet` command so make sure to have the `.NET SDK` installed on the machine before proceeding.

To install the `WiX Toolset` run the following command in a terminal:

```ps
dotnet tool install --global wix
```

Once the installation is completed, you can check that the tool is correctly installed by running the following command:

```ps
wix
```

You should use `WiX` v4.0 or higher.

Once `WiX` is installed you need to add the UI extensions as it's used to build the installer. If you skip this step you will get an error when building the installer.

To install the `WiX Toolset` UI extensions run the following command in a terminal:

```ps
wix extension add -g WixToolset.UI.wixext
```

Then you should be all set to build the installer.

## Build the installer

Before proceeding with the creation of the installer, you need to build the project. Just build the project using CMake:

```ps
# On the root folder of the project
mkdir build
cd build
cmake .. # eventual other flags
cmake --build . --config Release
```

Since the configuration of the installer is not a simple task, some scripts are there to help you. You can navigate to the [installer/Windows](../../installer/Windows) folder and run the [x64CreateInstaller.ps1 script](../../installer/Windows/x64CreateInstaller.ps1):

```ps
cd installer/Windows
.\x64CreateInstaller.ps1
```

This script will create the installer for the x64 architecture. This script will also generate a folder called `generated` where you can find all the generated `.wxs` files that contains resources like shaders, audio files etc that are automatically added when parsing the project.

Once the script has finished, you will find the installer in the `installer/Windows/build` folder: you can find the `.msi` file that you can deploy on your Windows machine or on your preferred distribution platform.