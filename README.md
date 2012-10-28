# Symella 1.42

<http://symella.aut.bme.hu>

<https://github.com/kenley/SymTorrent>

## 1. General info

Symella is a basic Gnutella client for Symbian OS. It is considered as the first Gnutella client released for mobile devices. Symella is not actively developed any more but it is still functional (as of 2012 november).

Symella is capable of searching and downloading, but do not upload any data. It supports multi-threaded downloads which means that if multiple users have the same file, Symella can download this file from several locations simultaneously.

The number of search results returned highly depend on the quality of the peers you are connected to. In the current version there isn't any kind of optimization for choosing which peers the client tries to connect. Searching for the same text after restarting the program can result in different number of hits. 

Symella 1.43 is compatible with any Nokia device running S60 5th, Symbian^3, Symbian Anna, Symbian Belle or more recent versions of Symbian.

## 2. Using Symella

The application has three so-called "views". The first one is the Connection View which lists the active Gnutella connections along with their statuses. The second view is the Search View, which lists the current search results (query hits returned for the last search request). The last view displays the download queue.

In order to be able to search for files you must be connected to at least a couple of other peers. The Connections View is for displaying information about the active Gnutella connections. You can start searching if you can see 2 or 3 connections with an "Ok" status. 

The main menu options are as follows:

- Connect: Selecting this menu item starts the connection procedure. You have to be connected to at least one peer to be able to start searching (at least one connection must have an "OK" status).

- Disconnect: Disconnects from all servants (this doesn't effect active downloads).

- Search: Sends a new search request. Previous search results are discarded.

- Settings: Opens the settings dialog.

- Add peer: It can be used to manually add a specific peer to the connection pool

- Reset cache: Deletes all entries from the host cache. This action forces the application to look for new servant addresses (connects to GWebCache).

- About: Opens the about dialog (contains version number).

## Building Symella

Symella is written in Symbian C++. To build it, the following tools are needed:

- Symbian^3 SDK 1.0 (theoretically it can be built using S60 5th SDK or any newer SDK, but I have tested this version with Symbian^3 1.0 only)
- Carbide C++ 3.2 (or more recent version)

After installing Carbide, you might have to manually add the SDK to it. To do so, go to Windows/Preferences/Carbide C++/Symbian SDKs, click "Add" and enter "Nokia_Symbian3_SDK_v1.0:com.nokia.symbian" as the SDK ID and "C:\Nokia\devices\Nokia_Symbian3_SDK_v1.0\" (or where you have installed the SDK) as the SDK location.

To import a project into Carbide, select File/Import/Symbian OS/Symbian OS Bld.inf file, pointing to the group/bld.inf.

When building the project, you may get a couple of extra qualification errors on a set of SDK files. This is a bug in the SDK, and can be easily fixed by editing each affected SDK header file and removing the extra qualifiers (e.g. CAknCcpuSupport::) from the method declarations.

To create an installable SIS file, go to Project/Properties/Carbide C++/Build Configurations/SIS Builder, click Add and select Symella.pkg. Self-signing the SIS is enough.

The project files arranged in the following structure:

    src\engine      - source files for the engine (UI independent) part of the project
    src\series60ui  - source files for the ui (the standard application framework classes)
    inc\engine      - include files for the engine
    inc\series60ui  - include files for the s60 ui dependent files
    data            - resource definitions included here
    group           - the standard project files
    gfx             - bitmaps, icons, etc.
    sis             - contains the package file for buildin SIS

Source files (and classes) of the engine are prefixed with the "SA" tag. The names of the UI dependent classes begin with "Symella".

## Configuration Files

The program creates 3 configuration files:

    gwebcaches.cfg  - stores a list of predefined Gnutella Web Cache addresses, all addresses must end with '/'
    hostcache.cfg   - stores a list of working servant addresses after the application exits
    settings.cfg    - general settings (download directory, maximum number of simultaneous downloads, etc.)

Theoretically these files can be edited with a text editor, but it isn't recommended. You can change most settings through the "Settings dialog" in the application.

## Version History

### 1.42
- Project updated to Symbian^3 SDK 1.0
- Fixed the download folder selection dialog so that now all drives are displayed

### 1.41
- Installer certificate update (should be valid for 10 years)
- Added support for dynamic screen size/layout change (rotating the screen while the program is running works)
- List of default GWebCaches has been updated to more recent 
- Briefly tested on S60 5th (Symbian^1) and seems to work well

### 1.40 
- Added access point selection setting (the default network connection can be set through the settings).
- Improved GWebCache support, the webcache addresses can be edited through the settings. Symella also queries webcaches for        addresses of other caches.
- More details are given in the connections view during the webcache connection 
- Fixed a bug which caused a crash when the user tried to download an already existing file.

### 1.32 
- S60 2nd edition is officially supported again.
- Added search field to the Search result view (the list of results can now be filtered based on a given 
- T9 has been enabled in the "Search for" dialog.
- The list of default GWebCache addresses has been extended and the setting for changing the list has been removed.

### 1.30 
- Ported to S60 3rd edition (S60 2nd edition is not supported in current version). 
- Tremendous number of bugs have been discovered and fixed. 
- Some UI tweaks (removed unnecessary menu items, renamed some of titles and we also have a new scalable application icon now, thank you Dorka! :). 

### 1.02 
- Modified the engine to be compatible with earlier (Symbian 6.1) phones (have to remove EscapeUtils::EscapeEncodeL and write an own url encoder).
- Now it compiles with Series 60 SDK 1.2 too.
- Before creating the settings file, the program checks for the default download directory (E:\Sounds or C:\Download) and makes it if it doesn't exist.


### 1.01 
- Modified the observer class MSASearchResultsObserver to allow the S80 port to display the file sizes in the search results list. Added some comments and created this file.
- We have some new application icons too (thx Hugo!)

### 1.0  
- Initial release


## Known issues

- For some unknown reason, the phone's built-in audio player fails to start playing downloaded audio files if they are started from Symella (if you open them manually then there is no problem).

- Sometimes a search query returns no hits. Doing the search once more usually solves the problem.

## Credits

### Developers
- Imre Kelényi (imre.kelenyi@aut.bme.hu) main engine, UI
- Bertalan Forstner: lots of work on the S60 3rd edition port, bugfixes, sempeer extension
- Balázs Molnár: part of the engine and UI of the initial release	
### Special Thanks
- Dorottya Takács for the nice new scalable application icon!
- Hugo for the S60 2nd edition application icons!
- Some of the gui icons (Series 80) were downloaded from http://www.treepad.com/free_icons/
