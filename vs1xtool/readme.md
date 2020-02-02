### How to build from command line

#### Visual Studio 2015
`msbuild vs1xtool.sln`

#### Visual Studio 2017
`msbuild vs1xtool.sln -property:WindowsTargetPlatformVersion=10.0 -property:PlatformToolset=v141`

#### Visual Studio 2019
`msbuild vs1xtool.sln -property:WindowsTargetPlatformVersion=10.0 -property:PlatformToolset=v142`
