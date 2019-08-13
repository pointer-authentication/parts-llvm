; RUN: llc -verify-machineinstrs -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-becfi=full -parts-fecfi < %s | FileCheck %s
;
; This triggers a bug with autcall - blr coupling, that causes a compile-time llvm_unreachable
; crash in AArchPartsCpiPass.
;

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }
%struct.inp_par = type { [255 x i8], [255 x i8], [255 x i8], i32, i32, i32, i32, i32, i32, %struct.frame_format, %struct.frame_format, i32, i32, %struct.video_data_file, %struct.video_data_file, %struct.video_data_file, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32 }
%struct.frame_format = type { i32, i32, double, [3 x i32], [3 x i32], i32, i32, i32, i32, i32, i32, i32, i32, [3 x i32], i32, [3 x i32], [3 x i32], [3 x i32], i32, i32 }
%struct.video_data_file = type { [255 x i8], [255 x i8], [255 x i8], i32, i32, %struct.frame_format, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32* }
%struct.Mapping = type { i8*, i8*, i32, double, i32, double, double, i32 }
%struct.BlockPos = type { i16, i16 }
%struct.Tiff = type { i16*, i8*, i8*, i32, i32, %struct.TiffImageFileHeader, i16, [3 x i32], i32, i32, i32, [1080 x i32], [1080 x i32], [2 x i32], [2 x i32], i32 (%struct.Tiff*)*, i32 (%struct.Tiff*)* }
%struct.TiffImageFileHeader = type { i16, i16, i32 }
%struct.decoder_params.1072 = type { %struct.inp_par*, %struct.video_par.1071*, i64, i32, %struct._IO_FILE*, i32 }
%struct.video_par.1071 = type { %struct.inp_par*, %struct.pic_parameter_set_rbsp_t*, %struct.seq_parameter_set_rbsp_t*, [32 x %struct.seq_parameter_set_rbsp_t], [256 x %struct.pic_parameter_set_rbsp_t], %struct.subset_seq_parameter_set_rbsp_t*, [32 x %struct.subset_seq_parameter_set_rbsp_t], i32, i32, i32, i32, %struct.sei_params*, %struct.old_slice_par*, %struct.snr_par*, i32, i32, i32, i32, i32, %struct.slice.1060**, i8*, [3 x i8*], i32, i32, i32, i32, i32, i8**, [3 x i8**], i8****, i32**, [3 x i32**], i32, i32, %struct.slice.1060*, %struct.macroblock.1054*, [3 x %struct.macroblock.1054*], i32, %struct.concealment_node*, %struct.concealment_node*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i16, i16, [2 x i32], i32, i32, [3 x i32], [3 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [3 x [2 x i32]], [3 x [2 x i32]], [3 x [2 x i32]], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i8*, i8*, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, i32, i32, i32, i64, i32, [1024 x i32], i32, i32, i32, i32, i32, i32, i32, i16**, i16***, i32*, i32*, %struct.frame_store*, [100 x i32], %struct.storable_picture*, [3 x %struct.storable_picture*], %struct.storable_picture*, %struct.object_buffer*, %struct.ercVariables_s*, i32, %struct.video_par.1071*, [20 x i32], %struct.annex_b_struct*, %struct.sBitsFile.1068*, %struct.frame_store*, %struct.storable_picture*, i32, i32, i32, %struct.decoded_picture_buffer.1049*, %struct.decoded_picture_buffer.1049*, [2 x %struct.decoded_picture_buffer.1049*], [9 x i8], i32*, i32*, i32, %struct.tone_mapping_struct_s*, void (i16**, i8*, i32, i32, i32, i32, i32, i32)*, void (%struct.macroblock.1054*, i32, i32, i32*, %struct.pix_pos*)*, void (i32, i16*, i16*)*, void (i8*, %struct.macroblock.1054*, i32, i32, %struct.storable_picture*)*, void (i8*, %struct.macroblock.1054*, i32, i32, %struct.storable_picture*)*, void (i32, i16**, i8*, %struct.macroblock.1054*, i32, %struct.storable_picture*)*, void (i32, i16**, i8*, %struct.macroblock.1054*, i32, %struct.storable_picture*)*, void (i16**, i8*, %struct.macroblock.1054*, i32, i32, %struct.storable_picture*)*, void (i16**, i8*, %struct.macroblock.1054*, i32, i32, %struct.storable_picture*)*, void (i16**, i8*, i32, i32, i32, i32, i32, i32, i32, i32)*, %struct.decodedpic_t*, i32, %struct.nalu_t*, i32, i32, i32, i32, i32, i32, i32, %struct.pic_parameter_set_rbsp_t* }
%struct.seq_parameter_set_rbsp_t = type { i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [12 x i32], [6 x [16 x i32]], [6 x [64 x i32]], [6 x i32], [6 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, [256 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %struct.vui_seq_parameters_t, i32, i32 }
%struct.vui_seq_parameters_t = type { i32, i32, i16, i16, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %struct.hrd_parameters_t, i32, %struct.hrd_parameters_t, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32 }
%struct.hrd_parameters_t = type { i32, i32, i32, [32 x i32], [32 x i32], [32 x i32], i32, i32, i32, i32 }
%struct.pic_parameter_set_rbsp_t = type { i32, i32, i32, i32, i32, i32, [12 x i32], [6 x [16 x i32]], [6 x [64 x i32]], [6 x i32], [6 x i32], i32, i32, i32, [8 x i32], [8 x i32], [8 x i32], i32, i32, i32, i8*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32 }
%struct.subset_seq_parameter_set_rbsp_t = type { %struct.seq_parameter_set_rbsp_t, i32, i32, i32*, i32*, i32**, i32*, i32**, i32*, i32**, i32*, i32**, i32, i32*, i32*, i32**, i32**, i32***, i32**, i32, i32, %struct.mvcvui_tag }
%struct.mvcvui_tag = type { i32, i8*, i32*, i32**, i8*, i32*, i32*, i8*, i8*, i8*, i8*, i8*, i8, i8, i8, [32 x i32], [32 x i32], [32 x i8], i8, i8, i8, i8 }
%struct.sei_params = type opaque
%struct.old_slice_par = type { i32, i32, i32, i32, i32, [2 x i32], i8, i8, i32, i32, i32, i32, i32 }
%struct.snr_par = type { i32, [3 x float], [3 x float], [3 x float], [3 x float], [3 x float] }
%struct.slice.1060 = type { %struct.video_par.1071*, %struct.inp_par*, %struct.pic_parameter_set_rbsp_t*, %struct.seq_parameter_set_rbsp_t*, i32, %struct.decoded_picture_buffer.1049*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [2 x i32], i32, i32, i32, i32, i32, i16, i32, i32, i32, i32, [2 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, i8, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %struct.DecRefPicMarking_s*, [6 x i8], [6 x %struct.storable_picture**], %struct.datapartition.1055*, %struct.MotionInfoContexts*, %struct.TextureInfoContexts*, [6 x [32 x i32]], [2 x i32], [2 x i32*], [2 x i32*], [2 x i32*], [2 x i32*], i32, i32, i32, %struct.nalunitheadermvcext_tag, i16, i16, i16, i32, i32, i32, i32, i16***, i16***, i32***, i32***, i32***, [16 x i32], i16**, i16**, i32**, i16**, i16**, [3 x [6 x [4 x [4 x i32]]]], [3 x [6 x [4 x [4 x i32]]]], [3 x [6 x [8 x [8 x i32]]]], [3 x [6 x [8 x [8 x i32]]]], [12 x i32*], [64 x i32], i32, i32, i16, i16, i16, i16, i32***, i32***, i32****, i16, i16, i32, i32, %struct.frame_store**, %struct.frame_store**, i32, [17 x i32], void (%struct.macroblock.1054*)*, i32 (%struct.macroblock.1054*, i32, i16**, %struct.storable_picture*)*, i32 (%struct.video_par.1071*, %struct.inp_par*)*, i32 (%struct.slice.1060*, i32)*, void (%struct.macroblock.1054*)*, void (%struct.macroblock.1054*)*, void (%struct.macroblock.1054*)*, void (%struct.slice.1060*)*, void (%struct.macroblock.1054*)*, void (i32, i32, i32*, i32*)*, void (i32, i32, i32*, i32*)*, void (%struct.macroblock.1054*)*, i32, %struct.macroblock.1054*, %struct.storable_picture*, i32**, i8**, i8*, [6 x [16 x i8]] }
%struct.DecRefPicMarking_s = type { i32, i32, i32, i32, i32, %struct.DecRefPicMarking_s* }
%struct.datapartition.1055 = type { %struct.bit_stream*, %struct.DecodingEnvironment, i32 (%struct.macroblock.1054*, %struct.syntaxelement.1053*, %struct.datapartition.1055*)* }
%struct.bit_stream = type { i32, i32, i32, i32, i8*, i32 }
%struct.DecodingEnvironment = type { i32, i32, i32, i8*, i32* }
%struct.syntaxelement.1053 = type { i32, i32, i32, i32, i32, i32, i32, i32, void (i32, i32, i32*, i32*)*, void (%struct.macroblock.1054*, %struct.syntaxelement.1053*, %struct.DecodingEnvironment*)* }
%struct.MotionInfoContexts = type { [3 x [11 x %struct.BiContextType]], [2 x [9 x %struct.BiContextType]], [2 x [10 x %struct.BiContextType]], [2 x [6 x %struct.BiContextType]], [4 x %struct.BiContextType], [4 x %struct.BiContextType] }
%struct.BiContextType = type { i16, i8, i8 }
%struct.TextureInfoContexts = type { [3 x %struct.BiContextType], [2 x %struct.BiContextType], [4 x %struct.BiContextType], [3 x [4 x %struct.BiContextType]], [22 x [4 x %struct.BiContextType]], [2 x [22 x [15 x %struct.BiContextType]]], [2 x [22 x [15 x %struct.BiContextType]]], [22 x [5 x %struct.BiContextType]], [22 x [5 x %struct.BiContextType]] }
%struct.nalunitheadermvcext_tag = type { i32, i32, i32, i32, i32, i32, i32, i32 }
%struct.macroblock.1054 = type { %struct.slice.1060*, %struct.video_par.1071*, %struct.inp_par*, i32, %struct.BlockPos, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [2 x i32], [3 x i32], i32, i32, i32, i32, i16, i8, i8, i16, i16, %struct.macroblock.1054*, %struct.macroblock.1054*, %struct.macroblock.1054*, %struct.macroblock.1054*, i16, [2 x [4 x [4 x [2 x i16]]]], i32, [3 x i64], [3 x i64], [3 x i64], i32, [4 x i8], [4 x i8], i8, i8, i8, i16, i16, i16, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, void (%struct.macroblock.1054*, i32, i32, i32)*, void (%struct.macroblock.1054*, i32, i32, i32)*, void (%struct.macroblock.1054*, %struct.pix_pos*, %struct.BlockPos*, i16, %struct.pic_motion_params**, i32, i32, i32, i32, i32)*, i32 (%struct.macroblock.1054*, %struct.DecodingEnvironment*, i32)*, i8 (%struct.macroblock.1054*, %struct.syntaxelement.1053*, %struct.datapartition.1055*, i8, i32)*, i8 }
%struct.pix_pos = type { i32, i32, i16, i16, i16, i16 }
%struct.pic_motion_params = type { [2 x %struct.storable_picture*], [2 x %struct.BlockPos], [2 x i8] }
%struct.concealment_node = type { %struct.storable_picture*, i32, %struct.concealment_node* }
%struct.image_data = type { %struct.frame_format, [3 x i16**], [3 x i16**], [3 x i16**], [3 x i16**], [3 x i16**], [3 x i16**], [3 x i32], [3 x i32], [3 x i32] }
%struct.object_buffer = type { i8, i32, i32, [3 x i32] }
%struct.ercVariables_s = type { i32, i32, i8*, i8*, i8*, %struct.ercSegment_s*, i32, i8*, i32, i32, i32 }
%struct.ercSegment_s = type { i16, i16, i8 }
%struct.annex_b_struct = type { i32, i8*, i8*, i32, i32, i32, i32, i32, i8* }
%struct.sBitsFile.1068 = type { void (%struct.video_par.1071*, i8*)*, {}*, i32 (%struct.video_par.1071*, %struct.nalu_t*)* }
%struct.frame_store = type { i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %struct.storable_picture*, %struct.storable_picture*, %struct.storable_picture*, i32, [2 x i32], [2 x i32] }
%struct.storable_picture = type { i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i8, i32, i32, i32, i32, i16, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i16**, i16***, i16***, %struct.pic_motion_params**, [3 x %struct.pic_motion_params**], %struct.pic_motion_params_old, [3 x %struct.pic_motion_params_old], i16**, %struct.storable_picture*, %struct.storable_picture*, %struct.storable_picture*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [2 x i32], i32, %struct.DecRefPicMarking_s*, i32, i32, i32, i32, i16*, i32, i32, i32, i32, i32, i32, i32, i16**, i32, i32, [2 x i8], [2 x %struct.storable_picture**] }
%struct.pic_motion_params_old = type { i8* }
%struct.decoded_picture_buffer.1049 = type { %struct.video_par.1071*, %struct.inp_par*, %struct.frame_store**, %struct.frame_store**, %struct.frame_store**, i32, i32, i32, i32, i32, i32, [1024 x i32], i32, i32, %struct.frame_store* }
%struct.tone_mapping_struct_s = type { i32, i32, i8, i8, i32, i32, [4096 x i16], %struct.bit_stream*, i32 }
%struct.decodedpic_t = type { i32, i32, i32, i32, i32, i32, i8*, i8*, i8*, i32, i32, i32, i32, i32, %struct.decodedpic_t* }
%struct.nalu_t = type { i32, i32, i32, i32, i32, i32, i8*, i16, i32, i32, i32, i32, i32, i32, i32, i32 }
%struct.video_size = type { i8*, i32, i32 }
%struct.slice = type { %struct.video_par*, %struct.inp_par*, %struct.pic_parameter_set_rbsp_t*, %struct.seq_parameter_set_rbsp_t*, i32, %struct.decoded_picture_buffer*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [2 x i32], i32, i32, i32, i32, i32, i16, i32, i32, i32, i32, [2 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, i8, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %struct.DecRefPicMarking_s*, [6 x i8], [6 x %struct.storable_picture**], %struct.datapartition*, %struct.MotionInfoContexts*, %struct.TextureInfoContexts*, [6 x [32 x i32]], [2 x i32], [2 x i32*], [2 x i32*], [2 x i32*], [2 x i32*], i32, i32, i32, %struct.nalunitheadermvcext_tag, i16, i16, i16, i32, i32, i32, i32, i16***, i16***, i32***, i32***, i32***, [16 x i32], i16**, i16**, i32**, i16**, i16**, [3 x [6 x [4 x [4 x i32]]]], [3 x [6 x [4 x [4 x i32]]]], [3 x [6 x [8 x [8 x i32]]]], [3 x [6 x [8 x [8 x i32]]]], [12 x i32*], [64 x i32], i32, i32, i16, i16, i16, i16, i32***, i32***, i32****, i16, i16, i32, i32, %struct.frame_store**, %struct.frame_store**, i32, [17 x i32], void (%struct.macroblock*)*, i32 (%struct.macroblock*, i32, i16**, %struct.storable_picture*)*, i32 (%struct.video_par*, %struct.inp_par*)*, i32 (%struct.slice*, i32)*, void (%struct.macroblock*)*, void (%struct.macroblock*)*, void (%struct.macroblock*)*, void (%struct.slice*)*, void (%struct.macroblock*)*, void (i32, i32, i32*, i32*)*, void (i32, i32, i32*, i32*)*, void (%struct.macroblock*)*, i32, %struct.macroblock*, %struct.storable_picture*, i32**, i8**, i8*, [6 x [16 x i8]] }
%struct.video_par = type { %struct.inp_par*, %struct.pic_parameter_set_rbsp_t*, %struct.seq_parameter_set_rbsp_t*, [32 x %struct.seq_parameter_set_rbsp_t], [256 x %struct.pic_parameter_set_rbsp_t], %struct.subset_seq_parameter_set_rbsp_t*, [32 x %struct.subset_seq_parameter_set_rbsp_t], i32, i32, i32, i32, %struct.sei_params*, %struct.old_slice_par*, %struct.snr_par*, i32, i32, i32, i32, i32, %struct.slice**, i8*, [3 x i8*], i32, i32, i32, i32, i32, i8**, [3 x i8**], i8****, i32**, [3 x i32**], i32, i32, %struct.slice*, %struct.macroblock*, [3 x %struct.macroblock*], i32, %struct.concealment_node*, %struct.concealment_node*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i16, i16, [2 x i32], i32, i32, [3 x i32], [3 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [3 x [2 x i32]], [3 x [2 x i32]], [3 x [2 x i32]], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i8*, i8*, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, i32, i32, i32, i64, i32, [1024 x i32], i32, i32, i32, i32, i32, i32, i32, i16**, i16***, i32*, i32*, %struct.frame_store*, [100 x i32], %struct.storable_picture*, [3 x %struct.storable_picture*], %struct.storable_picture*, %struct.object_buffer*, %struct.ercVariables_s*, i32, %struct.video_par*, [20 x i32], %struct.annex_b_struct*, %struct.sBitsFile*, %struct.frame_store*, %struct.storable_picture*, i32, i32, i32, %struct.decoded_picture_buffer*, %struct.decoded_picture_buffer*, [2 x %struct.decoded_picture_buffer*], [9 x i8], i32*, i32*, i32, %struct.tone_mapping_struct_s*, void (i16**, i8*, i32, i32, i32, i32, i32, i32)*, void (%struct.macroblock*, i32, i32, i32*, %struct.pix_pos*)*, void (i32, i16*, i16*)*, void (i8*, %struct.macroblock*, i32, i32, %struct.storable_picture*)*, void (i8*, %struct.macroblock*, i32, i32, %struct.storable_picture*)*, void (i32, i16**, i8*, %struct.macroblock*, i32, %struct.storable_picture*)*, void (i32, i16**, i8*, %struct.macroblock*, i32, %struct.storable_picture*)*, void (i16**, i8*, %struct.macroblock*, i32, i32, %struct.storable_picture*)*, void (i16**, i8*, %struct.macroblock*, i32, i32, %struct.storable_picture*)*, void (i16**, i8*, i32, i32, i32, i32, i32, i32, i32, i32)*, %struct.decodedpic_t*, i32, %struct.nalu_t*, i32, i32, i32, i32, i32, i32, i32, %struct.pic_parameter_set_rbsp_t* }
%struct.sBitsFile = type { void (%struct.video_par*, i8*)*, void (%struct.video_par*)*, i32 (%struct.video_par*, %struct.nalu_t*)* }
%struct.decoded_picture_buffer = type { %struct.video_par*, %struct.inp_par*, %struct.frame_store**, %struct.frame_store**, %struct.frame_store**, i32, i32, i32, i32, i32, i32, [1024 x i32], i32, i32, %struct.frame_store* }
%struct.datapartition = type { %struct.bit_stream*, %struct.DecodingEnvironment, i32 (%struct.macroblock*, %struct.syntaxelement*, %struct.datapartition*)* }
%struct.syntaxelement = type { i32, i32, i32, i32, i32, i32, i32, i32, void (i32, i32, i32*, i32*)*, void (%struct.macroblock*, %struct.syntaxelement*, %struct.DecodingEnvironment*)* }
%struct.macroblock = type { %struct.slice*, %struct.video_par*, %struct.inp_par*, i32, %struct.BlockPos, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [2 x i32], [3 x i32], i32, i32, i32, i32, i16, i8, i8, i16, i16, %struct.macroblock*, %struct.macroblock*, %struct.macroblock*, %struct.macroblock*, i16, [2 x [4 x [4 x [2 x i16]]]], i32, [3 x i64], [3 x i64], [3 x i64], i32, [4 x i8], [4 x i8], i8, i8, i8, i16, i16, i16, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, void (%struct.macroblock*, i32, i32, i32)*, void (%struct.macroblock*, i32, i32, i32)*, void (%struct.macroblock*, %struct.pix_pos*, %struct.BlockPos*, i16, %struct.pic_motion_params**, i32, i32, i32, i32, i32)*, i32 (%struct.macroblock*, %struct.DecodingEnvironment*, i32)*, i8 (%struct.macroblock*, %struct.syntaxelement*, %struct.datapartition*, i8, i32)*, i8 }
%struct.decoder_params = type { %struct.inp_par*, %struct.video_par*, i64, i32, %struct._IO_FILE*, i32 }
%struct.distortion_data = type { [4 x [4 x i32]], [4 x [4 x i64]], [2 x [2 x i64]], [2 x [2 x i32]], i32, i32, double }
%struct.lambda_params = type { double, [3 x double], [3 x i32] }
%struct.level_quant_params = type { i32, i32, i32 }
%struct.macroblock.165 = type { %struct.slice.163*, %struct.video_par.154*, %struct.inp_par*, i32, %struct.BlockPos, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [2 x i32], [3 x i32], i32, i32, i32, i32, i16, i8, i8, i16, i16, %struct.macroblock.165*, %struct.macroblock.165*, %struct.macroblock.165*, %struct.macroblock.165*, i16, [2 x [4 x [4 x [2 x i16]]]], i32, [3 x i64], [3 x i64], [3 x i64], i32, [4 x i8], [4 x i8], i8, i8, i8, i16, i16, i16, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, void (%struct.macroblock.165*, i32, i32, i32)*, void (%struct.macroblock.165*, i32, i32, i32)*, void (%struct.macroblock.165*, %struct.pix_pos*, %struct.BlockPos*, i16, %struct.pic_motion_params**, i32, i32, i32, i32, i32)*, i32 (%struct.macroblock.165*, %struct.DecodingEnvironment*, i32)*, i8 (%struct.macroblock.165*, %struct.syntaxelement.157*, %struct.datapartition.158*, i8, i32)*, i8 }
%struct.slice.163 = type { %struct.video_par.154*, %struct.inp_par*, %struct.pic_parameter_set_rbsp_t*, %struct.seq_parameter_set_rbsp_t*, i32, %struct.decoded_picture_buffer.149*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [2 x i32], i32, i32, i32, i32, i32, i16, i32, i32, i32, i32, [2 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, i8, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %struct.DecRefPicMarking_s*, [6 x i8], [6 x %struct.storable_picture**], %struct.datapartition.158*, %struct.MotionInfoContexts*, %struct.TextureInfoContexts*, [6 x [32 x i32]], [2 x i32], [2 x i32*], [2 x i32*], [2 x i32*], [2 x i32*], i32, i32, i32, %struct.nalunitheadermvcext_tag, i16, i16, i16, i32, i32, i32, i32, i16***, i16***, i32***, i32***, i32***, [16 x i32], i16**, i16**, i32**, i16**, i16**, [3 x [6 x [4 x [4 x i32]]]], [3 x [6 x [4 x [4 x i32]]]], [3 x [6 x [8 x [8 x i32]]]], [3 x [6 x [8 x [8 x i32]]]], [12 x i32*], [64 x i32], i32, i32, i16, i16, i16, i16, i32***, i32***, i32****, i16, i16, i32, i32, %struct.frame_store**, %struct.frame_store**, i32, [17 x i32], void (%struct.macroblock.165*)*, {}*, i32 (%struct.video_par.154*, %struct.inp_par*)*, i32 (%struct.slice.163*, i32)*, void (%struct.macroblock.165*)*, void (%struct.macroblock.165*)*, void (%struct.macroblock.165*)*, void (%struct.slice.163*)*, void (%struct.macroblock.165*)*, void (i32, i32, i32*, i32*)*, void (i32, i32, i32*, i32*)*, void (%struct.macroblock.165*)*, i32, %struct.macroblock.165*, %struct.storable_picture*, i32**, i8**, i8*, [6 x [16 x i8]] }
%struct.decoded_picture_buffer.149 = type { %struct.video_par.154*, %struct.inp_par*, %struct.frame_store**, %struct.frame_store**, %struct.frame_store**, i32, i32, i32, i32, i32, i32, [1024 x i32], i32, i32, %struct.frame_store* }
%struct.datapartition.158 = type { %struct.bit_stream*, %struct.DecodingEnvironment, i32 (%struct.macroblock.165*, %struct.syntaxelement.157*, %struct.datapartition.158*)* }
%struct.syntaxelement.157 = type { i32, i32, i32, i32, i32, i32, i32, i32, void (i32, i32, i32*, i32*)*, void (%struct.macroblock.165*, %struct.syntaxelement.157*, %struct.DecodingEnvironment*)* }
%struct.video_par.154 = type { %struct.inp_par*, %struct.pic_parameter_set_rbsp_t*, %struct.seq_parameter_set_rbsp_t*, [32 x %struct.seq_parameter_set_rbsp_t], [256 x %struct.pic_parameter_set_rbsp_t], %struct.subset_seq_parameter_set_rbsp_t*, [32 x %struct.subset_seq_parameter_set_rbsp_t], i32, i32, i32, i32, %struct.sei_params*, %struct.old_slice_par*, %struct.snr_par*, i32, i32, i32, i32, i32, %struct.slice.163**, i8*, [3 x i8*], i32, i32, i32, i32, i32, i8**, [3 x i8**], i8****, i32**, [3 x i32**], i32, i32, %struct.slice.163*, %struct.macroblock.165*, [3 x %struct.macroblock.165*], i32, %struct.concealment_node*, %struct.concealment_node*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i16, i16, [2 x i32], i32, i32, [3 x i32], [3 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [3 x [2 x i32]], [3 x [2 x i32]], [3 x [2 x i32]], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i8*, i8*, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, i32, i32, i32, i64, i32, [1024 x i32], i32, i32, i32, i32, i32, i32, i32, i16**, i16***, i32*, i32*, %struct.frame_store*, [100 x i32], %struct.storable_picture*, [3 x %struct.storable_picture*], %struct.storable_picture*, %struct.object_buffer*, %struct.ercVariables_s*, i32, %struct.video_par.154*, [20 x i32], %struct.annex_b_struct*, %struct.sBitsFile*, %struct.frame_store*, %struct.storable_picture*, i32, i32, i32, %struct.decoded_picture_buffer.149*, %struct.decoded_picture_buffer.149*, [2 x %struct.decoded_picture_buffer.149*], [9 x i8], i32*, i32*, i32, %struct.tone_mapping_struct_s*, void (i16**, i8*, i32, i32, i32, i32, i32, i32)*, void (%struct.macroblock.165*, i32, i32, i32*, %struct.pix_pos*)*, void (i32, i16*, i16*)*, void (i8*, %struct.macroblock.165*, i32, i32, %struct.storable_picture*)*, void (i8*, %struct.macroblock.165*, i32, i32, %struct.storable_picture*)*, void (i32, i16**, i8*, %struct.macroblock.165*, i32, %struct.storable_picture*)*, void (i32, i16**, i8*, %struct.macroblock.165*, i32, %struct.storable_picture*)*, void (i16**, i8*, %struct.macroblock.165*, i32, i32, %struct.storable_picture*)*, void (i16**, i8*, %struct.macroblock.165*, i32, i32, %struct.storable_picture*)*, void (i16**, i8*, i32, i32, i32, i32, i32, i32, i32, i32)*, %struct.decodedpic_t*, i32, %struct.nalu_t*, i32, i32, i32, i32, i32, i32, i32, %struct.pic_parameter_set_rbsp_t* }
%struct.slice.284 = type { %struct.video_par.278*, %struct.inp_par*, %struct.pic_parameter_set_rbsp_t*, %struct.seq_parameter_set_rbsp_t*, i32, %struct.decoded_picture_buffer*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [2 x i32], i32, i32, i32, i32, i32, i16, i32, i32, i32, i32, [2 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, i8, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %struct.DecRefPicMarking_s*, [6 x i8], [6 x %struct.storable_picture**], %struct.datapartition.264*, %struct.MotionInfoContexts*, %struct.TextureInfoContexts*, [6 x [32 x i32]], [2 x i32], [2 x i32*], [2 x i32*], [2 x i32*], [2 x i32*], i32, i32, i32, %struct.nalunitheadermvcext_tag, i16, i16, i16, i32, i32, i32, i32, i16***, i16***, i32***, i32***, i32***, [16 x i32], i16**, i16**, i32**, i16**, i16**, [3 x [6 x [4 x [4 x i32]]]], [3 x [6 x [4 x [4 x i32]]]], [3 x [6 x [8 x [8 x i32]]]], [3 x [6 x [8 x [8 x i32]]]], [12 x i32*], [64 x i32], i32, i32, i16, i16, i16, i16, i32***, i32***, i32****, i16, i16, i32, i32, %struct.frame_store**, %struct.frame_store**, i32, [17 x i32], void (%struct.macroblock.265*)*, i32 (%struct.macroblock.265*, i32, i16**, %struct.storable_picture*)*, i32 (%struct.video_par.278*, %struct.inp_par*)*, i32 (%struct.slice.284*, i32)*, void (%struct.macroblock.265*)*, void (%struct.macroblock.265*)*, void (%struct.macroblock.265*)*, {}*, void (%struct.macroblock.265*)*, void (i32, i32, i32*, i32*)*, void (i32, i32, i32*, i32*)*, void (%struct.macroblock.265*)*, i32, %struct.macroblock.265*, %struct.storable_picture*, i32**, i8**, i8*, [6 x [16 x i8]] }
%struct.video_par.278 = type { %struct.inp_par*, %struct.pic_parameter_set_rbsp_t*, %struct.seq_parameter_set_rbsp_t*, [32 x %struct.seq_parameter_set_rbsp_t], [256 x %struct.pic_parameter_set_rbsp_t], %struct.subset_seq_parameter_set_rbsp_t*, [32 x %struct.subset_seq_parameter_set_rbsp_t], i32, i32, i32, i32, %struct.sei_params*, %struct.old_slice_par*, %struct.snr_par*, i32, i32, i32, i32, i32, %struct.slice.284**, i8*, [3 x i8*], i32, i32, i32, i32, i32, i8**, [3 x i8**], i8****, i32**, [3 x i32**], i32, i32, %struct.slice.284*, %struct.macroblock.265*, [3 x %struct.macroblock.265*], i32, %struct.concealment_node*, %struct.concealment_node*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i16, i16, [2 x i32], i32, i32, [3 x i32], [3 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [3 x [2 x i32]], [3 x [2 x i32]], [3 x [2 x i32]], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i8*, i8*, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, i32, i32, i32, i64, i32, [1024 x i32], i32, i32, i32, i32, i32, i32, i32, i16**, i16***, i32*, i32*, %struct.frame_store*, [100 x i32], %struct.storable_picture*, [3 x %struct.storable_picture*], %struct.storable_picture*, %struct.object_buffer*, %struct.ercVariables_s*, i32, %struct.video_par.278*, [20 x i32], %struct.annex_b_struct*, %struct.sBitsFile*, %struct.frame_store*, %struct.storable_picture*, i32, i32, i32, %struct.decoded_picture_buffer*, %struct.decoded_picture_buffer*, [2 x %struct.decoded_picture_buffer*], [9 x i8], i32*, i32*, i32, %struct.tone_mapping_struct_s*, void (i16**, i8*, i32, i32, i32, i32, i32, i32)*, void (%struct.macroblock.265*, i32, i32, i32*, %struct.pix_pos*)*, void (i32, i16*, i16*)*, void (i8*, %struct.macroblock.265*, i32, i32, %struct.storable_picture*)*, void (i8*, %struct.macroblock.265*, i32, i32, %struct.storable_picture*)*, void (i32, i16**, i8*, %struct.macroblock.265*, i32, %struct.storable_picture*)*, void (i32, i16**, i8*, %struct.macroblock.265*, i32, %struct.storable_picture*)*, void (i16**, i8*, %struct.macroblock.265*, i32, i32, %struct.storable_picture*)*, void (i16**, i8*, %struct.macroblock.265*, i32, i32, %struct.storable_picture*)*, void (i16**, i8*, i32, i32, i32, i32, i32, i32, i32, i32)*, %struct.decodedpic_t*, i32, %struct.nalu_t*, i32, i32, i32, i32, i32, i32, i32, %struct.pic_parameter_set_rbsp_t* }
%struct.datapartition.264 = type { %struct.bit_stream*, %struct.DecodingEnvironment, i32 (%struct.macroblock.265*, %struct.syntaxelement.262*, %struct.datapartition.264*)* }
%struct.syntaxelement.262 = type { i32, i32, i32, i32, i32, i32, i32, i32, void (i32, i32, i32*, i32*)*, void (%struct.macroblock.265*, %struct.syntaxelement.262*, %struct.DecodingEnvironment*)* }
%struct.macroblock.265 = type { %struct.slice.284*, %struct.video_par.278*, %struct.inp_par*, i32, %struct.BlockPos, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [2 x i32], [3 x i32], i32, i32, i32, i32, i16, i8, i8, i16, i16, %struct.macroblock.265*, %struct.macroblock.265*, %struct.macroblock.265*, %struct.macroblock.265*, i16, [2 x [4 x [4 x [2 x i16]]]], i32, [3 x i64], [3 x i64], [3 x i64], i32, [4 x i8], [4 x i8], i8, i8, i8, i16, i16, i16, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, void (%struct.macroblock.265*, i32, i32, i32)*, void (%struct.macroblock.265*, i32, i32, i32)*, void (%struct.macroblock.265*, %struct.pix_pos*, %struct.BlockPos*, i16, %struct.pic_motion_params**, i32, i32, i32, i32, i32)*, i32 (%struct.macroblock.265*, %struct.DecodingEnvironment*, i32)*, i8 (%struct.macroblock.265*, %struct.syntaxelement.262*, %struct.datapartition.264*, i8, i32)*, i8 }
%struct.macroblock.329 = type { %struct.slice.327*, %struct.video_par.318*, %struct.inp_par*, i32, %struct.BlockPos, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [2 x i32], [3 x i32], i32, i32, i32, i32, i16, i8, i8, i16, i16, %struct.macroblock.329*, %struct.macroblock.329*, %struct.macroblock.329*, %struct.macroblock.329*, i16, [2 x [4 x [4 x [2 x i16]]]], i32, [3 x i64], [3 x i64], [3 x i64], i32, [4 x i8], [4 x i8], i8, i8, i8, i16, i16, i16, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, void (%struct.macroblock.329*, i32, i32, i32)*, void (%struct.macroblock.329*, i32, i32, i32)*, void (%struct.macroblock.329*, %struct.pix_pos*, %struct.BlockPos*, i16, %struct.pic_motion_params**, i32, i32, i32, i32, i32)*, i32 (%struct.macroblock.329*, %struct.DecodingEnvironment*, i32)*, i8 (%struct.macroblock.329*, %struct.syntaxelement.321*, %struct.datapartition.322*, i8, i32)*, i8 }
%struct.slice.327 = type { %struct.video_par.318*, %struct.inp_par*, %struct.pic_parameter_set_rbsp_t*, %struct.seq_parameter_set_rbsp_t*, i32, %struct.decoded_picture_buffer.313*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [2 x i32], i32, i32, i32, i32, i32, i16, i32, i32, i32, i32, [2 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, i8, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %struct.DecRefPicMarking_s*, [6 x i8], [6 x %struct.storable_picture**], %struct.datapartition.322*, %struct.MotionInfoContexts*, %struct.TextureInfoContexts*, [6 x [32 x i32]], [2 x i32], [2 x i32*], [2 x i32*], [2 x i32*], [2 x i32*], i32, i32, i32, %struct.nalunitheadermvcext_tag, i16, i16, i16, i32, i32, i32, i32, i16***, i16***, i32***, i32***, i32***, [16 x i32], i16**, i16**, i32**, i16**, i16**, [3 x [6 x [4 x [4 x i32]]]], [3 x [6 x [4 x [4 x i32]]]], [3 x [6 x [8 x [8 x i32]]]], [3 x [6 x [8 x [8 x i32]]]], [12 x i32*], [64 x i32], i32, i32, i16, i16, i16, i16, i32***, i32***, i32****, i16, i16, i32, i32, %struct.frame_store**, %struct.frame_store**, i32, [17 x i32], {}*, i32 (%struct.macroblock.329*, i32, i16**, %struct.storable_picture*)*, i32 (%struct.video_par.318*, %struct.inp_par*)*, i32 (%struct.slice.327*, i32)*, {}*, {}*, {}*, void (%struct.slice.327*)*, {}*, void (i32, i32, i32*, i32*)*, void (i32, i32, i32*, i32*)*, {}*, i32, %struct.macroblock.329*, %struct.storable_picture*, i32**, i8**, i8*, [6 x [16 x i8]] }
%struct.decoded_picture_buffer.313 = type { %struct.video_par.318*, %struct.inp_par*, %struct.frame_store**, %struct.frame_store**, %struct.frame_store**, i32, i32, i32, i32, i32, i32, [1024 x i32], i32, i32, %struct.frame_store* }
%struct.datapartition.322 = type { %struct.bit_stream*, %struct.DecodingEnvironment, i32 (%struct.macroblock.329*, %struct.syntaxelement.321*, %struct.datapartition.322*)* }
%struct.syntaxelement.321 = type { i32, i32, i32, i32, i32, i32, i32, i32, void (i32, i32, i32*, i32*)*, void (%struct.macroblock.329*, %struct.syntaxelement.321*, %struct.DecodingEnvironment*)* }
%struct.video_par.318 = type { %struct.inp_par*, %struct.pic_parameter_set_rbsp_t*, %struct.seq_parameter_set_rbsp_t*, [32 x %struct.seq_parameter_set_rbsp_t], [256 x %struct.pic_parameter_set_rbsp_t], %struct.subset_seq_parameter_set_rbsp_t*, [32 x %struct.subset_seq_parameter_set_rbsp_t], i32, i32, i32, i32, %struct.sei_params*, %struct.old_slice_par*, %struct.snr_par*, i32, i32, i32, i32, i32, %struct.slice.327**, i8*, [3 x i8*], i32, i32, i32, i32, i32, i8**, [3 x i8**], i8****, i32**, [3 x i32**], i32, i32, %struct.slice.327*, %struct.macroblock.329*, [3 x %struct.macroblock.329*], i32, %struct.concealment_node*, %struct.concealment_node*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i16, i16, [2 x i32], i32, i32, [3 x i32], [3 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [3 x [2 x i32]], [3 x [2 x i32]], [3 x [2 x i32]], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i8*, i8*, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, i32, i32, i32, i64, i32, [1024 x i32], i32, i32, i32, i32, i32, i32, i32, i16**, i16***, i32*, i32*, %struct.frame_store*, [100 x i32], %struct.storable_picture*, [3 x %struct.storable_picture*], %struct.storable_picture*, %struct.object_buffer*, %struct.ercVariables_s*, i32, %struct.video_par.318*, [20 x i32], %struct.annex_b_struct*, %struct.sBitsFile*, %struct.frame_store*, %struct.storable_picture*, i32, i32, i32, %struct.decoded_picture_buffer.313*, %struct.decoded_picture_buffer.313*, [2 x %struct.decoded_picture_buffer.313*], [9 x i8], i32*, i32*, i32, %struct.tone_mapping_struct_s*, void (i16**, i8*, i32, i32, i32, i32, i32, i32)*, void (%struct.macroblock.329*, i32, i32, i32*, %struct.pix_pos*)*, void (i32, i16*, i16*)*, void (i8*, %struct.macroblock.329*, i32, i32, %struct.storable_picture*)*, void (i8*, %struct.macroblock.329*, i32, i32, %struct.storable_picture*)*, void (i32, i16**, i8*, %struct.macroblock.329*, i32, %struct.storable_picture*)*, void (i32, i16**, i8*, %struct.macroblock.329*, i32, %struct.storable_picture*)*, void (i16**, i8*, %struct.macroblock.329*, i32, i32, %struct.storable_picture*)*, void (i16**, i8*, %struct.macroblock.329*, i32, i32, %struct.storable_picture*)*, void (i16**, i8*, i32, i32, i32, i32, i32, i32, i32, i32)*, %struct.decodedpic_t*, i32, %struct.nalu_t*, i32, i32, i32, i32, i32, i32, i32, %struct.pic_parameter_set_rbsp_t* }
%struct.tone_mapping_struct_tmp = type { i32, i8, i32, i8, i8, i32, i32, i32, i32, i32, [4096 x i32], i32, [4096 x i32], [4096 x i32] }
%struct.video_par.493 = type { %struct.inp_par*, %struct.pic_parameter_set_rbsp_t*, %struct.seq_parameter_set_rbsp_t*, [32 x %struct.seq_parameter_set_rbsp_t], [256 x %struct.pic_parameter_set_rbsp_t], %struct.subset_seq_parameter_set_rbsp_t*, [32 x %struct.subset_seq_parameter_set_rbsp_t], i32, i32, i32, i32, %struct.sei_params*, %struct.old_slice_par*, %struct.snr_par*, i32, i32, i32, i32, i32, %struct.slice.483**, i8*, [3 x i8*], i32, i32, i32, i32, i32, i8**, [3 x i8**], i8****, i32**, [3 x i32**], i32, i32, %struct.slice.483*, %struct.macroblock.477*, [3 x %struct.macroblock.477*], i32, %struct.concealment_node*, %struct.concealment_node*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i16, i16, [2 x i32], i32, i32, [3 x i32], [3 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [3 x [2 x i32]], [3 x [2 x i32]], [3 x [2 x i32]], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i8*, i8*, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, i32, i32, i32, i64, i32, [1024 x i32], i32, i32, i32, i32, i32, i32, i32, i16**, i16***, i32*, i32*, %struct.frame_store*, [100 x i32], %struct.storable_picture*, [3 x %struct.storable_picture*], %struct.storable_picture*, %struct.object_buffer*, %struct.ercVariables_s*, i32, %struct.video_par.493*, [20 x i32], %struct.annex_b_struct*, %struct.sBitsFile*, %struct.frame_store*, %struct.storable_picture*, i32, i32, i32, %struct.decoded_picture_buffer.471*, %struct.decoded_picture_buffer.471*, [2 x %struct.decoded_picture_buffer.471*], [9 x i8], i32*, i32*, i32, %struct.tone_mapping_struct_s*, void (i16**, i8*, i32, i32, i32, i32, i32, i32)*, void (%struct.macroblock.477*, i32, i32, i32*, %struct.pix_pos*)*, void (i32, i16*, i16*)*, void (i8*, %struct.macroblock.477*, i32, i32, %struct.storable_picture*)*, void (i8*, %struct.macroblock.477*, i32, i32, %struct.storable_picture*)*, void (i32, i16**, i8*, %struct.macroblock.477*, i32, %struct.storable_picture*)*, void (i32, i16**, i8*, %struct.macroblock.477*, i32, %struct.storable_picture*)*, void (i16**, i8*, %struct.macroblock.477*, i32, i32, %struct.storable_picture*)*, void (i16**, i8*, %struct.macroblock.477*, i32, i32, %struct.storable_picture*)*, void (i16**, i8*, i32, i32, i32, i32, i32, i32, i32, i32)*, %struct.decodedpic_t*, i32, %struct.nalu_t*, i32, i32, i32, i32, i32, i32, i32, %struct.pic_parameter_set_rbsp_t* }
%struct.slice.483 = type { %struct.video_par.493*, %struct.inp_par*, %struct.pic_parameter_set_rbsp_t*, %struct.seq_parameter_set_rbsp_t*, i32, %struct.decoded_picture_buffer.471*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [2 x i32], i32, i32, i32, i32, i32, i16, i32, i32, i32, i32, [2 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, i8, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %struct.DecRefPicMarking_s*, [6 x i8], [6 x %struct.storable_picture**], %struct.datapartition.478*, %struct.MotionInfoContexts*, %struct.TextureInfoContexts*, [6 x [32 x i32]], [2 x i32], [2 x i32*], [2 x i32*], [2 x i32*], [2 x i32*], i32, i32, i32, %struct.nalunitheadermvcext_tag, i16, i16, i16, i32, i32, i32, i32, i16***, i16***, i32***, i32***, i32***, [16 x i32], i16**, i16**, i32**, i16**, i16**, [3 x [6 x [4 x [4 x i32]]]], [3 x [6 x [4 x [4 x i32]]]], [3 x [6 x [8 x [8 x i32]]]], [3 x [6 x [8 x [8 x i32]]]], [12 x i32*], [64 x i32], i32, i32, i16, i16, i16, i16, i32***, i32***, i32****, i16, i16, i32, i32, %struct.frame_store**, %struct.frame_store**, i32, [17 x i32], void (%struct.macroblock.477*)*, i32 (%struct.macroblock.477*, i32, i16**, %struct.storable_picture*)*, {}*, i32 (%struct.slice.483*, i32)*, void (%struct.macroblock.477*)*, void (%struct.macroblock.477*)*, void (%struct.macroblock.477*)*, void (%struct.slice.483*)*, void (%struct.macroblock.477*)*, void (i32, i32, i32*, i32*)*, void (i32, i32, i32*, i32*)*, void (%struct.macroblock.477*)*, i32, %struct.macroblock.477*, %struct.storable_picture*, i32**, i8**, i8*, [6 x [16 x i8]] }
%struct.datapartition.478 = type { %struct.bit_stream*, %struct.DecodingEnvironment, i32 (%struct.macroblock.477*, %struct.syntaxelement.476*, %struct.datapartition.478*)* }
%struct.syntaxelement.476 = type { i32, i32, i32, i32, i32, i32, i32, i32, void (i32, i32, i32*, i32*)*, void (%struct.macroblock.477*, %struct.syntaxelement.476*, %struct.DecodingEnvironment*)* }
%struct.macroblock.477 = type { %struct.slice.483*, %struct.video_par.493*, %struct.inp_par*, i32, %struct.BlockPos, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [2 x i32], [3 x i32], i32, i32, i32, i32, i16, i8, i8, i16, i16, %struct.macroblock.477*, %struct.macroblock.477*, %struct.macroblock.477*, %struct.macroblock.477*, i16, [2 x [4 x [4 x [2 x i16]]]], i32, [3 x i64], [3 x i64], [3 x i64], i32, [4 x i8], [4 x i8], i8, i8, i8, i16, i16, i16, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, void (%struct.macroblock.477*, i32, i32, i32)*, void (%struct.macroblock.477*, i32, i32, i32)*, void (%struct.macroblock.477*, %struct.pix_pos*, %struct.BlockPos*, i16, %struct.pic_motion_params**, i32, i32, i32, i32, i32)*, i32 (%struct.macroblock.477*, %struct.DecodingEnvironment*, i32)*, i8 (%struct.macroblock.477*, %struct.syntaxelement.476*, %struct.datapartition.478*, i8, i32)*, i8 }
%struct.decoded_picture_buffer.471 = type { %struct.video_par.493*, %struct.inp_par*, %struct.frame_store**, %struct.frame_store**, %struct.frame_store**, i32, i32, i32, i32, i32, i32, [1024 x i32], i32, i32, %struct.frame_store* }
%struct.RTPpacket_t = type { i32, i32, i32, i32, i32, i32, i16, i32, i32, i8*, i32, i8*, i32 }
%struct.macroblock.741 = type { %struct.slice.739*, %struct.video_par.730*, %struct.inp_par*, i32, %struct.BlockPos, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [2 x i32], [3 x i32], i32, i32, i32, i32, i16, i8, i8, i16, i16, %struct.macroblock.741*, %struct.macroblock.741*, %struct.macroblock.741*, %struct.macroblock.741*, i16, [2 x [4 x [4 x [2 x i16]]]], i32, [3 x i64], [3 x i64], [3 x i64], i32, [4 x i8], [4 x i8], i8, i8, i8, i16, i16, i16, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, void (%struct.macroblock.741*, i32, i32, i32)*, void (%struct.macroblock.741*, i32, i32, i32)*, void (%struct.macroblock.741*, %struct.pix_pos*, %struct.BlockPos*, i16, %struct.pic_motion_params**, i32, i32, i32, i32, i32)*, i32 (%struct.macroblock.741*, %struct.DecodingEnvironment*, i32)*, i8 (%struct.macroblock.741*, %struct.syntaxelement.733*, %struct.datapartition.734*, i8, i32)*, i8 }
%struct.slice.739 = type { %struct.video_par.730*, %struct.inp_par*, %struct.pic_parameter_set_rbsp_t*, %struct.seq_parameter_set_rbsp_t*, i32, %struct.decoded_picture_buffer.725*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [2 x i32], i32, i32, i32, i32, i32, i16, i32, i32, i32, i32, [2 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, i8, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %struct.DecRefPicMarking_s*, [6 x i8], [6 x %struct.storable_picture**], %struct.datapartition.734*, %struct.MotionInfoContexts*, %struct.TextureInfoContexts*, [6 x [32 x i32]], [2 x i32], [2 x i32*], [2 x i32*], [2 x i32*], [2 x i32*], i32, i32, i32, %struct.nalunitheadermvcext_tag, i16, i16, i16, i32, i32, i32, i32, i16***, i16***, i32***, i32***, i32***, [16 x i32], i16**, i16**, i32**, i16**, i16**, [3 x [6 x [4 x [4 x i32]]]], [3 x [6 x [4 x [4 x i32]]]], [3 x [6 x [8 x [8 x i32]]]], [3 x [6 x [8 x [8 x i32]]]], [12 x i32*], [64 x i32], i32, i32, i16, i16, i16, i16, i32***, i32***, i32****, i16, i16, i32, i32, %struct.frame_store**, %struct.frame_store**, i32, [17 x i32], {}*, i32 (%struct.macroblock.741*, i32, i16**, %struct.storable_picture*)*, i32 (%struct.video_par.730*, %struct.inp_par*)*, i32 (%struct.slice.739*, i32)*, {}*, {}*, {}*, void (%struct.slice.739*)*, {}*, void (i32, i32, i32*, i32*)*, void (i32, i32, i32*, i32*)*, {}*, i32, %struct.macroblock.741*, %struct.storable_picture*, i32**, i8**, i8*, [6 x [16 x i8]] }
%struct.decoded_picture_buffer.725 = type { %struct.video_par.730*, %struct.inp_par*, %struct.frame_store**, %struct.frame_store**, %struct.frame_store**, i32, i32, i32, i32, i32, i32, [1024 x i32], i32, i32, %struct.frame_store* }
%struct.datapartition.734 = type { %struct.bit_stream*, %struct.DecodingEnvironment, i32 (%struct.macroblock.741*, %struct.syntaxelement.733*, %struct.datapartition.734*)* }
%struct.syntaxelement.733 = type { i32, i32, i32, i32, i32, i32, i32, i32, void (i32, i32, i32*, i32*)*, void (%struct.macroblock.741*, %struct.syntaxelement.733*, %struct.DecodingEnvironment*)* }
%struct.video_par.730 = type { %struct.inp_par*, %struct.pic_parameter_set_rbsp_t*, %struct.seq_parameter_set_rbsp_t*, [32 x %struct.seq_parameter_set_rbsp_t], [256 x %struct.pic_parameter_set_rbsp_t], %struct.subset_seq_parameter_set_rbsp_t*, [32 x %struct.subset_seq_parameter_set_rbsp_t], i32, i32, i32, i32, %struct.sei_params*, %struct.old_slice_par*, %struct.snr_par*, i32, i32, i32, i32, i32, %struct.slice.739**, i8*, [3 x i8*], i32, i32, i32, i32, i32, i8**, [3 x i8**], i8****, i32**, [3 x i32**], i32, i32, %struct.slice.739*, %struct.macroblock.741*, [3 x %struct.macroblock.741*], i32, %struct.concealment_node*, %struct.concealment_node*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i16, i16, [2 x i32], i32, i32, [3 x i32], [3 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [3 x [2 x i32]], [3 x [2 x i32]], [3 x [2 x i32]], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i8*, i8*, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, i32, i32, i32, i64, i32, [1024 x i32], i32, i32, i32, i32, i32, i32, i32, i16**, i16***, i32*, i32*, %struct.frame_store*, [100 x i32], %struct.storable_picture*, [3 x %struct.storable_picture*], %struct.storable_picture*, %struct.object_buffer*, %struct.ercVariables_s*, i32, %struct.video_par.730*, [20 x i32], %struct.annex_b_struct*, %struct.sBitsFile*, %struct.frame_store*, %struct.storable_picture*, i32, i32, i32, %struct.decoded_picture_buffer.725*, %struct.decoded_picture_buffer.725*, [2 x %struct.decoded_picture_buffer.725*], [9 x i8], i32*, i32*, i32, %struct.tone_mapping_struct_s*, void (i16**, i8*, i32, i32, i32, i32, i32, i32)*, void (%struct.macroblock.741*, i32, i32, i32*, %struct.pix_pos*)*, void (i32, i16*, i16*)*, void (i8*, %struct.macroblock.741*, i32, i32, %struct.storable_picture*)*, void (i8*, %struct.macroblock.741*, i32, i32, %struct.storable_picture*)*, void (i32, i16**, i8*, %struct.macroblock.741*, i32, %struct.storable_picture*)*, void (i32, i16**, i8*, %struct.macroblock.741*, i32, %struct.storable_picture*)*, void (i16**, i8*, %struct.macroblock.741*, i32, i32, %struct.storable_picture*)*, void (i16**, i8*, %struct.macroblock.741*, i32, i32, %struct.storable_picture*)*, void (i16**, i8*, i32, i32, i32, i32, i32, i32, i32, i32)*, %struct.decodedpic_t*, i32, %struct.nalu_t*, i32, i32, i32, i32, i32, i32, i32, %struct.pic_parameter_set_rbsp_t* }
%struct.macroblock.781 = type { %struct.slice.779*, %struct.video_par.770*, %struct.inp_par*, i32, %struct.BlockPos, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [2 x i32], [3 x i32], i32, i32, i32, i32, i16, i8, i8, i16, i16, %struct.macroblock.781*, %struct.macroblock.781*, %struct.macroblock.781*, %struct.macroblock.781*, i16, [2 x [4 x [4 x [2 x i16]]]], i32, [3 x i64], [3 x i64], [3 x i64], i32, [4 x i8], [4 x i8], i8, i8, i8, i16, i16, i16, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, {}*, {}*, void (%struct.macroblock.781*, %struct.pix_pos*, %struct.BlockPos*, i16, %struct.pic_motion_params**, i32, i32, i32, i32, i32)*, i32 (%struct.macroblock.781*, %struct.DecodingEnvironment*, i32)*, i8 (%struct.macroblock.781*, %struct.syntaxelement.773*, %struct.datapartition.774*, i8, i32)*, i8 }
%struct.slice.779 = type { %struct.video_par.770*, %struct.inp_par*, %struct.pic_parameter_set_rbsp_t*, %struct.seq_parameter_set_rbsp_t*, i32, %struct.decoded_picture_buffer.765*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [2 x i32], i32, i32, i32, i32, i32, i16, i32, i32, i32, i32, [2 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, i8, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %struct.DecRefPicMarking_s*, [6 x i8], [6 x %struct.storable_picture**], %struct.datapartition.774*, %struct.MotionInfoContexts*, %struct.TextureInfoContexts*, [6 x [32 x i32]], [2 x i32], [2 x i32*], [2 x i32*], [2 x i32*], [2 x i32*], i32, i32, i32, %struct.nalunitheadermvcext_tag, i16, i16, i16, i32, i32, i32, i32, i16***, i16***, i32***, i32***, i32***, [16 x i32], i16**, i16**, i32**, i16**, i16**, [3 x [6 x [4 x [4 x i32]]]], [3 x [6 x [4 x [4 x i32]]]], [3 x [6 x [8 x [8 x i32]]]], [3 x [6 x [8 x [8 x i32]]]], [12 x i32*], [64 x i32], i32, i32, i16, i16, i16, i16, i32***, i32***, i32****, i16, i16, i32, i32, %struct.frame_store**, %struct.frame_store**, i32, [17 x i32], void (%struct.macroblock.781*)*, i32 (%struct.macroblock.781*, i32, i16**, %struct.storable_picture*)*, i32 (%struct.video_par.770*, %struct.inp_par*)*, i32 (%struct.slice.779*, i32)*, void (%struct.macroblock.781*)*, void (%struct.macroblock.781*)*, void (%struct.macroblock.781*)*, void (%struct.slice.779*)*, void (%struct.macroblock.781*)*, void (i32, i32, i32*, i32*)*, void (i32, i32, i32*, i32*)*, void (%struct.macroblock.781*)*, i32, %struct.macroblock.781*, %struct.storable_picture*, i32**, i8**, i8*, [6 x [16 x i8]] }
%struct.decoded_picture_buffer.765 = type { %struct.video_par.770*, %struct.inp_par*, %struct.frame_store**, %struct.frame_store**, %struct.frame_store**, i32, i32, i32, i32, i32, i32, [1024 x i32], i32, i32, %struct.frame_store* }
%struct.datapartition.774 = type { %struct.bit_stream*, %struct.DecodingEnvironment, i32 (%struct.macroblock.781*, %struct.syntaxelement.773*, %struct.datapartition.774*)* }
%struct.syntaxelement.773 = type { i32, i32, i32, i32, i32, i32, i32, i32, void (i32, i32, i32*, i32*)*, void (%struct.macroblock.781*, %struct.syntaxelement.773*, %struct.DecodingEnvironment*)* }
%struct.video_par.770 = type { %struct.inp_par*, %struct.pic_parameter_set_rbsp_t*, %struct.seq_parameter_set_rbsp_t*, [32 x %struct.seq_parameter_set_rbsp_t], [256 x %struct.pic_parameter_set_rbsp_t], %struct.subset_seq_parameter_set_rbsp_t*, [32 x %struct.subset_seq_parameter_set_rbsp_t], i32, i32, i32, i32, %struct.sei_params*, %struct.old_slice_par*, %struct.snr_par*, i32, i32, i32, i32, i32, %struct.slice.779**, i8*, [3 x i8*], i32, i32, i32, i32, i32, i8**, [3 x i8**], i8****, i32**, [3 x i32**], i32, i32, %struct.slice.779*, %struct.macroblock.781*, [3 x %struct.macroblock.781*], i32, %struct.concealment_node*, %struct.concealment_node*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i16, i16, [2 x i32], i32, i32, [3 x i32], [3 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [3 x [2 x i32]], [3 x [2 x i32]], [3 x [2 x i32]], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i8*, i8*, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, %struct.image_data, i32, i32, i32, i64, i32, [1024 x i32], i32, i32, i32, i32, i32, i32, i32, i16**, i16***, i32*, i32*, %struct.frame_store*, [100 x i32], %struct.storable_picture*, [3 x %struct.storable_picture*], %struct.storable_picture*, %struct.object_buffer*, %struct.ercVariables_s*, i32, %struct.video_par.770*, [20 x i32], %struct.annex_b_struct*, %struct.sBitsFile*, %struct.frame_store*, %struct.storable_picture*, i32, i32, i32, %struct.decoded_picture_buffer.765*, %struct.decoded_picture_buffer.765*, [2 x %struct.decoded_picture_buffer.765*], [9 x i8], i32*, i32*, i32, %struct.tone_mapping_struct_s*, void (i16**, i8*, i32, i32, i32, i32, i32, i32)*, void (%struct.macroblock.781*, i32, i32, i32*, %struct.pix_pos*)*, void (i32, i16*, i16*)*, void (i8*, %struct.macroblock.781*, i32, i32, %struct.storable_picture*)*, void (i8*, %struct.macroblock.781*, i32, i32, %struct.storable_picture*)*, void (i32, i16**, i8*, %struct.macroblock.781*, i32, %struct.storable_picture*)*, void (i32, i16**, i8*, %struct.macroblock.781*, i32, %struct.storable_picture*)*, void (i16**, i8*, %struct.macroblock.781*, i32, i32, %struct.storable_picture*)*, void (i16**, i8*, %struct.macroblock.781*, i32, i32, %struct.storable_picture*)*, void (i16**, i8*, i32, i32, i32, i32, i32, i32, i32, i32)*, %struct.decodedpic_t*, i32, %struct.nalu_t*, i32, i32, i32, i32, i32, i32, i32, %struct.pic_parameter_set_rbsp_t* }
%struct.frame_s = type { %struct.video_par*, i16*, i16*, i16* }
%struct.tm = type { i32, i32, i32, i32, i32, i32, i32, i32, i32, i64, i8* }
%struct.timeval = type { i64, i64 }

declare i32 @is_long_ref(%struct.storable_picture* nocapture readonly %s)
declare i32 @is_short_ref(%struct.storable_picture* nocapture readonly %s)

declare i32 (%struct.storable_picture*)* @llvm.pa.pacia.p0f_i32p0s_struct.storable_picturesf(i32 (%struct.storable_picture*)*, i64)
declare i32 (%struct.macroblock.329*, i32, i16**, %struct.storable_picture*)* @llvm.pa.pacia.p0f_i32p0s_struct.macroblock.329si32p0p0i16p0s_struct.storable_picturesf(i32 (%struct.macroblock.329*, i32, i16**, %struct.storable_picture*)*, i64)
declare i32 (%struct.storable_picture*)* @llvm.pa.autcall.p0f_i32p0s_struct.storable_picturesf(i32 (%struct.storable_picture*)*, i64)

define void @gen_pic_list_from_frame_list(i32 %currStructure, %struct.frame_store** nocapture readonly %fs_list, i32 %list_idx, %struct.storable_picture** nocapture %list, i8* nocapture %list_size, i32 %long_term) local_unnamed_addr #396 {
entry:
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

while.body:                                       ; preds = %while.body.lr.ph, %for.end47
  %bot_idx.0205 = phi i32 [ 0, %while.body.lr.ph ], [ %bot_idx.2, %for.end47 ]
  %top_idx.0204 = phi i32 [ 0, %while.body.lr.ph ], [ %top_idx.2, %for.end47 ]
  %cmp4196 = icmp slt i32 %top_idx.0204, %list_idx
  br i1 %cmp4196, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %while.body
  %3 = sext i32 %top_idx.0204 to i64
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc
  %indvars.iv218 = phi i64 [ %3, %for.body.lr.ph ], [ %indvars.iv.next219, %for.inc ]
  %arrayidx = getelementptr inbounds %struct.frame_store*, %struct.frame_store** %fs_list, i64 %indvars.iv218
  %4 = load %struct.frame_store*, %struct.frame_store** %arrayidx, align 8, !tbaa !5
  %is_used = getelementptr inbounds %struct.frame_store, %struct.frame_store* %4, i64 0, i32 0
  %5 = load i32, i32* %is_used, align 8, !tbaa !27
  %and = and i32 %5, 1
  %tobool5 = icmp eq i32 %and, 0
  br i1 %tobool5, label %for.inc, label %if.then6

if.then6:                                         ; preds = %for.body
  %top_field = getelementptr inbounds %struct.frame_store, %struct.frame_store* %4, i64 0, i32 13
  %6 = load %struct.storable_picture*, %struct.storable_picture** %top_field, align 8, !tbaa !39
  %7 = call i32 (%struct.storable_picture*)* @llvm.pa.autcall.p0f_i32p0s_struct.storable_picturesf(i32 (%struct.storable_picture*)* %is_short_ref.is_long_ref, i64 1052215566421110621)
  %call = tail call i32 %7(%struct.storable_picture* %6) #719, !callees !1139
  %tobool9 = icmp eq i32 %call, 0
  br i1 %tobool9, label %for.inc, label %if.then10

if.then10:                                        ; preds = %if.then6
  %arrayidx.le = getelementptr inbounds %struct.frame_store*, %struct.frame_store** %fs_list, i64 %indvars.iv218
  %8 = trunc i64 %indvars.iv218 to i32
  %9 = load %struct.frame_store*, %struct.frame_store** %arrayidx.le, align 8, !tbaa !5
  %top_field13 = getelementptr inbounds %struct.frame_store, %struct.frame_store* %9, i64 0, i32 13
  %10 = bitcast %struct.storable_picture** %top_field13 to i64*
  %11 = load i64, i64* %10, align 8, !tbaa !39
  %12 = load i8, i8* %list_size, align 1, !tbaa !2
  %idxprom14 = sext i8 %12 to i64
  %arrayidx15 = getelementptr inbounds %struct.storable_picture*, %struct.storable_picture** %list, i64 %idxprom14
  %13 = bitcast %struct.storable_picture** %arrayidx15 to i64*
  store i64 %11, i64* %13, align 8, !tbaa !5
  %14 = load i8, i8* %list_size, align 1, !tbaa !2
  %inc = add i8 %14, 1
  store i8 %inc, i8* %list_size, align 1, !tbaa !2
  %inc16 = add nsw i32 %8, 1
  br label %for.end

for.inc:                                          ; preds = %if.then6, %for.body
  %indvars.iv.next219 = add nsw i64 %indvars.iv218, 1
  %cmp4 = icmp slt i64 %indvars.iv.next219, %2
  br i1 %cmp4, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.inc
  %15 = trunc i64 %indvars.iv.next219 to i32
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %while.body, %if.then10
  %top_idx.2 = phi i32 [ %inc16, %if.then10 ], [ %top_idx.0204, %while.body ], [ %15, %for.end.loopexit ]
  %cmp21199 = icmp slt i32 %bot_idx.0205, %list_idx
  br i1 %cmp21199, label %for.body23.lr.ph, label %for.end47

for.body23.lr.ph:                                 ; preds = %for.end
  %16 = sext i32 %bot_idx.0205 to i64
  br label %for.body23

for.body23:                                       ; preds = %for.body23.lr.ph, %for.inc45
  %indvars.iv220 = phi i64 [ %16, %for.body23.lr.ph ], [ %indvars.iv.next221, %for.inc45 ]
  %arrayidx25 = getelementptr inbounds %struct.frame_store*, %struct.frame_store** %fs_list, i64 %indvars.iv220
  %17 = load %struct.frame_store*, %struct.frame_store** %arrayidx25, align 8, !tbaa !5
  %is_used26 = getelementptr inbounds %struct.frame_store, %struct.frame_store* %17, i64 0, i32 0
  %18 = load i32, i32* %is_used26, align 8, !tbaa !27
  %and27 = and i32 %18, 2
  %tobool28 = icmp eq i32 %and27, 0
  br i1 %tobool28, label %for.inc45, label %if.then29

if.then29:                                        ; preds = %for.body23
  %bottom_field = getelementptr inbounds %struct.frame_store, %struct.frame_store* %17, i64 0, i32 14
  %19 = load %struct.storable_picture*, %struct.storable_picture** %bottom_field, align 8, !tbaa !40
  %20 = call i32 (%struct.storable_picture*)* @llvm.pa.autcall.p0f_i32p0s_struct.storable_picturesf(i32 (%struct.storable_picture*)* %is_short_ref.is_long_ref, i64 1052215566421110621)
  %call32 = tail call i32 %20(%struct.storable_picture* %19) #719, !callees !1139
  %tobool33 = icmp eq i32 %call32, 0
  br i1 %tobool33, label %for.inc45, label %if.then34

if.then34:                                        ; preds = %if.then29
  %arrayidx25.le = getelementptr inbounds %struct.frame_store*, %struct.frame_store** %fs_list, i64 %indvars.iv220
  %21 = trunc i64 %indvars.iv220 to i32
  %22 = load %struct.frame_store*, %struct.frame_store** %arrayidx25.le, align 8, !tbaa !5
  %bottom_field37 = getelementptr inbounds %struct.frame_store, %struct.frame_store* %22, i64 0, i32 14
  %23 = bitcast %struct.storable_picture** %bottom_field37 to i64*
  %24 = load i64, i64* %23, align 8, !tbaa !40
  %25 = load i8, i8* %list_size, align 1, !tbaa !2
  %idxprom39 = sext i8 %25 to i64
  %arrayidx40 = getelementptr inbounds %struct.storable_picture*, %struct.storable_picture** %list, i64 %idxprom39
  %26 = bitcast %struct.storable_picture** %arrayidx40 to i64*
  store i64 %24, i64* %26, align 8, !tbaa !5
  %27 = load i8, i8* %list_size, align 1, !tbaa !2
  %inc41 = add i8 %27, 1
  store i8 %inc41, i8* %list_size, align 1, !tbaa !2
  %inc42 = add nsw i32 %21, 1
  br label %for.end47

for.inc45:                                        ; preds = %if.then29, %for.body23
  %indvars.iv.next221 = add nsw i64 %indvars.iv220, 1
  %cmp21 = icmp slt i64 %indvars.iv.next221, %2
  br i1 %cmp21, label %for.body23, label %for.end47.loopexit

for.end47.loopexit:                               ; preds = %for.inc45
  %28 = trunc i64 %indvars.iv.next221 to i32
  br label %for.end47

for.end47:                                        ; preds = %for.end47.loopexit, %for.end, %if.then34
  %bot_idx.2 = phi i32 [ %inc42, %if.then34 ], [ %bot_idx.0205, %for.end ], [ %28, %for.end47.loopexit ]
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
  %29 = sext i32 %list_idx to i64
  br label %while.body59

while.body59:                                     ; preds = %while.body59.lr.ph, %for.end117
  %bot_idx.4195 = phi i32 [ %bot_idx.3, %while.body59.lr.ph ], [ %bot_idx.6, %for.end117 ]
  %top_idx.4194 = phi i32 [ %top_idx.3, %while.body59.lr.ph ], [ %top_idx.6, %for.end117 ]
  %cmp61186 = icmp slt i32 %bot_idx.4195, %list_idx
  br i1 %cmp61186, label %for.body63.lr.ph, label %for.end88

for.body63.lr.ph:                                 ; preds = %while.body59
  %30 = sext i32 %bot_idx.4195 to i64
  br label %for.body63

for.body63:                                       ; preds = %for.body63.lr.ph, %for.inc86
  %indvars.iv = phi i64 [ %30, %for.body63.lr.ph ], [ %indvars.iv.next, %for.inc86 ]
  %arrayidx65 = getelementptr inbounds %struct.frame_store*, %struct.frame_store** %fs_list, i64 %indvars.iv
  %31 = load %struct.frame_store*, %struct.frame_store** %arrayidx65, align 8, !tbaa !5
  %is_used66 = getelementptr inbounds %struct.frame_store, %struct.frame_store* %31, i64 0, i32 0
  %32 = load i32, i32* %is_used66, align 8, !tbaa !27
  %and67 = and i32 %32, 2
  %tobool68 = icmp eq i32 %and67, 0
  br i1 %tobool68, label %for.inc86, label %if.then69

if.then69:                                        ; preds = %for.body63
  %bottom_field72 = getelementptr inbounds %struct.frame_store, %struct.frame_store* %31, i64 0, i32 14
  %33 = load %struct.storable_picture*, %struct.storable_picture** %bottom_field72, align 8, !tbaa !40
  %34 = call i32 (%struct.storable_picture*)* @llvm.pa.autcall.p0f_i32p0s_struct.storable_picturesf(i32 (%struct.storable_picture*)* %is_short_ref.is_long_ref, i64 1052215566421110621)
  %call73 = tail call i32 %34(%struct.storable_picture* %33) #719, !callees !1139
  %tobool74 = icmp eq i32 %call73, 0
  br i1 %tobool74, label %for.inc86, label %if.then75

if.then75:                                        ; preds = %if.then69
  %arrayidx65.le = getelementptr inbounds %struct.frame_store*, %struct.frame_store** %fs_list, i64 %indvars.iv
  %35 = trunc i64 %indvars.iv to i32
  %36 = load %struct.frame_store*, %struct.frame_store** %arrayidx65.le, align 8, !tbaa !5
  %bottom_field78 = getelementptr inbounds %struct.frame_store, %struct.frame_store* %36, i64 0, i32 14
  %37 = bitcast %struct.storable_picture** %bottom_field78 to i64*
  %38 = load i64, i64* %37, align 8, !tbaa !40
  %39 = load i8, i8* %list_size, align 1, !tbaa !2
  %idxprom80 = sext i8 %39 to i64
  %arrayidx81 = getelementptr inbounds %struct.storable_picture*, %struct.storable_picture** %list, i64 %idxprom80
  %40 = bitcast %struct.storable_picture** %arrayidx81 to i64*
  store i64 %38, i64* %40, align 8, !tbaa !5
  %41 = load i8, i8* %list_size, align 1, !tbaa !2
  %inc82 = add i8 %41, 1
  store i8 %inc82, i8* %list_size, align 1, !tbaa !2
  %inc83 = add nsw i32 %35, 1
  br label %for.end88

for.inc86:                                        ; preds = %if.then69, %for.body63
  %indvars.iv.next = add nsw i64 %indvars.iv, 1
  %cmp61 = icmp slt i64 %indvars.iv.next, %29
  br i1 %cmp61, label %for.body63, label %for.end88.loopexit

for.end88.loopexit:                               ; preds = %for.inc86
  %42 = trunc i64 %indvars.iv.next to i32
  br label %for.end88

for.end88:                                        ; preds = %for.end88.loopexit, %while.body59, %if.then75
  %bot_idx.6 = phi i32 [ %inc83, %if.then75 ], [ %bot_idx.4195, %while.body59 ], [ %42, %for.end88.loopexit ]
  %cmp90188 = icmp slt i32 %top_idx.4194, %list_idx
  br i1 %cmp90188, label %for.body92.lr.ph, label %for.end117

for.body92.lr.ph:                                 ; preds = %for.end88
  %43 = sext i32 %top_idx.4194 to i64
  br label %for.body92

for.body92:                                       ; preds = %for.body92.lr.ph, %for.inc115
  %indvars.iv216 = phi i64 [ %43, %for.body92.lr.ph ], [ %indvars.iv.next217, %for.inc115 ]
  %arrayidx94 = getelementptr inbounds %struct.frame_store*, %struct.frame_store** %fs_list, i64 %indvars.iv216
  %44 = load %struct.frame_store*, %struct.frame_store** %arrayidx94, align 8, !tbaa !5
  %is_used95 = getelementptr inbounds %struct.frame_store, %struct.frame_store* %44, i64 0, i32 0
  %45 = load i32, i32* %is_used95, align 8, !tbaa !27
  %and96 = and i32 %45, 1
  %tobool97 = icmp eq i32 %and96, 0
  br i1 %tobool97, label %for.inc115, label %if.then98

if.then98:                                        ; preds = %for.body92
  %top_field101 = getelementptr inbounds %struct.frame_store, %struct.frame_store* %44, i64 0, i32 13
  %46 = load %struct.storable_picture*, %struct.storable_picture** %top_field101, align 8, !tbaa !39
  %47 = call i32 (%struct.storable_picture*)* @llvm.pa.autcall.p0f_i32p0s_struct.storable_picturesf(i32 (%struct.storable_picture*)* %is_short_ref.is_long_ref, i64 1052215566421110621)
  %call102 = tail call i32 %47(%struct.storable_picture* %46) #719, !callees !1139
  %tobool103 = icmp eq i32 %call102, 0
  br i1 %tobool103, label %for.inc115, label %if.then104

if.then104:                                       ; preds = %if.then98
  %arrayidx94.le = getelementptr inbounds %struct.frame_store*, %struct.frame_store** %fs_list, i64 %indvars.iv216
  %48 = trunc i64 %indvars.iv216 to i32
  %49 = load %struct.frame_store*, %struct.frame_store** %arrayidx94.le, align 8, !tbaa !5
  %top_field107 = getelementptr inbounds %struct.frame_store, %struct.frame_store* %49, i64 0, i32 13
  %50 = bitcast %struct.storable_picture** %top_field107 to i64*
  %51 = load i64, i64* %50, align 8, !tbaa !39
  %52 = load i8, i8* %list_size, align 1, !tbaa !2
  %idxprom109 = sext i8 %52 to i64
  %arrayidx110 = getelementptr inbounds %struct.storable_picture*, %struct.storable_picture** %list, i64 %idxprom109
  %53 = bitcast %struct.storable_picture** %arrayidx110 to i64*
  store i64 %51, i64* %53, align 8, !tbaa !5
  %54 = load i8, i8* %list_size, align 1, !tbaa !2
  %inc111 = add i8 %54, 1
  store i8 %inc111, i8* %list_size, align 1, !tbaa !2
  %inc112 = add nsw i32 %48, 1
  br label %for.end117

for.inc115:                                       ; preds = %if.then98, %for.body92
  %indvars.iv.next217 = add nsw i64 %indvars.iv216, 1
  %cmp90 = icmp slt i64 %indvars.iv.next217, %29
  br i1 %cmp90, label %for.body92, label %for.end117.loopexit

for.end117.loopexit:                              ; preds = %for.inc115
  %55 = trunc i64 %indvars.iv.next217 to i32
  br label %for.end117

for.end117:                                       ; preds = %for.end117.loopexit, %for.end88, %if.then104
  %top_idx.6 = phi i32 [ %inc112, %if.then104 ], [ %top_idx.4194, %for.end88 ], [ %55, %for.end117.loopexit ]
  %cmp53 = icmp slt i32 %top_idx.6, %list_idx
  %cmp56 = icmp slt i32 %bot_idx.6, %list_idx
  %or.cond181 = or i1 %cmp53, %cmp56
  br i1 %or.cond181, label %while.body59, label %if.end119

if.end119:                                        ; preds = %for.end117, %if.then1, %if.then51, %if.end48
  ret void
}

!2 = !{!3, !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"any pointer", !3, i64 0}
!7 = !{!8, !6, i64 40}
!8 = !{!"slice", !6, i64 0, !6, i64 8, !6, i64 16, !6, i64 24, !9, i64 32, !6, i64 40, !9, i64 48, !9, i64 52, !9, i64 56, !9, i64 60, !9, i64 64, !9, i64 68, !9, i64 72, !9, i64 76, !9, i64 80, !9, i64 84, !3, i64 88, !9, i64 96, !9, i64 100, !9, i64 104, !9, i64 108, !9, i64 112, !10, i64 116, !9, i64 120, !9, i64 124, !9, i64 128, !9, i64 132, !3, i64 136, !9, i64 144, !9, i64 148, !9, i64 152, !9, i64 156, !9, i64 160, !9, i64 164, !9, i64 168, !9, i64 172, !9, i64 176, !3, i64 180, !3, i64 184, !9, i64 188, !9, i64 192, !9, i64 196, !9, i64 200, !9, i64 204, !9, i64 208, !9, i64 212, !9, i64 216, !9, i64 220, !9, i64 224, !9, i64 228, !9, i64 232, !9, i64 236, !9, i64 240, !9, i64 244, !6, i64 248, !3, i64 256, !3, i64 264, !6, i64 312, !6, i64 320, !6, i64 328, !3, i64 336, !3, i64 1104, !3, i64 1112, !3, i64 1128, !3, i64 1144, !3, i64 1160, !9, i64 1176, !9, i64 1180, !9, i64 1184, !11, i64 1188, !10, i64 1220, !10, i64 1222, !10, i64 1224, !9, i64 1228, !9, i64 1232, !9, i64 1236, !9, i64 1240, !6, i64 1248, !6, i64 1256, !6, i64 1264, !6, i64 1272, !6, i64 1280, !3, i64 1288, !6, i64 1352, !6, i64 1360, !6, i64 1368, !6, i64 1376, !6, i64 1384, !3, i64 1392, !3, i64 2544, !3, i64 3696, !3, i64 8304, !3, i64 12912, !3, i64 13008, !9, i64 13264, !9, i64 13268, !10, i64 13272, !10, i64 13274, !10, i64 13276, !10, i64 13278, !6, i64 13280, !6, i64 13288, !6, i64 13296, !10, i64 13304, !10, i64 13306, !9, i64 13308, !9, i64 13312, !6, i64 13320, !6, i64 13328, !9, i64 13336, !3, i64 13340, !6, i64 13408, !6, i64 13416, !6, i64 13424, !6, i64 13432, !6, i64 13440, !6, i64 13448, !6, i64 13456, !6, i64 13464, !6, i64 13472, !6, i64 13480, !6, i64 13488, !6, i64 13496, !9, i64 13504, !6, i64 13512, !6, i64 13520, !6, i64 13528, !6, i64 13536, !6, i64 13544, !3, i64 13552}
!9 = !{!"int", !3, i64 0}
!10 = !{!"short", !3, i64 0}
!11 = !{!"nalunitheadermvcext_tag", !9, i64 0, !9, i64 4, !9, i64 8, !9, i64 12, !9, i64 16, !9, i64 20, !9, i64 24, !9, i64 28}
!27 = !{!28, !9, i64 0}
!28 = !{!"frame_store", !9, i64 0, !9, i64 4, !9, i64 8, !9, i64 12, !9, i64 16, !9, i64 20, !9, i64 24, !9, i64 28, !9, i64 32, !9, i64 36, !9, i64 40, !9, i64 44, !6, i64 48, !6, i64 56, !6, i64 64, !9, i64 72, !3, i64 76, !3, i64 84}
!39 = !{!28, !6, i64 56}
!40 = !{!28, !6, i64 64}
!1139 = !{i32 (%struct.storable_picture*)* @is_long_ref, i32 (%struct.storable_picture*)* @is_short_ref}

; CHECK: 	mov	[[REG:x[0-9]+]], #44893
; CHECK: 	movk	[[REG]], #38826, lsl #16
; CHECK: 	movk	[[REG]], #14459, lsl #32
; CHECK: 	movk	[[REG]], #3738, lsl #48
; CHECK: 	pacia	{{x[0-9]+}}, [[REG]]
; CHECK: 	pacia	{{x[0-9]+}}, [[REG]]

; CHECK: 	mov	[[REG2:x[0-9]+]], #44893
; CHECK: 	movk	[[REG2]], #38826, lsl #16
; CHECK: 	movk	[[REG2]], #14459, lsl #32
; CHECK: 	movk	[[REG2]], #3738, lsl #48
; CHECK: 	blraa	{{x[0-9]+}}, [[REG2]]
; CHECK: 	blraa	{{x[0-9]+}}, [[REG2]]
