## Notes for iConfig4Audio

The original open sourced project came without project file or build instructions.
I created a new project and figured out how to add the files needed to build iConfig4Audio for IOS. This is based on the work I've done for iConfig (called reConfig). The communication uses the same shared library GeneSysLib.
### Prerequisites
GeneSysLib is the library that seems to do some of the heavy lifting for the communication and has a dependency to boost. I installed boost version 1.76 using homebrew. This is a dependency found in the project.
- XCode 13.2.1
- boost 1.76.0 as served by brew
### Changes
Again - the same applies as for iConfgi/reConfig:
I had to add additional imports/includes of UIKit. This seems caused by the newer XCode. Also some of the boost functionality was tr1 when it was originally added. I changed it to use the released (removing tr1) or using the now available std (unordered_map). 
### Observations
### Changes
The main change was to remove the possibility of having several threads with timers on iOS. I simplified the Midi Sysex sending and let the asynchronous part be handled by the operating system. Like this I was able to remove the offending timers and locking and make everything simpler. To better understand the code I changed the formatting on certain files which makes them look less uniform.
### Beta test
I do have a version of the software which works for me on an IPad Air 2 (iOS 12.4) and a more recent IPad Pro (running the newest iOS). I renamed it to reConfig4Audio and changed the icons (not the nicest ones).


