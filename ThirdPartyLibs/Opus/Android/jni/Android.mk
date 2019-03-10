LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE       := opus
LOCAL_SRC_FILES    := \
   ../../file/src/http.c \
   ../../file/src/info.c \
   ../../file/src/internal.c \
   ../../file/src/opusfile.c \
   ../../file/src/stream.c \
   ../../file/src/wincerts.c \
   ../../lib/celt/bands.c \
   ../../lib/celt/celt.c \
   ../../lib/celt/celt_decoder.c \
   ../../lib/celt/celt_encoder.c \
   ../../lib/celt/celt_lpc.c \
   ../../lib/celt/cwrs.c \
   ../../lib/celt/entcode.c \
   ../../lib/celt/entdec.c \
   ../../lib/celt/entenc.c \
   ../../lib/celt/kiss_fft.c \
   ../../lib/celt/laplace.c \
   ../../lib/celt/mathops.c \
   ../../lib/celt/mdct.c \
   ../../lib/celt/modes.c \
   ../../lib/celt/pitch.c \
   ../../lib/celt/quant_bands.c \
   ../../lib/celt/rate.c \
   ../../lib/celt/vq.c \
   ../../lib/silk/A2NLSF.c \
   ../../lib/silk/ana_filt_bank_1.c \
   ../../lib/silk/biquad_alt.c \
   ../../lib/silk/bwexpander.c \
   ../../lib/silk/bwexpander_32.c \
   ../../lib/silk/check_control_input.c \
   ../../lib/silk/CNG.c \
   ../../lib/silk/code_signs.c \
   ../../lib/silk/control_audio_bandwidth.c \
   ../../lib/silk/control_codec.c \
   ../../lib/silk/control_SNR.c \
   ../../lib/silk/debug.c \
   ../../lib/silk/dec_API.c \
   ../../lib/silk/decode_core.c \
   ../../lib/silk/decode_frame.c \
   ../../lib/silk/decode_indices.c \
   ../../lib/silk/decode_parameters.c \
   ../../lib/silk/decode_pitch.c \
   ../../lib/silk/decode_pulses.c \
   ../../lib/silk/decoder_set_fs.c \
   ../../lib/silk/enc_API.c \
   ../../lib/silk/encode_indices.c \
   ../../lib/silk/encode_pulses.c \
   ../../lib/silk/float/apply_sine_window_FLP.c \
   ../../lib/silk/float/autocorrelation_FLP.c \
   ../../lib/silk/float/burg_modified_FLP.c \
   ../../lib/silk/float/bwexpander_FLP.c \
   ../../lib/silk/float/corrMatrix_FLP.c \
   ../../lib/silk/float/encode_frame_FLP.c \
   ../../lib/silk/float/energy_FLP.c \
   ../../lib/silk/float/find_LPC_FLP.c \
   ../../lib/silk/float/find_LTP_FLP.c \
   ../../lib/silk/float/find_pitch_lags_FLP.c \
   ../../lib/silk/float/find_pred_coefs_FLP.c \
   ../../lib/silk/float/inner_product_FLP.c \
   ../../lib/silk/float/k2a_FLP.c \
   ../../lib/silk/float/LPC_analysis_filter_FLP.c \
   ../../lib/silk/float/LPC_inv_pred_gain_FLP.c \
   ../../lib/silk/float/LTP_analysis_filter_FLP.c \
   ../../lib/silk/float/LTP_scale_ctrl_FLP.c \
   ../../lib/silk/float/noise_shape_analysis_FLP.c \
   ../../lib/silk/float/pitch_analysis_core_FLP.c \
   ../../lib/silk/float/process_gains_FLP.c \
   ../../lib/silk/float/regularize_correlations_FLP.c \
   ../../lib/silk/float/residual_energy_FLP.c \
   ../../lib/silk/float/scale_copy_vector_FLP.c \
   ../../lib/silk/float/scale_vector_FLP.c \
   ../../lib/silk/float/schur_FLP.c \
   ../../lib/silk/float/sort_FLP.c \
   ../../lib/silk/float/warped_autocorrelation_FLP.c \
   ../../lib/silk/float/wrappers_FLP.c \
   ../../lib/silk/gain_quant.c \
   ../../lib/silk/HP_variable_cutoff.c \
   ../../lib/silk/init_decoder.c \
   ../../lib/silk/init_encoder.c \
   ../../lib/silk/inner_prod_aligned.c \
   ../../lib/silk/interpolate.c \
   ../../lib/silk/lin2log.c \
   ../../lib/silk/log2lin.c \
   ../../lib/silk/LP_variable_cutoff.c \
   ../../lib/silk/LPC_analysis_filter.c \
   ../../lib/silk/LPC_fit.c \
   ../../lib/silk/LPC_inv_pred_gain.c \
   ../../lib/silk/NLSF_decode.c \
   ../../lib/silk/NLSF_del_dec_quant.c \
   ../../lib/silk/NLSF_encode.c \
   ../../lib/silk/NLSF_stabilize.c \
   ../../lib/silk/NLSF_unpack.c \
   ../../lib/silk/NLSF_VQ.c \
   ../../lib/silk/NLSF_VQ_weights_laroia.c \
   ../../lib/silk/NLSF2A.c \
   ../../lib/silk/NSQ.c \
   ../../lib/silk/NSQ_del_dec.c \
   ../../lib/silk/pitch_est_tables.c \
   ../../lib/silk/PLC.c \
   ../../lib/silk/process_NLSFs.c \
   ../../lib/silk/quant_LTP_gains.c \
   ../../lib/silk/resampler.c \
   ../../lib/silk/resampler_down2.c \
   ../../lib/silk/resampler_down2_3.c \
   ../../lib/silk/resampler_private_AR2.c \
   ../../lib/silk/resampler_private_down_FIR.c \
   ../../lib/silk/resampler_private_IIR_FIR.c \
   ../../lib/silk/resampler_private_up2_HQ.c \
   ../../lib/silk/resampler_rom.c \
   ../../lib/silk/shell_coder.c \
   ../../lib/silk/sigm_Q15.c \
   ../../lib/silk/sort.c \
   ../../lib/silk/stereo_decode_pred.c \
   ../../lib/silk/stereo_encode_pred.c \
   ../../lib/silk/stereo_find_predictor.c \
   ../../lib/silk/stereo_LR_to_MS.c \
   ../../lib/silk/stereo_MS_to_LR.c \
   ../../lib/silk/stereo_quant_pred.c \
   ../../lib/silk/sum_sqr_shift.c \
   ../../lib/silk/table_LSF_cos.c \
   ../../lib/silk/tables_gain.c \
   ../../lib/silk/tables_LTP.c \
   ../../lib/silk/tables_NLSF_CB_NB_MB.c \
   ../../lib/silk/tables_NLSF_CB_WB.c \
   ../../lib/silk/tables_other.c \
   ../../lib/silk/tables_pitch_lag.c \
   ../../lib/silk/tables_pulses_per_block.c \
   ../../lib/silk/VAD.c \
   ../../lib/silk/VQ_WMat_EC.c \
   ../../lib/src/analysis.c \
   ../../lib/src/mapping_matrix.c \
   ../../lib/src/mlp.c \
   ../../lib/src/mlp_data.c \
   ../../lib/src/opus.c \
   ../../lib/src/opus_decoder.c \
   ../../lib/src/opus_encoder.c \
   ../../lib/src/opus_multistream.c \
   ../../lib/src/opus_multistream_decoder.c \
   ../../lib/src/opus_multistream_encoder.c \
   ../../lib/src/opus_projection_decoder.c \
   ../../lib/src/opus_projection_encoder.c \
   ../../lib/src/repacketizer.c
LOCAL_CFLAGS       := -DHAVE_CONFIG_H -I.. -I../lib -I../lib/celt -I../lib/silk -I../lib/silk/float -I../lib/silk/fixed -I../lib/include -I../file/include -I../../Ogg/include -O3 -fshort-wchar -ffast-math -fomit-frame-pointer
LOCAL_CPPFLAGS     := -DHAVE_CONFIG_H -I.. -I../lib -I../lib/celt -I../lib/silk -I../lib/silk/float -I../lib/silk/fixed -I../lib/include -I../file/include -I../../Ogg/include -O3 -fshort-wchar -ffast-math -fomit-frame-pointer
LOCAL_CPP_FEATURES := # rtti exceptions
LOCAL_ARM_NEON     := true # force NEON usage for all files

ifeq ($(TARGET_ARCH_ABI), $(filter $(TARGET_ARCH_ABI), armeabi-v7a arm64-v8a))
   LOCAL_SRC_FILES += \
      ../../lib/celt/arm/arm_celt_map.c.neon \
      ../../lib/celt/arm/armcpu.c.neon \
      ../../lib/celt/arm/celt_neon_intr.c.neon \
      ../../lib/celt/arm/pitch_neon_intr.c.neon \
      ../../lib/silk/arm/arm_silk_map.c.neon \
      ../../lib/silk/arm/biquad_alt_neon_intr.c.neon \
      ../../lib/silk/arm/LPC_inv_pred_gain_neon_intr.c.neon \
      ../../lib/silk/arm/NSQ_del_dec_neon_intr.c.neon \
      ../../lib/silk/arm/NSQ_neon.c.neon
endif

ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
   LOCAL_SRC_FILES += \
      ../../lib/celt/arm/celt_pitch_xcorr_arm-gnu.S.neon
endif

ifeq ($(TARGET_ARCH_ABI), x86)
   LOCAL_SRC_FILES += \
      ../../lib/celt/x86/celt_lpc_sse4_1.c \
      ../../lib/celt/x86/pitch_sse.c \
      ../../lib/celt/x86/pitch_sse2.c \
      ../../lib/celt/x86/pitch_sse4_1.c \
      ../../lib/celt/x86/vq_sse2.c \
      ../../lib/celt/x86/x86_celt_map.c \
      ../../lib/celt/x86/x86cpu.c \
      ../../lib/silk/x86/VAD_sse4_1.c \
      ../../lib/silk/x86/x86_silk_map.c
      #../../lib/silk/x86/NSQ_sse4_1.c
      #../../lib/silk/x86/NSQ_del_dec_sse4_1.c
      #../../lib/silk/x86/VQ_WMat_EC_sse4_1.c
endif

include $(BUILD_STATIC_LIBRARY)
