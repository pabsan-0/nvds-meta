#include <gst/gst.h>
#include <gst/gstmeta.h>
#include <gst/app/gstappsrc.h>
#include <gst/codecparsers/gsth264parser.h>
#include <glib.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>

#include "h264_sei_ntp.h"

// #include "bs.h"
#include "h264_avcc.h"
#include "h264_sei.h"
#include "h264_slice_data.h"
#include "h264_stream.h"



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


static void
need_data_callback (GstElement *appsrc, guint unused, gpointer udata)
{
    GST_LOG("Need data callback");
    GstBuffer *buffer;
    GstFlowReturn ret;
    static uint64_t next_ms_time_insert_sei = 0;
    struct timespec one_ms;
    struct timespec rem;
    uint8_t *h264_sei = NULL;
    size_t length = 0;

    one_ms.tv_sec = 0;
    one_ms.tv_nsec = 1000000;

    while (now_ms() <= next_ms_time_insert_sei) {
        GST_TRACE ("sleep to wait time trigger");
        nanosleep(&one_ms, &rem);
    }

    if (!h264_sei_ntp_new(&h264_sei, &length)) {
        printf("ouch\n");
        GST_ERROR("h264_sei_ntp_new failed");
        return;
    }

    if (NULL != h264_sei && length > 0) {

        buffer = gst_buffer_new_allocate(NULL, START_CODE_PREFIX_BYTES + length, NULL);
        if (NULL != buffer)  {

            // fill start_code_prefix: 0x00000001/
            uint8_t start_code_prefix[] = START_CODE_PREFIX;
            gst_buffer_fill (buffer, 0, start_code_prefix, START_CODE_PREFIX_BYTES);

            size_t bytes_copied = gst_buffer_fill (buffer, START_CODE_PREFIX_BYTES, h264_sei, length);
            if (bytes_copied == length) {
                g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);
                GST_DEBUG("H264 sei ntp timestamp inserted");
            } else {
                GST_ERROR("gst buffer fill did not copy all bytes");
            }
        } else {
            GST_ERROR("Gst buffer new allocate failed");
        }
        gst_buffer_unref(buffer);
    }

    next_ms_time_insert_sei = now_ms() + 1000;
    free(h264_sei);
}



static void 
handoff_callback (GstElement *identity, GstBuffer *buffer, 
    gpointer user_data)
{
    GST_TRACE("handoff callback");
    GstMapInfo info = GST_MAP_INFO_INIT;
    GstH264NalParser *nalparser = NULL;
    GstH264NalUnit nalu;

    if (gst_buffer_map(buffer, &info, GST_MAP_READ)) {
        nalparser = gst_h264_nal_parser_new();
        if (NULL != nalparser) {
            if (GST_H264_PARSER_OK ==
                gst_h264_parser_identify_nalu_unchecked(nalparser, info.data, 0, 
                                                         info.size, &nalu)) {
                if (GST_H264_NAL_SEI == nalu.type) {
                    GST_LOG("identify sei nalu with size = %d, offset %d, sc_offset = %d", 
                        nalu.size, nalu.offset, nalu.sc_offset);
                    int64_t delay = -1;
                    if (TRUE == h264_sei_ntp_parse(nalu.data + nalu.offset, nalu.size, &delay)) {
                        GST_LOG("delay = %ld ms", delay);
                    }
                }
            } else {
                GST_WARNING("gst_h264_parser_identify_nalu_unchecked failed");
            }
            gst_h264_nal_parser_free(nalparser);
        } else {
            GST_WARNING("gst_nal_parser_new failed");
        }
        gst_buffer_unmap(buffer, &info);
    } else {
        GST_WARNING("gst_buffer_map failed");
    }
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
        " videotestsrc name=src is-live=true "
        "     ! x264enc "
        "     ! video/x-h264, stream-format=byte-stream, alignment=au, profile=baseline"
        "     ! identity name=video "
        "     ! queue "
        "     ! fun. "
        " appsrc name=appsrc0  do-timestamp=true block=true is-live=true "
        "     ! video/x-h264, stream-format=byte-stream, alignment=au "
        "     ! identity name=newmeta "
        "     ! queue "
        "     ! fun. "
        " funnel name=fun "
        " ! queue "
        " ! h264parse "
        " ! video/x-h264, stream-format=byte-stream, alignment=au "
        " ! rtph264pay"
#if 0
        " ! udpsink host=127.0.0.1 port=1234 async=false sync=false "
        " udpsrc port=1234 caps=application/x-rtp,media=video,encoding-name=H264  "
#endif
        " ! rtph264depay "
        " ! video/x-h264, stream-format=byte-stream, alignment=nal  "
        " ! identity name=retriever "
        " ! fakesink "
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
    GstElement* appsrc = gst_bin_get_by_name (GST_BIN (pipeline), "appsrc0");
    g_signal_connect(appsrc, "need-data", G_CALLBACK(need_data_callback), NULL); 
    gst_object_unref(appsrc);

    GstElement* identity_retriever = gst_bin_get_by_name (GST_BIN (pipeline), "retriever");
    g_signal_connect(identity_retriever, "handoff", G_CALLBACK(handoff_callback), NULL); 
    gst_object_unref(identity_retriever);

    // Probe with callback every time a buffer flows through
    place_probe(pipeline, "newmeta", probe_read_any);
    //place_probe(pipeline, "video", probe_read_any);

    /* play */
    gst_element_set_state (pipeline, GST_STATE_PLAYING);
    g_main_loop_run (loop);

    /* clean up */
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (GST_OBJECT (pipeline));
    g_main_loop_unref (loop);
    return 0;
}



