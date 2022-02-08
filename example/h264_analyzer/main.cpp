#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iomanip>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "bs.h"

const uint32_t kBufferSize = 200000;
std::ifstream fin;
#define Extended_SAR 255

struct NALU {
    int start_code_len;
    uint32_t len;
    int forbidden_bit;
    int nal_reference_idc;
    int nal_unit_type;
    char* data;
};

enum NaluType {
    NALU_TYPE_SLICE = 1,
    NALU_TYPE_DPA = 2,
    NALU_TYPE_DPB = 3,
    NALU_TYPE_DPC = 4,
    NALU_TYPE_IDR = 5,
    NALU_TYPE_SEI = 6,
    NALU_TYPE_SPS = 7,
    NALU_TYPE_PPS = 8,
    NALU_TYPE_AUD = 9,
    NALU_TYPE_EOSEQ = 10,
    NALU_TYPE_EOSTREAM = 11,
    NALU_TYPE_FILL = 12,
};

bool IsStartCode(char* buffer) {
    // std::cout << buffer[0] << std::endl;
    if (buffer[0] == 0x00 && buffer[1] == 0x00 && buffer[2] == 0x00 && buffer[3] == 0x01) {
        return true;
    }

    if (buffer[0] == 0x00 && buffer[1] == 0x00 && buffer[2] == 0x01) {
        return true;
    }

    return false;
}

int GetNALDara(NALU* nalu) {
    char start_code[4] = "";
    char* buffer = new char[kBufferSize];
    fin.read(start_code, 3);
    if (fin.gcount() != 3) {
        std::cout << "h264 stream error" << std::endl;
        delete buffer;
        return 0;
    }
    if (!IsStartCode(start_code)) {
        fin.read(start_code + 3, 1);
        if (!IsStartCode(start_code)) {
            std::cout << "not found start code" << std::endl;
            delete buffer;
            return 0;
        }
        nalu->start_code_len = 4;
    } else {
        nalu->start_code_len = 3;
    }
    int pos = 0;
    // 找到start code
    int rewind = 0;
    while (true) {
        // 读取数据，每次读取一个字节
        if (fin.eof()) {
            // 表示是最后一个数据了
            nalu->len = pos;
            memcpy(nalu->data, buffer, nalu->len);          //
            nalu->forbidden_bit = nalu->data[0] & 0x80;     // 1 bit
            nalu->nal_reference_idc = nalu->data[0] & 0x60; // 2 bit
            nalu->nal_unit_type = (nalu->data[0]) & 0x1f;   // 5 bit
            delete buffer;
            return pos + nalu->start_code_len;
        }

        fin.read(buffer + pos, 1);
        pos++;
        if (buffer[pos - 1] != 0x01) {
            continue;
        }
        // std::cout << "0x01" << std::endl;
        if (IsStartCode(buffer + pos - 4)) {
            // 读取结束了
            rewind = -4;
            break;
        }
        if (IsStartCode(buffer + pos - 3)) {
            rewind = -3;
            break;
        }
    }
    // std::cout << "rewind:" << rewind << std::endl;
    // 回退
    fin.seekg(rewind, std::ios::cur);
    nalu->len = pos + rewind;
    memcpy(nalu->data, buffer, nalu->len);          //
    nalu->forbidden_bit = nalu->data[0] & 0x80;     // 1 bit
    nalu->nal_reference_idc = nalu->data[0] & 0x60; // 2 bit
    nalu->nal_unit_type = (nalu->data[0]) & 0x1f;   // 5 bit
    delete buffer;
    return pos + rewind + nalu->start_code_len;
}

void ParseH264Stream(const std::string& filename) {
    fin.open(filename, std::ios::binary | std::ios::in);
    if (!fin) {
        std::cout << "open file failed" << std::endl;
        return;
    }

    // 定义一个NALU
    NALU nalu;
    nalu.data = new char[kBufferSize];

    int data_offset = 0;
    int nal_num = 0;

    std::cout << "-----+-------- NALU Table ------+---------+" << std::endl;
    std::cout << " NUM |    POS  |    IDC |  TYPE |   LEN   |" << std::endl;
    std::cout << "-----+---------+--------+-------+---------+" << std::endl;

    // 读取数据
    while (!fin.eof()) {
        int len = GetNALDara(&nalu); // len = start_code + nalu_len
        if (len < 1) {
            std::cout << "get NALU data error" << std::endl;
            break;
        }
        std::string type = "";
        switch (nalu.nal_unit_type) {
        case NALU_TYPE_SLICE:
            type = "SLICE";
            break;
        case NALU_TYPE_DPA:
            type = "DPA";
            break;
        case NALU_TYPE_DPB:
            type = "DPB";
            break;
        case NALU_TYPE_DPC:
            type = "DPC";
            break;
        case NALU_TYPE_IDR:
            type = "IDR";
            break;
        case NALU_TYPE_SEI:
            type = "SEI";
            break;
        case NALU_TYPE_SPS:
            type = "SPS";
            break;
        case NALU_TYPE_PPS:
            type = "PPS";
            break;
        case NALU_TYPE_AUD:
            type = "AUD";
            break;
        case NALU_TYPE_EOSEQ:
            type = "EOSEQ";
            break;
        case NALU_TYPE_EOSTREAM:
            type = "EOSTREAM";
            break;
        case NALU_TYPE_FILL:
            type = "FILL";
            break;
        default:
            break;
        }
        std::cout << std::setw(5) << nal_num++ << "|" << std::setw(9) << data_offset << "|"
                  << std::setw(8) << (nalu.nal_reference_idc >> 5) << "|" << std::setw(7) << type
                  << "|" << std::setw(9) << len - nalu.start_code_len << "|" << std::endl;
        data_offset = data_offset + len;
    }

    if (nalu.data) {
        delete nalu.data;
    }
    fin.close();
}

bs_t* b;
void scaling_list(uint32_t* scalingList, int sizeOfScalingList,
                  uint32_t useDefaultScalingMatrixFlag) {
    int lastScale = 8;
    int nextScale = 8;
    for (int j = 0; j < sizeOfScalingList; j++) {
        if (nextScale != 0) {
            int delta_scale = bs_read_se(b);
            std::cout << "delta_scale:" << delta_scale << std::endl;
            nextScale = (lastScale + delta_scale + 256) % 256;
            useDefaultScalingMatrixFlag = (j == 0 && nextScale == 0);
        }
        scalingList[j] = (nextScale == 0) ? lastScale : nextScale;
        lastScale = scalingList[j];
    }
}

void hrd_parameters() {
    std::cout << "==hrd==" << std::endl;
    uint32_t cpb_cnt_minus1 = bs_read_ue(b);
    std::cout << "cpb_cnt_minus1:" << cpb_cnt_minus1 << std::endl;
    std::cout << "bit_rate_scale:" << bs_read_u(b, 4) << std::endl;
    std::cout << "cpb_size_scale:" << bs_read_u(b, 4) << std::endl;
    uint32_t bit_rate_value_minus1[5];
    uint32_t cpb_size_value_minus1[5];
    uint32_t cbr_flag[5];
    for (int SchedSelIdx = 0; SchedSelIdx <= cpb_cnt_minus1; SchedSelIdx++) {
        bit_rate_value_minus1[SchedSelIdx] = bs_read_ue(b);
        cpb_size_value_minus1[SchedSelIdx] = bs_read_ue(b);
        cbr_flag[SchedSelIdx] = bs_read_u1(b);
    }
    std::cout << "initial_cpb_removal_delay_length_minus1:" << bs_read_u(b, 5) << std::endl;
    std::cout << "cpb_removal_delay_length_minus1:" << bs_read_u(b, 5) << std::endl;
    std::cout << "dpb_output_delay_length_minus1:" << bs_read_u(b, 5) << std::endl;
    std::cout << "time_offset_length:" << bs_read_u(b, 5) << std::endl;
    std::cout << "==hrd end==" << std::endl;
}

void vui_parameters() {
    std::cout << "==VUI==" << std::endl;
    uint32_t aspect_ratio_info_present_flag = bs_read_u1(b);
    std::cout << "aspect_ratio_info_present_flag:" << aspect_ratio_info_present_flag << std::endl;
    if (aspect_ratio_info_present_flag) {
        uint32_t aspect_ratio_idc = bs_read_u8(b);
        std::cout << "  aspect_ratio_idc:" << aspect_ratio_idc << std::endl;
        if (aspect_ratio_idc == Extended_SAR) {
            std::cout << "    sar_width:" << bs_read_u(b, 16);
            std::cout << "    sar_height:" << bs_read_u(b, 16);
        }
    }
    uint32_t overscan_info_present_flag = bs_read_u1(b);
    std::cout << "overscan_info_present_flag:" << overscan_info_present_flag << std::endl;
    if (overscan_info_present_flag) {
        std::cout << "  overscan_appropriate_flag:" << bs_read_u1(b) << std::endl;
    }
    uint32_t video_signal_type_present_flag = bs_read_u1(b);
    std::cout << "video_signal_type_present_flag:" << video_signal_type_present_flag << std::endl;
    if (video_signal_type_present_flag) {
        std::cout << "  video_format:" << bs_read_u(b, 3) << std::endl;
        std::cout << "  video_full_range_flag:" << bs_read_u1(b) << std::endl;
        uint32_t colour_description_present_flag = bs_read_u1(b);
        std::cout << "  colour_description_present_flag:" << colour_description_present_flag
                  << std::endl;
        if (colour_description_present_flag) {
            std::cout << "    colour_primaries:" << bs_read_u8(b) << std::endl;
            std::cout << "    transfer_characteristics:" << bs_read_u8(b) << std::endl;
            std::cout << "    matrix_coefficients:" << bs_read_u8(b) << std::endl;
        }
    }
    uint32_t chroma_loc_info_present_flag = bs_read_u1(b);
    std::cout << "chroma_loc_info_present_flag:" << chroma_loc_info_present_flag << std::endl;
    if (chroma_loc_info_present_flag) {
        std::cout << "  chroma_sample_loc_type_top_field:" << bs_read_ue(b) << std::endl;
        std::cout << "  chroma_sample_loc_type_bottom_field:" << bs_read_ue(b) << std::endl;
    }
    uint32_t timing_info_present_flag = bs_read_u1(b);
    std::cout << "timing_info_present_flag:" << timing_info_present_flag << std::endl;
    if (timing_info_present_flag) {
        std::cout << "  num_units_in_tick:" << bs_read_u(b, 32) << std::endl;
        std::cout << "  time_scale:" << bs_read_u(b, 32) << std::endl;
        std::cout << "  fixed_frame_rate_flag:" << bs_read_u1(b) << std::endl;
    }
    uint32_t nal_hrd_parameters_present_flag = bs_read_u1(b);
    std::cout << "nal_hrd_parameters_present_flag:" << nal_hrd_parameters_present_flag << std::endl;
    if (nal_hrd_parameters_present_flag) {
        hrd_parameters();
    }
    uint32_t vcl_hrd_parameters_present_flag = bs_read_u1(b);
    std::cout << "vcl_hrd_parameters_present_flag:" << vcl_hrd_parameters_present_flag << std::endl;
    if (vcl_hrd_parameters_present_flag) {
        hrd_parameters();
    }
    if (nal_hrd_parameters_present_flag || vcl_hrd_parameters_present_flag) {
        std::cout << "  low_delay_hrd_flag:" << bs_read_u1(b) << std::endl;
    }
    std::cout << "pic_struct_present_flag:" << bs_read_u1(b) << std::endl;
    uint32_t bitstream_restriction_flag = bs_read_u1(b);
    std::cout << "bitstream_restriction_flag:" << bitstream_restriction_flag << std::endl;
    if (bitstream_restriction_flag) {
        std::cout << "  motion_vectors_over_pic_boundaries_flag:" << bs_read_u1(b) << std::endl;
        std::cout << "  max_bytes_per_pic_denom:" << bs_read_ue(b) << std::endl;
        std::cout << "  max_bits_per_mb_denom:" << bs_read_ue(b) << std::endl;
        std::cout << "  log2_max_mv_length_horizontal:" << bs_read_ue(b) << std::endl;
        std::cout << "  log2_max_mv_length_vertical:" << bs_read_ue(b) << std::endl;
        std::cout << "  num_reorder_frames:" << bs_read_ue(b) << std::endl;
        std::cout << "  max_dec_frame_buffering:" << bs_read_ue(b) << std::endl;
    }
    std::cout << "==VUI== End" << std::endl;
}

std::vector<uint8_t> EBSP2RBSP(uint8_t* buffer, int len) {
    // 00 00 03 去掉03
    std::vector<uint8_t> ebsp;
    int i = 0;
    for (i = 0; i < len - 2; ++i) {
        if (buffer[i] == 0x00 && buffer[i + 1] == 0x00 && buffer[i + 2] == 0x03) {
            ebsp.push_back(buffer[i++]);
            ebsp.push_back(buffer[i++]);
        } else {
            ebsp.push_back(buffer[i]);
        }
    }
    for (; i < len; ++i) {
        ebsp.push_back(buffer[i]);
    }
    return ebsp;
}
uint32_t chroma_format_idc; // pps要使用
void ParseSPS(uint8_t* buffer, int len) {
    std::vector<uint8_t> ebsp = EBSP2RBSP(buffer, len);
    b = bs_new(ebsp.data(), ebsp.size());

    std::cout << "forbidden_zero_bit :" << bs_read_u(b, 1) << std::endl;
    std::cout << "nal_ref_idc:" << bs_read_u(b, 2) << std::endl;
    std::cout << "nal_unit_type:" << bs_read_u(b, 5) << std::endl;
    uint32_t profile_idc = bs_read_u8(b);
    std::cout << "profile_idc:" << profile_idc << std::endl;
    std::cout << "constraint_set0_flag:" << bs_read_u(b, 1) << std::endl;
    std::cout << "constraint_set1_flag:" << bs_read_u(b, 1) << std::endl;
    std::cout << "constraint_set2_flag:" << bs_read_u(b, 1) << std::endl;
    std::cout << "constraint_set3_flag:" << bs_read_u(b, 1) << std::endl;
    std::cout << "constraint_set4_flag:" << bs_read_u(b, 1) << std::endl;
    std::cout << "constraint_set5_flag:" << bs_read_u(b, 1) << std::endl;
    std::cout << "reserved_zero_2bits :" << bs_read_u(b, 2) << std::endl;
    std::cout << "level_idc:" << bs_read_u8(b) << std::endl;
    std::cout << "seq_parameter_set_id:" << bs_read_ue(b) << std::endl;
    if (profile_idc == 100 || profile_idc == 110 || profile_idc == 122 || profile_idc == 244 ||
        profile_idc == 44 || profile_idc == 83 || profile_idc == 86 || profile_idc == 118 ||
        profile_idc == 128) {
        chroma_format_idc = bs_read_ue(b);
        std::cout << "  chroma_format_idc:" << chroma_format_idc << std::endl;
        if (chroma_format_idc == 3) {
            std::cout << "  separate_colour_plane_flag:" << bs_read_u(b, 1) << std::endl;
        }
        std::cout << "  bit_depth_luma_minus8:" << bs_read_ue(b) << std::endl;
        std::cout << "  bit_depth_chroma_minus8:" << bs_read_ue(b) << std::endl;
        std::cout << "  qpprime_y_zero_transform_bypass_flag:" << bs_read_u(b, 1) << std::endl;
        uint32_t seq_scaling_matrix_present_flag = bs_read_u(b, 1);
        std::cout << "  seq_scaling_matrix_present_flag:" << seq_scaling_matrix_present_flag
                  << std::endl;

        if (seq_scaling_matrix_present_flag) {
            uint32_t seq_scaling_list_present_flag[12];
            uint32_t* ScalingList4x4[12];
            uint32_t UseDefaultScalingMatrix4x4Flag[12];
            uint32_t* ScalingList8x8[12];
            uint32_t UseDefaultScalingMatrix8x8Flag[12];
            for (int i = 0; i < ((chroma_format_idc != 3) ? 8 : 12); i++) {
                seq_scaling_list_present_flag[i] = bs_read_u(b, 1);
                if (seq_scaling_list_present_flag[i]) {
                    if (i < 6) {
                        scaling_list(ScalingList4x4[i], 16, UseDefaultScalingMatrix4x4Flag[i]);
                    } else {
                        scaling_list(ScalingList8x8[i - 6], 64,
                                     UseDefaultScalingMatrix8x8Flag[i - 6]);
                    }
                }
            }
        }
    }
    std::cout << "log2_max_frame_num_minus4:" << bs_read_ue(b) << std::endl;
    uint32_t pic_order_cnt_type = bs_read_ue(b);
    std::cout << "pic_order_cnt_type:" << pic_order_cnt_type << std::endl;
    if (pic_order_cnt_type == 0) {
        std::cout << "  log2_max_pic_order_cnt_lsb_minus4:" << bs_read_ue(b) << std::endl;
    } else if (pic_order_cnt_type == 1) {
        std::cout << "  delta_pic_order_always_zero_flag:" << bs_read_u(b, 1) << std::endl;
        std::cout << "  offset_for_non_ref_pic:" << bs_read_se(b) << std::endl;
        std::cout << "  offset_for_top_to_bottom_field:" << bs_read_se(b) << std::endl;
        uint32_t num_ref_frames_in_pic_order_cnt_cycle = bs_read_ue(b);
        std::cout << "  num_ref_frames_in_pic_order_cnt_cycle:"
                  << num_ref_frames_in_pic_order_cnt_cycle << std::endl;
        int offset_for_ref_frame[256];
        for (int i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++) {
            offset_for_ref_frame[i] = bs_read_se(b);
        }
    }
    std::cout << "max_num_ref_frames:" << bs_read_ue(b) << std::endl;
    std::cout << "gaps_in_frame_num_value_allowed_flag:" << bs_read_u(b, 1) << std::endl;
    std::cout << "pic_width_in_mbs_minus1:" << bs_read_ue(b) << std::endl;
    std::cout << "pic_height_in_map_units_minus1:" << bs_read_ue(b) << std::endl;
    uint32_t frame_mbs_only_flag = bs_read_u(b, 1);
    std::cout << "frame_mbs_only_flag:" << frame_mbs_only_flag << std::endl;
    if (!frame_mbs_only_flag) {
        std::cout << "  mb_adaptive_frame_field_flag:" << bs_read_u(b, 1) << std::endl;
    }
    std::cout << "direct_8x8_inference_flag:" << bs_read_u(b, 1) << std::endl;
    uint32_t frame_cropping_flag = bs_read_u(b, 1);
    std::cout << "frame_cropping_flag:" << frame_cropping_flag << std::endl;
    if (frame_cropping_flag) {
        std::cout << "  frame_crop_left_offset:" << bs_read_ue(b) << std::endl;
        std::cout << "  frame_crop_right_offset:" << bs_read_ue(b) << std::endl;
        std::cout << "  frame_crop_top_offset:" << bs_read_ue(b) << std::endl;
        std::cout << "  frame_crop_bottom_offset:" << bs_read_ue(b) << std::endl;
    }
    uint32_t vui_parameters_present_flag = bs_read_u(b, 1);
    std::cout << "vui_parameters_present_flag:" << vui_parameters_present_flag << std::endl;
    if (vui_parameters_present_flag) {
        vui_parameters();
    }
    bs_free(b);
}

// 一个bit 1，若干个bit 0
void rbsp_trailing_bits() {
    int rbsp_stop_one_bit = bs_read_u1(b); // equal to 1
    std::cout << "rbsp_stop_one_bit:" << rbsp_stop_one_bit << std::endl;
    while (!bs_byte_aligned(b)) {
        int rbsp_alignment_zero_bit = bs_read_u1(b); // equal to 0
    }
}

// 是不是有更多的RBSP数据
uint32_t more_rbsp_data() {
    if (bs_eof(b)) {
        return 0;
    }
    if (bs_peek_u1(b) == 1) {
        return 0;
    } // if next bit is 1, we've reached the stop bit
    return 1;
}

void ParsePPS(uint8_t* buffer, int len) {
    std::cout << "=========================PPS=======================" << std::endl;
    std::vector<uint8_t> ebsp = EBSP2RBSP(buffer, len);
    // std::cout << "pps buffer len:" << ebsp.size() << std::endl;
    b = bs_new(ebsp.data(), ebsp.size());
    std::cout << "forbidden_zero_bit :" << bs_read_u(b, 1) << std::endl;
    std::cout << "nal_ref_idc:" << bs_read_u(b, 2) << std::endl;
    std::cout << "nal_unit_type:" << bs_read_u(b, 5) << std::endl;
    std::cout << "pic_parameter_set_id:" << bs_read_ue(b) << std::endl;
    std::cout << "seq_parameter_set_id:" << bs_read_ue(b) << std::endl;
    std::cout << "entropy_coding_mode_flag:" << bs_read_u1(b) << std::endl;
    std::cout << "bottom_field_pic_order_in_frame_present_flag:" << bs_read_u1(b) << std::endl;
    uint32_t num_slice_groups_minus1 = bs_read_ue(b);
    std::cout << "num_slice_groups_minus1:" << num_slice_groups_minus1 << std::endl;
    if (num_slice_groups_minus1 > 0) {
        uint32_t slice_group_map_type = bs_read_ue(b);
        std::cout << "  slice_group_map_type:" << slice_group_map_type << std::endl;
        if (slice_group_map_type == 0) {
            uint32_t run_length_minus1[8];
            for (int iGroup = 0; iGroup <= num_slice_groups_minus1; iGroup++) {
                run_length_minus1[iGroup] = bs_read_ue(b);
            }
        } else if (slice_group_map_type == 2) {
            uint32_t top_left[8];
            uint32_t bottom_right[8];
            for (int iGroup = 0; iGroup < num_slice_groups_minus1; iGroup++) {
                top_left[iGroup] = bs_read_ue(b);
                bottom_right[iGroup] = bs_read_ue(b);
            }
        } else if (slice_group_map_type == 3 || slice_group_map_type == 4 ||
                   slice_group_map_type == 5) {
            std::cout << "    slice_group_change_direction_flag:" << bs_read_u1(b) << std::endl;
            std::cout << "    slice_group_change_rate_minus1:" << bs_read_ue(b) << std::endl;
        } else if (slice_group_map_type == 6) {
            uint32_t pic_size_in_map_units_minus1 = bs_read_ue(b);
            std::cout << "    pic_size_in_map_units_minus1:" << pic_size_in_map_units_minus1
                      << std::endl;
            uint32_t slice_group_id[8];
            for (int i = 0; i <= pic_size_in_map_units_minus1; i++) {
                slice_group_id[i] = bs_read_ue(b);
            }
        }
    }
    std::cout << "num_ref_idx_l0_default_active_minus1:" << bs_read_ue(b) << std::endl;
    std::cout << "num_ref_idx_l1_default_active_minus1:" << bs_read_ue(b) << std::endl;
    std::cout << "weighted_pred_flag:" << bs_read_u1(b) << std::endl;
    std::cout << "weighted_bipred_idc:" << bs_read_u(b, 2) << std::endl;
    std::cout << "pic_init_qp_minus26:" << bs_read_se(b) << std::endl;
    std::cout << "pic_init_qs_minus26:" << bs_read_se(b) << std::endl;
    std::cout << "chroma_qp_index_offset:" << bs_read_se(b) << std::endl;
    std::cout << "deblocking_filter_control_present_flag:" << bs_read_u1(b) << std::endl;
    std::cout << "constrained_intra_pred_flag:" << bs_read_u1(b) << std::endl;
    std::cout << "redundant_pic_cnt_present_flag:" << bs_read_u1(b) << std::endl;
    if (more_rbsp_data()) {
        uint32_t transform_8x8_mode_flag = bs_read_u1(b);
        std::cout << "  transform_8x8_mode_flag:" << transform_8x8_mode_flag << std::endl;
        uint32_t pic_scaling_matrix_present_flag = bs_read_u1(b);
        std::cout << "  pic_scaling_matrix_present_flag:" << std::endl;
        uint32_t pic_scaling_list_present_flag[6];
        uint32_t* ScalingList4x4[12];
        uint32_t UseDefaultScalingMatrix4x4Flag[12];
        uint32_t* ScalingList8x8[12];
        uint32_t UseDefaultScalingMatrix8x8Flag[12];
        if (pic_scaling_matrix_present_flag) {
            for (int i = 0; i < 6 + ((chroma_format_idc != 3) ? 2 : 6) * transform_8x8_mode_flag;
                 i++) {
                pic_scaling_list_present_flag[i] = bs_read_u1(b);
                if (pic_scaling_list_present_flag[i]) {
                    if (i < 6) {
                        scaling_list(ScalingList4x4[i], 16, UseDefaultScalingMatrix4x4Flag[i]);
                    } else {
                        scaling_list(ScalingList8x8[i - 6], 64,
                                     UseDefaultScalingMatrix8x8Flag[i - 6]);
                    }
                }
            }
        }
        std::cout << "  second_chroma_qp_index_offset:" << bs_read_se(b) << std::endl;
    }
    rbsp_trailing_bits();
    bs_free(b);
}

// https://github.com/TedaLIEz/SimpleH264/tree/cdc3f45dceda51ff72cf4e62cbce5a1c9f1a1960/include/parser
int main(void) {
    std::string filename = "../../test.h264";
    // 解析 I/P/B/SE/SI Slice
    ParseH264Stream(filename);
    getchar();
    return 0;
}