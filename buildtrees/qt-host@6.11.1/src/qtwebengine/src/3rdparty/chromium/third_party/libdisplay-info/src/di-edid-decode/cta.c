#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <libdisplay-info/cta.h>

#include "di-edid-decode.h"

static const char *
video_format_picture_aspect_ratio_name(enum di_cta_video_format_picture_aspect_ratio ar)
{
	switch (ar) {
	case DI_CTA_VIDEO_FORMAT_PICTURE_ASPECT_RATIO_4_3:
		return "  4:3  ";
	case DI_CTA_VIDEO_FORMAT_PICTURE_ASPECT_RATIO_16_9:
		return " 16:9  ";
	case DI_CTA_VIDEO_FORMAT_PICTURE_ASPECT_RATIO_64_27:
		return " 64:27 ";
	case DI_CTA_VIDEO_FORMAT_PICTURE_ASPECT_RATIO_256_135:
		return "256:135";
	}
	abort(); /* unreachable */
}

static void
print_vic(uint8_t vic)
{
	const struct di_cta_video_format *fmt;
	int32_t h_blank, v_blank, v_active;
	double refresh, h_freq_hz, pixel_clock_mhz, h_total, v_total;
	char buf[10];

	printf("    VIC %3" PRIu8, vic);

	fmt = di_cta_video_format_from_vic(vic);
	if (fmt == NULL)
		return;

	v_active = fmt->v_active;
	if (fmt->interlaced)
		v_active /= 2;

	h_blank = fmt->h_front + fmt->h_sync + fmt->h_back;
	v_blank = fmt->v_front + fmt->v_sync + fmt->v_back;
	h_total = fmt->h_active + h_blank;

	v_total = v_active + v_blank;
	if (fmt->interlaced)
		v_total += 0.5;

	refresh = (double) fmt->pixel_clock_hz / (h_total * v_total);
	h_freq_hz = (double) fmt->pixel_clock_hz / h_total;
	pixel_clock_mhz = (double) fmt->pixel_clock_hz / (1000 * 1000);

	snprintf(buf, sizeof(buf), "%d%s",
		 fmt->v_active,
		 fmt->interlaced ? "i" : "");

	printf(":");
	printf(" %5dx%-5s", fmt->h_active, buf);
	printf(" %10.6f Hz", refresh);
	printf(" %s", video_format_picture_aspect_ratio_name(fmt->picture_aspect_ratio));
	printf(" %8.3f kHz %13.6f MHz", h_freq_hz / 1000, pixel_clock_mhz);
}

static void
printf_cta_svd(const struct di_cta_svd *svd)
{
	print_vic(svd->vic);
	if (svd->native)
		printf(" (native)");
	printf("\n");
}

static void
printf_cta_svds(const struct di_cta_svd *const *svds)
{
	size_t i;

	for (i = 0; svds[i] != NULL; i++)
		printf_cta_svd(svds[i]);
}

static const char *
vesa_dddb_interface_type_name(enum di_cta_vesa_dddb_interface_type type)
{
	switch (type) {
	case DI_CTA_VESA_DDDB_INTERFACE_VGA:
		return "Analog (15HD/VGA)";
	case DI_CTA_VESA_DDDB_INTERFACE_NAVI_V:
		return "Analog (VESA NAVI-V (15HD))";
	case DI_CTA_VESA_DDDB_INTERFACE_NAVI_D:
		return "Analog (VESA NAVI-D)";
	case DI_CTA_VESA_DDDB_INTERFACE_LVDS:
		return "LVDS";
	case DI_CTA_VESA_DDDB_INTERFACE_RSDS:
		return "RSDS";
	case DI_CTA_VESA_DDDB_INTERFACE_DVI_D:
		return "DVI-D";
	case DI_CTA_VESA_DDDB_INTERFACE_DVI_I_ANALOG:
		return "DVI-I analog";
	case DI_CTA_VESA_DDDB_INTERFACE_DVI_I_DIGITAL:
		return "DVI-I digital";
	case DI_CTA_VESA_DDDB_INTERFACE_HDMI_A:
		return "HDMI-A";
	case DI_CTA_VESA_DDDB_INTERFACE_HDMI_B:
		return "HDMI-B";
	case DI_CTA_VESA_DDDB_INTERFACE_MDDI:
		return "MDDI";
	case DI_CTA_VESA_DDDB_INTERFACE_DISPLAYPORT:
		return "DisplayPort";
	case DI_CTA_VESA_DDDB_INTERFACE_IEEE_1394:
		return "IEEE-1394";
	case DI_CTA_VESA_DDDB_INTERFACE_M1_ANALOG:
		return "M1 analog";
	case DI_CTA_VESA_DDDB_INTERFACE_M1_DIGITAL:
		return "M1 digital";
	}
	abort(); /* unreachable */
}

static const char *
vesa_dddb_content_protection_name(enum di_cta_vesa_dddb_content_protection cp)
{
	switch (cp) {
	case DI_CTA_VESA_DDDB_CONTENT_PROTECTION_NONE:
		return "None";
	case DI_CTA_VESA_DDDB_CONTENT_PROTECTION_HDCP:
		return "HDCP";
	case DI_CTA_VESA_DDDB_CONTENT_PROTECTION_DTCP:
		return "DTCP";
	case DI_CTA_VESA_DDDB_CONTENT_PROTECTION_DPCP:
		return "DPCP";
	}
	abort(); /* unreachable */
}

static const char *
vesa_dddb_default_orientation_name(enum di_cta_vesa_dddb_default_orientation orientation)
{
	switch (orientation) {
	case DI_CTA_VESA_DDDB_DEFAULT_ORIENTATION_LANDSCAPE:
		return "Landscape";
	case DI_CTA_VESA_DDDB_DEFAULT_ORIENTATION_PORTAIT:
		return "Portrait";
	case DI_CTA_VESA_DDDB_DEFAULT_ORIENTATION_UNFIXED:
		return "Not Fixed";
	case DI_CTA_VESA_DDDB_DEFAULT_ORIENTATION_UNDEFINED:
		return "Undefined";
	}
	abort(); /* unreachable */
}

static const char *
vesa_dddb_rotation_cap_name(enum di_cta_vesa_dddb_rotation_cap rot)
{
	switch (rot) {
	case DI_CTA_VESA_DDDB_ROTATION_CAP_NONE:
		return "None";
	case DI_CTA_VESA_DDDB_ROTATION_CAP_90DEG_CLOCKWISE:
		return "Can rotate 90 degrees clockwise";
	case DI_CTA_VESA_DDDB_ROTATION_CAP_90DEG_COUNTERCLOCKWISE:
		return "Can rotate 90 degrees counterclockwise";
	case DI_CTA_VESA_DDDB_ROTATION_CAP_90DEG_EITHER:
		return "Can rotate 90 degrees in either direction";
	}
	abort(); /* unreachable */
}

static const char *
vesa_dddb_zero_pixel_location_name(enum di_cta_vesa_dddb_zero_pixel_location loc)
{
	switch (loc) {
	case DI_CTA_VESA_DDDB_ZERO_PIXEL_UPPER_LEFT:
		return "Upper Left";
	case DI_CTA_VESA_DDDB_ZERO_PIXEL_UPPER_RIGHT:
		return "Upper Right";
	case DI_CTA_VESA_DDDB_ZERO_PIXEL_LOWER_LEFT:
		return "Lower Left";
	case DI_CTA_VESA_DDDB_ZERO_PIXEL_LOWER_RIGHT:
		return "Lower Right";
	}
	abort(); /* unreachable */
}

static const char *
vesa_dddb_scan_direction_name(enum di_cta_vesa_dddb_scan_direction dir)
{
	switch (dir) {
	case DI_CTA_VESA_DDDB_SCAN_DIRECTION_UNDEFINED:
		return "Not defined";
	case DI_CTA_VESA_DDDB_SCAN_DIRECTION_FAST_LONG_SLOW_SHORT:
		return "Fast Scan is on the Major (Long) Axis and Slow Scan is on the Minor Axis";
	case DI_CTA_VESA_DDDB_SCAN_DIRECTION_FAST_SHORT_SLOW_LONG:
		return "Fast Scan is on the Minor (Short) Axis and Slow Scan is on the Major Axis";
	}
	abort(); /* unreachable */
}

static const char *
vesa_dddb_subpixel_layout_name(enum di_cta_vesa_dddb_subpixel_layout subpixel)
{
	switch (subpixel) {
	case DI_CTA_VESA_DDDB_SUBPIXEL_UNDEFINED:
		return "Not defined";
	case DI_CTA_VESA_DDDB_SUBPIXEL_RGB_VERT:
		return "RGB vertical stripes";
	case DI_CTA_VESA_DDDB_SUBPIXEL_RGB_HORIZ:
		return "RGB horizontal stripes";
	case DI_CTA_VESA_DDDB_SUBPIXEL_EDID_CHROM_VERT:
		return "Vertical stripes using primary order";
	case DI_CTA_VESA_DDDB_SUBPIXEL_EDID_CHROM_HORIZ:
		return "Horizontal stripes using primary order";
	case DI_CTA_VESA_DDDB_SUBPIXEL_QUAD_RGGB:
		return "Quad sub-pixels, red at top left";
	case DI_CTA_VESA_DDDB_SUBPIXEL_QUAD_GBRG:
		return "Quad sub-pixels, red at bottom left";
	case DI_CTA_VESA_DDDB_SUBPIXEL_DELTA_RGB:
		return "Delta (triad) RGB sub-pixels";
	case DI_CTA_VESA_DDDB_SUBPIXEL_MOSAIC:
		return "Mosaic";
	case DI_CTA_VESA_DDDB_SUBPIXEL_QUAD_ANY:
		return "Quad sub-pixels, RGB + 1 additional color";
	case DI_CTA_VESA_DDDB_SUBPIXEL_FIVE:
		return "Five sub-pixels, RGB + 2 additional colors";
	case DI_CTA_VESA_DDDB_SUBPIXEL_SIX:
		return "Six sub-pixels, RGB + 3 additional colors";
	case DI_CTA_VESA_DDDB_SUBPIXEL_CLAIRVOYANTE_PENTILE:
		return "Clairvoyante, Inc. PenTile Matrix (tm) layout";
	}
	abort(); /* unreachable */
}

static const char *
vesa_dddb_dithering_type_name(enum di_cta_vesa_dddb_dithering_type dithering)
{
	switch (dithering) {
	case DI_CTA_VESA_DDDB_DITHERING_NONE:
		return "None";
	case DI_CTA_VESA_DDDB_DITHERING_SPACIAL:
		return "Spacial";
	case DI_CTA_VESA_DDDB_DITHERING_TEMPORAL:
		return "Temporal";
	case DI_CTA_VESA_DDDB_DITHERING_SPATIAL_AND_TEMPORAL:
		return "Spatial and Temporal";
	}
	abort(); /* unreachable */
}

static const char *
vesa_dddb_frame_rate_conversion_name(enum di_cta_vesa_dddb_frame_rate_conversion conv)
{
	switch (conv) {
	case DI_CTA_VESA_DDDB_FRAME_RATE_CONVERSION_NONE:
		return "None";
	case DI_CTA_VESA_DDDB_FRAME_RATE_CONVERSION_SINGLE_BUFFERING:
		return "Single Buffering";
	case DI_CTA_VESA_DDDB_FRAME_RATE_CONVERSION_DOUBLE_BUFFERING:
		return "Double Buffering";
	case DI_CTA_VESA_DDDB_FRAME_RATE_CONVERSION_ADVANCED:
		return "Advanced Frame Rate Conversion";
	}
	abort(); /* unreachable */
}

static float
truncate_chromaticity_coord(float coord)
{
	return floorf(coord * 10000) / 10000;
}

static const char *
vesa_dddb_resp_time_transition_name(enum di_cta_vesa_dddb_resp_time_transition t)
{
	switch (t) {
	case DI_CTA_VESA_DDDB_RESP_TIME_BLACK_TO_WHITE:
		return "Black -> White";
	case DI_CTA_VESA_DDDB_RESP_TIME_WHITE_TO_BLACK:
		return "White -> Black";
	}
	abort(); /* unreachable */
}

static void
print_cta_vesa_dddb(const struct di_cta_vesa_dddb *dddb)
{
	size_t i;

	printf("    Interface Type: %s",
	       vesa_dddb_interface_type_name(dddb->interface_type));
	if (dddb->num_channels != 0) {
		const char *kind;
		switch (dddb->interface_type) {
		case DI_CTA_VESA_DDDB_INTERFACE_LVDS:
		case DI_CTA_VESA_DDDB_INTERFACE_RSDS:
			kind = "lanes";
			break;
		default:
			kind = "channels";
			break;
		}
		printf(" %d %s", dddb->num_channels, kind);
	}
	printf("\n");

	printf("    Interface Standard Version: %d.%d\n",
	       dddb->interface_version, dddb->interface_release);

	printf("    Content Protection Support: %s\n",
	       vesa_dddb_content_protection_name(dddb->content_protection));

	printf("    Minimum Clock Frequency: %d MHz\n", dddb->min_clock_freq_mhz);
	printf("    Maximum Clock Frequency: %d MHz\n", dddb->max_clock_freq_mhz);
	printf("    Device Native Pixel Format: %dx%d\n",
	       dddb->native_horiz_pixels, dddb->native_vert_pixels);
	printf("    Aspect Ratio: %.2f\n", dddb->aspect_ratio);
	printf("    Default Orientation: %s\n",
	       vesa_dddb_default_orientation_name(dddb->default_orientation));
	printf("    Rotation Capability: %s\n",
	       vesa_dddb_rotation_cap_name(dddb->rotation_cap));
	printf("    Zero Pixel Location: %s\n",
	       vesa_dddb_zero_pixel_location_name(dddb->zero_pixel_location));
	printf("    Scan Direction: %s\n",
	       vesa_dddb_scan_direction_name(dddb->scan_direction));
	printf("    Subpixel Information: %s\n",
	       vesa_dddb_subpixel_layout_name(dddb->subpixel_layout));
	printf("    Horizontal and vertical dot/pixel pitch: %.2f x %.2f mm\n",
	       dddb->horiz_pitch_mm, dddb->vert_pitch_mm);
	printf("    Dithering: %s\n",
	       vesa_dddb_dithering_type_name(dddb->dithering_type));
	printf("    Direct Drive: %s\n", dddb->direct_drive ? "Yes" : "No");
	printf("    Overdrive %srecommended\n",
	       dddb->overdrive_not_recommended ? "not " : "");
	printf("    Deinterlacing: %s\n", dddb->deinterlacing ? "Yes" : "No");

	printf("    Audio Support: %s\n", dddb->audio_support ? "Yes" : "No");
	printf("    Separate Audio Inputs Provided: %s\n",
	       dddb->separate_audio_inputs ? "Yes" : "No");
	printf("    Audio Input Override: %s\n",
	       dddb->audio_input_override ? "Yes" : "No");
	if (dddb->audio_delay_provided)
		printf("    Audio Delay: %d ms\n", dddb->audio_delay_ms);
	else
		printf("    Audio Delay: no information provided\n");

	printf("    Frame Rate/Mode Conversion: %s\n",
	       vesa_dddb_frame_rate_conversion_name(dddb->frame_rate_conversion));
	if (dddb->frame_rate_range_hz != 0)
		printf("    Frame Rate Range: %d fps +/- %d fps\n",
		       dddb->frame_rate_native_hz, dddb->frame_rate_range_hz);
	else
		printf("    Nominal Frame Rate: %d fps\n",
		       dddb->frame_rate_native_hz);
	printf("    Color Bit Depth: %d @ interface, %d @ display\n",
	       dddb->bit_depth_interface, dddb->bit_depth_display);

	if (dddb->additional_primary_chromaticities_len > 0) {
		printf("    Additional Primary Chromaticities:\n");
		for (i = 0; i < dddb->additional_primary_chromaticities_len; i++)
			printf("      Primary %zu:   %.4f, %.4f\n", 4 + i,
			       truncate_chromaticity_coord(dddb->additional_primary_chromaticities[i].x),
			       truncate_chromaticity_coord(dddb->additional_primary_chromaticities[i].y));
	}

	printf("    Response Time %s: %d ms\n",
	       vesa_dddb_resp_time_transition_name(dddb->resp_time_transition),
	       dddb->resp_time_ms);
	printf("    Overscan: %d%% x %d%%\n",
	       dddb->overscan_horiz_pct, dddb->overscan_vert_pct);
}

static uint8_t
encode_max_luminance(float max)
{
	if (max == 0)
		return 0;
	return (uint8_t) (log2f(max / 50) * 32);
}

static uint8_t
encode_min_luminance(float min, float max)
{
	if (min == 0)
		return 0;
	return (uint8_t) (255 * sqrtf(min / max * 100));
}

static void
print_cta_hdr_static_metadata(const struct di_cta_hdr_static_metadata_block *metadata)
{
	printf("    Electro optical transfer functions:\n");
	if (metadata->eotfs->traditional_sdr)
		printf("      Traditional gamma - SDR luminance range\n");
	if (metadata->eotfs->traditional_hdr)
		printf("      Traditional gamma - HDR luminance range\n");
	if (metadata->eotfs->pq)
		printf("      SMPTE ST2084\n");
	if (metadata->eotfs->hlg)
		printf("      Hybrid Log-Gamma\n");

	printf("    Supported static metadata descriptors:\n");
	if (metadata->descriptors->type1)
		printf("      Static metadata type 1\n");

	/* TODO: figure out a way to print raw values? */
	if (metadata->desired_content_max_luminance != 0)
		printf("    Desired content max luminance: %" PRIu8 " (%.3f cd/m^2)\n",
		       encode_max_luminance(metadata->desired_content_max_luminance),
		       metadata->desired_content_max_luminance);
	if (metadata->desired_content_max_frame_avg_luminance != 0)
		printf("    Desired content max frame-average luminance: %" PRIu8 " (%.3f cd/m^2)\n",
		       encode_max_luminance(metadata->desired_content_max_frame_avg_luminance),
		       metadata->desired_content_max_frame_avg_luminance);
	if (metadata->desired_content_min_luminance != 0)
		printf("    Desired content min luminance: %" PRIu8 " (%.3f cd/m^2)\n",
		       encode_min_luminance(metadata->desired_content_min_luminance,
					    metadata->desired_content_max_luminance),
		       metadata->desired_content_min_luminance);
}

static void
print_cta_hdr_dynamic_metadata(const struct di_cta_hdr_dynamic_metadata_block *metadata)
{
	if (metadata->type1) {
		printf("    HDR Dynamic Metadata Type 1\n");
		printf("      Version: %d\n", metadata->type1->type_1_hdr_metadata_version);
	}
	if (metadata->type2) {
		printf("    HDR Dynamic Metadata Type 2\n");
		printf("      Version: %d\n", metadata->type2->ts_103_433_spec_version);
		if (metadata->type2->ts_103_433_1_capable)
			printf("      ETSI TS 103 433-1 capable\n");
		if (metadata->type2->ts_103_433_2_capable)
			printf("      ETSI TS 103 433-2 [i.12] capable\n");
		if (metadata->type2->ts_103_433_3_capable)
			printf("      ETSI TS 103 433-3 [i.13] capable\n");
	}
	if (metadata->type3) {
		printf("    HDR Dynamic Metadata Type 3\n");
	}
	if (metadata->type4) {
		printf("    HDR Dynamic Metadata Type 4\n");
		printf("      Version: %d\n", metadata->type4->type_4_hdr_metadata_version);
	}
	if (metadata->type256) {
		printf("    HDR Dynamic Metadata Type 256\n");
		printf("      Version: %d\n", metadata->type256->graphics_overlay_flag_version);
	}
}

static void
print_cta_vesa_transfer_characteristics(const struct di_cta_vesa_transfer_characteristics *tf)
{
	size_t i;

	switch (tf->usage) {
	case DI_CTA_VESA_TRANSFER_CHARACTERISTIC_USAGE_WHITE:
		printf("    White");
		break;
	case DI_CTA_VESA_TRANSFER_CHARACTERISTIC_USAGE_RED:
		printf("    Red");
		break;
	case DI_CTA_VESA_TRANSFER_CHARACTERISTIC_USAGE_GREEN:
		printf("    Green");
		break;
	case DI_CTA_VESA_TRANSFER_CHARACTERISTIC_USAGE_BLUE:
		printf("    Blue");
		break;
	}

	printf(" transfer characteristics:");
	for (i = 0; i < tf->points_len; i++)
		printf(" %u", (uint16_t) roundf(tf->points[i] * 1023.0f));
	printf("\n");

	uncommon_features.cta_transfer_characteristics = true;
}

static const char *
cta_audio_format_name(enum di_cta_audio_format format)
{
	switch (format) {
	case DI_CTA_AUDIO_FORMAT_LPCM:
		return "Linear PCM";
	case DI_CTA_AUDIO_FORMAT_AC3:
		return "AC-3";
	case DI_CTA_AUDIO_FORMAT_MPEG1:
		return "MPEG 1 (Layers 1 & 2)";
	case DI_CTA_AUDIO_FORMAT_MP3:
		return "MPEG 1 Layer 3 (MP3)";
	case DI_CTA_AUDIO_FORMAT_MPEG2:
		return "MPEG2 (multichannel)";
	case DI_CTA_AUDIO_FORMAT_AAC_LC:
		return "AAC LC";
	case DI_CTA_AUDIO_FORMAT_DTS:
		return "DTS";
	case DI_CTA_AUDIO_FORMAT_ATRAC:
		return "ATRAC";
	case DI_CTA_AUDIO_FORMAT_ONE_BIT_AUDIO:
		return "One Bit Audio";
	case DI_CTA_AUDIO_FORMAT_ENHANCED_AC3:
		return "Enhanced AC-3 (DD+)";
	case DI_CTA_AUDIO_FORMAT_DTS_HD:
		return "DTS-HD";
	case DI_CTA_AUDIO_FORMAT_MAT:
		return "MAT (MLP)";
	case DI_CTA_AUDIO_FORMAT_DST:
		return "DST";
	case DI_CTA_AUDIO_FORMAT_WMA_PRO:
		return "WMA Pro";
	case DI_CTA_AUDIO_FORMAT_MPEG4_HE_AAC:
		return "MPEG-4 HE AAC";
	case DI_CTA_AUDIO_FORMAT_MPEG4_HE_AAC_V2:
		return "MPEG-4 HE AAC v2";
	case DI_CTA_AUDIO_FORMAT_MPEG4_AAC_LC:
		return "MPEG-4 AAC LC";
	case DI_CTA_AUDIO_FORMAT_DRA:
		return "DRA";
	case DI_CTA_AUDIO_FORMAT_MPEG4_HE_AAC_MPEG_SURROUND:
		return "MPEG-4 HE AAC + MPEG Surround";
	case DI_CTA_AUDIO_FORMAT_MPEG4_AAC_LC_MPEG_SURROUND:
		return "MPEG-4 AAC LC + MPEG Surround";
	case DI_CTA_AUDIO_FORMAT_MPEGH_3D:
		return "MPEG-H 3D Audio";
	case DI_CTA_AUDIO_FORMAT_AC4:
		return "AC-4";
	case DI_CTA_AUDIO_FORMAT_LPCM_3D:
		return "L-PCM 3D Audio";
	}
	abort();
}

static const char *
cta_sad_mpegh_3d_level_name(enum di_cta_sad_mpegh_3d_level level)
{
	switch (level) {
	case DI_CTA_SAD_MPEGH_3D_LEVEL_UNSPECIFIED:
		return "Unspecified";
	case DI_CTA_SAD_MPEGH_3D_LEVEL_1:
		return "Level 1";
	case DI_CTA_SAD_MPEGH_3D_LEVEL_2:
		return "Level 2";
	case DI_CTA_SAD_MPEGH_3D_LEVEL_3:
		return "Level 3";
	case DI_CTA_SAD_MPEGH_3D_LEVEL_4:
		return "Level 4";
	case DI_CTA_SAD_MPEGH_3D_LEVEL_5:
		return "Level 5";
	}
	abort();
}

static void
print_cta_sads(const struct di_cta_sad *const *sads)
{
	size_t i;
	const struct di_cta_sad *sad;

	for (i = 0; sads[i] != NULL; i++) {
		sad = sads[i];

		printf("    %s:\n", cta_audio_format_name(sad->format));
		if (sad->max_channels != 0)
			printf("      Max channels: %d\n", sad->max_channels);

		if (sad->mpegh_3d)
			printf("      MPEG-H 3D Audio Level: %s\n",
			       cta_sad_mpegh_3d_level_name(sad->mpegh_3d->level));

		printf("      Supported sample rates (kHz):");
		if (sad->supported_sample_rates->has_192_khz)
			printf(" 192");
		if (sad->supported_sample_rates->has_176_4_khz)
			printf(" 176.4");
		if (sad->supported_sample_rates->has_96_khz)
			printf(" 96");
		if (sad->supported_sample_rates->has_88_2_khz)
			printf(" 88.2");
		if (sad->supported_sample_rates->has_48_khz)
			printf(" 48");
		if (sad->supported_sample_rates->has_44_1_khz)
			printf(" 44.1");
		if (sad->supported_sample_rates->has_32_khz)
			printf(" 32");
		printf("\n");

		if (sad->lpcm) {
			printf("      Supported sample sizes (bits):");
			if (sad->lpcm->has_sample_size_24_bits)
				printf(" 24");
			if (sad->lpcm->has_sample_size_20_bits)
				printf(" 20");
			if (sad->lpcm->has_sample_size_16_bits)
				printf(" 16");
			printf("\n");
		}

		if (sad->max_bitrate_kbs != 0)
			printf("      Maximum bit rate: %d kb/s\n", sad->max_bitrate_kbs);

		if (sad->enhanced_ac3 && sad->enhanced_ac3->supports_joint_object_coding)
			printf("      Supports Joint Object Coding\n");
		if (sad->enhanced_ac3 && sad->enhanced_ac3->supports_joint_object_coding_ACMOD28)
			printf("      Supports Joint Object Coding with ACMOD28\n");

		if (sad->mat) {
			if (sad->mat->supports_object_audio_and_channel_based) {
				printf("      Supports Dolby TrueHD, object audio PCM and channel-based PCM\n");
				printf("      Hash calculation %srequired for object audio PCM or channel-based PCM\n",
				       sad->mat->requires_hash_calculation ? "" : "not ");
			} else {
				printf("      Supports only Dolby TrueHD\n");
			}
		}

		if (sad->wma_pro) {
			printf("      Profile: %u\n",sad->wma_pro->profile);
		}

		if (sad->mpegh_3d && sad->mpegh_3d->low_complexity_profile)
			printf("      Supports MPEG-H 3D Audio Low Complexity Profile\n");
		if (sad->mpegh_3d && sad->mpegh_3d->baseline_profile)
			printf("      Supports MPEG-H 3D Audio Baseline Profile\n");

		if (sad->mpeg_aac) {
			printf("      AAC audio frame lengths:%s%s\n",
			       sad->mpeg_aac->has_frame_length_1024 ? " 1024_TL" : "",
			       sad->mpeg_aac->has_frame_length_960 ? " 960_TL" : "");
		}

		if (sad->mpeg_surround) {
			printf("      Supports %s signaled MPEG Surround data\n",
			       sad->mpeg_surround->signaling == DI_CTA_SAD_MPEG_SURROUND_SIGNALING_IMPLICIT ?
			       "only implicitly" : "implicitly and explicitly");
		}

		if (sad->mpeg_aac_le && sad->mpeg_aac_le->supports_multichannel_sound)
			printf("      Supports 22.2ch System H\n");
	}
}

static void
print_ycbcr420_cap_map(const struct di_edid_cta *cta,
		       const struct di_cta_ycbcr420_cap_map *map)
{
	const struct di_cta_data_block *const *data_blocks;
	const struct di_cta_data_block *data_block;
	enum di_cta_data_block_tag tag;
	const struct di_cta_svd *const *svds;
	size_t i, j, svd_index = 0;

	data_blocks = di_edid_cta_get_data_blocks(cta);

	for (i = 0; data_blocks[i] != NULL; i++) {
		data_block = data_blocks[i];

		tag = di_cta_data_block_get_tag(data_block);
		if (tag != DI_CTA_DATA_BLOCK_VIDEO)
			continue;

		svds = di_cta_data_block_get_svds(data_block);
		for (j = 0; svds[j] != NULL; j++) {
			if (di_cta_ycbcr420_cap_map_supported(map, svd_index))
				printf_cta_svd(svds[j]);

			svd_index++;
		}
	}
}

static void
printf_cta_svrs(const struct di_cta_svr *const *svrs)
{
	size_t i;
	const struct di_cta_svr *svr;

	/* TODO: resolve the references once we parse all timings and print
	 * the resolved timings */

	for (i = 0; svrs[i] != NULL; i++) {
		svr = svrs[i];

		switch (svr->type) {
		case DI_CTA_SVR_TYPE_VIC:
			printf("    VIC %3u\n", svr->vic);
			break;
		case DI_CTA_SVR_TYPE_DTD_INDEX:
			printf("    DTD %3u\n", svr->dtd_index + 1);
			break;
		case DI_CTA_SVR_TYPE_T7T10VTDB:
			printf("    VTDB %3u\n", svr->t7_t10_vtdb_index + 1);
			break;
		case DI_CTA_SVR_TYPE_FIRST_T8VTDB:
			printf("    T8VTDB\n");
			break;
		}
	}
}

static const char *
cta_infoframe_type_name(enum di_cta_infoframe_type type)
{
	switch (type) {
	case DI_CTA_INFOFRAME_TYPE_AUXILIARY_VIDEO_INFORMATION:
		return "Auxiliary Video Information InfoFrame (2)";
	case DI_CTA_INFOFRAME_TYPE_SOURCE_PRODUCT_DESCRIPTION:
		return "Source Product Description InfoFrame (3)";
	case DI_CTA_INFOFRAME_TYPE_AUDIO:
		return "Audio InfoFrame (4)";
	case DI_CTA_INFOFRAME_TYPE_MPEG_SOURCE:
		return "MPEG Source InfoFrame (5)";
	case DI_CTA_INFOFRAME_TYPE_NTSC_VBI:
		return "NTSC VBI InfoFrame (6)";
	case DI_CTA_INFOFRAME_TYPE_DYNAMIC_RANGE_AND_MASTERING:
		return "Dynamic Range and Mastering InfoFrame (7)";
	}
	abort();
}

static void
print_infoframes(const struct di_cta_infoframe_descriptor *const *infoframes)
{
	size_t i;
	const struct di_cta_infoframe_descriptor *infoframe;

	for (i = 0; infoframes[i] != NULL; i++) {
		infoframe = infoframes[i];
		printf("    %s\n",
		       cta_infoframe_type_name(infoframe->type));
	}
}

static void
print_did_type_vii_timing(const struct di_displayid_type_i_ii_vii_timing *t, int vtdb_index)
{
	char buf[32];
	snprintf(buf, 32, "VTDB %d", vtdb_index + 1);
	print_displayid_type_i_ii_vii_timing(t, 4, buf);
}

static void
print_speaker_alloc(const struct di_cta_speaker_allocation *speaker_alloc, const char *prefix)
{
	if (speaker_alloc->fl_fr)
		printf("%sFL/FR - Front Left/Right\n", prefix);
	if (speaker_alloc->lfe1)
		printf("%sLFE1 - Low Frequency Effects 1\n", prefix);
	if (speaker_alloc->fc)
		printf("%sFC - Front Center\n", prefix);
	if (speaker_alloc->bl_br)
		printf("%sBL/BR - Back Left/Right\n", prefix);
	if (speaker_alloc->bc)
		printf("%sBC - Back Center\n", prefix);
	if (speaker_alloc->flc_frc)
		printf("%sFLc/FRc - Front Left/Right of Center\n", prefix);
	if (speaker_alloc->flw_frw)
		printf("%sFLw/FRw - Front Left/Right Wide\n", prefix);
	if (speaker_alloc->tpfl_tpfr)
		printf("%sTpFL/TpFR - Top Front Left/Right\n", prefix);
	if (speaker_alloc->tpc)
		printf("%sTpC - Top Center\n", prefix);
	if (speaker_alloc->tpfc)
		printf("%sTpFC - Top Front Center\n", prefix);
	if (speaker_alloc->ls_rs)
		printf("%sLS/RS - Left/Right Surround\n", prefix);
	if (speaker_alloc->tpbc)
		printf("%sTpBC - Top Back Center\n", prefix);
	if (speaker_alloc->lfe2)
		printf("%sLFE2 - Low Frequency Effects 2\n", prefix);
	if (speaker_alloc->sil_sir)
		printf("%sSiL/SiR - Side Left/Right\n", prefix);
	if (speaker_alloc->tpsil_tpsir)
		printf("%sTpSiL/TpSiR - Top Side Left/Right\n", prefix);
	if (speaker_alloc->tpbl_tpbr)
		printf("%sTpBL/TpBR - Top Back Left/Right\n", prefix);
	if (speaker_alloc->btfc)
		printf("%sBtFC - Bottom Front Center\n", prefix);
	if (speaker_alloc->btfl_btfr)
		printf("%sBtFL/BtFR - Bottom Front Left/Right\n", prefix);
}

static void
print_hdmi_audio(const struct di_cta_hdmi_audio_block *hdmi_audio)
{
	const struct di_cta_hdmi_audio_3d *audio_3d = hdmi_audio->audio_3d;
	const struct di_cta_hdmi_audio_multi_stream *ms = hdmi_audio->multi_stream;

	if (ms) {
		printf("    Max Stream Count: %u\n", ms->max_streams);
		if (ms->supports_non_mixed)
			printf("    Supports MS NonMixed\n");
	}

	if (!audio_3d)
		return;

	print_cta_sads(audio_3d->sads);

	switch (audio_3d->channels) {
	case DI_CTA_HDMI_AUDIO_3D_CHANNELS_UNKNOWN:
		printf("    Unknown Speaker Allocation\n");
		break;
	case DI_CTA_HDMI_AUDIO_3D_CHANNELS_10_2:
		printf("    Speaker Allocation for 10.2 channels:\n");
		break;
	case DI_CTA_HDMI_AUDIO_3D_CHANNELS_22_2:
		printf("    Speaker Allocation for 22.2 channels:\n");
		break;
	case DI_CTA_HDMI_AUDIO_3D_CHANNELS_30_2:
		printf("    Speaker Allocation for 30.2 channels:\n");
		break;
	}

	print_speaker_alloc (&audio_3d->speakers, "      ");
}

static const char *
cta_data_block_tag_name(enum di_cta_data_block_tag tag)
{
	switch (tag) {
	case DI_CTA_DATA_BLOCK_AUDIO:
		return "Audio Data Block";
	case DI_CTA_DATA_BLOCK_VIDEO:
		return "Video Data Block";
	case DI_CTA_DATA_BLOCK_SPEAKER_ALLOC:
		return "Speaker Allocation Data Block";
	case DI_CTA_DATA_BLOCK_VESA_DISPLAY_TRANSFER_CHARACTERISTIC:
		return "VESA Display Transfer Characteristics Data Block";
	case DI_CTA_DATA_BLOCK_VIDEO_FORMAT:
		return "Video Format Data Block";
	case DI_CTA_DATA_BLOCK_VIDEO_CAP:
		return "Video Capability Data Block";
	case DI_CTA_DATA_BLOCK_VESA_DISPLAY_DEVICE:
		return "VESA Video Display Device Data Block";
	case DI_CTA_DATA_BLOCK_COLORIMETRY:
		return "Colorimetry Data Block";
	case DI_CTA_DATA_BLOCK_HDR_STATIC_METADATA:
		return "HDR Static Metadata Data Block";
	case DI_CTA_DATA_BLOCK_HDR_DYNAMIC_METADATA:
		return "HDR Dynamic Metadata Data Block";
	case DI_CTA_DATA_BLOCK_NATIVE_VIDEO_RESOLUTION:
		return "Native Video Resolution Data Block";
	case DI_CTA_DATA_BLOCK_VIDEO_FORMAT_PREF:
		return "Video Format Preference Data Block";
	case DI_CTA_DATA_BLOCK_YCBCR420:
		return "YCbCr 4:2:0 Video Data Block";
	case DI_CTA_DATA_BLOCK_YCBCR420_CAP_MAP:
		return "YCbCr 4:2:0 Capability Map Data Block";
	case DI_CTA_DATA_BLOCK_HDMI_AUDIO:
		return "HDMI Audio Data Block";
	case DI_CTA_DATA_BLOCK_ROOM_CONFIG:
		return "Room Configuration Data Block";
	case DI_CTA_DATA_BLOCK_SPEAKER_LOCATION:
		return "Speaker Location Data Block";
	case DI_CTA_DATA_BLOCK_INFOFRAME:
		return "InfoFrame Data Block";
	case DI_CTA_DATA_BLOCK_DISPLAYID_VIDEO_TIMING_VII:
		return "DisplayID Type VII Video Timing Data Block";
	case DI_CTA_DATA_BLOCK_DISPLAYID_VIDEO_TIMING_VIII:
		return "DisplayID Type VIII Video Timing Data Block";
	case DI_CTA_DATA_BLOCK_DISPLAYID_VIDEO_TIMING_X:
		return "DisplayID Type X Video Timing Data Block";
	case DI_CTA_DATA_BLOCK_HDMI_EDID_EXT_OVERRIDE :
		return "HDMI Forum EDID Extension Override Data Block";
	case DI_CTA_DATA_BLOCK_HDMI_SINK_CAP:
		return "HDMI Forum Sink Capability Data Block";
	}
	return "Unknown CTA-861 Data Block";
}

static const char *
video_cap_over_underscan_name(enum di_cta_video_cap_over_underscan over_underscan,
			      const char *unknown)
{
	switch (over_underscan) {
	case DI_CTA_VIDEO_CAP_UNKNOWN_OVER_UNDERSCAN:
		return unknown;
	case DI_CTA_VIDEO_CAP_ALWAYS_OVERSCAN:
		return "Always Overscanned";
	case DI_CTA_VIDEO_CAP_ALWAYS_UNDERSCAN:
		return "Always Underscanned";
	case DI_CTA_VIDEO_CAP_BOTH_OVER_UNDERSCAN:
		return "Supports both over- and underscan";
	}
	abort();
}

void
print_cta(const struct di_edid_cta *cta)
{
	const struct di_edid_cta_flags *cta_flags;
	const struct di_cta_data_block *const *data_blocks;
	const struct di_cta_data_block *data_block;
	enum di_cta_data_block_tag data_block_tag;
	const struct di_cta_svd *const *svds;
	const struct di_cta_speaker_alloc_block *speaker_alloc;
	const struct di_cta_video_cap_block *video_cap;
	const struct di_cta_vesa_dddb *vesa_dddb;
	const struct di_cta_colorimetry_block *colorimetry;
	const struct di_cta_hdr_static_metadata_block *hdr_static_metadata;
	const struct di_cta_hdr_dynamic_metadata_block *hdr_dynamic_metadata;
	const struct di_cta_vesa_transfer_characteristics *transfer_characteristics;
	const struct di_cta_sad *const *sads;
	const struct di_cta_ycbcr420_cap_map *ycbcr420_cap_map;
	const struct di_cta_infoframe_block *infoframe;
	const struct di_cta_svr *const *svrs;
	const struct di_edid_detailed_timing_def *const *detailed_timing_defs;
	const struct di_displayid_type_i_ii_vii_timing *type_vii_timing;
	const struct di_cta_hdmi_audio_block *hdmi_audio;
	size_t i;
	int vtdb_index = 0;

	printf("  Revision: %d\n", di_edid_cta_get_revision(cta));

	cta_flags = di_edid_cta_get_flags(cta);
	if (cta_flags->it_underscan) {
		printf("  Underscans IT Video Formats by default\n");
	}
	if (cta_flags->basic_audio) {
		printf("  Basic audio support\n");
	}
	if (cta_flags->ycc444) {
		printf("  Supports YCbCr 4:4:4\n");
	}
	if (cta_flags->ycc422) {
		printf("  Supports YCbCr 4:2:2\n");
	}
	printf("  Native detailed modes: %d\n", cta_flags->native_dtds);

	data_blocks = di_edid_cta_get_data_blocks(cta);
	for (i = 0; data_blocks[i] != NULL; i++) {
		data_block = data_blocks[i];

		data_block_tag = di_cta_data_block_get_tag(data_block);
		printf("  %s:\n", cta_data_block_tag_name(data_block_tag));

		switch (data_block_tag) {
		case DI_CTA_DATA_BLOCK_VIDEO:
			svds = di_cta_data_block_get_svds(data_block);
			printf_cta_svds(svds);
			break;
		case DI_CTA_DATA_BLOCK_YCBCR420:
			svds = di_cta_data_block_get_ycbcr420_svds(data_block);
			printf_cta_svds(svds);
			break;
		case DI_CTA_DATA_BLOCK_SPEAKER_ALLOC:
			speaker_alloc = di_cta_data_block_get_speaker_alloc(data_block);
			print_speaker_alloc(&speaker_alloc->speakers, "    ");
			break;
		case DI_CTA_DATA_BLOCK_VIDEO_CAP:
			video_cap = di_cta_data_block_get_video_cap(data_block);
			printf("    YCbCr quantization: %s\n",
			       video_cap->selectable_ycc_quantization_range ?
			       "Selectable (via AVI YQ)" : "No Data");
			printf("    RGB quantization: %s\n",
			       video_cap->selectable_rgb_quantization_range ?
			       "Selectable (via AVI Q)" : "No Data");
			printf("    PT scan behavior: %s\n",
			       video_cap_over_underscan_name(video_cap->pt_over_underscan,
							     "No Data"));
			printf("    IT scan behavior: %s\n",
			       video_cap_over_underscan_name(video_cap->it_over_underscan,
							     "IT video formats not supported"));
			printf("    CE scan behavior: %s\n",
			       video_cap_over_underscan_name(video_cap->ce_over_underscan,
							     "CE video formats not supported"));
			break;
		case DI_CTA_DATA_BLOCK_VESA_DISPLAY_DEVICE:
			vesa_dddb = di_cta_data_block_get_vesa_dddb(data_block);
			print_cta_vesa_dddb(vesa_dddb);
			break;
		case DI_CTA_DATA_BLOCK_COLORIMETRY:
			colorimetry = di_cta_data_block_get_colorimetry(data_block);
			if (colorimetry->xvycc_601)
				printf("    xvYCC601\n");
			if (colorimetry->xvycc_709)
				printf("    xvYCC709\n");
			if (colorimetry->sycc_601)
				printf("    sYCC601\n");
			if (colorimetry->opycc_601)
				printf("    opYCC601\n");
			if (colorimetry->oprgb)
				printf("    opRGB\n");
			if (colorimetry->bt2020_cycc)
				printf("    BT2020cYCC\n");
			if (colorimetry->bt2020_ycc)
				printf("    BT2020YCC\n");
			if (colorimetry->bt2020_rgb)
				printf("    BT2020RGB\n");
			if (colorimetry->ictcp)
				printf("    ICtCp\n");
			if (colorimetry->st2113_rgb)
				printf("    ST2113RGB\n");
			break;
		case DI_CTA_DATA_BLOCK_HDR_STATIC_METADATA:
			hdr_static_metadata = di_cta_data_block_get_hdr_static_metadata(data_block);
			print_cta_hdr_static_metadata(hdr_static_metadata);
			break;
		case DI_CTA_DATA_BLOCK_HDR_DYNAMIC_METADATA:
			hdr_dynamic_metadata = di_cta_data_block_get_hdr_dynamic_metadata(data_block);
			print_cta_hdr_dynamic_metadata(hdr_dynamic_metadata);
			break;
		case DI_CTA_DATA_BLOCK_VESA_DISPLAY_TRANSFER_CHARACTERISTIC:
			transfer_characteristics = di_cta_data_block_get_vesa_transfer_characteristics(data_block);
			print_cta_vesa_transfer_characteristics(transfer_characteristics);
			break;
		case DI_CTA_DATA_BLOCK_AUDIO:
			sads = di_cta_data_block_get_sads(data_block);
			print_cta_sads(sads);
			break;
		case DI_CTA_DATA_BLOCK_YCBCR420_CAP_MAP:
			ycbcr420_cap_map = di_cta_data_block_get_ycbcr420_cap_map(data_block);
			print_ycbcr420_cap_map(cta, ycbcr420_cap_map);
			break;
		case DI_CTA_DATA_BLOCK_INFOFRAME:
			infoframe = di_cta_data_block_get_infoframe(data_block);
			printf("    VSIFs: %d\n", infoframe->num_simultaneous_vsifs - 1);
			print_infoframes(infoframe->infoframes);
			break;
		case DI_CTA_DATA_BLOCK_VIDEO_FORMAT_PREF:
			svrs = di_cta_data_block_get_svrs(data_block);
			printf_cta_svrs(svrs);
			break;
		case DI_CTA_DATA_BLOCK_DISPLAYID_VIDEO_TIMING_VII:
			type_vii_timing = di_cta_data_block_get_did_type_vii_timing(data_block);
			print_did_type_vii_timing(type_vii_timing, vtdb_index);
			vtdb_index++;
			break;
		case DI_CTA_DATA_BLOCK_HDMI_AUDIO:
			hdmi_audio = di_cta_data_block_get_hdmi_audio(data_block);
			print_hdmi_audio(hdmi_audio);
			break;
		default:
			break; /* Ignore */
		}
	}

	detailed_timing_defs = di_edid_cta_get_detailed_timing_defs(cta);
	if (detailed_timing_defs[0]) {
		printf("  Detailed Timing Descriptors:\n");
	}
	for (i = 0; detailed_timing_defs[i] != NULL; i++) {
		print_detailed_timing_def(detailed_timing_defs[i]);
	}
}
