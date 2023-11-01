param (
    [Parameter(Mandatory=$true)]
    [string]$dir,

    [Parameter(Mandatory=$true)]
    [string]$groupName,

    [Parameter(Mandatory=$true)]
    [string]$outputDir
)

# The path of the directory that we want to travers
# so that we can create the WIX components
$directoryPath = $dir

# Create an empty array to hold the file data
$fileData = @()

# XML header
$fileData += "<?xml version=`"1.0`" encoding=`"utf-8`"?>"
# WIX v4 header
$fileData += "<Wix xmlns=`"http://wixtoolset.org/schemas/v4/wxs`">"
# Fragment header
$fileData += "`t<Fragment>"
# Component group header
$fileData += "`t`t<ComponentGroup Id=`"${groupName}Group`" Directory=`"${groupName}`">"

# Get all the files in the directory recursively
Get-ChildItem -Path $directoryPath -Recurse -File | ForEach-Object {
    # Now we need to construct the XML entry for the current file.
    # The XML entry will be a WIX component unde the Component group. 
    $fileData += "`t`t`t<Component Id=`"ID_$($_.Name)_COMP`">"

    # Create a new variable with the full path of the file
    $fullPath = $_.FullName

    # Add the file entry to the component
    $fileData += "`t`t`t`t<File Id=`"ID_$($_.Name)`" Source=`"$fullPath`" KeyPath=`"yes`" />"    

    # Add the component to the component group
    $fileData += "`t`t`t</Component>"

    # Add the hashtable to the array
    $fileData += $fileInfo
}

# Component group footer
$fileData += "`t`t</ComponentGroup>"
# Fragment footer
$fileData += "`t</Fragment>"
# WIX footer
$fileData += "</Wix>"

# If the output directory does not exist, create it
if (!(Test-Path -Path $outputDir -PathType Container)) {
    New-Item -ItemType Directory -Path $outputDir
}

# Write the file data to the file
$fileData | Out-File -FilePath "${outputDir}/${groupName}.wxs" -Encoding utf8
