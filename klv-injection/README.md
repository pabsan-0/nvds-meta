# GST to GST metadata

So mpegtsmux and tsdemux work by what's called program IDs.
- At mpegtsmux you can set a programID table which will link each pad to a "program"
- At tsdemux you can set a single program ID, meaninig you need a demuxer per program. yay!

See more here: https://lists.freedesktop.org/archives/gstreamer-devel/2015-May/052946.html


## Continuous

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


## On-request [FAILING]

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

https://lists.freedesktop.org/archives/gstreamer-devel/2021-September/079056.html

https://stackoverflow.com/questions/40630098/there-is-a-standard-template-of-gstreamer-metadata-already-defined

https://gstreamer-devel.narkive.com/GlIqaK1k/example-code-for-muxing-klv-meta-x-klv-with-mpegtsmux-plugins-bad-and-gstreamer-1-8-3

https://www.impleotv.com/content/klvstreaminjector/help/index.html