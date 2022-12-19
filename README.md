# AutoBright

Display utility that automatically adjusts screen brightness based on the luminance of the content currently displayed on-screen. AutoBright raises/lowers brightness if the screen is mostly dark/light (respectively) to maintain an optimal viewing brightness and reduce eye strain. 

Built using Windows32 API and OpenCV.

## Implementation Details
AutoBright uses Win32 methods to captures the current screen contents as a bitmap and saves them as a JPG image, which is then parsed using OpenCV to get the RGB values for each pixel. These values are then searched using a hashtable (implemented with a custom hashing function) to find the most frequently occuring colour in linear time. This RGB colour is converted to a [YUV](https://en.wikipedia.org/wiki/YUV) colour so it's luminance can be compared to a cutoff to determine whether to raise or lower screen brightness using Win32 system calls. 

This process runs in a loop to actively monitor changes in screen content and adjust brightness accordingly. 
