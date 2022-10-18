# H264 SEI metadata

H264 SEI injection works by pushing GstBuffers of type `video/x-h264` carrying meta, which get embedded in the SEI region of a parallel h264 video stream. This method is most flexible since videos are usually compressed, however not always do we have access to RTP packets for other synchronous methods such as injection on RTP headers. 


## ✖ csdn-example

Failing. This sample uses an `appsrc` to push h264 buffers with meta onto a funnel, mixing them with h264 frames. This approach was tested as it was found [here  at the China Software Developer Network](https://blog-csdn-net.translate.goog/Cheers724?type=blog&_x_tr_sl=auto&_x_tr_tl=en&_x_tr_hl=es&_x_tr_pto=wapp) (this link will try to redirect you, just interrupt when the website starts loading again). Based on supporting [h264bitstream library repo](https://github.com/D-Y-Innovations/h264bitstream), clone the repo dir next to `main.c`.

```
cd csdn-example
make main
./main.o
```

```
                 video         video                      
┌──────────────┐ x-raw ┌─────┐ x-264 ┌──────┐    
│ videotestsrc ├───────│xh264├──────►│      │  ┌─────────┐  ┌──────────┐ UDP  ┌──────────┐  ┌──────────┐  ┌──────────┐
└──────────────┘ meta  └─────┘       │funnel├─►│h264parse├─►│rtph264pay├─ * ─►│rtph264pay├─►│ identity ├─►│ fakesink |
┌──────────────┐ x-h264              │      │  └─────────┘  └──────────┘      └──────────┘  └──────────┘  └──────────┘ 
│ appsrc       ├────────────────────►│      │                                                         │                                                                  
└──────────────┘                     └──────┘                                                         │              
      ▲    │                                                                                          │               
      └────┘                                                                                          ▼ 
  need-data signal                                                                              retrieve meta  
```

## Related reads

- [Complete example on h264 sei prelude](https://stackoverflow.com/questions/52364752/injecting-inserting-adding-h-264-sei)
- [Complete example on h264 sei main content](https://blog.csdn.net/Cheers724/article/details/99822937) (chinese, pass through google translate)
- [RidgeRun SEI basic theory plus plugin promotion](https://developer.ridgerun.com/wiki/index.php/GstSEIMetadata/GstSEIMetadata_Basics) 
- [Gstreamer documentation for H265 parsing utilities](https://gstreamer.freedesktop.org/documentation/gst-plugins-bad-codecparsers/gsth265parser.html?gi-language=c)
- [Gstreamer documentation for H264 parsing utilities](https://gstreamer.freedesktop.org/documentation/gst-plugins-bad-codecparsers/gsth264parser.html?gi-language=c)
- [H265 standard description](https://www.itu.int/rec/T-REC-H.265-202108-I/en)
- [Details on video/x-h264 alignment (mailing list)](https://gstreamer-devel.narkive.com/2i5BzQYy/what-is-the-alignment-capability-in-video-x-h264)
- [Network abstraction layer (NAL) at wikipedia](https://en.wikipedia.org/wiki/Network_Abstraction_Layer)