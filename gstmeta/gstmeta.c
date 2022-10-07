#include <gst/gst.h>
#include <gst/gstmeta.h>
#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "gstdetectionsmeta.h"

static GMainLoop *loop;


GstPadProbeReturn
probe_meta_write (GstPad * pad, GstPadProbeInfo * info,
    gpointer u_data)
{
    GstBuffer *buf = (GstBuffer *) info->data;
    GstDetectionMetas* mymeta = GST_DETECTIONMETAS_ADD(buf);
    mymeta->detections_count = 2;
    return GST_PAD_PROBE_OK;
}


GstPadProbeReturn
probe_meta_read (GstPad * pad, GstPadProbeInfo * info,
    gpointer u_data)
{
    GstBuffer *buf = (GstBuffer *) info->data;
    GstDetectionMetas* mymeta = GST_DETECTIONMETAS_GET(buf);
    if (mymeta != NULL){
        const int retval = mymeta->detections_count;
        printf("%s: Probe received: %d\n", (char *) u_data, retval);
    } else {
        printf("%s: No meta\n", (char *) u_data);
    }
    return GST_PAD_PROBE_OK;
}


gint
place_probe (GstElement *pipeline, gchar *elementName,
    GstPadProbeCallback cb_probe)
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
        "   videotestsrc name=src "
        " ! queue max-size-buffers=10 "
        " ! identity name=injector "
        " ! x264enc tune=zerolatency "
        " ! identity name=receiver_after_h264 "
        " ! rtph264pay config-interval=-1 "
        " ! identity name=receiver_after_rtppay "
#if 0
        " ! udpsink host=127.0.0.1 port=1234 async=false "
        " udpsrc port=1234 retrieve-sender-address=0 "
#endif
        " ! application/x-rtp "
        " ! rtph264depay "
        " ! h264parse config-interval=-1 "
        " ! avdec_h264  "
        " ! identity name=receiver_at_sink "
        " ! videoconvert "
        " ! autovideosink ";




	/* Launch the pipeline*/
	gchar *desc = g_strdup (desc_templ);
	pipeline = gst_parse_launch (desc, &error);
	if (error) {
		g_printerr ("pipeline parsing error: %s\n", error->message);
		g_error_free (error);
		return 1;
	}

    // Probe with callback every time a buffer flows through
    place_probe(pipeline, "injector",       probe_meta_write);
    place_probe(pipeline, "receiver_after_h264", probe_meta_read);
    place_probe(pipeline, "receiver_after_rtppay", probe_meta_read);
    place_probe(pipeline, "receiver_at_sink", probe_meta_read);

    /* play */
    gst_element_set_state (pipeline, GST_STATE_PLAYING);
    g_main_loop_run (loop);

    /* clean up */
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (GST_OBJECT (pipeline));
    g_main_loop_unref (loop);
    return 0;
}



