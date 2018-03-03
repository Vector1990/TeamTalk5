# TeamTalk 5

Repository for TeamTalk 5 development.

## Download TeamTalk 5 SDK

To build the TeamTalk client or server projects you must first download the
[TeamTalk 5 SDK](http://www.bearware.dk/?page_id=393) to obtain the client and server binaries.

* TeamTalk 5 SDK Standard Edition - **Beta** releases
  * [Windows 32-bit](http://bearware.dk/test/teamtalksdk/v5.3.1.4920/tt5sdk_v5.3.1.4920_win32.7z) **rev. 4920**
  * [Windows 64-bit](http://bearware.dk/test/teamtalksdk/v5.3.1.4920/tt5sdk_v5.3.1.4920_win64.7z) **rev. 4920**
  * [Mac 64-bit](http://bearware.dk/test/teamtalksdk/v5.3.1.4920/tt5sdk_v5.3.1.4920_macos_x86_64.7z) **rev. 4920**
  * [Debian 9 32-bit](http://bearware.dk/test/teamtalksdk/v5.3.1.4920/tt5sdk_v5.3.1.4920_debian9_i386.7z) **rev. 4920**
  * [Debian 9 64-bit](http://bearware.dk/test/teamtalksdk/v5.3.1.4920/tt5sdk_v5.3.1.4920_debian9_x86_64.7z) **rev. 4920**
  * [Raspberry Pi (armhf)](http://bearware.dk/test/teamtalksdk/v5.3.1.4920/tt5sdk_v5.3.1.4920_raspbian_armhf.7z) **rev. 4920**
  * [Android arm-v7](http://bearware.dk/test/teamtalksdk/v5.3.1.4920/tt5sdk_v5.3.1.4920_android_armv7a.7z)  **rev. 4920**
  * [iOS 7.0+ universal](http://bearware.dk/test/teamtalksdk/v5.3.1.4920/tt5sdk_v5.3.1.4920_ios_universal.7z)  **rev. 4920**
* TeamTalk 5 SDK Professional Edition - **Beta** releases
  * [Windows 32-bit](http://bearware.dk/test/teamtalksdk/v5.3.1.4920/tt5prosdk_v5.3.1.4920_win32.7z) **rev. 4920**
  * [Windows 64-bit](http://bearware.dk/test/teamtalksdk/v5.3.1.4920/tt5prosdk_v5.3.1.4920_win64.7z) **rev. 4920**
  * [Mac 64-bit](http://bearware.dk/test/teamtalksdk/v5.3.1.4920/tt5prosdk_v5.3.1.4920_macos_x86_64.7z) **rev. 4920**
  * [Debian 9 32-bit](http://bearware.dk/test/teamtalksdk/v5.3.1.4920/tt5prosdk_v5.3.1.4920_debian9_i386.7z) **rev. 4920**
  * [Debian 9 64-bit](http://bearware.dk/test/teamtalksdk/v5.3.1.4920/tt5prosdk_v5.3.1.4920_debian9_x86_64.7z) **rev. 4920**
  * [Raspberry Pi](http://bearware.dk/test/teamtalksdk/v5.3.1.4920/tt5prosdk_v5.3.1.4920_raspbian_armhf.7z) **rev. 4920**
  * [Android arm-v7](http://bearware.dk/test/teamtalksdk/v5.3.1.4920/tt5prosdk_v5.3.1.4920_android_armv7a.7z)  **rev. 4920**
  * [iOS 7.0+ universal](http://bearware.dk/test/teamtalksdk/v5.3.1.4920/tt5prosdk_v5.3.1.4920_ios_universal.7z)  **rev. 4920**

## TeamTalk 5 Libraries
Projects wrapping the client DLL file in the TeamTalk SDK.
* **TeamTalk_DLL**
  * TeamTalk 5 C-API DLL project 
  * C-API header files for TeamTalk 5 DLL
    * Preliminary API [Documentation](http://bearware.dk/test/teamtalksdk/v5.3.1.4920/docs/C-API/)
* **TeamTalk.NET** (dependency: **TeamTalk_DLL**)
  * TeamTalk 5 .NET DLL wrapper for C-API TeamTalk 5 DLL (**TeamTalk_DLL**)
    * Preliminary API [Documentation](http://bearware.dk/test/teamtalksdk/v5.3.1.4920/docs/NET/)
  * Requires DLL file from **TeamTalk_DLL** project, either 32-bit or 64-bit
* **TeamTalkJNI**
  * TeamTalk 5 JNI project with Java wrapper classes
    * Preliminary API [Documentation](http://bearware.dk/test/teamtalksdk/v5.3.1.4920/docs/Java/)
  * Import in Eclipse using [Android SDK](http://developer.android.com/sdk/index.html)
  * Requires ARM-v7a JNI shared object in sub-folder *TeamTalkJNI/libs/armeabi-v7a*
    * Based on Android API Level 16
  * The following features are currently *not* supported in the JNI API:
    * Video capture (webcam)
    * Media file streaming

## TeamTalk 5 Clients
Projects containing client applications which use the TeamTalk 5 client DLL.
* **qtTeamTalk** (dependency: **TeamTalk_DLL**)
  * TeamTalk 5 client application written in C++ and based on [Qt](http://www.qt.io)
  * Requires **TeamTalk_DLL** project for DLL dependency
* **TeamTalkClassic** (dependency: **TeamTalk_DLL**)
  * TeamTalk 5 accessible client application written in C++ and based on MFC
    * Works well with screen-readers
  * Requires [Tolk](https://github.com/dkager/tolk) project as dependency. Remove macro *ENABLE_TOLK* to disable Tolk.
    * Run ```git submodule init``` followed by ```git submodule update```
  * Requires [tinyxml](https://github.com/bear101/tinyxml) project as dependency.
    * Run ```git submodule init``` followed by ```git submodule update```
  * Requires **TeamTalk_DLL** project for DLL dependency
* **TeamTalkApp.NET** (dependency: **TeamTalk.NET**)
  * TeamTalk 5 .NET client application written in C#
  * Requires **TeamTalk.NET** project for DLL dependency
* **iTeamTalk** (dependency: **TeamTalk_DLL**, **FacebookSDK**)
  * TeamTalk 5 iOS client application written in Swift
  * Requires **TeamTalk_DLL** project for bridging header
  * Requires **[FacebookSDK](https://developers.facebook.com/docs/ios)**
  * Open project in [Xcode](https://developer.apple.com/xcode/)
* **TeamTalkAndroid** (dependency: **TeamTalkJNI**)
  * TeamTalk 5 Android client application written in Java
  * Open project in [Android Studio](https://developer.android.com/studio/intro/index.html)
    * ... or build using [ant](http://ant.apache.org), run the following command: ```android update project -p . -s -t android-17```
  * Copy the TeamTalk 5 Java library to *TeamTalkAndroid/libs* directory:
    * TeamTalk5.jar
      * Located in ```Library/TeamTalkJNI/libs``` folder of the TeamTalk 5 SDK for Android
    * TeamTalk5Test.jar (optional, only for unit-tests)
      * Located in ```Library/TeamTalkJNI/libs``` folder of the TeamTalk 5 SDK for Android
  * Copy the TeamTalk 5 JNI shared library to *TeamTalkAndroid/src/main/jniLibs/armeabi-v7a*
    * libTeamTalk5-jni.so
        * Located in ```Library/TeamTalkJNI/libs/armeabi-v7a``` (Standard Edition)
    * libTeamTalk5Pro-jni.so
        * Located in ```Library/TeamTalkJNI/libs/armeabi-v7a``` (Professional Edition)
* **ttphpadmin**
  * Console PHP-script for administrating a TeamTalk 5 server.
  * Run ```php ttphpadmin.php```

## TeamTalk 5 Servers
Sample applications for writing a TeamTalk 5 server are located in the Examples folder. Building a TeamTalk 5 server requires TeamTalk 5 Professional Edition.
* **TeamTalkServer**
  * TeamTalk 5 server application written in C++
  * Requires **TeamTalk_DLL** project for DLL dependency
* **TeamTalkServer.NET**
  * TeamTalk 5 server application written in C#
  * Requires **TeamTalk_DLL** and **TeamTalk.NET** projects for DLL dependencies
* **jTeamTalkServer**
  * TeamTalk 5 server application written in Java
  * Requires **TeamTalk_DLL** and **TeamTalkJNI** for DLL dependencies
