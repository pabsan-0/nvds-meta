# GST to GST metadata

This example built from scratch

- No deepstream dependency
- Attempts to add bytes to GstMemory then to GstBuffer carrying custom data
- To compile one must use rtp lib as `pkg-config --libs --cflags gstreamer-1.0 gstreamer-app-1.0 gstreamer-rtp-1.0`
- **Status**: working



```
┌──────────────┐
│ h264 image   │
│              │
└──────────────┘
       │ ◄──────────────── Injection (GstRTPBuffer)
       ▼
┌──────────────┐┌──────┐
│ RTP h264 pay ││ Meta │
│              ││      │
└──────────────┘└──────┘
       │ ────────────────► Retrieval
       ▼
┌──────────────┐
│ h264 image   │
│              │
└──────────────┘
```