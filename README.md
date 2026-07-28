# gadget-libs

Collection of modules that can be run using [libpak runtime](https://github.com/petabyt/libpak/). These modules implement proprietary protocols that can communicate with gadgets such
as cameras, earbuds, dashcams, etc.

Rules for in tree projects:
- Code made from inspecting/decompiling is not allowed in this tree (but might be included through submodule)
- Only black box approach is allowed (and perhaps clean room)
- LLM/agent usage must be done through offline/local models
- No copyrighted blobs

Technique for black-box reversing on a stock Android phone:
- Screen recording of testing app functionality
- Extract btsnoop dump from bug report or /data/misc/bluetooth/logs/
- Align packet dump with screen recording
- Match packets with button presses in video
