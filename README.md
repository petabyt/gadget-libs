# gadget-libs

Collection of modules that can be run using [libpak runtime](https://github.com/petabyt/libpak/). These modules implement proprietary protocols that can communicate with gadgets such
as cameras, earbuds, dashcams, etc.

My personal rules for reverse-engineering:
- Code made from inspecting/decompiling is not allowed in this tree (but might be included through submodule)
- Black box approach is preferred (and clean room only if absolutely necessary)
- Zero data retention/training policy for LLMs
- No copyrighted blobs or packet dumps uploaded

Technique for black-box reversing on a stock Android phone:
- Screen recording of testing app functionality
- Extract btsnoop dump from bug report or /data/misc/bluetooth/logs/
- Align packet dump with screen recording
- Match packets with button presses in video
