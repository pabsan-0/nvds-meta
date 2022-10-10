#include <gst/gst.h>
#include <gst/gstmeta.h>
#include <gst/app/gstappsrc.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

static GMainLoop *loop;


GstPadProbeReturn
probe_read_any (GstPad * pad, GstPadProbeInfo * info, gpointer u_data)
{
    printf("%s: Buffer passed!\n", (char*) u_data);
    return GST_PAD_PROBE_OK;
}

GstPadProbeReturn
probe_read_data (GstPad * pad, GstPadProbeInfo * info, gpointer u_data)
{
    // GstBuffer *buf = (GstBuffer *) info->data;
    printf("%s: Buffer passed!\n", (char*) u_data);
    // GST_BUFFER_PTS(buf) = GST_CLOCK_TIME_NONE;
    return GST_PAD_PROBE_OK;
}


GstPadProbeReturn
probe_read_meta (GstPad * pad, GstPadProbeInfo * info, gpointer u_data)
{
    GstBuffer *buf = (GstBuffer *) info->data;

    GstMapInfo map;
    gst_buffer_map(buf, &map, GST_MAP_READ);

    g_print("%s: %s \n", (char*) u_data, map.data);
    return GST_PAD_PROBE_OK;
}


GstPadProbeReturn
probe_klv_request (GstPad *pad, GstPadProbeInfo *info, gpointer u_data)
{
    GstBuffer *buffer_data = (GstBuffer *) info->data;
    GstElement *appsrc = u_data;


    // Fake a string as klv and allocate a buffer to place it
    char* message = "hello gstreamer";
    guint8 message_size = 16;
    GstBuffer *buffer_meta = gst_buffer_new_allocate(NULL, 16, NULL);
    gst_buffer_fill(buffer_meta, 0, message, message_size);

    // New buffer has same timings as data buffer (sync)
    GST_BUFFER_PTS(buffer_meta) = GST_BUFFER_PTS(buffer_data);
    GST_BUFFER_DTS(buffer_meta) = GST_BUFFER_DTS(buffer_data);
    GST_BUFFER_DURATION(buffer_meta) = GST_BUFFER_DURATION(buffer_data);
    GST_BUFFER_TIMESTAMP(buffer_meta) = GST_BUFFER_TIMESTAMP(buffer_data);

    printf("videotestsrc_src probe: a meta buffer should have been injected....\n");
    GstFlowReturn ret = gst_app_src_push_buffer((GstAppSrc *)appsrc, buffer_meta);

    if (ret != GST_FLOW_OK)
    {
        printf("Whooops!!!!\n");
        g_printerr("Flow error");
        g_main_loop_quit(loop);
    }

    return GST_PAD_PROBE_PASS;
}


gint
place_probe (GstElement *pipeline, gchar *elementName, GstPadProbeCallback cb_probe)
{
    GstElement* id;
    GstPad* id_src;

    id = gst_bin_get_by_name (GST_BIN (pipeline), elementName);
    id_src = gst_element_get_static_pad (id, "src");
    gst_pad_add_probe (id_src, GST_PAD_PROBE_TYPE_BUFFER,
                cb_probe, elementName, NULL);
    gst_object_unref(id_src);
    gst_object_unref(id);
    return 0;
}


gint
main (gint argc, gchar *argv[])
{
    GstElement *pipeline;
	GError *error = NULL;

    gst_init (&argc, &argv);
    loop = g_main_loop_new (NULL, FALSE);

    // define pipeline
    const gchar *desc_templ =
        " videotestsrc name=src ! video/x-raw, framerate=25/1, is-live=true, do-timestamp=true"
        "     ! x264enc tune=zerolatency "
        "     ! video/x-h264, stream-format=byte-stream "
        "     ! queue "
        "     ! identity name=injector "
        "     ! identity name=data_0 "
        "     ! mux. "
        " appsrc name=appsrc0 is-live=true"
        "     ! meta/x-klv, parsed=true, sparse=false, is-live=true, do-timestamp=true "
        "     ! queue "
        "     ! identity name=meta_0 "
        "     ! mux. "
        " mpegtsmux name=mux alignment=7 "
        //" ! rtpmp2tpay "
#if 0
        " ! udpsink host=127.0.0.1 port=1234 async=false sync=false "
        " udpsrc port=1234  "

        // " ! application/x-rtp "
#endif
        //" ! rtpjitterbuffer "
        //" ! rtpmp2tdepay "
        " ! video/mpegts, systemstream=true "
        " ! identity name=mpeg "
        " ! tee name=teee "
        " teee. "
        "     ! queue " // leaky=2 max-size-buffers=20 max-size-time=0 max-size-bytes=0 "
        "     ! tsdemux "
        "     ! video/x-h264, stream-format=byte-stream "
        "     ! identity name=data_1 "
        "     ! h264parse config-interval=-1 "
        "     ! avdec_h264  "
        "     ! videoconvert "
        "     ! autovideosink "
        " teee. "
        "     ! queue " // leaky=2 max-size-buffers=20 max-size-time=0 max-size-bytes=0 "
        "     ! tsdemux "
        "     ! meta/x-klv, parsed=true, sparse=false, is-live=true "
        "     ! identity name=meta_1 "
        "     ! identity name=meta_2 "
        "     ! fakesink ";
        ;;;;;

	// Launch the pipeline
	gchar *desc = g_strdup (desc_templ);
	pipeline = gst_parse_launch (desc, &error);
	if (error) {
		g_printerr ("pipeline parsing error: %s\n", error->message);
		g_error_free (error);
		return 1;
	}

    // Handle the metadata generator
    // Using blocking mode to force pad probe execution. Macros for future reference
    // GST_PAD_PROBE_TYPE_BLOCK ; GST_PAD_PROBE_TYPE_BUFFER
    GstElement* appsrc = gst_bin_get_by_name (GST_BIN (pipeline), "appsrc0");
    GstElement* injector = gst_bin_get_by_name (GST_BIN (pipeline), "injector");
    GstPad* injector_src = gst_element_get_static_pad (injector, "src");
    gst_pad_add_probe (injector_src, GST_PAD_PROBE_TYPE_BUFFER ,
        probe_klv_request, (gpointer) appsrc, NULL);
    gst_object_unref(injector);
    gst_object_unref(injector_src);

    // These stolen from the example...
    g_object_set(G_OBJECT(appsrc), "caps", gst_caps_new_simple("meta/x-klv", "parsed", G_TYPE_BOOLEAN, TRUE, NULL), NULL);
    g_object_set(G_OBJECT(appsrc), "format", GST_FORMAT_TIME, NULL);
    g_object_set(G_OBJECT(appsrc), "do-timestamp", TRUE, NULL);
    // not in original example but adviced stack overflow
    // g_object_set(G_OBJECT(appsrc), "stream-type", 0, NULL);
    // g_object_set(G_OBJECT(appsrc), "is-live", FALSE, NULL);
    // g_object_set(G_OBJECT(appsrc), "block", FALSE, NULL);

    // Probe with callback every time a buffer flows through
    // place_probe(pipeline, "data_0", probe_read_data);
    place_probe(pipeline, "meta_0", probe_read_meta);
    place_probe(pipeline, "data_0", probe_read_data);
    place_probe(pipeline, "mpeg", probe_read_any);
    place_probe(pipeline, "data_1", probe_read_any);
    place_probe(pipeline, "meta_1", probe_read_any);
    place_probe(pipeline, "meta_2", probe_read_meta);

    /* play */
    gst_element_set_state (pipeline, GST_STATE_PLAYING);
    g_main_loop_run (loop);

    /* clean up */
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (GST_OBJECT (pipeline));
    g_main_loop_unref (loop);
    return 0;
}



