# NVDS-Meta

This repo holds several examples on metadata **handling** and **streaming**, pertaining to both GStreamer and Nvidia Deepstream pipelines.

These files have two purposes:

- To showcase how metadata can be handled and serve as boilerplate code for further developments
- To prompt the development over the baselines posted here, as not all leads for efficient metadata processing have been exhausted

Metadata handling requires some tools unreachable via `gst-launch`, hence some prior knowledge in C's GLib, GObject and GStreamer will be most useful as all samples are written in C.


<!-- vim-markdown-toc GFM -->

* [Setup](#setup)
* [Repository contents](#repository-contents)
* [Compatibility table](#compatibility-table)
* [Convenient extra tools](#convenient-extra-tools)

<!-- vim-markdown-toc -->

## Setup

This repo was built on this setup:

- Ubuntu 20.04
- GStreamer 1.16.3

Extra dependencies for only some samples (will be indicated):

- Nvidia Deepstream (`nvinfer` prepared beforehand)

All of the code in here should run out of the box on [NVDS-Lite](https://bitbucket.org/fadacatec-ondemand/nvds-lite/src/master/) environment (adding `--ipc=host` to docker run launcher might be needed to run some examples).


## Repository contents

This repo showcases the following approaches to manage metadata, those marked ✔ carrying at least one working sample in this repo:

- ✔ GstMeta: the "canonical" approach to metadata, using GstMeta objects attached to buffers. **GstMeta can't send meta over the network**
- ✔ KLV injection: consists on creating buffers carrying metadata as if they were data, and mux them with actual video in a Transport Stream that gets sent through the network.
- ✔ RTP headers: appends disposable headers to RTP packets right before take-off, which can be retrieved at the other end.
- ✖ h264 SEI: some media types allow users to embed data inside their containers. For example the SEI field in h264/h265 streams.
- ✖ Custom media types: this approach requires building coding/decoding tools capable of appending/retrieving extra bytes to encoded media formats, which would carry the per-frame meta.

Each of these approaches has its own subdirectory. In there, you will find some samples, as well as some extra information of interest. Along with the samples, we pack a `.dump` folder to carry non-working experiments for reference. Feel free to contribute pushing these use cases!


**101 reads for network metadata**

- [Nicolas Dufresne confirms GstMeta is useless over the network](https://lists.freedesktop.org/archives/gstreamer-devel/2016-June/059135.html)
- [Eslam tells you his approach for muxing video and meta](https://stackoverflow.com/questions/68098185/add-stream-meta-to-a-stream-via-gstreamer)
- [Eslam provides top value intel on metadata approaches](https://lists.freedesktop.org/archives/gstreamer-devel/2021-September/079056.html)
- [RidgeRun's surface theory on klv injection plus plugin promotion](https://developer.ridgerun.com/wiki/index.php/GStreamer_and_in-band_metadata)


## Compatibility table

Below, a compatibility table of each approach with a few different network communication sinks. These are not written in stone, feel free to try and make options marked ✖ work. Unmarked fields stand for untested use cases.

|                |  udpsink  | rtspclientsink  |  srtsink  | Details                                      |
|----------------|-----------|-----------------|-----------|----------------------------------------------|
| klv injection  |     ✔     |                 |           | Not really synchronous w/o RigdeRun's plugin |
| rtp headers    |     ✔     |        ✖        |     ✖     | Requires access to rtp buffers               |
| h264 sei       |           |                 |           |                                              |



## Convenient extra tools

- `wireshark`: Allows you to oversee udp communications and see their contents.
    - When your meta is not encoded, you might see it here!
    - Quickstart: Select loopback (lo) and then user the filter `udp.port == 1234`.
- `seergdb`: Basically the same as the above, only with C variables.
