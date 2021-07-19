# CAMERA\_SAMPLE\_APP<a name="EN-US_TOPIC_0000001080462608"></a>

-   [Introduction](#section11660541593)
-   [Directory Structure](#section176641621345)
-   [Usage](#section1648194512427)
-   [Constraints](#section722512541395)
-   [Repositories Involved](#section16511040154318)

## Introduction<a name="section11660541593"></a>

This repository provides media-related applications, such as the home screen and settings.

**Figure  1**  Position of this repository \(in the dotted blue box\) in the multimedia subsystem<a name="fig189881143114217"></a>  
![](figures/position-of-this-repository-(in-the-dotted-blue-box)-in-the-multimedia-subsystem.png "position-of-this-repository-(in-the-dotted-blue-box)-in-the-multimedia-subsystem")

## Directory Structure<a name="section176641621345"></a>

```
applications/sample/camera
├── cameraApp   # Camera app
├── gallery     # Photo gallery
├── launcher    # Home screen
├── media       # Apps of photographing, video recording, and audio and video playback
└── setting     # Settings
```

## Usage<a name="section1648194512427"></a>

Build a single repository in the root directory.

```
# Select the development board.
hb set  
# Build this repository.
hb build camera_lite
```

## Constraints<a name="section722512541395"></a>

C++ 11 or later

## Repositories Involved<a name="section16511040154318"></a>

/hmf/multimedia/camera\_lite

/hmf/multimedia/audio\_lite

/hmf/multimedia/media\_lite

/hmf/multimedia/utils\_lite

/hmf/multimedia/service\_lite

