# KLV injection

KLV injection works by pushing GstBuffers of type `meta/x-klv` into the pipeline. These buffers need not be in `klv` format, but just be a bunch of bytes that the user can encode/decode at will.

These buffers are then muxed into a transport stream (ts) with the `mpegtsmux` element along with video frames. Muxed transport streams can be sent over the network without the need for rtp packetization. Ideally, the inputs to this element would be synchronous, however this is not the case on this gst-plugins-bad implementation. In practice, they are muxed in series, consecutively. The receiving end to the muxer is `tsdemux`. Against all logic and due to the sync-async problem, `tsdemux` is not a 1-to-N but a 1-to-1 element.

`mpegtsmux` and `tsdemux` work by what's called program IDs:

- The `mpegtsmux` element is an N-to-1 muxer.
- It possesses a program id table  at the property `prog-map` which will link each pad to a program number
- At `tsdemux` a single program ID can be set at the property `program-number`, meaninig you need a demuxer per program. yay!

More on PIDs here: [Setting PIDs for mpegtsmux](https://lists.freedesktop.org/archives/gstreamer-devel/2015-May/052946.html)

RidgeRun's GST-Metadata (paid) software bundle provides some modifications to gstreamer itself plus a few plugins to approach synchronous data + metadata transmission. The examples in here do not use such software and approach this problem from a default gstreamer install.


**TS program table snippets:**
```
# C-script
GstElement *mux;
GstStructure *pm;
mux = gst_element_factory_make ("mpegtsmux", NULL);
pm = gst_structure_new_empty ("program_map");
gst_structure_set (pm, "sink_300", G_TYPE_INT, 3, "sink_301", G_TYPE_INT, 4, NULL);
g_object_set (mux, "prog-map", pm, NULL);
```
```
# gst-launch approach
gst-launch-1.0 -v videotestsrc name=vsrc1 is-live=true do-timestamp=true
! x264enc ! queue ! h264parse ! muxer.sink_300 mpegtsmux name=muxer
prog-map=program_map,sink_300=10,sink_301=11 ! queue ! tcpserversink
host=0.0.0.0 sync-method=2 recover-policy=keyframe port=5000  videotestsrc
name=vsrc2 is-live=true do-timestamp=true ! x264enc ! queue ! h264parse !
muxer.sink_301
```


## ✔ continuous.c

This sample uses an `appsrc` continuously pushing metadata buffers containing a simple string, via the `need-data` signal. Such injection is manually throttled to match the videotestsrc framerate by a period-awaiting delay.

```
make continuous
./continuous.o
```
```
                 video         video                           video         video
┌──────────────┐ x-raw ┌─────┐ x-264 ┌─────┐  ┌───┐  ┌───────┐ x-264 ┌──────┐ x-raw
│ videotestsrc ├───────│xh264├──────►│tsmux│  │tee├─►│tsdemux│───────│decode├──────►
└──────────────┘ meta  └─────┘       │     ├─►│   │  └───────┘ meta  └──────┘
┌──────────────┐ x-klv               │     │  │   │  ┌───────┐ x-klv
│ appsrc       ├────────────────────►│     │  │   ├─►│tsdemux│─────────────────────►
└──────────────┘                     └─────┘  └───┘  └───────┘
      ▲    │
      └────┘
  need-data signal
```


## ✖ on-request.c

Failing. This sample uses an `appsrc` to push metadata buffers containing a simple string on demand. Every time a frame is emmited from `videotestsrc`, a buffer is manually pushed out of `appsrc` from a GstPad probe.

```
make on-request
./on-request.o
```

```
                 video         video                          video          video
┌──────────────┐ x-raw ┌─────┐ x-264 ┌─────┐  ┌───┐  ┌───────┐x-264 ┌──────┐ x-raw
│ videotestsrc ├───────│xh264├──────►│tsmux│  |tee├─►│tsdemux│──────│decode├──────►
└──────────────┘│      └─────┘       │     |  │   |  └───────┘      └──────┘
             ┌──┘                    │     │  │   │
 buf request │                       │     ├─►│   |
             ▼   meta                │     │  │   │           meta
┌──────────────┐ x-klv               │     │  │   │  ┌───────┐x-klv
│ appsrc       ├────────────────────►│     │  │   ├─►|tsdemux│────────────────────►
└──────────────┘                     └─────┘  └───┘  └───────┘

```



## Related reads

- [Eslam provides top value intel on metadata approaches](https://lists.freedesktop.org/archives/gstreamer-devel/2021-September/079056.html)
- [Static KLV muxing sample](https://gstreamer-devel.narkive.com/GlIqaK1k/example-code-for-muxing-klv-meta-x-klv-with-mpegtsmux-plugins-bad-and-gstreamer-1-8-3)
- [Notes behind MPEG and TS - main page](https://www.impleotv.com/content/klvstreaminjector/help/index.html)
- [Notes behind MPEG and TS - STANAG 4609](https://www.impleotv.com/content/klvstreaminjector/help/KLV/stanag4609.html)
- [Setting PIDs for mpegtsmux](https://lists.freedesktop.org/archives/gstreamer-devel/2015-May/052946.html)