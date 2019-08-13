; RUN: llc -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-dpi -parts-fecfi -verify-machineinstrs < %s | FileCheck %s
; XFAIL: *
;
; This test initially tested spilling of data pointers in a more complex function. Due to changes between LLVM 6.x and
; 8.x the spills are assigned differently, and the test no longer works.
; However, this now shows that the modifiers themselves might be spilled, which could allow an attacker to modify
; their value while on the stack. As such, this should be split into two tests that check for both.
; Not clear how to do this though. :/

%struct.storable_picture = type { i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i8, i32, i32, i32, i32, i16, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i16**, i16***, i16***, %struct.pic_motion_params**, [3 x %struct.pic_motion_params**], %struct.pic_motion_params_old, [3 x %struct.pic_motion_params_old], i16**, %struct.storable_picture*, %struct.storable_picture*, %struct.storable_picture*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [2 x i32], i32, %struct.DecRefPicMarking_s*, i32, i32, i32, i32, i16*, i32, i32, i32, i32, i32, i32, i32, i16**, i32, i32, [2 x i8], [2 x %struct.storable_picture**] }
%struct.pic_motion_params = type { [2 x %struct.storable_picture*], [2 x %struct.BlockPos], [2 x i8] }
%struct.BlockPos = type { i16, i16 }
%struct.pic_motion_params_old = type { i8* }
%struct.DecRefPicMarking_s = type { i32, i32, i32, i32, i32, %struct.DecRefPicMarking_s* }
%struct.macroblock.329 = type { %struct.slice.327*, %struct.video_par.318*, %struct.inp_par*, i32, %struct.BlockPos, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [2 x i32], [3 x i32], i32, i32, i32, i32, i16, i8, i8, i16, i16, %struct.macroblock.329*, %struct.macroblock.329*, %struct.macroblock.329*, %struct.macroblock.329*, i16, [2 x [4 x [4 x [2 x i16]]]], i32, [3 x i64], [3 x i64], [3 x i64], i32, [4 x i8], [4 x i8], i8, i8, i8, i16, i16, i16, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, void (%struct.macroblock.329*, i32, i32, i32)*, void (%struct.macroblock.329*, i32, i32, i32)*, void (%struct.macroblock.329*, %struct.pix_pos*, %struct.BlockPos*, i16, %struct.pic_motion_params**, i32, i32, i32, i32, i32)*, i32 (%struct.macroblock.329*, %struct.DecodingEnvironment*, i32)*, i8 (%struct.macroblock.329*, %struct.syntaxelement.321*, %struct.datapartition.322*, i8, i32)*, i8 }
%struct.slice.327 = type { %struct.video_par.318*, %struct.inp_par*, %struct.pic_parameter_set_rbsp_t*, %struct.seq_parameter_set_rbsp_t*, i32, %struct.decoded_picture_buffer.313*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [2 x i32], i32, i32, i32, i32, i32, i16, i32, i32, i32, i32, [2 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, i8, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %struct.DecRefPicMarking_s*, [6 x i8], [6 x %struct.storable_picture**], %struct.datapartition.322*, %struct.MotionInfoContexts*, %struct.TextureInfoContexts*, [6 x [32 x i32]], [2 x i32], [2 x i32*], [2 x i32*], [2 x i32*], [2 x i32*], i32, i32, i32, %struct.nalunitheadermvcext_tag, i16, i16, i16, i32, i32, i32, i32, i16***, i16***, i32***, i32***, i32***, [16 x i32], i16**, i16**, i32**, i16**, i16**, [3 x [6 x [4 x [4 x i32]]]], [3 x [6 x [4 x [4 x i32]]]], [3 x [6 x [8 x [8 x i32]]]], [3 x [6 x [8 x [8 x i32]]]], [12 x i32*], [64 x i32], i32, i32, i16, i16, i16, i16, i32***, i32***, i32****, i16, i16, i32, i32, %struct.frame_store**, %struct.frame_store**, i32, [17 x i32], {}*, i32 (%struct.macroblock.329*, i32, i16**, %struct.storable_picture*)*, i32 (%struct.video_par.318*, %struct.inp_par*)*, i32 (%struct.slice.327*, i32)*, {}*, {}*, {}*, void (%struct.slice.327*)*, {}*, void (i32, i32, i32*, i32*)*, void (i32, i32, i32*, i32*)*, {}*, i32, %struct.macroblock.329*, %struct.storable_picture*, i32**, i8**, i8*, [6 x [16 x i8]] }
%struct.pic_parameter_set_rbsp_t = type { i32, i32, i32, i32, i32, i32, [12 x i32], [6 x [16 x i32]], [6 x [64 x i32]], [6 x i32], [6 x i32], i32, i32, i32, [8 x i32], [8 x i32], [8 x i32], i32, i32, i32, i8*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32 }
%struct.seq_parameter_set_rbsp_t = type { i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [12 x i32], [6 x [16 x i32]], [6 x [64 x i32]], [6 x i32], [6 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, [256 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %struct.vui_seq_parameters_t, i32, i32 }
%struct.vui_seq_parameters_t = type { i32, i32, i16, i16, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %struct.hrd_parameters_t, i32, %struct.hrd_parameters_t, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32 }
%struct.hrd_parameters_t = type { i32, i32, i32, [32 x i32], [32 x i32], [32 x i32], i32, i32, i32, i32 }
%struct.decoded_picture_buffer.313 = type { %struct.video_par.318*, %struct.inp_par*, %struct.frame_store**, %struct.frame_store**, %struct.frame_store**, i32, i32, i32, i32, i32, i32, [1024 x i32], i32, i32, %struct.frame_store* }
%struct.frame_store = type { i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %struct.storable_picture*, %struct.storable_picture*, %struct.storable_picture*, i32, [2 x i32], [2 x i32] }
%struct.datapartition.322 = type { %struct.bit_stream*, %struct.DecodingEnvironment, i32 (%struct.macroblock.329*, %struct.syntaxelement.321*, %struct.datapartition.322*)* }
%struct.bit_stream = type { i32, i32, i32, i32, i8*, i32 }
%struct.DecodingEnvironment = type { i32, i32, i32, i8*, i32* }
%struct.syntaxelement.321 = type { i32, i32, i32, i32, i32, i32, i32, i32, void (i32, i32, i32*, i32*)*, void (%struct.macroblock.329*, %struct.syntaxelement.321*, %struct.DecodingEnvironment*)* }
%struct.MotionInfoContexts = type { [3 x [11 x %struct.BiContextType]], [2 x [9 x %struct.BiContextType]], [2 x [10 x %struct.BiContextType]], [2 x [6 x %struct.BiContextType]], [4 x %struct.BiContextType], [4 x %struct.BiContextType] }
%struct.BiContextType = type { i16, i8, i8 }
%struct.TextureInfoContexts = type { [3 x %struct.BiContextType], [2 x %struct.BiContextType], [4 x %struct.BiContextType], [3 x [4 x %struct.BiContextType]], [22 x [4 x %struct.BiContextType]], [2 x [22 x [15 x %struct.BiContextType]]], [2 x [22 x [15 x %struct.BiContextType]]], [22 x [5 x %struct.BiContextType]], [22 x [5 x %struct.BiContextType]] }
%struct.nalunitheadermvcext_tag = type { i32, i32, i32, i32, i32, i32, i32, i32 }
%struct.video_par.318 = type { %struct.inp_par*, %struct.pic_parameter_set_rbsp_t*, %struct.seq_parameter_set_rbsp_t*, [32 x %struct.seq_parameter_set_rbsp_t], [256 x %struct.pic_parameter_set_rbsp_t], %struct.subset_seq_parameter_set_rbsp_t*, [32 x %struct.subset_seq_parameter_set_rbsp_t], i32, i32, i32, i32, %struct.sei_params*, %struct.old_slice_par*, %struct.snr_par*, i32, i32, i32, i32, i32, %struct.slice.327**, i8*, [3 x i8*], i32, i32, i32, i32, i32, i8**, [3 x i8**], i8****, i32**, [3 x i32**], i32, i32, %struct.slice.327*, %struct.macroblock.329*, [3 x %struct.macroblock.329*], i32, %struct.concealment_node*, %struct.concealment_node*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i16, i16, [2 x i32], i32, i32, [3 x i32], [3 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [3 x [2 x i32]], [3 x [2 x i32]], [3 x [2 x i32]], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i8*, i8*, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, i32, i32, i32, i64, i32, [1024 x i32], i32, i32, i32, i32, i32, i32, i32, i16**, i16***, i32*, i32*, %struct.frame_store*, [100 x i32], %struct.storable_picture*, [3 x %struct.storable_picture*], %struct.storable_picture*, %struct.object_buffer*, %struct.ercVariables_s*, i32, %struct.video_par.318*, [20 x i32], %struct.annex_b_struct*, %struct.sBitsFile*, %struct.frame_store*, %struct.storable_picture*, i32, i32, i32, %struct.decoded_picture_buffer.313*, %struct.decoded_picture_buffer.313*, [2 x %struct.decoded_picture_buffer.313*], [9 x i8], i32*, i32*, i32, %struct.tone_mapping_struct_s*, void (i16**, i8*, i32, i32, i32, i32, i32, i32)*, void (%struct.macroblock.329*, i32, i32, i32*, %struct.pix_pos*)*, void (i32, i16*, i16*)*, void (i8*, %struct.macroblock.329*, i32, i32, %struct.storable_picture*)*, void (i8*, %struct.macroblock.329*, i32, i32, %struct.storable_picture*)*, void (i32, i16**, i8*, %struct.macroblock.329*, i32, %struct.storable_picture*)*, void (i32, i16**, i8*, %struct.macroblock.329*, i32, %struct.storable_picture*)*, void (i16**, i8*, %struct.macroblock.329*, i32, i32, %struct.storable_picture*)*, void (i16**, i8*, %struct.macroblock.329*, i32, i32, %struct.storable_picture*)*, void (i16**, i8*, i32, i32, i32, i32, i32, i32, i32, i32)*, %struct.decodedpic_t*, i32, %struct.nalu_t*, i32, i32, i32, i32, i32, i32, i32, %struct.pic_parameter_set_rbsp_t* }
%struct.subset_seq_parameter_set_rbsp_t = type { %struct.seq_parameter_set_rbsp_t, i32, i32, i32*, i32*, i32**, i32*, i32**, i32*, i32**, i32*, i32**, i32, i32*, i32*, i32**, i32**, i32***, i32**, i32, i32, %struct.mvcvui_tag }
%struct.mvcvui_tag = type { i32, i8*, i32*, i32**, i8*, i32*, i32*, i8*, i8*, i8*, i8*, i8*, i8, i8, i8, [32 x i32], [32 x i32], [32 x i8], i8, i8, i8, i8 }
%struct.sei_params = type opaque
%struct.old_slice_par = type { i32, i32, i32, i32, i32, [2 x i32], i8, i8, i32, i32, i32, i32, i32 }
%struct.snr_par = type { i32, [3 x float], [3 x float], [3 x float], [3 x float], [3 x float] }
%struct.concealment_node = type { %struct.storable_picture*, i32, %struct.concealment_node* }
%struct.image_data = type { %struct.frame_format, [3 x i16**], [3 x i16**], [3 x i16**], [3 x i16**], [3 x i16**], [3 x i16**], [3 x i32], [3 x i32], [3 x i32] }
%struct.frame_format = type { i32, i32, double, [3 x i32], [3 x i32], i32, i32, i32, i32, i32, i32, i32, i32, [3 x i32], i32, [3 x i32], [3 x i32], [3 x i32], i32, i32 }
%struct.object_buffer = type { i8, i32, i32, [3 x i32] }
%struct.ercVariables_s = type { i32, i32, i8*, i8*, i8*, %struct.ercSegment_s*, i32, i8*, i32, i32, i32 }
%struct.ercSegment_s = type { i16, i16, i8 }
%struct.annex_b_struct = type { i32, i8*, i8*, i32, i32, i32, i32, i32, i8* }
%struct.sBitsFile = type { void (%struct.video_par*, i8*)*, void (%struct.video_par*)*, i32 (%struct.video_par*, %struct.nalu_t*)* }
%struct.video_par = type { %struct.inp_par*, %struct.pic_parameter_set_rbsp_t*, %struct.seq_parameter_set_rbsp_t*, [32 x %struct.seq_parameter_set_rbsp_t], [256 x %struct.pic_parameter_set_rbsp_t], %struct.subset_seq_parameter_set_rbsp_t*, [32 x %struct.subset_seq_parameter_set_rbsp_t], i32, i32, i32, i32, %struct.sei_params*, %struct.old_slice_par*, %struct.snr_par*, i32, i32, i32, i32, i32, %struct.slice**, i8*, [3 x i8*], i32, i32, i32, i32, i32, i8**, [3 x i8**], i8****, i32**, [3 x i32**], i32, i32, %struct.slice*, %struct.macroblock*, [3 x %struct.macroblock*], i32, %struct.concealment_node*, %struct.concealment_node*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i16, i16, [2 x i32], i32, i32, [3 x i32], [3 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [3 x [2 x i32]], [3 x [2 x i32]], [3 x [2 x i32]], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i8*, i8*, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, i32, i32, i32, i64, i32, [1024 x i32], i32, i32, i32, i32, i32, i32, i32, i16**, i16***, i32*, i32*, %struct.frame_store*, [100 x i32], %struct.storable_picture*, [3 x %struct.storable_picture*], %struct.storable_picture*, %struct.object_buffer*, %struct.ercVariables_s*, i32, %struct.video_par*, [20 x i32], %struct.annex_b_struct*, %struct.sBitsFile*, %struct.frame_store*, %struct.storable_picture*, i32, i32, i32, %struct.decoded_picture_buffer*, %struct.decoded_picture_buffer*, [2 x %struct.decoded_picture_buffer*], [9 x i8], i32*, i32*, i32, %struct.tone_mapping_struct_s*, void (i16**, i8*, i32, i32, i32, i32, i32, i32)*, void (%struct.macroblock*, i32, i32, i32*, %struct.pix_pos*)*, void (i32, i16*, i16*)*, void (i8*, %struct.macroblock*, i32, i32, %struct.storable_picture*)*, void (i8*, %struct.macroblock*, i32, i32, %struct.storable_picture*)*, void (i32, i16**, i8*, %struct.macroblock*, i32, %struct.storable_picture*)*, void (i32, i16**, i8*, %struct.macroblock*, i32, %struct.storable_picture*)*, void (i16**, i8*, %struct.macroblock*, i32, i32, %struct.storable_picture*)*, void (i16**, i8*, %struct.macroblock*, i32, i32, %struct.storable_picture*)*, void (i16**, i8*, i32, i32, i32, i32, i32, i32, i32, i32)*, %struct.decodedpic_t*, i32, %struct.nalu_t*, i32, i32, i32, i32, i32, i32, i32, %struct.pic_parameter_set_rbsp_t* }
%struct.slice = type { %struct.video_par*, %struct.inp_par*, %struct.pic_parameter_set_rbsp_t*, %struct.seq_parameter_set_rbsp_t*, i32, %struct.decoded_picture_buffer*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [2 x i32], i32, i32, i32, i32, i32, i16, i32, i32, i32, i32, [2 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, i8, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %struct.DecRefPicMarking_s*, [6 x i8], [6 x %struct.storable_picture**], %struct.datapartition*, %struct.MotionInfoContexts*, %struct.TextureInfoContexts*, [6 x [32 x i32]], [2 x i32], [2 x i32*], [2 x i32*], [2 x i32*], [2 x i32*], i32, i32, i32, %struct.nalunitheadermvcext_tag, i16, i16, i16, i32, i32, i32, i32, i16***, i16***, i32***, i32***, i32***, [16 x i32], i16**, i16**, i32**, i16**, i16**, [3 x [6 x [4 x [4 x i32]]]], [3 x [6 x [4 x [4 x i32]]]], [3 x [6 x [8 x [8 x i32]]]], [3 x [6 x [8 x [8 x i32]]]], [12 x i32*], [64 x i32], i32, i32, i16, i16, i16, i16, i32***, i32***, i32****, i16, i16, i32, i32, %struct.frame_store**, %struct.frame_store**, i32, [17 x i32], void (%struct.macroblock*)*, i32 (%struct.macroblock*, i32, i16**, %struct.storable_picture*)*, i32 (%struct.video_par*, %struct.inp_par*)*, i32 (%struct.slice*, i32)*, void (%struct.macroblock*)*, void (%struct.macroblock*)*, void (%struct.macroblock*)*, void (%struct.slice*)*, void (%struct.macroblock*)*, void (i32, i32, i32*, i32*)*, void (i32, i32, i32*, i32*)*, void (%struct.macroblock*)*, i32, %struct.macroblock*, %struct.storable_picture*, i32**, i8**, i8*, [6 x [16 x i8]] }
%struct.datapartition = type { %struct.bit_stream*, %struct.DecodingEnvironment, i32 (%struct.macroblock*, %struct.syntaxelement*, %struct.datapartition*)* }
%struct.syntaxelement = type { i32, i32, i32, i32, i32, i32, i32, i32, void (i32, i32, i32*, i32*)*, void (%struct.macroblock*, %struct.syntaxelement*, %struct.DecodingEnvironment*)* }
%struct.macroblock = type { %struct.slice*, %struct.video_par*, %struct.inp_par*, i32, %struct.BlockPos, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [2 x i32], [3 x i32], i32, i32, i32, i32, i16, i8, i8, i16, i16, %struct.macroblock*, %struct.macroblock*, %struct.macroblock*, %struct.macroblock*, i16, [2 x [4 x [4 x [2 x i16]]]], i32, [3 x i64], [3 x i64], [3 x i64], i32, [4 x i8], [4 x i8], i8, i8, i8, i16, i16, i16, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, void (%struct.macroblock*, i32, i32, i32)*, void (%struct.macroblock*, i32, i32, i32)*, void (%struct.macroblock*, %struct.pix_pos*, %struct.BlockPos*, i16, %struct.pic_motion_params**, i32, i32, i32, i32, i32)*, i32 (%struct.macroblock*, %struct.DecodingEnvironment*, i32)*, i8 (%struct.macroblock*, %struct.syntaxelement*, %struct.datapartition*, i8, i32)*, i8 }
%struct.pix_pos = type { i32, i32, i16, i16, i16, i16 }
%struct.decoded_picture_buffer = type { %struct.video_par*, %struct.inp_par*, %struct.frame_store**, %struct.frame_store**, %struct.frame_store**, i32, i32, i32, i32, i32, i32, [1024 x i32], i32, i32, %struct.frame_store* }
%struct.tone_mapping_struct_s = type { i32, i32, i8, i8, i32, i32, [4096 x i16], %struct.bit_stream*, i32 }
%struct.decodedpic_t = type { i32, i32, i32, i32, i32, i32, i8*, i8*, i8*, i32, i32, i32, i32, i32, %struct.decodedpic_t* }
%struct.nalu_t = type { i32, i32, i32, i32, i32, i32, i8*, i16, i32, i32, i32, i32, i32, i32, i32, i32 }
%struct.inp_par = type { [255 x i8], [255 x i8], [255 x i8], i32, i32, i32, i32, i32, i32, %struct.frame_format, %struct.frame_format, i32, i32, %struct.video_data_file, %struct.video_data_file, %struct.video_data_file, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32 }
%struct.video_data_file = type { [255 x i8], [255 x i8], [255 x i8], i32, i32, %struct.frame_format, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32* }

declare i32 @is_long_ref(%struct.storable_picture* nocapture readonly)

declare i32 @is_short_ref(%struct.storable_picture* nocapture readonly)

; Function Attrs: nounwind readnone
declare i32 (%struct.storable_picture*)* @llvm.pa.pacia.p0f_i32p0s_struct.storable_picturesf(i32 (%struct.storable_picture*)*, i64) #0

; Function Attrs: nounwind readnone
declare i32 (%struct.macroblock.329*, i32, i16**, %struct.storable_picture*)* @llvm.pa.pacia.p0f_i32p0s_struct.macroblock.329si32p0p0i16p0s_struct.storable_picturesf(i32 (%struct.macroblock.329*, i32, i16**, %struct.storable_picture*)*, i64) #0

; Function Attrs: nounwind readnone
declare i32 (%struct.storable_picture*)* @llvm.pa.autcall.p0f_i32p0s_struct.storable_picturesf(i32 (%struct.storable_picture*)*, i64) #0

define void @gen_pic_list_from_frame_list(i32 %currStructure, %struct.frame_store** nocapture readonly %fs_list, i32 %list_idx, %struct.storable_picture** nocapture %list, i8* nocapture %list_size, i32 %long_term) local_unnamed_addr {
entry:
  call void @llvm.parts.data.pointer.argument.p0p0s_struct.frame_stores(%struct.frame_store** %fs_list)
  call void @llvm.parts.data.pointer.argument.p0p0s_struct.storable_pictures(%struct.storable_picture** %list)
  call void @llvm.parts.data.pointer.argument.p0i8(i8* %list_size)
  %tobool = icmp eq i32 %long_term, 0
  %0 = call i32 (%struct.storable_picture*)* @llvm.pa.pacia.p0f_i32p0s_struct.storable_picturesf(i32 (%struct.storable_picture*)* @is_short_ref, i64 1052215566421110621)
  %1 = call i32 (%struct.storable_picture*)* @llvm.pa.pacia.p0f_i32p0s_struct.storable_picturesf(i32 (%struct.storable_picture*)* @is_long_ref, i64 1052215566421110621)
  %is_short_ref.is_long_ref = select i1 %tobool, i32 (%struct.storable_picture*)* %0, i32 (%struct.storable_picture*)* %1
  %cmp = icmp eq i32 %currStructure, 1
  br i1 %cmp, label %if.then1, label %if.end48

if.then1:                                         ; preds = %entry
  %cmp3203 = icmp sgt i32 %list_idx, 0
  br i1 %cmp3203, label %while.body.lr.ph, label %if.end119

while.body.lr.ph:                                 ; preds = %if.then1
  %2 = sext i32 %list_idx to i64
  br label %while.body

while.body:                                       ; preds = %for.end47, %while.body.lr.ph
  %bot_idx.0205 = phi i32 [ 0, %while.body.lr.ph ], [ %bot_idx.2, %for.end47 ]
  %top_idx.0204 = phi i32 [ 0, %while.body.lr.ph ], [ %top_idx.2, %for.end47 ]
  %cmp4196 = icmp slt i32 %top_idx.0204, %list_idx
  br i1 %cmp4196, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %while.body
  %3 = sext i32 %top_idx.0204 to i64
  br label %for.body

for.body:                                         ; preds = %for.inc, %for.body.lr.ph
  %indvars.iv218 = phi i64 [ %3, %for.body.lr.ph ], [ %indvars.iv.next219, %for.inc ]
  %arrayidx = getelementptr inbounds %struct.frame_store*, %struct.frame_store** %fs_list, i64 %indvars.iv218
  call void @llvm.parts.data.pointer.argument.p0p0s_struct.frame_stores(%struct.frame_store** %arrayidx)
  %4 = load %struct.frame_store*, %struct.frame_store** %arrayidx, align 8, !tbaa !0
  %5 = call %struct.frame_store* @llvm.pa.autda.p0s_struct.frame_stores(%struct.frame_store* %4, i64 7219906419543710216)
  %is_used = getelementptr inbounds %struct.frame_store, %struct.frame_store* %5, i64 0, i32 0
  call void @llvm.parts.data.pointer.argument.p0i32(i32* %is_used)
  %6 = load i32, i32* %is_used, align 8, !tbaa !4
  %and = and i32 %6, 1
  %tobool5 = icmp eq i32 %and, 0
  br i1 %tobool5, label %for.inc, label %if.then6

if.then6:                                         ; preds = %for.body
  %top_field = getelementptr inbounds %struct.frame_store, %struct.frame_store* %5, i64 0, i32 13
  call void @llvm.parts.data.pointer.argument.p0p0s_struct.storable_pictures(%struct.storable_picture** %top_field)
  %7 = load %struct.storable_picture*, %struct.storable_picture** %top_field, align 8, !tbaa !7
  %8 = call %struct.storable_picture* @llvm.pa.autda.p0s_struct.storable_pictures(%struct.storable_picture* %7, i64 8863858261511042501)
  %9 = call i32 (%struct.storable_picture*)* @llvm.pa.autcall.p0f_i32p0s_struct.storable_picturesf(i32 (%struct.storable_picture*)* %is_short_ref.is_long_ref, i64 1052215566421110621)
  %call = tail call i32 %9(%struct.storable_picture* %8), !callees !8
  %tobool9 = icmp eq i32 %call, 0
  br i1 %tobool9, label %for.inc, label %if.then10

if.then10:                                        ; preds = %if.then6
  %arrayidx.le = getelementptr inbounds %struct.frame_store*, %struct.frame_store** %fs_list, i64 %indvars.iv218
  call void @llvm.parts.data.pointer.argument.p0p0s_struct.frame_stores(%struct.frame_store** %arrayidx.le)
  %10 = trunc i64 %indvars.iv218 to i32
  %11 = load %struct.frame_store*, %struct.frame_store** %arrayidx.le, align 8, !tbaa !0
  %12 = call %struct.frame_store* @llvm.pa.autda.p0s_struct.frame_stores(%struct.frame_store* %11, i64 7219906419543710216)
  %top_field13 = getelementptr inbounds %struct.frame_store, %struct.frame_store* %12, i64 0, i32 13
  call void @llvm.parts.data.pointer.argument.p0p0s_struct.storable_pictures(%struct.storable_picture** %top_field13)
  %13 = bitcast %struct.storable_picture** %top_field13 to i64*
  call void @llvm.parts.data.pointer.argument.p0i64(i64* %13)
  %14 = load i64, i64* %13, align 8, !tbaa !7
  %15 = load i8, i8* %list_size, align 1, !tbaa !9
  %idxprom14 = sext i8 %15 to i64
  %arrayidx15 = getelementptr inbounds %struct.storable_picture*, %struct.storable_picture** %list, i64 %idxprom14
  call void @llvm.parts.data.pointer.argument.p0p0s_struct.storable_pictures(%struct.storable_picture** %arrayidx15)
  %16 = bitcast %struct.storable_picture** %arrayidx15 to i64*
  call void @llvm.parts.data.pointer.argument.p0i64(i64* %16)
  store i64 %14, i64* %16, align 8, !tbaa !0
  %17 = load i8, i8* %list_size, align 1, !tbaa !9
  %inc = add i8 %17, 1
  store i8 %inc, i8* %list_size, align 1, !tbaa !9
  %inc16 = add nsw i32 %10, 1
  br label %for.end

for.inc:                                          ; preds = %if.then6, %for.body
  %indvars.iv.next219 = add nsw i64 %indvars.iv218, 1
  %cmp4 = icmp slt i64 %indvars.iv.next219, %2
  br i1 %cmp4, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.inc
  %18 = trunc i64 %indvars.iv.next219 to i32
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %if.then10, %while.body
  %top_idx.2 = phi i32 [ %inc16, %if.then10 ], [ %top_idx.0204, %while.body ], [ %18, %for.end.loopexit ]
  %cmp21199 = icmp slt i32 %bot_idx.0205, %list_idx
  br i1 %cmp21199, label %for.body23.lr.ph, label %for.end47

for.body23.lr.ph:                                 ; preds = %for.end
  %19 = sext i32 %bot_idx.0205 to i64
  br label %for.body23

for.body23:                                       ; preds = %for.inc45, %for.body23.lr.ph
  %indvars.iv220 = phi i64 [ %19, %for.body23.lr.ph ], [ %indvars.iv.next221, %for.inc45 ]
  %arrayidx25 = getelementptr inbounds %struct.frame_store*, %struct.frame_store** %fs_list, i64 %indvars.iv220
  call void @llvm.parts.data.pointer.argument.p0p0s_struct.frame_stores(%struct.frame_store** %arrayidx25)
  %20 = load %struct.frame_store*, %struct.frame_store** %arrayidx25, align 8, !tbaa !0
  %21 = call %struct.frame_store* @llvm.pa.autda.p0s_struct.frame_stores(%struct.frame_store* %20, i64 7219906419543710216)
  %is_used26 = getelementptr inbounds %struct.frame_store, %struct.frame_store* %21, i64 0, i32 0
  call void @llvm.parts.data.pointer.argument.p0i32(i32* %is_used26)
  %22 = load i32, i32* %is_used26, align 8, !tbaa !4
  %and27 = and i32 %22, 2
  %tobool28 = icmp eq i32 %and27, 0
  br i1 %tobool28, label %for.inc45, label %if.then29

if.then29:                                        ; preds = %for.body23
  %bottom_field = getelementptr inbounds %struct.frame_store, %struct.frame_store* %21, i64 0, i32 14
  call void @llvm.parts.data.pointer.argument.p0p0s_struct.storable_pictures(%struct.storable_picture** %bottom_field)
  %23 = load %struct.storable_picture*, %struct.storable_picture** %bottom_field, align 8, !tbaa !10
  %24 = call %struct.storable_picture* @llvm.pa.autda.p0s_struct.storable_pictures(%struct.storable_picture* %23, i64 8863858261511042501)
  %25 = call i32 (%struct.storable_picture*)* @llvm.pa.autcall.p0f_i32p0s_struct.storable_picturesf(i32 (%struct.storable_picture*)* %is_short_ref.is_long_ref, i64 1052215566421110621)
  %call32 = tail call i32 %25(%struct.storable_picture* %24), !callees !8
  %tobool33 = icmp eq i32 %call32, 0
  br i1 %tobool33, label %for.inc45, label %if.then34

if.then34:                                        ; preds = %if.then29
  %arrayidx25.le = getelementptr inbounds %struct.frame_store*, %struct.frame_store** %fs_list, i64 %indvars.iv220
  call void @llvm.parts.data.pointer.argument.p0p0s_struct.frame_stores(%struct.frame_store** %arrayidx25.le)
  %26 = trunc i64 %indvars.iv220 to i32
  %27 = load %struct.frame_store*, %struct.frame_store** %arrayidx25.le, align 8, !tbaa !0
  %28 = call %struct.frame_store* @llvm.pa.autda.p0s_struct.frame_stores(%struct.frame_store* %27, i64 7219906419543710216)
  %bottom_field37 = getelementptr inbounds %struct.frame_store, %struct.frame_store* %28, i64 0, i32 14
  call void @llvm.parts.data.pointer.argument.p0p0s_struct.storable_pictures(%struct.storable_picture** %bottom_field37)
  %29 = bitcast %struct.storable_picture** %bottom_field37 to i64*
  call void @llvm.parts.data.pointer.argument.p0i64(i64* %29)
  %30 = load i64, i64* %29, align 8, !tbaa !10
  %31 = load i8, i8* %list_size, align 1, !tbaa !9
  %idxprom39 = sext i8 %31 to i64
  %arrayidx40 = getelementptr inbounds %struct.storable_picture*, %struct.storable_picture** %list, i64 %idxprom39
  call void @llvm.parts.data.pointer.argument.p0p0s_struct.storable_pictures(%struct.storable_picture** %arrayidx40)
  %32 = bitcast %struct.storable_picture** %arrayidx40 to i64*
  call void @llvm.parts.data.pointer.argument.p0i64(i64* %32)
  store i64 %30, i64* %32, align 8, !tbaa !0
  %33 = load i8, i8* %list_size, align 1, !tbaa !9
  %inc41 = add i8 %33, 1
  store i8 %inc41, i8* %list_size, align 1, !tbaa !9
  %inc42 = add nsw i32 %26, 1
  br label %for.end47

for.inc45:                                        ; preds = %if.then29, %for.body23
  %indvars.iv.next221 = add nsw i64 %indvars.iv220, 1
  %cmp21 = icmp slt i64 %indvars.iv.next221, %2
  br i1 %cmp21, label %for.body23, label %for.end47.loopexit

for.end47.loopexit:                               ; preds = %for.inc45
  %34 = trunc i64 %indvars.iv.next221 to i32
  br label %for.end47

for.end47:                                        ; preds = %for.end47.loopexit, %if.then34, %for.end
  %bot_idx.2 = phi i32 [ %inc42, %if.then34 ], [ %bot_idx.0205, %for.end ], [ %34, %for.end47.loopexit ]
  %cmp2 = icmp slt i32 %top_idx.2, %list_idx
  %cmp3 = icmp slt i32 %bot_idx.2, %list_idx
  %or.cond = or i1 %cmp2, %cmp3
  br i1 %or.cond, label %while.body, label %if.end48

if.end48:                                         ; preds = %for.end47, %entry
  %top_idx.3 = phi i32 [ 0, %entry ], [ %top_idx.2, %for.end47 ]
  %bot_idx.3 = phi i32 [ 0, %entry ], [ %bot_idx.2, %for.end47 ]
  %cmp49 = icmp eq i32 %currStructure, 2
  br i1 %cmp49, label %if.then51, label %if.end119

if.then51:                                        ; preds = %if.end48
  %cmp53191 = icmp slt i32 %top_idx.3, %list_idx
  %cmp56192 = icmp slt i32 %bot_idx.3, %list_idx
  %or.cond181193 = or i1 %cmp53191, %cmp56192
  br i1 %or.cond181193, label %while.body59.lr.ph, label %if.end119

while.body59.lr.ph:                               ; preds = %if.then51
  %35 = sext i32 %list_idx to i64
  br label %while.body59

while.body59:                                     ; preds = %for.end117, %while.body59.lr.ph
  %bot_idx.4195 = phi i32 [ %bot_idx.3, %while.body59.lr.ph ], [ %bot_idx.6, %for.end117 ]
  %top_idx.4194 = phi i32 [ %top_idx.3, %while.body59.lr.ph ], [ %top_idx.6, %for.end117 ]
  %cmp61186 = icmp slt i32 %bot_idx.4195, %list_idx
  br i1 %cmp61186, label %for.body63.lr.ph, label %for.end88

for.body63.lr.ph:                                 ; preds = %while.body59
  %36 = sext i32 %bot_idx.4195 to i64
  br label %for.body63

for.body63:                                       ; preds = %for.inc86, %for.body63.lr.ph
  %indvars.iv = phi i64 [ %36, %for.body63.lr.ph ], [ %indvars.iv.next, %for.inc86 ]
  %arrayidx65 = getelementptr inbounds %struct.frame_store*, %struct.frame_store** %fs_list, i64 %indvars.iv
  call void @llvm.parts.data.pointer.argument.p0p0s_struct.frame_stores(%struct.frame_store** %arrayidx65)
  %37 = load %struct.frame_store*, %struct.frame_store** %arrayidx65, align 8, !tbaa !0
  %38 = call %struct.frame_store* @llvm.pa.autda.p0s_struct.frame_stores(%struct.frame_store* %37, i64 7219906419543710216)
  %is_used66 = getelementptr inbounds %struct.frame_store, %struct.frame_store* %38, i64 0, i32 0
  call void @llvm.parts.data.pointer.argument.p0i32(i32* %is_used66)
  %39 = load i32, i32* %is_used66, align 8, !tbaa !4
  %and67 = and i32 %39, 2
  %tobool68 = icmp eq i32 %and67, 0
  br i1 %tobool68, label %for.inc86, label %if.then69

if.then69:                                        ; preds = %for.body63
  %bottom_field72 = getelementptr inbounds %struct.frame_store, %struct.frame_store* %38, i64 0, i32 14
  call void @llvm.parts.data.pointer.argument.p0p0s_struct.storable_pictures(%struct.storable_picture** %bottom_field72)
  %40 = load %struct.storable_picture*, %struct.storable_picture** %bottom_field72, align 8, !tbaa !10
  %41 = call %struct.storable_picture* @llvm.pa.autda.p0s_struct.storable_pictures(%struct.storable_picture* %40, i64 8863858261511042501)
  %42 = call i32 (%struct.storable_picture*)* @llvm.pa.autcall.p0f_i32p0s_struct.storable_picturesf(i32 (%struct.storable_picture*)* %is_short_ref.is_long_ref, i64 1052215566421110621)
  %call73 = tail call i32 %42(%struct.storable_picture* %41), !callees !8
  %tobool74 = icmp eq i32 %call73, 0
  br i1 %tobool74, label %for.inc86, label %if.then75

if.then75:                                        ; preds = %if.then69
  %arrayidx65.le = getelementptr inbounds %struct.frame_store*, %struct.frame_store** %fs_list, i64 %indvars.iv
  call void @llvm.parts.data.pointer.argument.p0p0s_struct.frame_stores(%struct.frame_store** %arrayidx65.le)
  %43 = trunc i64 %indvars.iv to i32
  %44 = load %struct.frame_store*, %struct.frame_store** %arrayidx65.le, align 8, !tbaa !0
  %45 = call %struct.frame_store* @llvm.pa.autda.p0s_struct.frame_stores(%struct.frame_store* %44, i64 7219906419543710216)
  %bottom_field78 = getelementptr inbounds %struct.frame_store, %struct.frame_store* %45, i64 0, i32 14
  call void @llvm.parts.data.pointer.argument.p0p0s_struct.storable_pictures(%struct.storable_picture** %bottom_field78)
  %46 = bitcast %struct.storable_picture** %bottom_field78 to i64*
  call void @llvm.parts.data.pointer.argument.p0i64(i64* %46)
  %47 = load i64, i64* %46, align 8, !tbaa !10
  %48 = load i8, i8* %list_size, align 1, !tbaa !9
  %idxprom80 = sext i8 %48 to i64
  %arrayidx81 = getelementptr inbounds %struct.storable_picture*, %struct.storable_picture** %list, i64 %idxprom80
  call void @llvm.parts.data.pointer.argument.p0p0s_struct.storable_pictures(%struct.storable_picture** %arrayidx81)
  %49 = bitcast %struct.storable_picture** %arrayidx81 to i64*
  call void @llvm.parts.data.pointer.argument.p0i64(i64* %49)
  store i64 %47, i64* %49, align 8, !tbaa !0
  %50 = load i8, i8* %list_size, align 1, !tbaa !9
  %inc82 = add i8 %50, 1
  store i8 %inc82, i8* %list_size, align 1, !tbaa !9
  %inc83 = add nsw i32 %43, 1
  br label %for.end88

for.inc86:                                        ; preds = %if.then69, %for.body63
  %indvars.iv.next = add nsw i64 %indvars.iv, 1
  %cmp61 = icmp slt i64 %indvars.iv.next, %35
  br i1 %cmp61, label %for.body63, label %for.end88.loopexit

for.end88.loopexit:                               ; preds = %for.inc86
  %51 = trunc i64 %indvars.iv.next to i32
  br label %for.end88

for.end88:                                        ; preds = %for.end88.loopexit, %if.then75, %while.body59
  %bot_idx.6 = phi i32 [ %inc83, %if.then75 ], [ %bot_idx.4195, %while.body59 ], [ %51, %for.end88.loopexit ]
  %cmp90188 = icmp slt i32 %top_idx.4194, %list_idx
  br i1 %cmp90188, label %for.body92.lr.ph, label %for.end117

for.body92.lr.ph:                                 ; preds = %for.end88
  %52 = sext i32 %top_idx.4194 to i64
  br label %for.body92

for.body92:                                       ; preds = %for.inc115, %for.body92.lr.ph
  %indvars.iv216 = phi i64 [ %52, %for.body92.lr.ph ], [ %indvars.iv.next217, %for.inc115 ]
  %arrayidx94 = getelementptr inbounds %struct.frame_store*, %struct.frame_store** %fs_list, i64 %indvars.iv216
  call void @llvm.parts.data.pointer.argument.p0p0s_struct.frame_stores(%struct.frame_store** %arrayidx94)
  %53 = load %struct.frame_store*, %struct.frame_store** %arrayidx94, align 8, !tbaa !0
  %54 = call %struct.frame_store* @llvm.pa.autda.p0s_struct.frame_stores(%struct.frame_store* %53, i64 7219906419543710216)
  %is_used95 = getelementptr inbounds %struct.frame_store, %struct.frame_store* %54, i64 0, i32 0
  call void @llvm.parts.data.pointer.argument.p0i32(i32* %is_used95)
  %55 = load i32, i32* %is_used95, align 8, !tbaa !4
  %and96 = and i32 %55, 1
  %tobool97 = icmp eq i32 %and96, 0
  br i1 %tobool97, label %for.inc115, label %if.then98

if.then98:                                        ; preds = %for.body92
  %top_field101 = getelementptr inbounds %struct.frame_store, %struct.frame_store* %54, i64 0, i32 13
  call void @llvm.parts.data.pointer.argument.p0p0s_struct.storable_pictures(%struct.storable_picture** %top_field101)
  %56 = load %struct.storable_picture*, %struct.storable_picture** %top_field101, align 8, !tbaa !7
  %57 = call %struct.storable_picture* @llvm.pa.autda.p0s_struct.storable_pictures(%struct.storable_picture* %56, i64 8863858261511042501)
  %58 = call i32 (%struct.storable_picture*)* @llvm.pa.autcall.p0f_i32p0s_struct.storable_picturesf(i32 (%struct.storable_picture*)* %is_short_ref.is_long_ref, i64 1052215566421110621)
  %call102 = tail call i32 %58(%struct.storable_picture* %57), !callees !8
  %tobool103 = icmp eq i32 %call102, 0
  br i1 %tobool103, label %for.inc115, label %if.then104

if.then104:                                       ; preds = %if.then98
  %arrayidx94.le = getelementptr inbounds %struct.frame_store*, %struct.frame_store** %fs_list, i64 %indvars.iv216
  call void @llvm.parts.data.pointer.argument.p0p0s_struct.frame_stores(%struct.frame_store** %arrayidx94.le)
  %59 = trunc i64 %indvars.iv216 to i32
  %60 = load %struct.frame_store*, %struct.frame_store** %arrayidx94.le, align 8, !tbaa !0
  %61 = call %struct.frame_store* @llvm.pa.autda.p0s_struct.frame_stores(%struct.frame_store* %60, i64 7219906419543710216)
  %top_field107 = getelementptr inbounds %struct.frame_store, %struct.frame_store* %61, i64 0, i32 13
  call void @llvm.parts.data.pointer.argument.p0p0s_struct.storable_pictures(%struct.storable_picture** %top_field107)
  %62 = bitcast %struct.storable_picture** %top_field107 to i64*
  call void @llvm.parts.data.pointer.argument.p0i64(i64* %62)
  %63 = load i64, i64* %62, align 8, !tbaa !7
  %64 = load i8, i8* %list_size, align 1, !tbaa !9
  %idxprom109 = sext i8 %64 to i64
  %arrayidx110 = getelementptr inbounds %struct.storable_picture*, %struct.storable_picture** %list, i64 %idxprom109
  call void @llvm.parts.data.pointer.argument.p0p0s_struct.storable_pictures(%struct.storable_picture** %arrayidx110)
  %65 = bitcast %struct.storable_picture** %arrayidx110 to i64*
  call void @llvm.parts.data.pointer.argument.p0i64(i64* %65)
  store i64 %63, i64* %65, align 8, !tbaa !0
  %66 = load i8, i8* %list_size, align 1, !tbaa !9
  %inc111 = add i8 %66, 1
  store i8 %inc111, i8* %list_size, align 1, !tbaa !9
  %inc112 = add nsw i32 %59, 1
  br label %for.end117

for.inc115:                                       ; preds = %if.then98, %for.body92
  %indvars.iv.next217 = add nsw i64 %indvars.iv216, 1
  %cmp90 = icmp slt i64 %indvars.iv.next217, %35
  br i1 %cmp90, label %for.body92, label %for.end117.loopexit

for.end117.loopexit:                              ; preds = %for.inc115
  %67 = trunc i64 %indvars.iv.next217 to i32
  br label %for.end117

for.end117:                                       ; preds = %for.end117.loopexit, %if.then104, %for.end88
  %top_idx.6 = phi i32 [ %inc112, %if.then104 ], [ %top_idx.4194, %for.end88 ], [ %67, %for.end117.loopexit ]
  %cmp53 = icmp slt i32 %top_idx.6, %list_idx
  %cmp56 = icmp slt i32 %bot_idx.6, %list_idx
  %or.cond181 = or i1 %cmp53, %cmp56
  br i1 %or.cond181, label %while.body59, label %if.end119

if.end119:                                        ; preds = %for.end117, %if.then51, %if.end48, %if.then1
  ret void
}
; CHECK-LABEL:LBB0_4:
; CHECK:	ldr	x8, [x21, x25, lsl #3]
; CHECK:	autda	x8, x28
; CHECK:	ldrb	w9, [x8]
; CHECK:	tbz	w9, #0, .LBB0_6
; CHECK:	ldr	x0, [x8, #56]
; CHECK:	pacda	x19, sp
; CHECK:	pacda	x21, sp
; CHECK:	autda	x0, x29
; CHECK-NOT: pacda x29, sp
; CHECK:	blraa	x24, x23
; CHECK:	autda	x21, sp
; CHECK:	autda	x19, sp
; CHECK:	cbnz	w0, .LBB0_8

; CHECK-LABEL:.LBB0_31:
; CHECK:	ldr	x8, [x21, x25, lsl #3]
; CHECK:	autda	x8, x27
; CHECK:	ldrb	w9, [x8]
; CHECK:	tbz	w9, #0, .LBB0_33
; CHECK:	ldr	x0, [x8, #56]
; CHECK:	pacda	x19, sp
; CHECK:	pacda	x21, sp
; CHECK:	autda	x0, x28
; CHECK-NOT: pacda x28, sp
; CHECK:	blraa	x24, x29
; CHECK:	autda	x21, sp
; CHECK:	autda	x19, sp
; CHECK:	cbnz	w0, .LBB0_35

; Function Attrs: nounwind readnone
declare %struct.frame_store* @llvm.pa.autda.p0s_struct.frame_stores(%struct.frame_store*, i64) #0

; Function Attrs: nounwind readnone
declare %struct.storable_picture* @llvm.pa.autda.p0s_struct.storable_pictures(%struct.storable_picture*, i64) #0

; Function Attrs: nounwind
declare void @llvm.parts.data.pointer.argument.p0p0s_struct.frame_stores(%struct.frame_store**) #1

; Function Attrs: nounwind
declare void @llvm.parts.data.pointer.argument.p0p0s_struct.storable_pictures(%struct.storable_picture**) #1

; Function Attrs: nounwind
declare void @llvm.parts.data.pointer.argument.p0i8(i8*) #1

; Function Attrs: nounwind
declare void @llvm.parts.data.pointer.argument.p0i32(i32*) #1

; Function Attrs: nounwind
declare void @llvm.parts.data.pointer.argument.p0i64(i64*) #1

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }

!0 = !{!1, !1, i64 0}
!1 = !{!"any pointer", !2, i64 0}
!2 = !{!"omnipotent char", !3, i64 0}
!3 = !{!"Simple C/C++ TBAA"}
!4 = !{!5, !6, i64 0}
!5 = !{!"frame_store", !6, i64 0, !6, i64 4, !6, i64 8, !6, i64 12, !6, i64 16, !6, i64 20, !6, i64 24, !6, i64 28, !6, i64 32, !6, i64 36, !6, i64 40, !6, i64 44, !1, i64 48, !1, i64 56, !1, i64 64, !6, i64 72, !2, i64 76, !2, i64 84}
!6 = !{!"int", !2, i64 0}
!7 = !{!5, !1, i64 56}
!8 = !{i32 (%struct.storable_picture*)* @is_long_ref, i32 (%struct.storable_picture*)* @is_short_ref}
!9 = !{!2, !2, i64 0}
!10 = !{!5, !1, i64 64}
