# Pavilion

This repo is the main development repo for the Unreal-based robot simulator Pavilion.

# Introduction

This is a total recreation of Gazebo in Unreal Engine 4. This work is different with existing UE4-based simulators in that it employs a tightly-coupled approach with ROS and libsdformat. This eliminates the need for intermediate compatibility layers.

Moreover, the model import process is done at runtime, which allows the simulator to be distributed without the Unreal Editor, a component not allowed to be redistributed by the UE4 License.

# Features

 - RGB-D Camera, LIDAR Sensor Emulation
 - Direct import URDF and SDF models
 - Real-time interop with ROS
 - Real-time Autonomous Driving Demo

# How to Build

First, clone this repo.

Then go to `Plugins/PavilionPlugin` and run `./BuildThirdParty.sh`.

Finally open `Pavilion.uproject` with the Unreal Editor.

Please note that to enable real-time optical flow output, you need to have a version of UE4 patched with [my patch](https://github.com/ProfFan/UnrealOpticalFlowDemo).

# Supported Versions

Unreal Engine version > 4.18

# License

MIT
