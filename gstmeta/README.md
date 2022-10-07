# GstMeta

Gstreamer Metadata is a tricky concept. The Gstreamer [documentation in here](https://gstreamer.freedesktop.org/documentation/application-development/advanced/metadata.html?gi-language=c) defines metadata as information adhered to a stream, and states there are two kinds of metadata: *tags* and *info*. It is also to be understood that *info* is somewhat private information that we should not touch, hence we should focus on tags instead for our user metadata. These are mentioned in the `GstMeta` documentation page too ([link here](https://gstreamer.freedesktop.org/documentation/gstreamer/gstmeta.html?gi-language=c)), which appears to be some kind of stream metadata API. While hard to wrap your head around to, these may do the trick when moving data within a pipeline, but are useless towards streaming information to the outside world, for example through UDP or RTP. The following two concetps are relevant:

- GstBuffer: An unit of GStreamer data which flows from one element in a pipeline to the next. It might be an image frame, or a portion of a frame. For example: a `video/x-h264` type buffer *may* carry a whole frame, but an `application/x-rtp` type buffer will surely carry only a portion of it: a single frame needs to be sent in many packets through the network.
- GstMeta: Some piece of information pertaining to a stream (not neccesarily a frame) which lives inside a GstBuffer but is clearly separated from the actual data it holds.


## gstmeta.c

- This example piggybacks on a metadata API structure from (deprecated) [gst-darknet](https://github.com/aler9/gst-darknet).
- What's going on:
    - A text string is written in one of the fields defined in such structure.
    - The stream then gets h264 encoded and decoded.
    - Optionally, the stream is sent and retrieved from UDP
- What has been observed:
    - Read after compression works through h264 or h265:
    - Sending over UDP does NOT work
        - Only the data is converted in the pipelilne
        - The meta does not get payloaded (or encoded, for that matter)
        - This means the meta does not leave the pipeline, hence cannot be reached from the other side

**Related reads**

https://lists.freedesktop.org/archives/gstreamer-devel/2021-September/079056.html

https://stackoverflow.com/questions/40630098/there-is-a-standard-template-of-gstreamer-metadata-already-defined
