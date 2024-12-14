# PathmapResolver - USD Custom Resolver

## Overview
`PathmapResolver` is a custom resolver for USD (Universal Scene Description) that adds path mapping functionality.
This plugin extends the `ArDefaultResolver` and provides custom logic for dynamically resolving paths in USD files, allowing for flexible path resolution.


## Prerequisites
To build and use this plugin, you will need the following software:

- CMake version 3.25 or higher
- Houdini version 19.5 or higher
- Compiler: The choice of compiler depends on the DCC tool you're using
  - On Windows (for Houdini), I have tested with MSVC 2019.
  - On Linux (for Houdini), I have tested with GCC 8.5.0.

  
## Installation
### Clone the repository:
```bash
git clone https://github.com/yu-kita89/USD_PathmapResolver.git
cd USD_PathmapResolver
```

### Set HFS environment variable:
```bash
# Windows
set HFS=C:\Program Files\Side Effects Software\Houdini 20.0.653

# Linux
export HFS="/opt/hfs20.0.653"
```
This is used internally during the CMake build process.

### Build the project using CMake:
```bash
mkdir build
cd build
cmake --fresh ..
cmake --build . --config Release --clean-first
cmake --install .
```


## Usage
### Loading the Package
To load the pathmapResolver.json package located in the package directory,
set the `HOUDINI_PACKAGE_DIR` environment variable.
```bash
export HOUDINI_PACKAGE_DIR=<USD_PathmapResolver>/dist/pathmapResolver/<Platform>/packages
#This will allow Houdini to properly recognize the package.
```

### Pathmap Configuration
By default, the `HOUDINI_PATHMAP` environment variable is used to set up the pathmap.

However, you can change the environment variable used for the pathmap with `AR_PATHMAP_ENVIRONMENT` or by using the `SetDefaultPathmapEnvironment` function.

#### Example
`AR_PATHMAP_ENVIRONMENT`
```bash
# Windows
set ANY_PATHMAP={'foo':'var'}
set AR_PATHMAP_ENVIRONMENT=ANY_PATHMAP

# Linux
export ANY_PATHMAP="{'foo':'var'}"
set AR_PATHMAP_ENVIRONMENT=ANY_PATHMAP
```

`SetDefaultPathmapEnvironment()`
```python
from pxr import Ar
from usdCustom import PathmapResolver

# Get the resolver
resolver = Ar.GetUnderlyingResolver()

# Set the default pathmap environment
resolver.SetDefaultPathmapEnvironment("ANY_PATHMAP")
```

### Context-Based Pathmap Switching
Pathmap settings can be dynamically switched based on the context.

You can also specify a pathmap directly using the `CreateContextFromString` function,
where the string after the comma is treated as a pathmap.

```python
from pxr import Ar
from usdCustom import PathmapResolver

# Get the resolver
resolver = Ar.GetUnderlyingResolver()

# Specify the context and pathmap
resolver.CreateContextFromString("hoge:fuga,{'foo':'var'}")

"""
Search path: [
    hoge
    fuga
]
Pathmap dict: [
    foo : var
]
"""
```

### Debugging with PathmapResolver
You can use the following debug codes with TF_DEBUG:
 - AR_PATHMAPRESOLVER
 - AR_PATHMAPRESOLVER_CONTEXT


## Acknowledgements
The development of this plugin was greatly influenced by the [VFX-UsdAssetResolver](https://github.com/LucaScheller/VFX-UsdAssetResolver) page.
Special thanks to the developer, Luca Scheller.
