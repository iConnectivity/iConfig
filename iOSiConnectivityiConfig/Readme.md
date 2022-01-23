## Notes for iConfig

The original open sourced project came without project file or build instructions.
I created a new project and figured out how to add the files needed to build iConfig for IOS. Before doing this here I tried different approaches "offline". Since I am using the newest XCode 13.2.1 the templates suggested the usage of a scene delegate. By targeting IOS 12.4 and tweaking the project I got rid of it and of some deprecation warnings. 
### Prerequisites
GeneSysLib is the library that seems to do some of the heavy lifting for the communication and has a dependency to boost. I installed boost version 1.76 using homebrew. This is a dependency found in the project.
- XCode 13.2.1
- boost 1.76.0 as served by brew
### Changes
I had to add additional imports/includes of UIKit. This seems caused by the newer XCode. Also some of the boost functionality was tr1 when it was originally added. I changed it to use the released (removing tr1) or using the now available std (unordered_map). 
### Observations
The problem I am encountering is that the device is not found sometimes at all or not sufficiently - It appears but only with the Product ID. This is the same as what I experienced with the released version. 
Right now it seems that there are certain issues with how the timers for sending and receiving interact. 
The software sends out GetDevice which is answered by the device with RetDevice (see also "[Common System Exclusive Commands.pdf]( https://github.com/mabu4ch/iConfig/blob/master/Docs/Common%20System%20Exclusive%20Commands.pdf) in the Docs folder). After that it seems that the following requests are not being sent or received correctly by the software. Maybe the timer handling is not yet correct in every case. See ICDeviceSelectionViewController and Communicator (in GeneSysLib).
### Changes
The main change was to remove the possibility of having several threads with timers on iOS. I simplified the Midi Sysex sending and let the asynchronous part be handled by the operating system. Like this I was able to remove the offending timers and locking and make everything simpler. To better understand the code I changed the formatting on certain files which makes them look less uniform.
### Beta test
I do have a version of the software which works for me on an IPad Air 2 (iOS 12.4) and a more recent IPad Pro (running the newest iOS). I renamed it to reConfig and changed the icons (not the nicest ones) and some of the colors. The app is available for beta testing. Drop me message with your E-Mail (I guess it needs to be your AppleId) if you want to be on the beta testers list.


