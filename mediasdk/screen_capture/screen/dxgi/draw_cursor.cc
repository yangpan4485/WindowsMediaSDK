#include "draw_cursor.h"

const int kBytesPerPixel = 4;

void AlphaBlend(uint8_t* dest, int dest_stride, const uint8_t* src, int src_stride, int width,
                int height) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int pixel_index = x * kBytesPerPixel;
            uint8_t base_alpha = 255 - src[pixel_index + 3];
            if (base_alpha == 255) {
                continue;
            } else if (base_alpha == 0) {
                memcpy(dest + pixel_index, src + pixel_index, kBytesPerPixel);
            } else {
                double alpha = base_alpha / 255.0;
                dest[pixel_index] =
                    static_cast<uint8_t>(dest[pixel_index] * alpha) + src[pixel_index];
                dest[pixel_index + 1] =
                    static_cast<uint8_t>(dest[pixel_index + 1] * alpha) + src[pixel_index + 1];
                dest[pixel_index + 2] =
                    static_cast<uint8_t>(dest[pixel_index + 2] * alpha) + src[pixel_index + 2];
            }
        }
        src += src_stride;
        dest += dest_stride;
    }
}

void ProcessMaskColor(bool is_mono, uint8_t* frame_data, int frame_stride, uint8_t* cursor_data,
                      int cursor_stride, int skip_x, int skip_y, int width, int height,
                      int mono_pitch_y) {

    uint32_t* frame_data32 = reinterpret_cast<uint32_t*>(frame_data);
    uint32_t frame_pitch_in_pixels = frame_stride / sizeof(uint32_t);

    if (is_mono) {
        for (int row = 0; row < height; row++) {
            uint8_t mask = 0x80;
            mask = mask >> (skip_x % 8);
            for (int col = 0; col < width; col++) {
                uint32_t and_mask_index = ((col + skip_x) / 8) + ((row + skip_y) * cursor_stride);
                uint8_t and_mask = cursor_data[and_mask_index] & mask;
                uint8_t xor_mask =
                    cursor_data[and_mask_index + (mono_pitch_y + skip_y) * cursor_stride] & mask;

                uint32_t index = (row * frame_pitch_in_pixels) + col;

                // code block begin
                if (and_mask == 0) {
                    if (xor_mask) {
                        frame_data32[index] = frame_data32[index] & 0xFF000000 ^ 0x00FFFFFF;
                    } else {
                        frame_data32[index] = frame_data32[index] & 0xFF000000;
                    }
                } else if (xor_mask) {
                    frame_data32[index] = frame_data32[index] ^ 0x00FFFFFF;
                }

                // code block end
                /*
                The code above is same as below, the above is optimized
                version:) if (and_mask == 0) { frame_data32[index] &=
                0xFF000000;
                }
                if (xor_mask) {
                        frame_data32[index] ^= 0x00FFFFFF;
                }
                */
                // Adjust mask
                if (mask == 0x01) {
                    mask = 0x80;
                } else {
                    mask = mask >> 1;
                }
            }
        }
    } else {
        uint32_t* cursor_data32 = reinterpret_cast<uint32_t*>(cursor_data);
        for (int row = 0; row < height; row++) {
            for (int col = 0; col < width; col++) {
                // Set up mask
                uint32_t MaskVal =
                    0xFF000000 &
                    cursor_data32[(col + skip_x) +
                                  ((row + skip_y) * (cursor_stride / sizeof(uint32_t)))];
                if (MaskVal) {
                    // Mask was 0xFF
                    frame_data32[(row * frame_pitch_in_pixels) + col] =
                        frame_data32[(row * frame_pitch_in_pixels) + col] ^
                            cursor_data32[(col + skip_x) +
                                          ((row + skip_y) * (cursor_stride / sizeof(uint32_t)))] |
                        0xFF000000;
                } else {
                    // Mask was 0x00
                    frame_data32[(row * frame_pitch_in_pixels) + col] =
                        cursor_data32[(col + skip_x) +
                                      ((row + skip_y) * (cursor_stride / sizeof(uint32_t)))] |
                        0xFF000000;
                }
            }
        }
    }
}

DrawCursor::DrawCursor() {}

DrawCursor::~DrawCursor() {}

void DrawCursor::Draw(std::shared_ptr<VideoFrame> video_frame, DXGI_OUTDUPL_POINTER_POSITION& postion,
                      DXGI_OUTDUPL_POINTER_SHAPE_INFO& pointer_info, uint8_t* cursor_data) {
    uint32_t frame_pitch = video_frame->GetPitch();
    uint8_t* frame_data = video_frame->GetData();
    width_ = video_frame->GetWidth();
    height_ = video_frame->GetHeight();
    int32_t frame_offset_x = postion.Position.x;
    int32_t frame_offset_y = postion.Position.y;
    int32_t skip_x = 0;
    int32_t skip_y = 0;
    int32_t draw_rect_width = pointer_info.Width;
    int32_t draw_rect_height = pointer_info.Height;

    int32_t mono_pitch_y = 0;

    if (pointer_info.Type == DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MONOCHROME) {
        draw_rect_height = draw_rect_height / 2;
        mono_pitch_y = draw_rect_height;
    }

    if (frame_offset_x < 0) {
        skip_x = 0 - frame_offset_x;
        draw_rect_width = draw_rect_width - skip_x;
        frame_offset_x = 0;
    } else if (frame_offset_x + draw_rect_width > width_) {
        draw_rect_width = width_ - frame_offset_x;
    }

    if (frame_offset_y < 0) {
        skip_y = 0 - frame_offset_y;
        draw_rect_height = draw_rect_height - skip_y;
        frame_offset_y = 0;
    } else if (frame_offset_y + draw_rect_height > height_) {
        draw_rect_height = height_ - frame_offset_y;
    }

    if (draw_rect_width <= 0 || draw_rect_height <= 0) {
        return;
    }

    uint8_t* dest = frame_data + frame_pitch * frame_offset_y + kBytesPerPixel * frame_offset_x;

    if (pointer_info.Type == DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MONOCHROME ||
        pointer_info.Type == DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MASKED_COLOR) {
        bool is_mono = (pointer_info.Type == DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MONOCHROME);
        ProcessMaskColor(is_mono, dest, frame_pitch, cursor_data, pointer_info.Pitch, skip_x,
                         skip_y, draw_rect_width, draw_rect_height, mono_pitch_y);
    } else {
        uint8_t* src = cursor_data + pointer_info.Pitch * skip_y + kBytesPerPixel * skip_x;
        AlphaBlend(dest, frame_pitch, src, pointer_info.Pitch, draw_rect_width, draw_rect_height);
    }
}