This library encapsulates PVRTC lib + compression calls, it is made separately, so that EXE files can be smaller when PVRTC is not used.
For example if PVRTC lib is linked with the engine directly, then it ALWAYS increases the EXE size, no matter if it's used or not.

Currently PVRTC will crash on 64-bit if compiled with platform toolset 2015, so 2013 or older is used!
Once PVRTC lib is updated, remove the "VS 2015 Missing Symbols" folder