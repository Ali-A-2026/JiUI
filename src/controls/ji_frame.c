/**
 * JiUI - Frame implementation
 */

#include <jiui/ji_frame.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>

JiFrame* ji_frame_new(void) {
    JiFrame* frame = (JiFrame*)ji_calloc(1, sizeof(JiFrame));
    if (!frame) { JI_ERROR_LOG("ji_frame_new: out of memory"); return NULL; }
    frame->shape = JI_FRAME_STYLED_PANEL;
    frame->shadow = JI_FRAME_SUNKEN;
    frame->line_width = 1;
    frame->mid_line_width = 0;
    frame->frame_margin = 0;
    return frame;
}

void ji_frame_destroy(JiFrame* frame) { if (frame) ji_free(frame); }
void ji_frame_set_shape(JiFrame* frame, JiFrameShape shape) { if (frame) frame->shape = shape; }
JiFrameShape ji_frame_get_shape(const JiFrame* frame) { return frame ? frame->shape : JI_FRAME_NO_FRAME; }
void ji_frame_set_shadow(JiFrame* frame, JiFrameShadow shadow) { if (frame) frame->shadow = shadow; }
JiFrameShadow ji_frame_get_shadow(const JiFrame* frame) { return frame ? frame->shadow : JI_FRAME_PLAIN; }
void ji_frame_set_line_width(JiFrame* frame, int width) { if (frame) frame->line_width = width; }
int ji_frame_get_line_width(const JiFrame* frame) { return frame ? frame->line_width : 0; }
void ji_frame_set_content(JiFrame* frame, JiControl* content) { if (frame) frame->content = content; }
JiControl* ji_frame_get_content(const JiFrame* frame) { return frame ? frame->content : NULL; }
