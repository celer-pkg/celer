#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "bits.h"
#include "cta.h"
#include "log.h"
#include "edid.h"
#include "displayid.h"

/**
 * Number of bytes in the CTA header (tag + revision + DTD offset + flags).
 */
#define CTA_HEADER_SIZE 4
/**
 * Exclusive upper bound for the detailed timing definitions in the CTA block.
 */
#define CTA_DTD_END 127
/**
 * Number of bytes in a CTA short audio descriptor.
 */
#define CTA_SAD_SIZE 3
/**
 * Number of bytes in a HDMI 3D audio descriptor.
 */
#define CTA_HDMI_AUDIO_3D_DESCRIPTOR_SIZE 4

const struct di_cta_video_format *
di_cta_video_format_from_vic(uint8_t vic)
{
	if (vic > _di_cta_video_formats_len ||
	    _di_cta_video_formats[vic].vic == 0)
		return NULL;
	return &_di_cta_video_formats[vic];
}

static void
add_failure(struct di_edid_cta *cta, const char fmt[], ...)
{
	va_list args;

	va_start(args, fmt);
	_di_logger_va_add_failure(cta->logger, fmt, args);
	va_end(args);
}

static void
add_failure_until(struct di_edid_cta *cta, int revision, const char fmt[], ...)
{
	va_list args;

	if (cta->revision > revision) {
		return;
	}

	va_start(args, fmt);
	_di_logger_va_add_failure(cta->logger, fmt, args);
	va_end(args);
}

static struct di_cta_svd *
parse_svd(struct di_edid_cta *cta, uint8_t raw, const char *prefix)
{
	struct di_cta_svd svd, *svd_ptr;

	if (raw == 0 || raw == 128 || raw >= 254) {
		/* Reserved */
		add_failure_until(cta, 3,
				  "%s: Unknown VIC %" PRIu8 ".",
				  prefix,
				  raw);
		return NULL;
	} else if (raw <= 127 || raw >= 193) {
		svd = (struct di_cta_svd) {
			.vic = raw,
		};
	} else {
		svd = (struct di_cta_svd) {
			.vic = get_bit_range(raw, 6, 0),
			.native = true,
		};
	}

	svd_ptr = calloc(1, sizeof(*svd_ptr));
	if (!svd_ptr)
		return NULL;
	*svd_ptr = svd;
	return svd_ptr;
}

static bool
parse_video_block(struct di_edid_cta *cta, struct di_cta_video_block *video,
		  const uint8_t *data, size_t size)
{
	size_t i;
	struct di_cta_svd *svd;

	if (size == 0)
		add_failure(cta, "Video Data Block: Empty Data Block");

	for (i = 0; i < size; i++) {
		svd = parse_svd(cta, data[i], "Video Data Block");
		if (!svd)
			continue;
		assert(video->svds_len < EDID_CTA_MAX_VIDEO_BLOCK_ENTRIES);
		video->svds[video->svds_len++] = svd;
	}

	return true;
}

static bool
parse_ycbcr420_block(struct di_edid_cta *cta,
		     struct di_cta_video_block *ycbcr420,
		     const uint8_t *data, size_t size)
{
	size_t i;
	struct di_cta_svd *svd;

	if (size == 0)
		add_failure(cta, "YCbCr 4:2:0 Video Data Block: Empty Data Block");

	for (i = 0; i < size; i++) {
		svd = parse_svd(cta, data[i], "YCbCr 4:2:0 Video Data Block");
		if (!svd)
			continue;
		assert(ycbcr420->svds_len < EDID_CTA_MAX_VIDEO_BLOCK_ENTRIES);
		ycbcr420->svds[ycbcr420->svds_len++] = svd;
	}

	return true;
}

static bool
parse_sad_format(struct di_edid_cta *cta, uint8_t code, uint8_t code_ext,
		 enum di_cta_audio_format *format, const char *prefix)
{
	switch (code) {
	case 0x0:
		add_failure_until(cta, 3, "%s: Audio Format Code 0x00 is reserved.", prefix);
		return false;
	case 0x1:
		*format = DI_CTA_AUDIO_FORMAT_LPCM;
		break;
	case 0x2:
		*format = DI_CTA_AUDIO_FORMAT_AC3;
		break;
	case 0x3:
		*format = DI_CTA_AUDIO_FORMAT_MPEG1;
		break;
	case 0x4:
		*format = DI_CTA_AUDIO_FORMAT_MP3;
		break;
	case 0x5:
		*format = DI_CTA_AUDIO_FORMAT_MPEG2;
		break;
	case 0x6:
		*format = DI_CTA_AUDIO_FORMAT_AAC_LC;
		break;
	case 0x7:
		*format = DI_CTA_AUDIO_FORMAT_DTS;
		break;
	case 0x8:
		*format = DI_CTA_AUDIO_FORMAT_ATRAC;
		break;
	case 0x9:
		*format = DI_CTA_AUDIO_FORMAT_ONE_BIT_AUDIO;
		break;
	case 0xA:
		*format = DI_CTA_AUDIO_FORMAT_ENHANCED_AC3;
		break;
	case 0xB:
		*format = DI_CTA_AUDIO_FORMAT_DTS_HD;
		break;
	case 0xC:
		*format = DI_CTA_AUDIO_FORMAT_MAT;
		break;
	case 0xD:
		*format = DI_CTA_AUDIO_FORMAT_DST;
		break;
	case 0xE:
		*format = DI_CTA_AUDIO_FORMAT_WMA_PRO;
		break;
	case 0xF:
		switch (code_ext) {
		case 0x04:
			*format = DI_CTA_AUDIO_FORMAT_MPEG4_HE_AAC;
			break;
		case 0x05:
			*format = DI_CTA_AUDIO_FORMAT_MPEG4_HE_AAC_V2;
			break;
		case 0x06:
			*format = DI_CTA_AUDIO_FORMAT_MPEG4_AAC_LC;
			break;
		case 0x07:
			*format = DI_CTA_AUDIO_FORMAT_DRA;
			break;
		case 0x08:
			*format = DI_CTA_AUDIO_FORMAT_MPEG4_HE_AAC_MPEG_SURROUND;
			break;
		case 0x0A:
			*format = DI_CTA_AUDIO_FORMAT_MPEG4_AAC_LC_MPEG_SURROUND;
			break;
		case 0x0B:
			*format = DI_CTA_AUDIO_FORMAT_MPEGH_3D;
			break;
		case 0x0C:
			*format = DI_CTA_AUDIO_FORMAT_AC4;
			break;
		case 0x0D:
			*format = DI_CTA_AUDIO_FORMAT_LPCM_3D;
			break;
		default:
			add_failure_until(cta, 3, "%s: Unknown Audio Ext Format 0x%02x.",
					  prefix, code_ext);
			return false;
		}
		break;
	default:
		add_failure_until(cta, 3, "%s: Unknown Audio Format 0x%02x.", prefix, code);
		return false;
	}

	return true;
}

static bool
parse_sad(struct di_edid_cta *cta, struct di_cta_audio_block *audio,
	  const uint8_t data[static CTA_SAD_SIZE])
{
	enum di_cta_audio_format format;
	struct di_cta_sad_priv *priv;
	struct di_cta_sad *sad;
	struct di_cta_sad_sample_rates *sample_rates;
	struct di_cta_sad_lpcm *lpcm;
	struct di_cta_sad_mpegh_3d *mpegh_3d;
	struct di_cta_sad_mpeg_aac *mpeg_aac;
	struct di_cta_sad_mpeg_surround *mpeg_surround;
	struct di_cta_sad_mpeg_aac_le *mpeg_aac_le;
	struct di_cta_sad_enhanced_ac3 *enhanced_ac3;
	struct di_cta_sad_mat *mat;
	struct di_cta_sad_wma_pro *wma_pro;
	uint8_t code, code_ext;

	code = get_bit_range(data[0], 6, 3);
	code_ext = get_bit_range(data[2], 7, 3);

	if (!parse_sad_format(cta, code, code_ext, &format, "Audio Data Block"))
		return true;

	priv = calloc(1, sizeof(*priv));
	if (!priv)
		return false;

	sad = &priv->base;
	sample_rates = &priv->supported_sample_rates;
	lpcm = &priv->lpcm;
	mpegh_3d = &priv->mpegh_3d;
	mpeg_aac = &priv->mpeg_aac;
	mpeg_surround = &priv->mpeg_surround;
	mpeg_aac_le = &priv->mpeg_aac_le;
	enhanced_ac3 = &priv->enhanced_ac3;
	mat = &priv->mat;
	wma_pro = &priv->wma_pro;

	sad->format = format;

	/* TODO: Find DRA documentation */

	switch (format) {
	case DI_CTA_AUDIO_FORMAT_LPCM:
	case DI_CTA_AUDIO_FORMAT_AC3:
	case DI_CTA_AUDIO_FORMAT_MPEG1:
	case DI_CTA_AUDIO_FORMAT_MP3:
	case DI_CTA_AUDIO_FORMAT_MPEG2:
	case DI_CTA_AUDIO_FORMAT_AAC_LC:
	case DI_CTA_AUDIO_FORMAT_DTS:
	case DI_CTA_AUDIO_FORMAT_ATRAC:
	case DI_CTA_AUDIO_FORMAT_ONE_BIT_AUDIO:
	case DI_CTA_AUDIO_FORMAT_ENHANCED_AC3:
	case DI_CTA_AUDIO_FORMAT_DTS_HD:
	case DI_CTA_AUDIO_FORMAT_MAT:
	case DI_CTA_AUDIO_FORMAT_DST:
	case DI_CTA_AUDIO_FORMAT_WMA_PRO:
	case DI_CTA_AUDIO_FORMAT_MPEG4_HE_AAC:
	case DI_CTA_AUDIO_FORMAT_MPEG4_HE_AAC_V2:
	case DI_CTA_AUDIO_FORMAT_MPEG4_AAC_LC:
	/* DRA is not documented but this is what edid-decode does */
	case DI_CTA_AUDIO_FORMAT_DRA:
	case DI_CTA_AUDIO_FORMAT_MPEG4_HE_AAC_MPEG_SURROUND:
	case DI_CTA_AUDIO_FORMAT_MPEG4_AAC_LC_MPEG_SURROUND:
		sad->max_channels = get_bit_range(data[0], 2, 0) + 1;
		break;
	case DI_CTA_AUDIO_FORMAT_LPCM_3D:
		sad->max_channels = (get_bit_range(data[0], 2, 0) |
				     (get_bit_range(data[0], 7, 7) << 3) |
				     (get_bit_range(data[1], 7, 7) << 4)) + 1;
		break;
	case DI_CTA_AUDIO_FORMAT_MPEGH_3D:
	case DI_CTA_AUDIO_FORMAT_AC4:
		break;
	}

	switch (format) {
	case DI_CTA_AUDIO_FORMAT_LPCM:
	case DI_CTA_AUDIO_FORMAT_AC3:
	case DI_CTA_AUDIO_FORMAT_MPEG1:
	case DI_CTA_AUDIO_FORMAT_MP3:
	case DI_CTA_AUDIO_FORMAT_MPEG2:
	case DI_CTA_AUDIO_FORMAT_AAC_LC:
	case DI_CTA_AUDIO_FORMAT_DTS:
	case DI_CTA_AUDIO_FORMAT_ATRAC:
	case DI_CTA_AUDIO_FORMAT_ONE_BIT_AUDIO:
	case DI_CTA_AUDIO_FORMAT_ENHANCED_AC3:
	case DI_CTA_AUDIO_FORMAT_DTS_HD:
	case DI_CTA_AUDIO_FORMAT_MAT:
	case DI_CTA_AUDIO_FORMAT_DST:
	case DI_CTA_AUDIO_FORMAT_WMA_PRO:
	/* DRA is not documented but this is what edid-decode does */
	case DI_CTA_AUDIO_FORMAT_DRA:
	case DI_CTA_AUDIO_FORMAT_MPEGH_3D:
	case DI_CTA_AUDIO_FORMAT_LPCM_3D:
		sample_rates->has_192_khz = has_bit(data[1], 6);
		sample_rates->has_176_4_khz = has_bit(data[1], 5);
		/* fallthrough */
	case DI_CTA_AUDIO_FORMAT_MPEG4_HE_AAC:
	case DI_CTA_AUDIO_FORMAT_MPEG4_HE_AAC_V2:
	case DI_CTA_AUDIO_FORMAT_MPEG4_AAC_LC:
	case DI_CTA_AUDIO_FORMAT_MPEG4_HE_AAC_MPEG_SURROUND:
	case DI_CTA_AUDIO_FORMAT_MPEG4_AAC_LC_MPEG_SURROUND:
		sample_rates->has_96_khz = has_bit(data[1], 4);
		sample_rates->has_88_2_khz = has_bit(data[1], 3);
		sample_rates->has_48_khz = has_bit(data[1], 2);
		sample_rates->has_44_1_khz = has_bit(data[1], 1);
		sample_rates->has_32_khz = has_bit(data[1], 0);
		break;
	case DI_CTA_AUDIO_FORMAT_AC4:
		sample_rates->has_192_khz = has_bit(data[1], 6);
		sample_rates->has_96_khz = has_bit(data[1], 4);
		sample_rates->has_48_khz = has_bit(data[1], 2);
		sample_rates->has_44_1_khz = has_bit(data[1], 1);
		break;
	}
	sad->supported_sample_rates = sample_rates;

	switch (format) {
	case DI_CTA_AUDIO_FORMAT_AC3:
	case DI_CTA_AUDIO_FORMAT_MPEG1:
	case DI_CTA_AUDIO_FORMAT_MP3:
	case DI_CTA_AUDIO_FORMAT_MPEG2:
	case DI_CTA_AUDIO_FORMAT_AAC_LC:
	case DI_CTA_AUDIO_FORMAT_DTS:
	case DI_CTA_AUDIO_FORMAT_ATRAC:
		sad->max_bitrate_kbs = data[2] * 8;
		break;
	default:
		break;
	}

	switch (format) {
	case DI_CTA_AUDIO_FORMAT_LPCM:
	case DI_CTA_AUDIO_FORMAT_LPCM_3D:
		lpcm->has_sample_size_24_bits = has_bit(data[2], 2);
		lpcm->has_sample_size_20_bits = has_bit(data[2], 1);
		lpcm->has_sample_size_16_bits = has_bit(data[2], 0);
		sad->lpcm = lpcm;
	default:
		break;
	}

	switch (format) {
	case DI_CTA_AUDIO_FORMAT_MPEG4_HE_AAC:
	case DI_CTA_AUDIO_FORMAT_MPEG4_HE_AAC_V2:
	case DI_CTA_AUDIO_FORMAT_MPEG4_AAC_LC:
	case DI_CTA_AUDIO_FORMAT_MPEG4_HE_AAC_MPEG_SURROUND:
	case DI_CTA_AUDIO_FORMAT_MPEG4_AAC_LC_MPEG_SURROUND:
		mpeg_aac->has_frame_length_1024 = has_bit(data[2], 2);
		mpeg_aac->has_frame_length_960 = has_bit(data[2], 1);
		sad->mpeg_aac = mpeg_aac;
		break;
	default:
		break;
	}

	if (format == DI_CTA_AUDIO_FORMAT_MPEG4_AAC_LC) {
		mpeg_aac_le->supports_multichannel_sound = has_bit(data[2], 0);
		sad->mpeg_aac_le = mpeg_aac_le;
	}

	switch (format) {
	case DI_CTA_AUDIO_FORMAT_MPEG4_HE_AAC_MPEG_SURROUND:
	case DI_CTA_AUDIO_FORMAT_MPEG4_AAC_LC_MPEG_SURROUND:
		mpeg_surround->signaling = has_bit(data[2], 0);
		sad->mpeg_surround = mpeg_surround;
		break;
	default:
		break;
	}

	if (format == DI_CTA_AUDIO_FORMAT_MPEGH_3D) {
		mpegh_3d->low_complexity_profile = has_bit(data[2], 0);
		mpegh_3d->baseline_profile = has_bit(data[2], 1);
		mpegh_3d->level = get_bit_range(data[0], 2, 0);
		if (mpegh_3d->level > DI_CTA_SAD_MPEGH_3D_LEVEL_5) {
			add_failure_until(cta, 3,
					  "Unknown MPEG-H 3D Audio Level 0x%02x.",
					  mpegh_3d->level);
			mpegh_3d->level = DI_CTA_SAD_MPEGH_3D_LEVEL_UNSPECIFIED;
		}
		sad->mpegh_3d = mpegh_3d;
	}

	if (format == DI_CTA_AUDIO_FORMAT_ENHANCED_AC3) {
		enhanced_ac3->supports_joint_object_coding =
			has_bit(data[2], 0);
		enhanced_ac3->supports_joint_object_coding_ACMOD28 =
			has_bit(data[2], 1);
		sad->enhanced_ac3 = enhanced_ac3;
	}

	if (format == DI_CTA_AUDIO_FORMAT_MAT) {
		mat->supports_object_audio_and_channel_based =
			has_bit(data[2], 0);
		if (mat->supports_object_audio_and_channel_based)
			mat->requires_hash_calculation = !has_bit(data[2], 0);
		sad->mat = mat;
	}

	if (format == DI_CTA_AUDIO_FORMAT_WMA_PRO) {
		wma_pro->profile = get_bit_range(data[2], 2, 0);
		sad->wma_pro = wma_pro;
	}

	switch (format) {
	case DI_CTA_AUDIO_FORMAT_ONE_BIT_AUDIO:
	case DI_CTA_AUDIO_FORMAT_DTS_HD:
	case DI_CTA_AUDIO_FORMAT_DST:
		/* TODO data[2] 7:0 contains unknown Audio Format Code dependent value */
		break;
	default:
		break;
	}

	if (format == DI_CTA_AUDIO_FORMAT_AC4) {
		/* TODO data[2] 2:0 contains unknown Audio Format Code dependent value */
	}

	switch (format) {
	case DI_CTA_AUDIO_FORMAT_LPCM:
	case DI_CTA_AUDIO_FORMAT_WMA_PRO:
		if (has_bit(data[0], 7) || has_bit(data[1], 7) ||
		    get_bit_range(data[2], 7, 3) != 0)
			add_failure_until(cta, 3,
					  "Bits F17, F27, F37:F33 must be 0.");
		break;
	case DI_CTA_AUDIO_FORMAT_AC3:
	case DI_CTA_AUDIO_FORMAT_MPEG1:
	case DI_CTA_AUDIO_FORMAT_MP3:
	case DI_CTA_AUDIO_FORMAT_MPEG2:
	case DI_CTA_AUDIO_FORMAT_AAC_LC:
	case DI_CTA_AUDIO_FORMAT_DTS:
	case DI_CTA_AUDIO_FORMAT_ATRAC:
	case DI_CTA_AUDIO_FORMAT_ONE_BIT_AUDIO:
	case DI_CTA_AUDIO_FORMAT_ENHANCED_AC3:
	case DI_CTA_AUDIO_FORMAT_DTS_HD:
	case DI_CTA_AUDIO_FORMAT_MAT:
	case DI_CTA_AUDIO_FORMAT_DST:
		if (has_bit(data[0], 7) || has_bit(data[1], 7))
			add_failure_until(cta, 3,
					  "Bits F17, F27 must be 0.");
		break;
	case DI_CTA_AUDIO_FORMAT_MPEG4_HE_AAC:
	case DI_CTA_AUDIO_FORMAT_MPEG4_HE_AAC_V2:
	case DI_CTA_AUDIO_FORMAT_MPEG4_AAC_LC:
	case DI_CTA_AUDIO_FORMAT_MPEG4_HE_AAC_MPEG_SURROUND:
	case DI_CTA_AUDIO_FORMAT_MPEG4_AAC_LC_MPEG_SURROUND:
		if (has_bit(data[0], 7) || get_bit_range(data[2], 7, 5) != 0)
			add_failure_until(cta, 3,
					  "Bits F17, F27:F25 must be 0.");
		break;
	case DI_CTA_AUDIO_FORMAT_MPEGH_3D:
		if (has_bit(data[0], 7) || has_bit(data[1], 7) ||
		    has_bit(data[2], 2))
			add_failure_until(cta, 3,
					  "Bits F17, F27, F32 must be 0.");
		break;
	case DI_CTA_AUDIO_FORMAT_AC4:
		if ((data[0] & 0x87) != 0 || (data[1] & 0xA9) != 0)
			add_failure_until(cta, 3,
					  "Bits F17, F12:F10, F27, F25, F23, "
					  "F20 must be 0.");
		break;
	/* DRA documentation missing */
	case DI_CTA_AUDIO_FORMAT_DRA:
	case DI_CTA_AUDIO_FORMAT_LPCM_3D:
		break;
	}

	assert(audio->sads_len < EDID_CTA_MAX_AUDIO_BLOCK_ENTRIES);
	audio->sads[audio->sads_len++] = priv;
	return true;
}

static bool
parse_audio_block(struct di_edid_cta *cta, struct di_cta_audio_block *audio,
		  const uint8_t *data, size_t size)
{
	size_t i;

	if (size % 3 != 0)
		add_failure(cta, "Broken CTA-861 audio block length %d.", size);

	for (i = 0; i + 3 <= size; i += 3) {
		if (!parse_sad(cta, audio, &data[i]))
			return false;
	}

	return true;
}

static bool
parse_speaker_alloc(struct di_edid_cta *cta, struct di_cta_speaker_allocation *speaker_alloc,
		    const uint8_t data[3], const char *prefix)
{
	bool rlc_rrc;

	speaker_alloc->flw_frw = has_bit(data[0], 7);
	rlc_rrc = has_bit(data[0], 6);
	speaker_alloc->flc_frc = has_bit(data[0], 5);
	speaker_alloc->bc = has_bit(data[0], 4);
	speaker_alloc->bl_br = has_bit(data[0], 3);
	speaker_alloc->fc = has_bit(data[0], 2);
	speaker_alloc->lfe1 = has_bit(data[0], 1);
	speaker_alloc->fl_fr = has_bit(data[0], 0);
	if (rlc_rrc) {
		if (cta->revision >= 3)
			add_failure(cta, "%s: Deprecated bit F16 must be 0.", prefix);
		else
			speaker_alloc->bl_br = true;
	}

	speaker_alloc->tpsil_tpsir = has_bit(data[1], 7);
	speaker_alloc->sil_sir = has_bit(data[1], 6);
	speaker_alloc->tpbc = has_bit(data[1], 5);
	speaker_alloc->lfe2 = has_bit(data[1], 4);
	speaker_alloc->ls_rs = has_bit(data[1], 3);
	speaker_alloc->tpfc = has_bit(data[1], 2);
	speaker_alloc->tpc = has_bit(data[1], 1);
	speaker_alloc->tpfl_tpfr = has_bit(data[1], 0);

	if (get_bit_range(data[2], 7, 4) != 0)
		add_failure(cta, "%s: Bits F37, F36, F34 must be 0.", prefix);
	if (cta->revision >= 3 && has_bit(data[2], 3))
		add_failure(cta, "%s: Deprecated bit F33 must be 0.", prefix);
	speaker_alloc->btfl_btfr = has_bit(data[2], 2);
	speaker_alloc->btfc = has_bit(data[2], 1);
	speaker_alloc->tpbl_tpbr = has_bit(data[2], 0);

	return true;
}

static bool
parse_speaker_alloc_block(struct di_edid_cta *cta,
			  struct di_cta_speaker_alloc_block *speaker_alloc,
			  const uint8_t *data, size_t size)
{
	if (size < 3) {
		add_failure(cta,
			    "Speaker Allocation Data Block: Empty Data Block with length %zu.",
			    size);
		return false;
	}

	parse_speaker_alloc(cta, &speaker_alloc->speakers, data,
			    "Speaker Allocation Data Block");

	return true;
}

static bool
parse_video_cap_block(struct di_edid_cta *cta,
		      struct di_cta_video_cap_block *video_cap,
		      const uint8_t *data, size_t size)
{
	if (size < 1) {
		add_failure(cta,
			    "Video Capability Data Block: Empty Data Block with length %u.",
			    size);
		return false;
	}

	video_cap->selectable_ycc_quantization_range = has_bit(data[0], 7);
	video_cap->selectable_rgb_quantization_range = has_bit(data[0], 6);
	video_cap->pt_over_underscan = get_bit_range(data[0], 5, 4);
	video_cap->it_over_underscan = get_bit_range(data[0], 3, 2);
	video_cap->ce_over_underscan = get_bit_range(data[0], 1, 0);

	if (!video_cap->selectable_rgb_quantization_range && cta->revision >= 3)
		add_failure(cta,
			    "Video Capability Data Block: Set Selectable RGB Quantization to avoid interop issues.");
	/* TODO: add failure if selectable_ycc_quantization_range is unset,
	 * the sink supports YCbCr formats and the revision is 3+ */

	switch (video_cap->it_over_underscan) {
	case DI_CTA_VIDEO_CAP_ALWAYS_OVERSCAN:
		if (cta->flags.it_underscan)
			add_failure(cta, "Video Capability Data Block: IT video formats are always overscanned, but bit 7 of Byte 3 of the CTA-861 Extension header is set to underscanned.");
		break;
	case DI_CTA_VIDEO_CAP_ALWAYS_UNDERSCAN:
		if (!cta->flags.it_underscan)
			add_failure(cta, "Video Capability Data Block: IT video formats are always underscanned, but bit 7 of Byte 3 of the CTA-861 Extension header is set to overscanned.");
	default:
		break;
	}

	return true;
}

static bool
check_vesa_dddb_num_channels(enum di_cta_vesa_dddb_interface_type interface,
			     uint8_t num_channels)
{
	switch (interface) {
	case DI_CTA_VESA_DDDB_INTERFACE_VGA:
	case DI_CTA_VESA_DDDB_INTERFACE_NAVI_V:
	case DI_CTA_VESA_DDDB_INTERFACE_NAVI_D:
		return num_channels == 0;
	case DI_CTA_VESA_DDDB_INTERFACE_LVDS:
	case DI_CTA_VESA_DDDB_INTERFACE_RSDS:
		return true;
	case DI_CTA_VESA_DDDB_INTERFACE_DVI_D:
		return num_channels == 1 || num_channels == 2;
	case DI_CTA_VESA_DDDB_INTERFACE_DVI_I_ANALOG:
		return num_channels == 0;
	case DI_CTA_VESA_DDDB_INTERFACE_DVI_I_DIGITAL:
		return num_channels == 1 || num_channels == 2;
	case DI_CTA_VESA_DDDB_INTERFACE_HDMI_A:
		return num_channels == 1;
	case DI_CTA_VESA_DDDB_INTERFACE_HDMI_B:
		return num_channels == 2;
	case DI_CTA_VESA_DDDB_INTERFACE_MDDI:
		return num_channels == 1 || num_channels == 2;
	case DI_CTA_VESA_DDDB_INTERFACE_DISPLAYPORT:
		return num_channels == 1 || num_channels == 2 || num_channels == 4;
	case DI_CTA_VESA_DDDB_INTERFACE_IEEE_1394:
	case DI_CTA_VESA_DDDB_INTERFACE_M1_ANALOG:
		return num_channels == 0;
	case DI_CTA_VESA_DDDB_INTERFACE_M1_DIGITAL:
		return num_channels == 1 || num_channels == 2;
	}
	abort(); /* unreachable */
}

static void
parse_vesa_dddb_additional_primary_chromaticity(struct di_cta_vesa_dddb_additional_primary_chromaticity *coords,
						uint8_t low,
						const uint8_t high[static 2])
{
	uint16_t raw_x, raw_y; /* only 10 bits are used */

	raw_x = (uint16_t) ((high[0] << 2) | get_bit_range(low, 3, 2));
	raw_y = (uint16_t) ((high[1] << 2) | get_bit_range(low, 1, 0));

	*coords = (struct di_cta_vesa_dddb_additional_primary_chromaticity) {
		.x = (float) raw_x / 1024,
		.y = (float) raw_y / 1024,
	};
}

static bool
parse_vesa_dddb(struct di_edid_cta *cta, struct di_cta_vesa_dddb *dddb,
		const uint8_t *data, size_t size)
{
	const size_t offset = 2; /* CTA block header */
	uint8_t interface_type, num_channels, content_protection, scan_direction,
		subpixel_layout;

	if (size + offset != 32) {
		add_failure(cta, "VESA Video Display Device Data Block: Invalid length %u.", size);
		return false;
	}

	interface_type = get_bit_range(data[0x02 - offset], 7, 4);
	num_channels = get_bit_range(data[0x02 - offset], 3, 0);
	switch (interface_type) {
	case 0x0: /* Analog */
		/* Special case: num_channels contains the detailed interface
		 * type. */
		switch (num_channels) {
		case 0x0:
			dddb->interface_type = DI_CTA_VESA_DDDB_INTERFACE_VGA;
			break;
		case 0x1:
			dddb->interface_type = DI_CTA_VESA_DDDB_INTERFACE_NAVI_V;
			break;
		case 0x2:
			dddb->interface_type = DI_CTA_VESA_DDDB_INTERFACE_NAVI_D;
			break;
		default:
			add_failure(cta,
				    "VESA Video Display Device Data Block: Unknown analog interface type 0x%x.",
				    num_channels);
			return false;
		}
		num_channels = 0;
		break;
	case 0x1:
		dddb->interface_type = DI_CTA_VESA_DDDB_INTERFACE_LVDS;
		break;
	case 0x2:
		dddb->interface_type = DI_CTA_VESA_DDDB_INTERFACE_RSDS;
		break;
	case 0x3:
		dddb->interface_type = DI_CTA_VESA_DDDB_INTERFACE_DVI_D;
		break;
	case 0x4:
		dddb->interface_type = DI_CTA_VESA_DDDB_INTERFACE_DVI_I_ANALOG;
		break;
	case 0x5:
		dddb->interface_type = DI_CTA_VESA_DDDB_INTERFACE_DVI_I_DIGITAL;
		break;
	case 0x6:
		dddb->interface_type = DI_CTA_VESA_DDDB_INTERFACE_HDMI_A;
		break;
	case 0x7:
		dddb->interface_type = DI_CTA_VESA_DDDB_INTERFACE_HDMI_B;
		break;
	case 0x8:
		dddb->interface_type = DI_CTA_VESA_DDDB_INTERFACE_MDDI;
		break;
	case 0x9:
		dddb->interface_type = DI_CTA_VESA_DDDB_INTERFACE_DISPLAYPORT;
		break;
	case 0xA:
		dddb->interface_type = DI_CTA_VESA_DDDB_INTERFACE_IEEE_1394;
		break;
	case 0xB:
		dddb->interface_type = DI_CTA_VESA_DDDB_INTERFACE_M1_ANALOG;
		break;
	case 0xC:
		dddb->interface_type = DI_CTA_VESA_DDDB_INTERFACE_M1_DIGITAL;
		break;
	default:
		add_failure(cta,
			    "VESA Video Display Device Data Block: Unknown interface type 0x%x.",
			    interface_type);
		return false;
	}

	if (check_vesa_dddb_num_channels(dddb->interface_type, num_channels))
		dddb->num_channels = num_channels;
	else
		add_failure(cta,
			    "VESA Video Display Device Data Block: Invalid number of lanes/channels %u.",
			    num_channels);

	dddb->interface_version = get_bit_range(data[0x03 - offset], 7, 4);
	dddb->interface_release = get_bit_range(data[0x03 - offset], 3, 0);

	content_protection = data[0x04 - offset];
	switch (content_protection) {
	case DI_CTA_VESA_DDDB_CONTENT_PROTECTION_NONE:
	case DI_CTA_VESA_DDDB_CONTENT_PROTECTION_HDCP:
	case DI_CTA_VESA_DDDB_CONTENT_PROTECTION_DTCP:
	case DI_CTA_VESA_DDDB_CONTENT_PROTECTION_DPCP:
		dddb->content_protection = content_protection;
		break;
	default:
		add_failure(cta,
			    "VESA Video Display Device Data Block: Invalid content protection 0x%x.",
			    content_protection);
	}

	dddb->min_clock_freq_mhz = get_bit_range(data[0x05 - offset], 7, 2);
	dddb->max_clock_freq_mhz =
		(get_bit_range(data[0x05 - offset], 1, 0) << 8) | data[0x06 - offset];
	if (dddb->min_clock_freq_mhz > dddb->max_clock_freq_mhz) {
		add_failure(cta,
			    "VESA Video Display Device Data Block: Minimum clock frequency (%d MHz) greater than maximum (%d MHz).",
			    dddb->min_clock_freq_mhz, dddb->max_clock_freq_mhz);
		dddb->min_clock_freq_mhz = dddb->max_clock_freq_mhz = 0;
	}

	dddb->native_horiz_pixels = data[0x07 - offset] | (data[0x08 - offset] << 8);
	dddb->native_vert_pixels = data[0x09 - offset] | (data[0x0A - offset] << 8);

	dddb->aspect_ratio = (float)data[0x0B - offset] / 100 + 1;
	dddb->default_orientation = get_bit_range(data[0x0C - offset], 7, 6);
	dddb->rotation_cap = get_bit_range(data[0x0C - offset], 5, 4);
	dddb->zero_pixel_location = get_bit_range(data[0x0C - offset], 3, 2);
	scan_direction = get_bit_range(data[0x0C - offset], 1, 0);
	if (scan_direction != 3)
		dddb->scan_direction = scan_direction;
	else
		add_failure(cta,
			   "VESA Video Display Device Data Block: Invalid scan direction 0x%x.",
			   scan_direction);

	subpixel_layout = data[0x0D - offset];
	switch (subpixel_layout) {
	case DI_CTA_VESA_DDDB_SUBPIXEL_UNDEFINED:
	case DI_CTA_VESA_DDDB_SUBPIXEL_RGB_VERT:
	case DI_CTA_VESA_DDDB_SUBPIXEL_RGB_HORIZ:
	case DI_CTA_VESA_DDDB_SUBPIXEL_EDID_CHROM_VERT:
	case DI_CTA_VESA_DDDB_SUBPIXEL_EDID_CHROM_HORIZ:
	case DI_CTA_VESA_DDDB_SUBPIXEL_QUAD_RGGB:
	case DI_CTA_VESA_DDDB_SUBPIXEL_QUAD_GBRG:
	case DI_CTA_VESA_DDDB_SUBPIXEL_DELTA_RGB:
	case DI_CTA_VESA_DDDB_SUBPIXEL_MOSAIC:
	case DI_CTA_VESA_DDDB_SUBPIXEL_QUAD_ANY:
	case DI_CTA_VESA_DDDB_SUBPIXEL_FIVE:
	case DI_CTA_VESA_DDDB_SUBPIXEL_SIX:
	case DI_CTA_VESA_DDDB_SUBPIXEL_CLAIRVOYANTE_PENTILE:
		dddb->subpixel_layout = subpixel_layout;
		break;
	default:
		add_failure(cta,
			   "VESA Video Display Device Data Block: Invalid subpixel layout 0x%x.",
			   subpixel_layout);
	}

	dddb->horiz_pitch_mm = (float)data[0x0E - offset] * 0.01f;
	dddb->vert_pitch_mm = (float)data[0x0F - offset] * 0.01f;

	dddb->dithering_type = get_bit_range(data[0x10 - offset], 7, 6);
	dddb->direct_drive = has_bit(data[0x10 - offset], 5);
	dddb->overdrive_not_recommended = has_bit(data[0x10 - offset], 4);
	dddb->deinterlacing = has_bit(data[0x10 - offset], 3);
	if (get_bit_range(data[0x10 - offset], 2, 0) != 0)
		add_failure(cta, "VESA Video Display Device Data Block: Reserved miscellaneous display capabilities bits 2-0 must be 0.");

	dddb->audio_support = has_bit(data[0x11 - offset], 7);
	dddb->separate_audio_inputs = has_bit(data[0x11 - offset], 6);
	dddb->audio_input_override = has_bit(data[0x11 - offset], 5);
	if (get_bit_range(data[0x11 - offset], 4, 0) != 0)
		add_failure(cta, "VESA Video Display Device Data Block: Reserved audio bits 4-0 must be 0.");

	dddb->audio_delay_provided = data[0x12 - offset] != 0;
	dddb->audio_delay_ms = 2 * get_bit_range(data[0x12 - offset], 6, 0);
	if (!has_bit(data[0x12 - offset], 7))
		dddb->audio_delay_ms = -dddb->audio_delay_ms;

	dddb->frame_rate_conversion = get_bit_range(data[0x13 - offset], 7, 6);
	dddb->frame_rate_range_hz = get_bit_range(data[0x13 - offset], 5, 0);
	dddb->frame_rate_native_hz = data[0x14 - offset];

	dddb->bit_depth_interface = get_bit_range(data[0x15 - offset], 7, 4) + 1;
	dddb->bit_depth_display = get_bit_range(data[0x15 - offset], 3, 0) + 1;

	dddb->additional_primary_chromaticities_len = get_bit_range(data[0x17 - offset], 1, 0);
	parse_vesa_dddb_additional_primary_chromaticity(&dddb->additional_primary_chromaticities[0],
							get_bit_range(data[0x16 - offset], 7, 4),
							&data[0x18 - offset]);
	parse_vesa_dddb_additional_primary_chromaticity(&dddb->additional_primary_chromaticities[1],
							get_bit_range(data[0x16 - offset], 3, 0),
							&data[0x1A - offset]);
	parse_vesa_dddb_additional_primary_chromaticity(&dddb->additional_primary_chromaticities[2],
							get_bit_range(data[0x17 - offset], 7, 4),
							&data[0x1C - offset]);
	if (get_bit_range(data[0x17 - offset], 3, 2) != 0)
		add_failure(cta, "VESA Video Display Device Data Block: Reserved additional primary chromaticities bits 3-2 of byte 0x17 must be 0.");

	dddb->resp_time_transition = has_bit(data[0x1E - offset], 7);
	dddb->resp_time_ms = get_bit_range(data[0x1E - offset], 6, 0);

	dddb->overscan_horiz_pct = get_bit_range(data[0x1F - offset], 7, 4);
	dddb->overscan_vert_pct = get_bit_range(data[0x1F - offset], 3, 0);

	return true;
}

static bool
parse_colorimetry_block(struct di_edid_cta *cta,
			struct di_cta_colorimetry_block *colorimetry,
			const uint8_t *data, size_t size)
{
	if (size < 2) {
		add_failure(cta, "Colorimetry Data Block: Empty Data Block with length %u.",
			    size);
		return false;
	}

	colorimetry->bt2020_rgb = has_bit(data[0], 7);
	colorimetry->bt2020_ycc = has_bit(data[0], 6);
	colorimetry->bt2020_cycc = has_bit(data[0], 5);
	colorimetry->oprgb = has_bit(data[0], 4);
	colorimetry->opycc_601 = has_bit(data[0], 3);
	colorimetry->sycc_601 = has_bit(data[0], 2);
	colorimetry->xvycc_709 = has_bit(data[0], 1);
	colorimetry->xvycc_601 = has_bit(data[0], 0);

	colorimetry->st2113_rgb = has_bit(data[1], 7);
	colorimetry->ictcp = has_bit(data[1], 6);

	if (get_bit_range(data[1], 5, 0) != 0)
		add_failure_until(cta, 3,
				  "Colorimetry Data Block: Reserved bits MD0-MD3 must be 0.");

	return true;
}

static float
parse_max_luminance(uint8_t raw)
{
	if (raw == 0)
		return 0;
	return 50 * powf(2, (float) raw / 32);
}

static float
parse_min_luminance(uint8_t raw, float max)
{
	if (raw == 0)
		return 0;
	return max * powf((float) raw / 255, 2) / 100;
}

static bool
parse_hdr_static_metadata_block(struct di_edid_cta *cta,
				struct di_cta_hdr_static_metadata_block_priv *metadata,
				const uint8_t *data, size_t size)
{
	uint8_t eotfs, descriptors;

	if (size < 2) {
		add_failure(cta, "HDR Static Metadata Data Block: Empty Data Block with length %u.",
			    size);
		return false;
	}

	metadata->base.eotfs = &metadata->eotfs;
	metadata->base.descriptors = &metadata->descriptors;

	eotfs = data[0];
	metadata->eotfs.traditional_sdr = has_bit(eotfs, 0);
	metadata->eotfs.traditional_hdr = has_bit(eotfs, 1);
	metadata->eotfs.pq = has_bit(eotfs, 2);
	metadata->eotfs.hlg = has_bit(eotfs, 3);
	if (get_bit_range(eotfs, 7, 4))
		add_failure_until(cta, 3, "HDR Static Metadata Data Block: Unknown EOTF.");

	descriptors = data[1];
	metadata->descriptors.type1 = has_bit(descriptors, 0);
	if (get_bit_range(descriptors, 7, 1))
		add_failure_until(cta, 3, "HDR Static Metadata Data Block: Unknown descriptor type.");

	if (size > 2)
		metadata->base.desired_content_max_luminance = parse_max_luminance(data[2]);
	if (size > 3)
		metadata->base.desired_content_max_frame_avg_luminance = parse_max_luminance(data[3]);
	if (size > 4) {
		if (metadata->base.desired_content_max_luminance == 0)
			add_failure(cta, "HDR Static Metadata Data Block: Desired content min luminance is set, but max luminance is unset.");
		else
			metadata->base.desired_content_min_luminance =
				parse_min_luminance(data[4], metadata->base.desired_content_max_luminance);
	}

	return true;
}
static bool
parse_hdr_dynamic_metadata_block(struct di_edid_cta *cta,
				 struct di_cta_hdr_dynamic_metadata_block_priv *priv,
				 const uint8_t *data, size_t size)
{
	struct di_cta_hdr_dynamic_metadata_block *base;
	struct di_cta_hdr_dynamic_metadata_block_type1 *type1;
	struct di_cta_hdr_dynamic_metadata_block_type2 *type2;
	struct di_cta_hdr_dynamic_metadata_block_type3 *type3;
	struct di_cta_hdr_dynamic_metadata_block_type4 *type4;
	struct di_cta_hdr_dynamic_metadata_block_type256 *type256;
	size_t length;
	int type;

	base = &priv->base;
	type1 = &priv->type1;
	type2 = &priv->type2;
	type3 = &priv->type3;
	type4 = &priv->type4;
	type256 = &priv->type256;

	if (size < 3) {
		add_failure(cta, "HDR Dynamic Metadata Data Block: Empty Data Block with length %u.",
			    size);
		return false;
	}

	while (size >= 3) {
		length = data[0];

		if (size < length + 1) {
			add_failure(cta, "HDR Dynamic Metadata Data Block: Length of type bigger than block size.");
			return false;
		}

		if (length < 2) {
			add_failure(cta, "HDR Dynamic Metadata Data Block: Type has wrong length.");
			return false;
		}

		type = (data[2] << 8) | data[1];
		switch (type) {
		case 0x0001:
			if (length < 3) {
				add_failure(cta, "HDR Dynamic Metadata Data Block: Type 1 missing Support Flags.");
				break;
			}
			if (length != 3)
				add_failure(cta, "HDR Dynamic Metadata Data Block: Type 1 length must be 3.");
			type1->type_1_hdr_metadata_version = get_bit_range(data[3], 3, 0);
			base->type1 = type1;
			if (get_bit_range(data[3], 7, 4) != 0)
				add_failure(cta, "HDR Dynamic Metadata Data Block: Type 1 support flags bits 7-4 must be 0.");
			break;
		case 0x0002:
			if (length < 3) {
				add_failure(cta, "HDR Dynamic Metadata Data Block: Type 2 missing Support Flags.");
				break;
			}
			if (length != 3)
				add_failure(cta, "HDR Dynamic Metadata Data Block: Type 2 length must be 3.");
			type2->ts_103_433_spec_version = get_bit_range(data[3], 3, 0);
			if (type2->ts_103_433_spec_version == 0) {
				add_failure(cta, "HDR Dynamic Metadata Data Block: Type 2 spec version of 0 is not allowed.");
				break;
			}
			type2->ts_103_433_1_capable = has_bit(data[3], 4);
			type2->ts_103_433_2_capable = has_bit(data[3], 5);
			type2->ts_103_433_3_capable = has_bit(data[3], 6);
			base->type2 = type2;
			if (has_bit(data[3], 7) != 0)
				add_failure(cta, "HDR Dynamic Metadata Data Block: Type 1 support flags bit 7 must be 0.");
			break;
		case 0x0003:
			if (length != 2)
				add_failure(cta, "HDR Dynamic Metadata Data Block: Type 3 length must be 2.");
			base->type3 = type3;
			break;
		case 0x0004:
			if (length < 3) {
				add_failure(cta, "HDR Dynamic Metadata Data Block: Type 4 missing Support Flags.");
				break;
			}
			if (length != 3)
				add_failure(cta, "HDR Dynamic Metadata Data Block: Type 4 length must be 3.");
			type4->type_4_hdr_metadata_version = get_bit_range(data[3], 3, 0);
			base->type4 = type4;
			if (get_bit_range(data[3], 7, 4) != 0)
				add_failure(cta, "HDR Dynamic Metadata Data Block: Type 4 support flags bits 7-4 must be 0.");
			break;
		case 0x0100:
			if (length < 3) {
				add_failure(cta, "HDR Dynamic Metadata Data Block: Type 256 missing Support Flags.");
				break;
			}
			if (length != 3)
				add_failure(cta, "HDR Dynamic Metadata Data Block: Type 256 length must be 3.");
			type256->graphics_overlay_flag_version = get_bit_range(data[3], 3, 0);
			base->type256 = type256;
			if (get_bit_range(data[3], 7, 4) != 0)
				add_failure(cta, "HDR Dynamic Metadata Data Block: Type 256 support flags bits 7-4 must be 0.");
			break;
		default:
			add_failure(cta, "HDR Dynamic Metadata Data Block: Unknown Type 0x%04x.", type);
			break;
		}

		size -= length + 1;
		data += length + 1;
	}

	return true;
}

static bool
parse_vesa_transfer_characteristics_block(struct di_edid_cta *cta,
					  struct di_cta_vesa_transfer_characteristics *tf,
					  const uint8_t *data, size_t size)
{
	size_t i;

	if (size != 7 && size != 15 && size != 31) {
		add_failure(cta, "Invalid length %u.", size);
		return false;
	}

	tf->points_len = (uint8_t) size + 1;
	tf->usage = get_bit_range(data[0], 7, 6);

	tf->points[0] = get_bit_range(data[0], 5, 0) / 1023.0f;
	for (i = 1; i < size; i++)
		tf->points[i] = tf->points[i - 1] + data[i] / 1023.0f;
	tf->points[i] = 1.0f;

	return true;
}

static bool
parse_video_format_pref_block(struct di_edid_cta *cta,
			      struct di_cta_video_format_pref_block *vfpdb,
			      const uint8_t *data, size_t size)
{
	struct di_cta_svr *svr;
	size_t i;
	uint8_t code;

	for (i = 0; i < size; i++) {
		code = data[i];

		if (code == 0 ||
		    code == 128 ||
		    (code >= 161 && code <= 192) ||
		    code == 255) {
			add_failure(cta, "Video Format Preference Data Block: "
				    "using reserved Short Video Reference value %u.",
				    code);
			continue;
		}

		svr = calloc(1, sizeof(*svr));
		if (!svr)
			return false;

		if ((code >= 1 && code <= 127) ||
		    (code >= 193 && code <= 253)) {
			svr->type = DI_CTA_SVR_TYPE_VIC;
			svr->vic = code;
		} else if (code >= 129 && code <= 144) {
			svr->type = DI_CTA_SVR_TYPE_DTD_INDEX;
			svr->dtd_index = code - 129;
		} else if (code >= 145 && code <= 160) {
			svr->type = DI_CTA_SVR_TYPE_T7T10VTDB;
			svr->dtd_index = code - 145;
		} else if (code == 254) {
			svr->type = DI_CTA_SVR_TYPE_FIRST_T8VTDB;
		} else {
			abort(); /* unreachable */
		}

		assert(vfpdb->svrs_len < EDID_CTA_MAX_VIDEO_FORMAT_PREF_BLOCK_ENTRIES);
		vfpdb->svrs[vfpdb->svrs_len++] = svr;
	}

	return true;
}


static void
parse_ycbcr420_cap_map(struct di_edid_cta *cta,
		       struct di_cta_ycbcr420_cap_map *ycbcr420_cap_map,
		       const uint8_t *data, size_t size)
{
	if (size == 0) {
		ycbcr420_cap_map->all = true;
		return;
	}

	assert(size <= sizeof(ycbcr420_cap_map->svd_bitmap));
	memcpy(ycbcr420_cap_map->svd_bitmap, data, size);
}

static bool
parse_hdmi_audio_3d_descriptor(struct di_edid_cta *cta,
			       struct di_cta_sad_priv *sad,
			       const uint8_t *data, size_t size)
{
	/* Contains the same data as the Short Audio Descriptor, packed differently */
	struct di_cta_sad *base = &sad->base;
	struct di_cta_sad_sample_rates *sample_rate = &sad->supported_sample_rates;
	struct di_cta_sad_lpcm *lpcm = &sad->lpcm;
	uint8_t code;

	assert(size >= CTA_HDMI_AUDIO_3D_DESCRIPTOR_SIZE);

	code = get_bit_range(data[0], 3, 0);
	if (!parse_sad_format(cta, code, 0, &base->format, "HDMI Audio Data Block"))
		return false;

	if (base->format != DI_CTA_AUDIO_FORMAT_LPCM &&
	    base->format != DI_CTA_AUDIO_FORMAT_ONE_BIT_AUDIO) {
		add_failure(cta,
			    "HDMI Audio Data Block: Unsupported 3D Audio Format 0x%04x.",
			    code);
		return false;
	}

	base->max_channels = get_bit_range(data[1], 4, 0) + 1;
	sample_rate->has_192_khz = has_bit(data[2], 6);
	sample_rate->has_176_4_khz = has_bit(data[2], 5);
	sample_rate->has_96_khz = has_bit(data[2], 4);
	sample_rate->has_88_2_khz = has_bit(data[2], 3);
	sample_rate->has_48_khz = has_bit(data[2], 2);
	sample_rate->has_44_1_khz = has_bit(data[2], 1);
	sample_rate->has_32_khz = has_bit(data[2], 0);
	base->supported_sample_rates = sample_rate;

	if (base->format == DI_CTA_AUDIO_FORMAT_LPCM) {
		lpcm->has_sample_size_24_bits = has_bit(data[3], 2);
		lpcm->has_sample_size_20_bits = has_bit(data[3], 1);
		lpcm->has_sample_size_16_bits = has_bit(data[3], 0);
		base->lpcm = lpcm;
	}

	if (base->format == DI_CTA_AUDIO_FORMAT_ONE_BIT_AUDIO) {
		/* TODO data[3] 7:0 contains unknown Audio Format Code dependent value */
	}

	return true;
}

static bool
parse_hdmi_audio_block(struct di_edid_cta *cta,
		       struct di_cta_hdmi_audio_block_priv *priv,
		       const uint8_t *data, size_t size)
{
	struct di_cta_hdmi_audio_block *hdmi_audio = &priv->base;
	struct di_cta_hdmi_audio_multi_stream *ms = &priv->ms;
	struct di_cta_hdmi_audio_3d *a3d = &priv->a3d;
	uint8_t multi_stream;
	bool ms_non_mixed;
	size_t num_3d_audio_descs;
	size_t num_descs;
	struct di_cta_sad_priv *sad_priv;
	uint8_t channels;

	if (size < 1) {
		add_failure(cta, "HDMI Audio Data Block: Empty Data Block with length 0.");
		return false;
	}

	multi_stream = get_bit_range(data[0], 1, 0);
	ms_non_mixed = has_bit(data[0], 2);

	if (multi_stream > 0) {
		hdmi_audio->multi_stream = ms;
		ms->max_streams = multi_stream + 1;
		ms->supports_non_mixed = ms_non_mixed;
	} else if (ms_non_mixed) {
		add_failure(cta, "HDMI Audio Data Block: MS NonMixed support indicated but "
				 "Max Stream Count == 0.");
	}

	if (size < 2)
		return true;

	num_3d_audio_descs = get_bit_range(data[1], 2, 0);
	if (num_3d_audio_descs == 0)
		return true;

	/* If there are 3d Audio Descriptors, there is one last Speaker Allocation Descriptor */
	num_descs = num_3d_audio_descs + 1;

	/* Skip to the first descriptor */
	size -= 2;
	data += 2;

	/* Make sure there is enough space for the descriptors */
	if (num_descs > size / CTA_HDMI_AUDIO_3D_DESCRIPTOR_SIZE) {
		add_failure(cta, "HDMI Audio Data Block: More descriptors indicated than block size allows.");
		return true;
	}

	hdmi_audio->audio_3d = a3d;
	a3d->sads = (const struct di_cta_sad * const*)priv->sads;

	/* First the 3D Audio Descriptors, the last one is the 3D Speaker Allocation Descriptor */
	while (num_descs > 1) {
		sad_priv = calloc(1, sizeof(*sad_priv));
		if (!sad_priv)
			return false;

		if (!parse_hdmi_audio_3d_descriptor(cta, sad_priv, data, size)) {
			free(sad_priv);
			goto skip;
		}

		assert(priv->sads_len < EDID_CTA_MAX_HDMI_AUDIO_BLOCK_ENTRIES);
		priv->sads[priv->sads_len++] = sad_priv;

skip:
		num_descs--;
		size -= CTA_HDMI_AUDIO_3D_DESCRIPTOR_SIZE;
		data += CTA_HDMI_AUDIO_3D_DESCRIPTOR_SIZE;
	}

	channels = get_bit_range(data[3], 7, 4);

	switch (channels) {
	case DI_CTA_HDMI_AUDIO_3D_CHANNELS_UNKNOWN:
	case DI_CTA_HDMI_AUDIO_3D_CHANNELS_10_2:
	case DI_CTA_HDMI_AUDIO_3D_CHANNELS_22_2:
	case DI_CTA_HDMI_AUDIO_3D_CHANNELS_30_2:
		a3d->channels = channels;
		break;
	default:
		a3d->channels = DI_CTA_HDMI_AUDIO_3D_CHANNELS_UNKNOWN;
		break;
	}

	parse_speaker_alloc(cta, &a3d->speakers, data, "Room Configuration Data Block");

	return true;
}

static struct di_cta_infoframe_descriptor *
parse_infoframe(struct di_edid_cta *cta, uint8_t type,
		const uint8_t *data, size_t size)
{
	struct di_cta_infoframe_descriptor infoframe = {0};
	struct di_cta_infoframe_descriptor *ifp;

	if (type >= 8 && type <= 0x1f) {
		add_failure(cta, "InfoFrame Data Block: Type code %u is reserved.",
			    type);
		return NULL;
	}

	if (type >= 0x20) {
		add_failure(cta, "InfoFrame Data Block: Type code %u is forbidden.",
			    type);
		return NULL;
	}

	if (type == 1) {
		/* No known vendor specific InfoFrames, yet */
		return NULL;
	} else {
		switch (type) {
		case 0x02:
			infoframe.type = DI_CTA_INFOFRAME_TYPE_AUXILIARY_VIDEO_INFORMATION;
			break;
		case 0x03:
			infoframe.type = DI_CTA_INFOFRAME_TYPE_SOURCE_PRODUCT_DESCRIPTION;
			break;
		case 0x04:
			infoframe.type = DI_CTA_INFOFRAME_TYPE_AUDIO;
			break;
		case 0x05:
			infoframe.type = DI_CTA_INFOFRAME_TYPE_MPEG_SOURCE;
			break;
		case 0x06:
			infoframe.type = DI_CTA_INFOFRAME_TYPE_NTSC_VBI;
			break;
		case 0x07:
			infoframe.type = DI_CTA_INFOFRAME_TYPE_DYNAMIC_RANGE_AND_MASTERING;
			break;
		default:
			abort(); /* unreachable */
		}
	}

	ifp = calloc(1, sizeof(*ifp));
	if (!ifp)
		return NULL;

	*ifp = infoframe;
	return ifp;
}

static bool
parse_infoframe_block(struct di_edid_cta *cta,
		      struct di_cta_infoframe_block_priv *ifb,
		      const uint8_t *data, size_t size)
{
	size_t index = 0, length;
	uint8_t type;
	struct di_cta_infoframe_descriptor *infoframe;

	if (size < 2) {
		add_failure(cta, "InfoFrame Data Block: Empty Data Block with length %u.",
			    size);
		return false;
	}

	ifb->block.num_simultaneous_vsifs = data[1] + 1;
	ifb->block.infoframes = (const struct di_cta_infoframe_descriptor *const *)ifb->infoframes;

	index = get_bit_range(data[0], 7, 5) + 2;
	if (get_bit_range(data[0], 4, 0) != 0)
		add_failure(cta, "InfoFrame Data Block: InfoFrame Processing "
				 "Descriptor Header bits F14-F10 shall be 0.");

	while (true) {
		if (index == size)
			break;
		if (index > size) {
			add_failure(cta, "InfoFrame Data Block: Payload length exceeds block size.");
			return false;
		}

		length = get_bit_range(data[index], 7, 5);
		type = get_bit_range(data[index], 4, 0);

		if (type == 0) {
			add_failure(cta, "InfoFrame Data Block: Short InfoFrame Descriptor with type 0 is forbidden.");
			return false;
		} else if (type == 1) {
			length += 4;
		} else {
			length += 1;
		}

		if (index + length > size) {
			add_failure(cta, "InfoFrame Data Block: Payload length exceeds block size.");
			return false;
		}

		infoframe = parse_infoframe(cta, type, &data[index], length);
		if (infoframe) {
			assert(ifb->infoframes_len < EDID_CTA_INFOFRAME_BLOCK_ENTRIES);
			ifb->infoframes[ifb->infoframes_len++] = infoframe;
		}

		index += length;
	}

	return true;
}

static double
decode_coord(unsigned char x)
{
	signed char s = (signed char)x;

	return s / 64.0;
}

static bool
parse_room_config_block(struct di_edid_cta *cta,
			struct di_cta_room_configuration *rc,
			const uint8_t *data, size_t size)
{
	bool has_display_coords;
	bool has_speaker_count;

	if (size < 4) {
		add_failure(cta, "Room Configuration Data Block: Empty Data Block with length %u.",
			    size);
		return false;
	}

	has_display_coords = has_bit(data[0], 7);
	has_speaker_count = has_bit(data[0], 6);
	rc->has_speaker_location_descriptors = has_bit(data[0], 5);

	if (has_speaker_count) {
		rc->speaker_count = get_bit_range(data[0], 4, 0) + 1;
	} else {
		if (get_bit_range(data[0], 4, 0) != 0) {
			add_failure(cta, "Room Configuration Data Block: "
					 "'Speaker' flag is 0, but the Speaker Count is not 0.");
		}

		if (rc->has_speaker_location_descriptors) {
			add_failure(cta, "Room Configuration Data Block: "
					 "'Speaker' flag is 0, but there are "
					 "Speaker Location Descriptors.");
		}
	}

	parse_speaker_alloc(cta, &rc->speakers, &data[1], "Room Configuration Data Block");

	rc->max_x = 16;
	rc->max_y = 16;
	rc->max_z = 8;
	rc->display_x = 0.0;
	rc->display_y = 1.0;
	rc->display_z = 0.0;

	if (size < 7) {
		if (has_display_coords)
			add_failure(cta, "Room Configuration Data Block: "
					 "'Display' flag is 1, but the Display and Maximum coordinates are not present.");
		return true;
	}

	rc->max_x = data[4];
	rc->max_y = data[5];
	rc->max_z = data[6];

	if (size < 10) {
		if (has_display_coords)
			add_failure(cta, "Room Configuration Data Block: "
					 "'Display' flag is 1, but the Display coordinates are not present.");
		return true;
	}

	rc->display_x = decode_coord(data[7]);
	rc->display_y = decode_coord(data[8]);
	rc->display_z = decode_coord(data[9]);

	return true;
}

static bool
parse_speaker_location_block(struct di_edid_cta *cta,
			     struct di_cta_speaker_location_block *sldb,
			     const uint8_t *data, size_t size)
{
	struct di_cta_speaker_locations speaker_loc, *slp;

	if (size < 2) {
		add_failure(cta, "Speaker Location Data Block: Empty Data Block with length %u.",
			    size);
		return false;
	}

	while (size >= 2) {
		speaker_loc.has_coords = has_bit(data[0], 6);
		speaker_loc.is_active = has_bit(data[0], 5);
		speaker_loc.channel_index = get_bit_range(data[0], 4, 0);
		speaker_loc.speaker_id = get_bit_range(data[1], 4, 0);

		if (has_bit(data[0], 7) || get_bit_range(data[1], 7, 5) != 0) {
			add_failure(cta, "Speaker Location Data Block: Bits F27-F25, F17 must be 0.");
		}

		if (speaker_loc.has_coords && size >= 5) {
			speaker_loc.x = decode_coord(data[2]);
			speaker_loc.y = decode_coord(data[3]);
			speaker_loc.z = decode_coord(data[4]);
			size -= 5;
			data += 5;
		} else if (speaker_loc.has_coords) {
			add_failure(cta, "Speaker Location Data Block: COORD bit "
					 "set but contains no Coordinates.");
			return false;
		} else {
			size -= 2;
			data += 2;
		}

		slp = calloc(1, sizeof(*slp));
		if (!slp)
			return false;

		*slp = speaker_loc;
		assert(sldb->locations_len < EDID_CTA_MAX_SPEAKER_LOCATION_BLOCK_ENTRIES);
		sldb->locations[sldb->locations_len++] = slp;
	}

	return true;
}

static bool
parse_did_type_vii_timing(struct di_edid_cta *cta,
			  struct di_displayid_type_i_ii_vii_timing *t,
			  const uint8_t *data, size_t size)
{
	uint8_t revision;

	if (size != 21) {
		add_failure(cta, "DisplayID Type VII Video Timing Data Block: "
				 "Empty Data Block with length %u.", size);
		return false;
	}

	if (get_bit_range(data[0], 6, 4) != 0) {
		add_failure(cta, "DisplayID Type VII Video Timing Data Block: "
				 "T7_M shall be 000b.");
		return false;
	}

	revision = get_bit_range(data[0], 2, 0);
	if (revision != 2) {
		add_failure(cta, "DisplayID Type VII Video Timing Data Block: "
				 "Unexpected revision (%u != %u).",
			    revision, 2);
		return false;
	}

	if (has_bit(data[0], 3)) {
		add_failure(cta, "DisplayID Type VII Video Timing Data Block: "
				 "DSC_PT shall be 0.");
	}
	if (has_bit(data[0], 7)) {
		add_failure(cta, "DisplayID Type VII Video Timing Data Block: "
				 "Block Revision and Other Data Bit 7 must be 0.");
	}

	data += 1;
	size -= 1;

	if (!_di_displayid_parse_type_1_7_timing(t, cta->logger,
						 "DisplayID Type VII Video Timing Data Block",
						 data, true))
		return false;

	return true;
}

static void
destroy_data_block(struct di_cta_data_block *data_block)
{
	size_t i;
	struct di_cta_video_block *video;
	struct di_cta_audio_block *audio;
	struct di_cta_infoframe_block_priv *infoframe;
	struct di_cta_speaker_location_block *speaker_location;
	struct di_cta_video_format_pref_block *vfpdb;
	struct di_cta_hdmi_audio_block_priv *hdmi_audio;

	switch (data_block->tag) {
	case DI_CTA_DATA_BLOCK_VIDEO:
		video = &data_block->video;
		for (i = 0; i < video->svds_len; i++)
			free(video->svds[i]);
		break;
	case DI_CTA_DATA_BLOCK_YCBCR420:
		video = &data_block->ycbcr420;
		for (i = 0; i < video->svds_len; i++)
			free(video->svds[i]);
		break;
	case DI_CTA_DATA_BLOCK_AUDIO:
		audio = &data_block->audio;
		for (i = 0; i < audio->sads_len; i++)
			free(audio->sads[i]);
		break;
	case DI_CTA_DATA_BLOCK_INFOFRAME:
		infoframe = &data_block->infoframe;
		for (i = 0; i < infoframe->infoframes_len; i++)
			free(infoframe->infoframes[i]);
		break;
	case DI_CTA_DATA_BLOCK_SPEAKER_LOCATION:
		speaker_location = &data_block->speaker_location;
		for (i = 0; i < speaker_location->locations_len; i++)
			free(speaker_location->locations[i]);
		break;
	case DI_CTA_DATA_BLOCK_VIDEO_FORMAT_PREF:
		vfpdb = &data_block->video_format_pref;
		for (i = 0; i < vfpdb->svrs_len; i++)
			free(vfpdb->svrs[i]);
		break;
	case DI_CTA_DATA_BLOCK_HDMI_AUDIO:
		hdmi_audio = &data_block->hdmi_audio;
		for (i = 0; i < hdmi_audio->sads_len; i++)
			free(hdmi_audio->sads[i]);
		break;
	default:
		break; /* Nothing to do */
	}

	free(data_block);
}

static bool
parse_data_block(struct di_edid_cta *cta, uint8_t raw_tag, const uint8_t *data, size_t size)
{
	enum di_cta_data_block_tag tag;
	uint8_t extended_tag;
	struct di_cta_data_block *data_block;

	data_block = calloc(1, sizeof(*data_block));
	if (!data_block) {
		return false;
	}

	switch (raw_tag) {
	case 1:
		tag = DI_CTA_DATA_BLOCK_AUDIO;
		if (!parse_audio_block(cta, &data_block->audio, data, size))
			goto error;
		break;
	case 2:
		tag = DI_CTA_DATA_BLOCK_VIDEO;
		if (!parse_video_block(cta, &data_block->video, data, size))
			goto error;
		break;
	case 3:
		/* Vendor-Specific Data Block */
		goto skip;
	case 4:
		tag = DI_CTA_DATA_BLOCK_SPEAKER_ALLOC;
		if (!parse_speaker_alloc_block(cta, &data_block->speaker_alloc,
					       data, size))
			goto error;
		break;
	case 5:
		tag = DI_CTA_DATA_BLOCK_VESA_DISPLAY_TRANSFER_CHARACTERISTIC;
		if (!parse_vesa_transfer_characteristics_block(cta,
							       &data_block->vesa_transfer_characteristics,
							       data, size))
			goto error;
		break;
	case 6:
		tag = DI_CTA_DATA_BLOCK_VIDEO_FORMAT;
		break;
	case 7:
		/* Use Extended Tag */
		if (size < 1) {
			add_failure(cta, "Empty block with extended tag.");
			goto skip;
		}

		extended_tag = data[0];
		data = &data[1];
		size--;

		switch (extended_tag) {
		case 0:
			tag = DI_CTA_DATA_BLOCK_VIDEO_CAP;
			if (!parse_video_cap_block(cta, &data_block->video_cap,
						   data, size))
				goto skip;
			break;
		case 2:
			tag = DI_CTA_DATA_BLOCK_VESA_DISPLAY_DEVICE;
			if (!parse_vesa_dddb(cta, &data_block->vesa_dddb,
					     data, size))
				goto skip;
			break;
		case 5:
			tag = DI_CTA_DATA_BLOCK_COLORIMETRY;
			if (!parse_colorimetry_block(cta,
						     &data_block->colorimetry,
						     data, size))
				goto skip;
			break;
		case 6:
			tag = DI_CTA_DATA_BLOCK_HDR_STATIC_METADATA;
			if (!parse_hdr_static_metadata_block(cta,
							     &data_block->hdr_static_metadata,
							     data, size))
				goto skip;
			break;
		case 7:
			tag = DI_CTA_DATA_BLOCK_HDR_DYNAMIC_METADATA;
			if (!parse_hdr_dynamic_metadata_block(cta,
							      &data_block->hdr_dynamic_metadata,
							      data, size))
				goto skip;
			break;
		case 8:
			tag = DI_CTA_DATA_BLOCK_NATIVE_VIDEO_RESOLUTION;
			break;
		case 13:
			tag = DI_CTA_DATA_BLOCK_VIDEO_FORMAT_PREF;
			if (!parse_video_format_pref_block(cta,
							   &data_block->video_format_pref,
							   data, size))
				goto skip;
			break;
		case 14:
			tag = DI_CTA_DATA_BLOCK_YCBCR420;
			if (!parse_ycbcr420_block(cta,
						  &data_block->ycbcr420,
						  data, size))
				goto skip;
			break;
		case 15:
			tag = DI_CTA_DATA_BLOCK_YCBCR420_CAP_MAP;
			parse_ycbcr420_cap_map(cta,
					       &data_block->ycbcr420_cap_map,
					       data, size);
			break;
		case 18:
			tag = DI_CTA_DATA_BLOCK_HDMI_AUDIO;
			if (!parse_hdmi_audio_block(cta,
						    &data_block->hdmi_audio,
						    data, size))
				goto skip;
			break;
		case 19:
			tag = DI_CTA_DATA_BLOCK_ROOM_CONFIG;
			if (!parse_room_config_block(cta,
						     &data_block->room_config,
						     data, size))
				goto skip;
			break;
		case 20:
			tag = DI_CTA_DATA_BLOCK_SPEAKER_LOCATION;
			if (!parse_speaker_location_block(cta,
							  &data_block->speaker_location,
							  data, size))
				goto skip;
			break;
		case 32:
			tag = DI_CTA_DATA_BLOCK_INFOFRAME;
			if (!parse_infoframe_block(cta,
						   &data_block->infoframe,
						   data, size))
				goto skip;
			break;
		case 34:
			tag = DI_CTA_DATA_BLOCK_DISPLAYID_VIDEO_TIMING_VII;
			if (!parse_did_type_vii_timing(cta,
						       &data_block->did_vii_timing,
						       data, size))
				goto skip;
			break;
		case 35:
			tag = DI_CTA_DATA_BLOCK_DISPLAYID_VIDEO_TIMING_VIII;
			break;
		case 42:
			tag = DI_CTA_DATA_BLOCK_DISPLAYID_VIDEO_TIMING_X;
			break;
		case 120:
			tag = DI_CTA_DATA_BLOCK_HDMI_EDID_EXT_OVERRIDE;
			break;
		case 121:
			tag = DI_CTA_DATA_BLOCK_HDMI_SINK_CAP;
			break;
		case 1: /* Vendor-Specific Video Data Block */
		case 17: /* Vendor-Specific Audio Data Block */
			goto skip;
		default:
			/* Reserved */
			add_failure_until(cta, 3,
					  "Unknown CTA-861 Data Block (extended tag 0x"PRIx8", length %zu).",
					  extended_tag, size);
			goto skip;
		}
		break;
	default:
		/* Reserved */
		add_failure_until(cta, 3, "Unknown CTA-861 Data Block (tag 0x"PRIx8", length %zu).",
				  raw_tag, size);
		goto skip;
	}

	data_block->tag = tag;
	assert(cta->data_blocks_len < EDID_CTA_MAX_DATA_BLOCKS);
	cta->data_blocks[cta->data_blocks_len++] = data_block;
	return true;

skip:
	free(data_block);
	return true;

error:
	destroy_data_block(data_block);
	return false;
}

bool
_di_edid_cta_parse(struct di_edid_cta *cta, const uint8_t *data, size_t size,
		   struct di_logger *logger)
{
	uint8_t flags, dtd_start;
	uint8_t data_block_header, data_block_tag, data_block_size;
	size_t i;
	struct di_edid_detailed_timing_def_priv *detailed_timing_def;

	assert(size == 128);
	assert(data[0] == 0x02);

	cta->logger = logger;

	cta->revision = data[1];
	dtd_start = data[2];

	flags = data[3];
	if (cta->revision >= 2) {
		cta->flags.it_underscan = has_bit(flags, 7);
		cta->flags.basic_audio = has_bit(flags, 6);
		cta->flags.ycc444 = has_bit(flags, 5);
		cta->flags.ycc422 = has_bit(flags, 4);
		cta->flags.native_dtds = get_bit_range(flags, 3, 0);
	} else if (flags != 0) {
		/* Reserved */
		add_failure(cta, "Non-zero byte 3.");
	}

	if (dtd_start == 0) {
		return true;
	} else if (dtd_start < CTA_HEADER_SIZE || dtd_start >= size) {
		errno = EINVAL;
		return false;
	}

	i = CTA_HEADER_SIZE;
	while (i < dtd_start) {
		data_block_header = data[i];
		data_block_tag = get_bit_range(data_block_header, 7, 5);
		data_block_size = get_bit_range(data_block_header, 4, 0);

		if (i + 1 + data_block_size > dtd_start) {
			add_failure(cta, "Data Block at offset %zu overlaps Detailed Timing "
					 "Definitions. Skipping all further Data Blocks.", i);
			break;
		}

		if (!parse_data_block(cta, data_block_tag,
				      &data[i + 1], data_block_size)) {
			_di_edid_cta_finish(cta);
			return false;
		}

		i += 1 + data_block_size;
	}

	if (i != dtd_start)
		add_failure(cta, "Offset is %"PRIu8", but should be %zu.",
			    dtd_start, i);

	for (i = dtd_start; i + EDID_BYTE_DESCRIPTOR_SIZE <= CTA_DTD_END;
	     i += EDID_BYTE_DESCRIPTOR_SIZE) {
		if (data[i] == 0)
			break;

		detailed_timing_def = _di_edid_parse_detailed_timing_def(&data[i]);
		if (!detailed_timing_def) {
			_di_edid_cta_finish(cta);
			return false;
		}
		assert(cta->detailed_timing_defs_len < EDID_CTA_MAX_DETAILED_TIMING_DEFS);
		cta->detailed_timing_defs[cta->detailed_timing_defs_len++] = detailed_timing_def;
	}

	/* All padding bytes after the last DTD must be zero */
	while (i < CTA_DTD_END) {
		if (data[i] != 0) {
			add_failure(cta, "Padding: Contains non-zero bytes.");
			break;
		}
		i++;
	}

	cta->logger = NULL;
	return true;
}

void
_di_edid_cta_finish(struct di_edid_cta *cta)
{
	size_t i;

	for (i = 0; i < cta->data_blocks_len; i++) {
		destroy_data_block(cta->data_blocks[i]);
	}

	for (i = 0; i < cta->detailed_timing_defs_len; i++) {
		free(cta->detailed_timing_defs[i]);
	}
}

int
di_edid_cta_get_revision(const struct di_edid_cta *cta)
{
	return cta->revision;
}

const struct di_edid_cta_flags *
di_edid_cta_get_flags(const struct di_edid_cta *cta)
{
	return &cta->flags;
}

const struct di_cta_data_block *const *
di_edid_cta_get_data_blocks(const struct di_edid_cta *cta)
{
	return (const struct di_cta_data_block *const *) cta->data_blocks;
}

enum di_cta_data_block_tag
di_cta_data_block_get_tag(const struct di_cta_data_block *block)
{
	return block->tag;
}

const struct di_cta_svd *const *
di_cta_data_block_get_svds(const struct di_cta_data_block *block)
{
	if (block->tag != DI_CTA_DATA_BLOCK_VIDEO) {
		return NULL;
	}
	return (const struct di_cta_svd *const *) block->video.svds;
}

const struct di_cta_svd *const *
di_cta_data_block_get_ycbcr420_svds(const struct di_cta_data_block *block)
{
	if (block->tag != DI_CTA_DATA_BLOCK_YCBCR420) {
		return NULL;
	}
	return (const struct di_cta_svd *const *) block->ycbcr420.svds;
}

const struct di_cta_svr *const *
di_cta_data_block_get_svrs(const struct di_cta_data_block *block)
{
	if (block->tag != DI_CTA_DATA_BLOCK_VIDEO_FORMAT_PREF) {
		return NULL;
	}
	return (const struct di_cta_svr *const *) block->video_format_pref.svrs;
}

const struct di_cta_sad *const *
di_cta_data_block_get_sads(const struct di_cta_data_block *block)
{
	if (block->tag != DI_CTA_DATA_BLOCK_AUDIO) {
		return NULL;
	}
	return (const struct di_cta_sad *const *) block->audio.sads;
}

const struct di_cta_speaker_alloc_block *
di_cta_data_block_get_speaker_alloc(const struct di_cta_data_block *block)
{
	if (block->tag != DI_CTA_DATA_BLOCK_SPEAKER_ALLOC) {
		return NULL;
	}
	return &block->speaker_alloc;
}

const struct di_cta_colorimetry_block *
di_cta_data_block_get_colorimetry(const struct di_cta_data_block *block)
{
	if (block->tag != DI_CTA_DATA_BLOCK_COLORIMETRY) {
		return NULL;
	}
	return &block->colorimetry;
}

const struct di_cta_hdr_static_metadata_block *
di_cta_data_block_get_hdr_static_metadata(const struct di_cta_data_block *block)
{
	if (block->tag != DI_CTA_DATA_BLOCK_HDR_STATIC_METADATA) {
		return NULL;
	}
	return &block->hdr_static_metadata.base;
}

const struct di_cta_hdr_dynamic_metadata_block *
di_cta_data_block_get_hdr_dynamic_metadata(const struct di_cta_data_block *block)
{
	if (block->tag != DI_CTA_DATA_BLOCK_HDR_DYNAMIC_METADATA) {
		return NULL;
	}
	return &block->hdr_dynamic_metadata.base;
}

const struct di_cta_video_cap_block *
di_cta_data_block_get_video_cap(const struct di_cta_data_block *block)
{
	if (block->tag != DI_CTA_DATA_BLOCK_VIDEO_CAP) {
		return NULL;
	}
	return &block->video_cap;
}

const struct di_cta_vesa_dddb *
di_cta_data_block_get_vesa_dddb(const struct di_cta_data_block *block)
{
	if (block->tag != DI_CTA_DATA_BLOCK_VESA_DISPLAY_DEVICE) {
		return NULL;
	}
	return &block->vesa_dddb;
}

bool
di_cta_ycbcr420_cap_map_supported(const struct di_cta_ycbcr420_cap_map *cap_map,
				  size_t svd_index)
{
	size_t byte, bit;

	if (cap_map->all)
		return true;

	byte = svd_index / 8;
	bit = svd_index % 8;

	if (byte >= EDID_CTA_MAX_YCBCR420_CAP_MAP_BLOCK_ENTRIES)
		return false;

	return cap_map->svd_bitmap[byte] & (1 << bit);
}

const struct di_cta_ycbcr420_cap_map *
di_cta_data_block_get_ycbcr420_cap_map(const struct di_cta_data_block *block)
{
	if (block->tag != DI_CTA_DATA_BLOCK_YCBCR420_CAP_MAP) {
		return NULL;
	}
	return &block->ycbcr420_cap_map;
}

const struct di_cta_hdmi_audio_block *
di_cta_data_block_get_hdmi_audio(const struct di_cta_data_block *block)
{
	if (block->tag != DI_CTA_DATA_BLOCK_HDMI_AUDIO) {
		return NULL;
	}
	return &block->hdmi_audio.base;
}

const struct di_cta_infoframe_block *
di_cta_data_block_get_infoframe(const struct di_cta_data_block *block)
{
	if (block->tag != DI_CTA_DATA_BLOCK_INFOFRAME) {
		return NULL;
	}
	return &block->infoframe.block;
}

const struct di_cta_speaker_locations *const *
di_cta_data_block_get_speaker_locations(const struct di_cta_data_block *block)
{
	if (block->tag != DI_CTA_DATA_BLOCK_SPEAKER_LOCATION) {
		return NULL;
	}
	return (const struct di_cta_speaker_locations *const *) block->speaker_location.locations;
}

const struct di_displayid_type_i_ii_vii_timing *
di_cta_data_block_get_did_type_vii_timing(const struct di_cta_data_block *block)
{
	if (block->tag != DI_CTA_DATA_BLOCK_DISPLAYID_VIDEO_TIMING_VII) {
		return NULL;
	}
	return &block->did_vii_timing;
}

const struct di_edid_detailed_timing_def *const *
di_edid_cta_get_detailed_timing_defs(const struct di_edid_cta *cta)
{
	return (const struct di_edid_detailed_timing_def *const *) cta->detailed_timing_defs;
}

const struct di_cta_vesa_transfer_characteristics *
di_cta_data_block_get_vesa_transfer_characteristics(const struct di_cta_data_block *block)
{
	if (block->tag != DI_CTA_DATA_BLOCK_VESA_DISPLAY_TRANSFER_CHARACTERISTIC) {
		return NULL;
	}
	return &block->vesa_transfer_characteristics;
}

const struct di_cta_room_configuration *
di_cta_data_block_get_room_configuration(const struct di_cta_data_block *block)
{
	if (block->tag != DI_CTA_DATA_BLOCK_ROOM_CONFIG) {
		return NULL;
	}
	return &block->room_config;
}
