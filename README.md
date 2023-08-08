![ARTful_logo1](https://github.com/LaurieWired/ARTful/assets/123765654/9fabd5fe-3f4c-46df-a140-963acaece7b8)

[![License](http://img.shields.io/:license-apache-blue.svg)](http://www.apache.org/licenses/LICENSE-2.0.html)
[![GitHub release (latest by date)](https://img.shields.io/github/v/release/LaurieWired/ARTful)](https://github.com/LaurieWired/JADXecute/ARTful)
[![GitHub stars](https://img.shields.io/github/stars/LaurieWired/ARTful)](https://github.com/LaurieWired/JADXecute/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/LaurieWired/ARTful)](https://github.com/LaurieWired/ARTful/network/members)
[![GitHub contributors](https://img.shields.io/github/contributors/LaurieWired/ARTful)](https://github.com/LaurieWired/ARTful/graphs/contributors)
[![Follow @lauriewired](https://img.shields.io/twitter/follow/lauriewired?style=social)](https://twitter.com/lauriewired)

# Description
ARTful is a native Android library the allows developers to modify the Android Runtime (ART) on Android13. With this tool, you can dynamically change the implementation of any static method within your application or the Android Framework to affect methods called from within your application. This eliminates the requirement of using plaintext references to Android ClassLoaders to execute unexpected code and thwarts Reverse Engineering by entirely removing method cross-references.

https://github.com/LaurieWired/ARTful/assets/123765654/7e946bcc-c2bc-4437-9bf9-3ce14103c932

# Installation
This code is based on the latest release of JADX version 1.4.6. It contains an additional plugin to enable dynamic Java coding using all of the standard Java libraries as well as the JADX libs and APIs.

You can import ARTful as a precompiled native binary or build from the example ARTful application to start your runtime manipulation. Follow the guides in the wiki if you need help importing your libraries.

# Usage

### Check out the **[Wiki](https://github.com/LaurieWired/ARTful/wiki/Usage)** for usage instructions and script examples.


# Contribute
- Make a pull request
- Add an Example Snippet to our Wiki
- Report an error/issue
- Suggest an improvement
- Share with others or give a star!

Your contributions are greatly appreciated and will help make ARTful an even more powerful tool.

# Screenshots

### Invoking ARTful
![artful_replace](https://github.com/LaurieWired/ARTful/assets/123765654/b4d10196-b264-4adb-bda4-7859ee450e0c)

### Replacing log.e() with Custom Code
![log_e_replace](https://github.com/LaurieWired/ARTful/assets/123765654/0e270355-229a-4ee2-8582-805027fef7fc)

