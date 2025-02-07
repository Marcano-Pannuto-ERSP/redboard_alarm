# Application to catch GPIO interrupts, using `asimple`

This is an application using the asimple and forked AmbiqSuiteSDK
projects to print a message when a GPIO interrupt occurs. This is intended 
to be used with an Ambiq AM1815 RTC configured to output alarms to its 
FOUT/nIRQ pin. That pin should be connected to the Redboard's pin 23.
There also needs to be a pull up resistor connected to FOUT since it is an
open drain pin. We used a 10k ohm resistor connected to VCC on the RTC.

## Dependencies
 - https://github.com/gemarcano/AmbiqSuiteSDK
 - https://github.com/gemarcano/asimple

In order for the libraries to be found, `pkgconf` must know where they are. The
special meson cross-file property `sys_root` is used for this, and the
`artemis` cross-file already has a shortcut for it-- it just needs a
variable to be overriden. To override a cross-file constant, you only need to
provide a second cross-file with that variable overriden. For example:

Contents of `my_cross`:
```
[constants]
prefix = '/home/gabriel/.local/redboard'
```

# Compiling and installing
```
mkdir build
cd build
# The `artemis` cross-file is assumed to be installed per recommendations from
# the `asimple` repository
meson setup --prefix [prefix-where-sdk-installed] --cross-file artemis --cross-file ../my_cross --buildtype release
meson install
```

# License

See the license file for details. In summary, this project is licensed
Apache-2.0, except for the bits copied from the Ambiq SDK.
