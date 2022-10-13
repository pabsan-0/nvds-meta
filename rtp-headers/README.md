# RTP header metadata

Injecting meta into RTP packets is one example of application-specific metadata handling.

The GstRTPBuffer is an abstraction on top of GstBuffer providing a special interface to handle RTP packets. See the [GST RTP Buffer documentation here](https://gstreamer.freedesktop.org/documentation/rtplib/gstrtpbuffer.html?gi-language=c#methods).

Custom metadata can be written to RTP buffers with either `gst_rtp_buffer_add_extension_onebyte_header` or `gst_rtp_buffer_add_extension_twobytes_header`. Because a single frame may be divided into many RTP packets, all of which will carry the encoded data, the receiving end must take care where to retrieve the data from. The last packet of an RTP burst can be detected with `gst_rtp_buffer_get_marker`, as it carries an special byte marking its end.

To compile these, one must use an additional gst-rtp lib: `pkg-config --libs --cflags gstreamer-1.0 gstreamer-rtp-1.0`.


## ✔ rtp-one-byte.c

A simple example using `gst_rtp_buffer_add_extension_onebyte_header` and `gst_rtp_buffer_get_marker` to send a message through udp.

```
make
./rtp-one-byte.c
```
```
                 video         video           application                      video
┌──────────────┐ x-raw ┌─────┐ x-264 ┌───────┐ x-rtp       ┌─────────┐  ┌──────┐ x-raw
│ videotestsrc ├───────│xh264├──────►│rtp*pay├────────────►│rtp*depay├─►│decode├──────►
└──────────────┘       └─────┘       └───────┘             └─────────┘  └──────┘
                                              ▲           │
                                              |           ▼
                                            inject     retrieve
```

## ✔ nvds-two-bytes.c

Nvidia Deepstream detections streaming through UDP. Uses `gst_rtp_buffer_add_extension_twobytes_header` to attach detections to every frame's last RTP packet.

NVDSMeta appears to be unreachable in the RTP buffers, so it is extracted first to GstMeta, then moved to RTP.

```
make -C nvds-two-bytes
./nvds-two-bytes/main  &
./nvds-two-bytes/receive.o
```
```
                      video         video           application                             video
┌─────────┐  ┌──────┐ x-raw ┌─────┐ x-264 ┌───────┐ x-rtp       ┌─────────┐  UDP   ┌──────┐ x-raw
│ v4l2src ├─►│ NVDS ├───────│xh264├──────►│rtp*pay├────────────►│rtp*depay├── * ──►│decode├──────►
└─────────┘  └──────┘       └─────┘       └───────┘             └─────────┘        └──────┘
                                    │ ▲               │ ▲                         │
                                    ▼ |               ▼ |                         ▼
                            NvdsMeta->GstMeta   GstMeta->rtp header        rtp header->stdout
```