# GstMeta

Gstreamer Metadata is a tricky concept. The Gstreamer [documentation in here](https://gstreamer.freedesktop.org/documentation/application-development/advanced/metadata.html?gi-language=c) defines metadata as information adhered to a stream, and states there are two kinds of metadata: *tags* and *info*. It is also to be understood that *info* is somewhat private information that we should not touch, hence we should focus on tags instead for our user metadata. These are mentioned in the `GstMeta` documentation page too ([link here](https://gstreamer.freedesktop.org/documentation/gstreamer/gstmeta.html?gi-language=c)), which appears to be some kind of stream metadata API.

While hard to wrap your head around to, these may do the trick when moving data within a pipeline, but are useless towards streaming information to the outside world, for example through UDP or RTP.

- GstBuffer: An unit of GStreamer data which flows from one element in a pipeline to the next. It might be an image frame, or a portion of a frame. For example: a `video/x-h264` type buffer *may* carry a whole frame, but an `application/x-rtp` type buffer will surely carry only a portion of it: a single frame needs to be sent in many packets through the network.
- GstMeta: Some piece of information pertaining to a stream (not neccesarily a frame) which lives next to a GstBuffer but is clearly separated from the actual data it holds.


## ✔ gstmeta.c

This example writes a GstMeta field to a GstBuffer next to videotestsrc, which is then read at many points in the pipeline. The metadata strcucture is reused from that of [gst-darknet](https://github.com/aler9/gst-darknet). Meta can be read regardless of the transformations applied to the buffers (encoding, rtp-*ing*...): the meta is neither encoded nor payloaded. Meta cannot be read after UDP transmission (find a switch in the pipeline), as it lives only within the pipeline.

```
                 video         video           application                    video
┌──────────────┐ x-raw ┌─────┐ x-264 ┌───────┐ x-rtp    ┌─────────┐  ┌──────┐ x-raw
│ videotestsrc ├───────│xh264├──────►│rtp*pay├─────────►│rtp*depay├─►│decode├──────►
└──────────────┘▲      └─────┘|      └───────┘  |       └─────────┘  └──────┘ |
                |             ▼                 ▼                             ▼
              inject         read              read                          read
```

**Related reads**

- [Nicolas Dufresne confirms GstMeta is useless over the network](https://lists.freedesktop.org/archives/gstreamer-devel/2016-June/059135.html)
- [Default metadata template for GstMeta](https://stackoverflow.com/questions/40630098/there-is-a-standard-template-of-gstreamer-metadata-already-defined)