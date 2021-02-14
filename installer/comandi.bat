heat dir resources -dr INSTALLDIR -cg ResourcesGroup -out ResourcesGroup.wxs -gg -var var.ResourcesDir
heat dir shaders -dr INSTALLDIR -cg ShadersGroup -out ShadersGroup.wxs -gg -var var.ShadersDir
candle -dResourcesDir=resources -dShadersDir=shaders SampleFirst.wxs ResourcesGroup.wxs ShadersGroup.wxs
light -out SampleFirst.msi SampleFirst.wixobj ResourcesGroup.wixobj ShadersGroup.wixobj