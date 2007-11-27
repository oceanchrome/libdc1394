/*
 * 1394-Based Digital Camera Control Library
 * Copyright (C) 2000 SMART Technologies Inc.
 *
 * Written by Gord Peters <GordPeters@smarttech.com>
 * Additions by Chris Urmson <curmson@ri.cmu.edu>
 * Additions by Damien Douxchamps <ddouxchamps@users.sf.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __DC1394_CAMERA_CONTROL_H__
#define __DC1394_CAMERA_CONTROL_H__

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#if HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#if HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#include <stdint.h>

/*! \file control.h
    \brief Controls the library

    More details soon
*/

/* Note on the coding of libdc1394 versions:
   - LIBDC1394_VERSION represent the release number of the library,
     e.g. 2.0.0. It consists in 4 digits, 2 for each number in the version.
     Version 2.1 will thus appear as 0201. The last version number is not
     used because a ".1" increment (as opposed to ".0.1") is necessary if
     the API had significant changes justifying the use of this number.
   - LIBDC1394_VERSION_xxx are the version codes used by libtool and defined
     in configure.in. _MAJOR is for API changes (compat break), _REVISION is
     for backward compatible API changes (e.g. new functions) and _AGE is
     when the API does not change (only code changes). At least that's my
     wild guess.
   These numbers can be used to adapt the function calls to the library
   version installed on the machine. They appeared in 2.0 so that you
   may need to check their existance if your code must work with 1.x.
*/


#define LIBDC1394_VERSION            0200
#define LIBDC1394_VERSION_MAJOR        20
#define LIBDC1394_VERSION_REVISION      0
#define LIBDC1394_VERSION_AGE           0

/* Enumeration of iso data speeds */
typedef enum {
  DC1394_ISO_SPEED_100= 0,
  DC1394_ISO_SPEED_200,
  DC1394_ISO_SPEED_400,
  DC1394_ISO_SPEED_800,
  DC1394_ISO_SPEED_1600,
  DC1394_ISO_SPEED_3200
} dc1394speed_t;
#define DC1394_ISO_SPEED_MIN                   DC1394_ISO_SPEED_100
#define DC1394_ISO_SPEED_MAX                   DC1394_ISO_SPEED_3200
#define DC1394_ISO_SPEED_NUM                  (DC1394_ISO_SPEED_MAX - DC1394_ISO_SPEED_MIN + 1)

/* Enumeration of video framerates */
typedef enum {
  DC1394_FRAMERATE_1_875= 32,
  DC1394_FRAMERATE_3_75,
  DC1394_FRAMERATE_7_5,
  DC1394_FRAMERATE_15,
  DC1394_FRAMERATE_30,
  DC1394_FRAMERATE_60,
  DC1394_FRAMERATE_120,
  DC1394_FRAMERATE_240
} dc1394framerate_t;
#define DC1394_FRAMERATE_MIN               DC1394_FRAMERATE_1_875
#define DC1394_FRAMERATE_MAX               DC1394_FRAMERATE_240
#define DC1394_FRAMERATE_NUM              (DC1394_FRAMERATE_MAX - DC1394_FRAMERATE_MIN + 1)

/* Enumeration of video modes */
typedef enum {
  DC1394_VIDEO_MODE_160x120_YUV444= 64,
  DC1394_VIDEO_MODE_320x240_YUV422,
  DC1394_VIDEO_MODE_640x480_YUV411,
  DC1394_VIDEO_MODE_640x480_YUV422,
  DC1394_VIDEO_MODE_640x480_RGB8,
  DC1394_VIDEO_MODE_640x480_MONO8,
  DC1394_VIDEO_MODE_640x480_MONO16,
  DC1394_VIDEO_MODE_800x600_YUV422,
  DC1394_VIDEO_MODE_800x600_RGB8,
  DC1394_VIDEO_MODE_800x600_MONO8,
  DC1394_VIDEO_MODE_1024x768_YUV422,
  DC1394_VIDEO_MODE_1024x768_RGB8,
  DC1394_VIDEO_MODE_1024x768_MONO8,
  DC1394_VIDEO_MODE_800x600_MONO16,
  DC1394_VIDEO_MODE_1024x768_MONO16,
  DC1394_VIDEO_MODE_1280x960_YUV422,
  DC1394_VIDEO_MODE_1280x960_RGB8,
  DC1394_VIDEO_MODE_1280x960_MONO8,
  DC1394_VIDEO_MODE_1600x1200_YUV422,
  DC1394_VIDEO_MODE_1600x1200_RGB8,
  DC1394_VIDEO_MODE_1600x1200_MONO8,
  DC1394_VIDEO_MODE_1280x960_MONO16,
  DC1394_VIDEO_MODE_1600x1200_MONO16,
  DC1394_VIDEO_MODE_EXIF,
  DC1394_VIDEO_MODE_FORMAT7_0,
  DC1394_VIDEO_MODE_FORMAT7_1,
  DC1394_VIDEO_MODE_FORMAT7_2,
  DC1394_VIDEO_MODE_FORMAT7_3,
  DC1394_VIDEO_MODE_FORMAT7_4,
  DC1394_VIDEO_MODE_FORMAT7_5,
  DC1394_VIDEO_MODE_FORMAT7_6,
  DC1394_VIDEO_MODE_FORMAT7_7
} dc1394video_mode_t;
#define DC1394_VIDEO_MODE_MIN	    DC1394_VIDEO_MODE_160x120_YUV444
#define DC1394_VIDEO_MODE_MAX       DC1394_VIDEO_MODE_FORMAT7_7
#define DC1394_VIDEO_MODE_NUM      (DC1394_VIDEO_MODE_MAX - DC1394_VIDEO_MODE_MIN + 1)

/* Special min/max are defined for Format_7 */
#define DC1394_VIDEO_MODE_FORMAT7_MIN       DC1394_VIDEO_MODE_FORMAT7_0
#define DC1394_VIDEO_MODE_FORMAT7_MAX       DC1394_VIDEO_MODE_FORMAT7_7
#define DC1394_VIDEO_MODE_FORMAT7_NUM      (DC1394_VIDEO_MODE_FORMAT7_MAX - DC1394_VIDEO_MODE_FORMAT7_MIN + 1)

/* Enumeration of colour modings */
typedef enum {
  DC1394_COLOR_CODING_MONO8= 320,
  DC1394_COLOR_CODING_YUV411,
  DC1394_COLOR_CODING_YUV422,
  DC1394_COLOR_CODING_YUV444,
  DC1394_COLOR_CODING_RGB8,
  DC1394_COLOR_CODING_MONO16,
  DC1394_COLOR_CODING_RGB16,
  DC1394_COLOR_CODING_MONO16S,
  DC1394_COLOR_CODING_RGB16S,
  DC1394_COLOR_CODING_RAW8,
  DC1394_COLOR_CODING_RAW16
} dc1394color_coding_t;
#define DC1394_COLOR_CODING_MIN     DC1394_COLOR_CODING_MONO8
#define DC1394_COLOR_CODING_MAX     DC1394_COLOR_CODING_RAW16
#define DC1394_COLOR_CODING_NUM    (DC1394_COLOR_CODING_MAX - DC1394_COLOR_CODING_MIN + 1)

/* Enumeration of trigger modes */
typedef enum {
  DC1394_TRIGGER_MODE_0= 352,
  DC1394_TRIGGER_MODE_1,
  DC1394_TRIGGER_MODE_2,
  DC1394_TRIGGER_MODE_3,
  DC1394_TRIGGER_MODE_4,
  DC1394_TRIGGER_MODE_5,
  DC1394_TRIGGER_MODE_14,
  DC1394_TRIGGER_MODE_15
} dc1394trigger_mode_t;
#define DC1394_TRIGGER_MODE_MIN     DC1394_TRIGGER_MODE_0
#define DC1394_TRIGGER_MODE_MAX     DC1394_TRIGGER_MODE_15
#define DC1394_TRIGGER_MODE_NUM    (DC1394_TRIGGER_MODE_MAX - DC1394_TRIGGER_MODE_MIN + 1)

/* Enumeration of camera features */
typedef enum {
  DC1394_FEATURE_BRIGHTNESS= 416,
  DC1394_FEATURE_EXPOSURE,
  DC1394_FEATURE_SHARPNESS,
  DC1394_FEATURE_WHITE_BALANCE,
  DC1394_FEATURE_HUE,
  DC1394_FEATURE_SATURATION,
  DC1394_FEATURE_GAMMA,
  DC1394_FEATURE_SHUTTER,
  DC1394_FEATURE_GAIN,
  DC1394_FEATURE_IRIS,
  DC1394_FEATURE_FOCUS,
  DC1394_FEATURE_TEMPERATURE,
  DC1394_FEATURE_TRIGGER,
  DC1394_FEATURE_TRIGGER_DELAY,
  DC1394_FEATURE_WHITE_SHADING,
  DC1394_FEATURE_FRAME_RATE,
  DC1394_FEATURE_ZOOM,
  DC1394_FEATURE_PAN,
  DC1394_FEATURE_TILT,
  DC1394_FEATURE_OPTICAL_FILTER,
  DC1394_FEATURE_CAPTURE_SIZE,
  DC1394_FEATURE_CAPTURE_QUALITY
} dc1394feature_t;
#define DC1394_FEATURE_MIN           DC1394_FEATURE_BRIGHTNESS
#define DC1394_FEATURE_MAX           DC1394_FEATURE_CAPTURE_QUALITY
#define DC1394_FEATURE_NUM          (DC1394_FEATURE_MAX - DC1394_FEATURE_MIN + 1)

/* Operation modes */
typedef enum {
  DC1394_OPERATION_MODE_LEGACY = 480,
  DC1394_OPERATION_MODE_1394B
} dc1394operation_mode_t;

/* Format 7 sensor layouts*/
typedef enum {
  DC1394_COLOR_FILTER_RGGB = 512,
  DC1394_COLOR_FILTER_GBRG,
  DC1394_COLOR_FILTER_GRBG,
  DC1394_COLOR_FILTER_BGGR
} dc1394color_filter_t;
#define DC1394_COLOR_FILTER_MIN        DC1394_COLOR_FILTER_RGGB
#define DC1394_COLOR_FILTER_MAX        DC1394_COLOR_FILTER_BGGR
#define DC1394_COLOR_FILTER_NUM       (DC1394_COLOR_FILTER_MAX - DC1394_COLOR_FILTER_MIN + 1)

/* IIDC versions*/
typedef enum {
  DC1394_IIDC_VERSION_1_04 = 544,
  DC1394_IIDC_VERSION_1_20,
  DC1394_IIDC_VERSION_PTGREY,
  DC1394_IIDC_VERSION_1_30,
  DC1394_IIDC_VERSION_1_31,
  DC1394_IIDC_VERSION_1_32,
  DC1394_IIDC_VERSION_1_33,
  DC1394_IIDC_VERSION_1_34,
  DC1394_IIDC_VERSION_1_35,
  DC1394_IIDC_VERSION_1_36,
  DC1394_IIDC_VERSION_1_37,
  DC1394_IIDC_VERSION_1_38,
  DC1394_IIDC_VERSION_1_39
} dc1394iidc_version_t;
#define DC1394_IIDC_VERSION_MIN        DC1394_IIDC_VERSION_1_04
#define DC1394_IIDC_VERSION_MAX        DC1394_IIDC_VERSION_1_39
#define DC1394_IIDC_VERSION_NUM       (DC1394_IIDC_VERSION_MAX - DC1394_IIDC_VERSION_MIN + 1)

/* Enumeration of trigger sources */
typedef enum {
  DC1394_TRIGGER_SOURCE_0= 576,
  DC1394_TRIGGER_SOURCE_1,
  DC1394_TRIGGER_SOURCE_2,
  DC1394_TRIGGER_SOURCE_3,
  DC1394_TRIGGER_SOURCE_SOFTWARE
} dc1394trigger_source_t;
#define DC1394_TRIGGER_SOURCE_MIN      DC1394_TRIGGER_SOURCE_0
#define DC1394_TRIGGER_SOURCE_MAX      DC1394_TRIGGER_SOURCE_SOFTWARE
#define DC1394_TRIGGER_SOURCE_NUM     (DC1394_TRIGGER_SOURCE_MAX - DC1394_TRIGGER_SOURCE_MIN + 1)

/* Enumeration of power classes */
typedef enum {
  DC1394_POWER_CLASS_NONE=608,
  DC1394_POWER_CLASS_PROV_MIN_15W,
  DC1394_POWER_CLASS_PROV_MIN_30W,
  DC1394_POWER_CLASS_PROV_MIN_45W,
  DC1394_POWER_CLASS_USES_MAX_1W,
  DC1394_POWER_CLASS_USES_MAX_3W,
  DC1394_POWER_CLASS_USES_MAX_6W,
  DC1394_POWER_CLASS_USES_MAX_10W
} dc1394power_class_t;
#define DC1394_POWER_CLASS_MIN         DC1394_POWER_CLASS_NONE
#define DC1394_POWER_CLASS_MAX         DC1394_POWER_CLASS_USES_MAX_10W
#define DC1394_POWER_CLASS_NUM        (DC1394_POWER_CLASS_MAX - DC1394_POWER_CLASS_MIN + 1)

/* Enumeration of PHY delays */
typedef enum {
  DC1394_PHY_DELAY_MAX_144_NS=640,
  DC1394_PHY_DELAY_UNKNOWN_0,
  DC1394_PHY_DELAY_UNKNOWN_1,
  DC1394_PHY_DELAY_UNKNOWN_2
} dc1394phy_delay_t;
#define DC1394_PHY_DELAY_MIN           DC1394_PHY_DELAY_MAX_144_NS
#define DC1394_PHY_DELAY_MAX           DC1394_PHY_DELAY_UNKNOWN_0
#define DC1394_PHY_DELAY_NUM          (DC1394_PHY_DELAY_MAX - DC1394_PHY_DELAY_MIN + 1)

/* Maximum number of characters in vendor and model strings */
#define MAX_CHARS                      256

/* Return values for visible functions*/
typedef enum {
  DC1394_SUCCESS = 0,   /* Success is zero */
  DC1394_FAILURE,       /* Errors are positive numbers */
  DC1394_NO_FRAME = -2, /* Warnings or info are negative numbers */
  DC1394_NO_CAMERA = 3,
  DC1394_NOT_A_CAMERA,
  DC1394_FUNCTION_NOT_SUPPORTED,
  DC1394_CAMERA_NOT_INITIALIZED,
  DC1394_INVALID_FEATURE,
  DC1394_INVALID_VIDEO_FORMAT,
  DC1394_INVALID_VIDEO_MODE,
  DC1394_INVALID_FRAMERATE,
  DC1394_INVALID_TRIGGER_MODE,
  DC1394_INVALID_TRIGGER_SOURCE,
  DC1394_INVALID_ISO_SPEED,
  DC1394_INVALID_IIDC_VERSION,
  DC1394_INVALID_COLOR_CODING,
  DC1394_INVALID_COLOR_FILTER,
  DC1394_INVALID_CAPTURE_MODE,
  DC1394_REQ_VALUE_OUTSIDE_RANGE,
  DC1394_INVALID_ERROR_CODE,
  DC1394_MEMORY_ALLOCATION_FAILURE,
  DC1394_TAGGED_REGISTER_NOT_FOUND,
  DC1394_FORMAT7_ERROR_FLAG_1,
  DC1394_FORMAT7_ERROR_FLAG_2,
  DC1394_INVALID_BAYER_METHOD,
  DC1394_INVALID_ARGUMENT_VALUE,
  DC1394_INVALID_VIDEO1394_DEVICE,
  DC1394_NO_ISO_CHANNEL,
  DC1394_NO_BANDWIDTH,
  DC1394_IOCTL_FAILURE,
  DC1394_CAPTURE_IS_NOT_SET = -30,
  DC1394_CAPTURE_IS_RUNNING = 31,
  DC1394_RAW1394_FAILURE,
  DC1394_BASLER_NO_MORE_SFF_CHUNKS,
  DC1394_BASLER_CORRUPTED_SFF_CHUNK,
  DC1394_BASLER_UNKNOWN_SFF_CHUNK
} dc1394error_t;
#define DC1394_ERROR_NUM DC1394_BASLER_UNKNOWN_SFF_CHUNK+1

/* Parameter flags for dc1394_setup_format7_capture() */
#define DC1394_QUERY_FROM_CAMERA -1
#define DC1394_USE_MAX_AVAIL     -2
#define DC1394_USE_RECOMMENDED   -3

/* The video1394 policy: blocking (wait for a frame forever)
   or polling (returns if no frames in buffer) */
typedef enum { 
  DC1394_CAPTURE_POLICY_WAIT=0,
  DC1394_CAPTURE_POLICY_POLL
} dc1394capture_policy_t;

/* Yet another boolean data type */
typedef enum {
  DC1394_FALSE= 0,
  DC1394_TRUE
} dc1394bool_t;

/* Yet another boolean data type */
typedef enum {
  DC1394_OFF= 0,
  DC1394_ON
} dc1394switch_t;

typedef enum {
  DC1394_TRIGGER_ACTIVE_LOW= 0,
  DC1394_TRIGGER_ACTIVE_HIGH
} dc1394trigger_polarity_t;

typedef enum {
  DC1394_FEATURE_MODE_MANUAL= 0,
  DC1394_FEATURE_MODE_AUTO,
  DC1394_FEATURE_MODE_ONE_PUSH_AUTO
} dc1394feature_mode_t;


/* Capture flags */
#define DC1394_CAPTURE_FLAGS_CHANNEL_ALLOC   0x00000001U
#define DC1394_CAPTURE_FLAGS_BANDWIDTH_ALLOC 0x00000002U
/* a reasonable default value: do alloc of bandwidth and channel */
#define DC1394_CAPTURE_FLAGS_DEFAULT         0x00000004U

/* Camera capture structure. Do not access directly from your application.
   Use dc1394_video_get_buffer and dc1394_video_get_fill_time instead. */

typedef struct
{
  uint32_t                num;
  dc1394color_coding_t    codings[DC1394_COLOR_CODING_NUM];
} dc1394color_codings_t;

typedef struct
{
  uint32_t                num;
  dc1394video_mode_t      modes[DC1394_VIDEO_MODE_NUM];
} dc1394video_modes_t;

typedef struct
{
  uint32_t                num;
  dc1394framerate_t       framerates[DC1394_FRAMERATE_NUM];
} dc1394framerates_t;

typedef struct
{
  uint32_t                num;
  dc1394trigger_mode_t    modes[DC1394_TRIGGER_MODE_NUM];
} dc1394trigger_modes_t;

typedef struct
{
  uint32_t                  num;
  dc1394trigger_source_t    sources[DC1394_TRIGGER_SOURCE_NUM];
} dc1394trigger_sources_t;

typedef struct
{
  uint16_t             unit;
  uint64_t             guid;
} dc1394camera_id_t;

typedef struct __dc1394camera_list_t
{
  uint32_t      num;
  dc1394camera_id_t    *ids;
} dc1394camera_list_t;

typedef struct __dc1394_t dc1394_t;

/* Camera structure */
typedef struct __dc1394_camera
{
  /* system/firmware information */
  uint64_t             guid;
  int                  unit;
  uint32_t             unit_spec_ID;
  uint32_t             unit_sw_version;
  uint32_t             unit_sub_sw_version;
  uint32_t             command_registers_base;
  uint32_t             unit_directory;
  uint32_t             unit_dependent_directory;
  uint64_t             advanced_features_csr;
  uint64_t             PIO_control_csr;
  uint64_t             SIO_control_csr;
  uint64_t             strobe_control_csr;
  uint64_t             format7_csr[DC1394_VIDEO_MODE_FORMAT7_NUM];
  dc1394iidc_version_t iidc_version;
  char               * vendor;
  char               * model;
  uint32_t             vendor_id;
  uint32_t             model_id;
  dc1394bool_t         bmode_capable;
  dc1394bool_t         one_shot_capable;
  dc1394bool_t         multi_shot_capable;
  dc1394bool_t         can_switch_on_off;
  dc1394bool_t         has_vmode_error_status;
  dc1394bool_t         has_feature_error_status;
  int                  max_mem_channel;

  /* some current values */
  dc1394video_mode_t   video_mode;
  dc1394framerate_t    framerate;
  dc1394switch_t       is_iso_on;
  int                  iso_channel; /* this variable contains the iso channel requests or the current iso channel */
  int                  iso_channel_is_set; /* >0 if the iso_channel above has been allocated within libraw1394 */
  uint32_t             iso_bandwidth;
  dc1394speed_t        iso_speed;
  int                  capture_is_set; /* 0 for not set, 1 for RAW1394 and 2 for DMA */

  /* for broadcast: */
  dc1394bool_t         broadcast;
  uint16_t             node_id_backup;

  /* 1394 PHY interface data: */
  dc1394speed_t        phy_speed;
  dc1394power_class_t  power_class;
  dc1394phy_delay_t    phy_delay;
  
  /* for Basler SFF */
  dc1394bool_t         sff_has_extended_data_stream;

  /* not used, for future use: */
  uint32_t             flags;

} dc1394camera_t;

typedef struct __dc1394feature_info_t_struct 
{
  dc1394feature_t    id;
  dc1394bool_t       available;
  dc1394bool_t       one_push_capable;
  dc1394bool_t       absolute_capable;
  dc1394bool_t       readout_capable;
  dc1394bool_t       on_off_capable;
  dc1394bool_t       auto_capable;
  dc1394bool_t       manual_capable;
  dc1394bool_t       polarity_capable;
  dc1394switch_t     one_push_active;
  dc1394switch_t     is_on;
  dc1394bool_t       auto_active;
  dc1394trigger_modes_t    trigger_modes;
  dc1394trigger_mode_t     trigger_mode;
  dc1394trigger_polarity_t trigger_polarity;
  dc1394trigger_sources_t  trigger_sources;
  dc1394trigger_source_t   trigger_source;
  uint32_t           min;
  uint32_t           max;
  uint32_t           value;
  uint32_t           BU_value;
  uint32_t           RV_value;
  uint32_t           B_value;
  uint32_t           R_value;
  uint32_t           G_value;
  uint32_t           target_value;
  
  dc1394switch_t     abs_control;
  float              abs_value;
  float              abs_max;
  float              abs_min;
  
} dc1394feature_info_t;

typedef struct __dc1394featureset_t
{
  dc1394feature_info_t    feature[DC1394_FEATURE_NUM];
} dc1394featureset_t;

typedef struct __dc1394format7mode_t
{
  dc1394bool_t present;

  uint32_t size_x;
  uint32_t size_y;
  uint32_t max_size_x;
  uint32_t max_size_y;

  uint32_t pos_x;
  uint32_t pos_y;

  uint32_t unit_size_x;
  uint32_t unit_size_y;
  uint32_t unit_pos_x;
  uint32_t unit_pos_y;

  dc1394color_codings_t color_codings;
  dc1394color_coding_t color_coding;

  uint32_t pixnum;

  uint32_t bpp; /* bpp is byte_per_packet, not bit per pixel. */
  uint32_t min_bpp;
  uint32_t max_bpp;

  uint64_t total_bytes;

  dc1394color_filter_t color_filter;

} dc1394format7mode_t;

typedef struct __dc1394format7modeset_t
{
  dc1394format7mode_t mode[DC1394_VIDEO_MODE_FORMAT7_NUM];
} dc1394format7modeset_t;

/* video frame structure */
typedef struct __dc1394_video_frame
{
  unsigned char          * image;                 /* the image. May contain padding data too (vendor specific) */
  uint32_t                 size[2];               /* the image size [width, height] */
  uint32_t                 position[2];           /* the WOI/ROI position [horizontal, vertical] == [0,0] for full frame */
  dc1394color_coding_t     color_coding;          /* the color coding used. This field is valid for all video modes. */
  dc1394color_filter_t     color_filter;          /* the color filter used. This field is valid only for RAW modes and IIDC 1.31 */
  uint32_t                 yuv_byte_order;        /* the order of the fields for 422 formats: YUYV or UYVY */
  uint32_t                 bit_depth;             /* the number of bits per pixel. The number of grayscale levels is 2^(this_number) */
  uint32_t                 stride;                /* the number of bytes per image line */
  dc1394video_mode_t       video_mode;            /* the video mode used for capturing this frame */
  uint64_t                 total_bytes;           /* the total size of the frame buffer in bytes. May include packet-
                                                   * multiple padding and intentional padding (vendor specific) */
  uint32_t                 image_bytes;           /* the number of bytes used for the image (image data only, no padding) */
  uint32_t                 padding_bytes;         /* the number of extra bytes, i.e. total_bytes-image_bytes.  */
  uint32_t                 bytes_per_packet;      /* the number of bytes per packet. (IIDC data) */
  uint32_t                 packets_per_frame;     /* the number of packets per frame. (IIDC data) */
  uint64_t                 timestamp;             /* the unix time [microseconds] at which the frame was captured in
					 	   * the video1394 ringbuffer */
  uint32_t                 frames_behind;         /* the number of frames in the ring buffer that are yet to be accessed by the user */
  dc1394camera_t           *camera;               /* the parent camera of this frame */
  uint32_t                 id;                    /* the frame position in the ring buffer */
  uint64_t                 allocated_image_bytes; /* amount of memory allocated in for the *image field. -1 for output
						   * of libdc1394? (this would avoid confusion between 'no allocated
						   * memory' and 'don't touch this buffer' -> signed int?? */ 
} dc1394video_frame_t;

enum {
  DC1394_BYTE_ORDER_UYVY=0,
  DC1394_BYTE_ORDER_YUYV
};

/* Feature descriptions */
extern const char *dc1394_feature_desc[DC1394_FEATURE_NUM];
extern const char *dc1394_error_strings[DC1394_ERROR_NUM];

#if ! defined (_MSC_VER)
/* Error checking function. displays an error string on stderr and exit current function
   if error is positive. Neg errors are messages and are thus ignored */

#define DC1394_WRN(err,...)                                  \
    {                                                        \
    if ((err<0)||(err>DC1394_ERROR_NUM))                     \
      err=DC1394_INVALID_ERROR_CODE;                         \
                                                             \
    if (err>DC1394_SUCCESS) {                                \
      fprintf(stderr,"Libdc1394 warning (%s:%s:%d): %s : ",  \
	      __FILE__, __FUNCTION__, __LINE__,              \
	      dc1394_error_strings[err]);                    \
      fprintf(stderr, __VA_ARGS__);                          \
      fprintf(stderr,"\n");                                  \
    }                                                        \
    }

#define DC1394_ERR(err,...)                                  \
    {                                                        \
    if ((err<0)||(err>DC1394_ERROR_NUM))                     \
      err=DC1394_INVALID_ERROR_CODE;                         \
                                                             \
    if (err>DC1394_SUCCESS) {                                \
      fprintf(stderr,"Libdc1394 error (%s:%s:%d): %s : ",    \
	      __FILE__, __FUNCTION__, __LINE__,              \
	      dc1394_error_strings[err]);                    \
      fprintf(stderr, __VA_ARGS__);                          \
      fprintf(stderr,"\n");                                  \
      return;                                                \
    }                                                        \
    }

#define DC1394_ERR_RTN(err,...)                              \
    {                                                        \
    if ((err<0)||(err>DC1394_ERROR_NUM))                     \
      err=DC1394_INVALID_ERROR_CODE;                         \
                                                             \
    if (err>DC1394_SUCCESS) {                                \
      fprintf(stderr,"Libdc1394 error (%s:%s:%d): %s : ",    \
	      __FILE__, __FUNCTION__, __LINE__,              \
	      dc1394_error_strings[err]);                    \
      fprintf(stderr, __VA_ARGS__);                          \
      fprintf(stderr,"\n");                                  \
      return err;                                            \
    }                                                        \
    }

#define DC1394_ERR_CLN(err, cleanup,...)                             \
    {                                                                \
    if ((err<0)||(err>DC1394_ERROR_NUM))                             \
      err=DC1394_INVALID_ERROR_CODE;                                 \
                                                                     \
    if (err>DC1394_SUCCESS) {                                        \
      fprintf(stderr,"Libdc1394 error (%s:%s:%d): %s : ",            \
	      __FILE__, __FUNCTION__, __LINE__,                      \
	      dc1394_error_strings[err]);                            \
      fprintf(stderr, __VA_ARGS__);                                  \
      fprintf(stderr,"\n");                                          \
      cleanup;                                                       \
      return;                                                        \
    }                                                                \
    }

#define DC1394_ERR_CLN_RTN(err, cleanup,...)                         \
    {                                                                \
    if ((err<0)||(err>DC1394_ERROR_NUM))                             \
      err=DC1394_INVALID_ERROR_CODE;                                 \
                                                                     \
    if (err>DC1394_SUCCESS) {                                        \
      fprintf(stderr,"Libdc1394 error (%s:%s:%d): %s : ",            \
	      __FILE__, __FUNCTION__, __LINE__,                      \
	      dc1394_error_strings[err]);                            \
      fprintf(stderr, __VA_ARGS__);                                  \
      fprintf(stderr,"\n");                                          \
      cleanup;                                                       \
      return err;                                                    \
    }                                                                \
    }

#endif /* _MSC_VER */

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
     Camera detection functions
 ***************************************************************************/

dc1394_t* dc1394_new (void);
void dc1394_free (dc1394_t *dc1394);
dc1394error_t dc1394_enumerate_cameras(dc1394_t *dc1394, dc1394camera_list_t **list);
void dc1394_free_camera_list(dc1394camera_list_t *list);
dc1394camera_t * dc1394_camera_new(dc1394_t *dc1394, uint64_t guid);
dc1394camera_t * dc1394_camera_new_unit(dc1394_t *dc1394, uint64_t guid, int unit);
void dc1394_camera_free(dc1394camera_t *camera);


/***************************************************************************
     General system functions
 ***************************************************************************/

/* print the camera information */
dc1394error_t dc1394_print_camera_info(dc1394camera_t *camera);

/* Sets and resets the broadcast flag of a camera. If the broadcast flag is set,
   all devices on the bus will execute the command. Useful to sync ISO start
   commands or setting a bunch of cameras at the same time. Broadcast only works
   with identical devices (brand/model). If the devices are not identical your
   mileage may vary. Some cameras may not answer broadcast commands at all. Also,
   this only works with cameras on the SAME bus (IOW, the same port).*/
dc1394error_t dc1394_camera_set_broadcast(dc1394camera_t *camera, dc1394bool_t pwr);

/* Resets the IEEE1394 bus which camera is attached to.  Calling this function is
   "rude" to other devices because it causes them to re-enumerate on the bus and
   may cause a temporary disruption in their current activities.  Thus, use it
   sparingly.  Its primary use is if a program shuts down uncleanly and needs to
   free leftover ISO channels or bandwidth.  A bus reset will free those things
   as a side effect. */
dc1394error_t dc1394_reset_bus(dc1394camera_t *camera);

dc1394error_t dc1394_read_cycle_timer (dc1394camera_t * camera,
        uint32_t * cycle_timer, uint64_t * local_time);

/***************************************************************************
     Other functionalities
 ***************************************************************************/

/* reset camera to factory default settings */
dc1394error_t dc1394_reset_camera(dc1394camera_t *camera);

/* turn camera on or off */
dc1394error_t dc1394_set_camera_power(dc1394camera_t *camera, dc1394switch_t pwr);

/* functions to read and write camera setups in memory channels */
dc1394error_t dc1394_memory_is_save_in_operation(dc1394camera_t *camera, dc1394bool_t *value);
dc1394error_t dc1394_memory_save(dc1394camera_t *camera, uint32_t channel);
dc1394error_t dc1394_memory_load(dc1394camera_t *camera, uint32_t channel);

/***************************************************************************
     Trigger
 ***************************************************************************/

/* external trigger feature functions */
/* change this name to EXTERNAL TRIGGER */
dc1394error_t dc1394_external_trigger_set_polarity(dc1394camera_t *camera, dc1394trigger_polarity_t polarity);
dc1394error_t dc1394_external_trigger_get_polarity(dc1394camera_t *camera, dc1394trigger_polarity_t *polarity);
dc1394error_t dc1394_external_trigger_has_polarity(dc1394camera_t *camera, dc1394bool_t *polarity_capable);
dc1394error_t dc1394_external_trigger_set_power(dc1394camera_t *camera, dc1394switch_t pwr);
dc1394error_t dc1394_external_trigger_get_power(dc1394camera_t *camera, dc1394switch_t *pwr);
dc1394error_t dc1394_external_trigger_set_mode(dc1394camera_t *camera, dc1394trigger_mode_t mode);
dc1394error_t dc1394_external_trigger_get_mode(dc1394camera_t *camera, dc1394trigger_mode_t *mode);
dc1394error_t dc1394_external_trigger_set_source(dc1394camera_t *camera, dc1394trigger_source_t source);
dc1394error_t dc1394_external_trigger_get_source(dc1394camera_t *camera, dc1394trigger_source_t *source);
dc1394error_t dc1394_external_trigger_get_supported_sources(dc1394camera_t *camera, dc1394trigger_sources_t *sources);

/* Turn one software trigger on or off and get state */
dc1394error_t dc1394_software_trigger_set_power(dc1394camera_t *camera, dc1394switch_t pwr);
dc1394error_t dc1394_software_trigger_get_power(dc1394camera_t *camera, dc1394switch_t *pwr);

/***************************************************************************
     Features
 ***************************************************************************/

/* Collects the available features for the camera described by node and stores them in features. */
dc1394error_t dc1394_get_camera_feature_set(dc1394camera_t *camera, dc1394featureset_t *features);

/* Stores the bounds and options associated with the feature described by feature->feature_id */
dc1394error_t dc1394_get_camera_feature(dc1394camera_t *camera, dc1394feature_info_t *feature);

/* Displays the bounds and options of the given feature or of the entire feature set */
dc1394error_t dc1394_print_feature(dc1394feature_info_t *feature);
dc1394error_t dc1394_print_feature_set(dc1394featureset_t *features);

/* get/set the values of multiple-value features on the camera */
dc1394error_t dc1394_feature_whitebalance_get_value(dc1394camera_t *camera, uint32_t *u_b_value, uint32_t *v_r_value);
dc1394error_t dc1394_feature_whitebalance_set_value(dc1394camera_t *camera, uint32_t u_b_value, uint32_t v_r_value);
dc1394error_t dc1394_feature_temperature_get_value(dc1394camera_t *camera, uint32_t *target_temperature, uint32_t *temperature);
dc1394error_t dc1394_feature_temperature_set_value(dc1394camera_t *camera, uint32_t target_temperature);
dc1394error_t dc1394_feature_whiteshading_get_value(dc1394camera_t *camera, uint32_t *r_value, uint32_t *g_value, uint32_t *b_value);
dc1394error_t dc1394_feature_whiteshading_set_value(dc1394camera_t *camera, uint32_t r_value, uint32_t g_value, uint32_t b_value);

/* get/set the values of single-value features on the camera */
dc1394error_t dc1394_feature_get_value(dc1394camera_t *camera, dc1394feature_t feature, uint32_t *value);
dc1394error_t dc1394_feature_set_value(dc1394camera_t *camera, dc1394feature_t feature, uint32_t value);

/* query/set specific feature characteristics */
dc1394error_t dc1394_feature_is_present(dc1394camera_t *camera, dc1394feature_t feature, dc1394bool_t *value);
dc1394error_t dc1394_feature_is_readable(dc1394camera_t *camera, dc1394feature_t feature, dc1394bool_t *value);
dc1394error_t dc1394_feature_get_boundaries(dc1394camera_t *camera, dc1394feature_t feature, uint32_t *min, uint32_t *max);
dc1394error_t dc1394_feature_is_switchable(dc1394camera_t *camera, dc1394feature_t feature, dc1394bool_t *value);
dc1394error_t dc1394_feature_get_power(dc1394camera_t *camera, dc1394feature_t feature, dc1394switch_t *pwr);
dc1394error_t dc1394_feature_set_power(dc1394camera_t *camera, dc1394feature_t feature, dc1394switch_t pwr);
dc1394error_t dc1394_feature_has_auto_mode(dc1394camera_t *camera,  dc1394feature_t feature, dc1394bool_t *value);
dc1394error_t dc1394_feature_has_manual_mode(dc1394camera_t *camera, dc1394feature_t feature, dc1394bool_t *value);
dc1394error_t dc1394_feature_has_one_push_auto(dc1394camera_t *camera, dc1394feature_t feature, dc1394bool_t *value);
dc1394error_t dc1394_feature_get_mode(dc1394camera_t *camera, dc1394feature_t feature, dc1394feature_mode_t *mode);
dc1394error_t dc1394_feature_set_mode(dc1394camera_t *camera, dc1394feature_t feature, dc1394feature_mode_t mode);

/* absolute settings */
dc1394error_t dc1394_feature_has_absolute_control(dc1394camera_t *camera, dc1394feature_t feature, dc1394bool_t *value);
dc1394error_t dc1394_feature_get_absolute_boundaries(dc1394camera_t *camera, dc1394feature_t feature, float *min, float *max);
dc1394error_t dc1394_feature_get_absolute_value(dc1394camera_t *camera, dc1394feature_t feature, float *value);
dc1394error_t dc1394_feature_set_absolute_value(dc1394camera_t *camera, dc1394feature_t feature, float value);
dc1394error_t dc1394_feature_get_absolute_control(dc1394camera_t *camera, dc1394feature_t feature, dc1394switch_t *pwr);
dc1394error_t dc1394_feature_set_absolute_control(dc1394camera_t *camera, dc1394feature_t feature, dc1394switch_t pwr);

/***************************************************************************
     Video functions: formats, framerates,...
 ***************************************************************************/

/* functions for querying camera attributes */
dc1394error_t dc1394_video_get_supported_modes(dc1394camera_t *camera, dc1394video_modes_t *video_modes);
dc1394error_t dc1394_video_get_supported_framerates(dc1394camera_t *camera, dc1394video_mode_t video_mode, dc1394framerates_t *framerates);

/* get/set the framerate, mode, format, iso speed for the video */
dc1394error_t dc1394_video_get_framerate(dc1394camera_t *camera, dc1394framerate_t *framerate);
dc1394error_t dc1394_video_set_framerate(dc1394camera_t *camera, dc1394framerate_t framerate);
dc1394error_t dc1394_video_get_mode(dc1394camera_t *camera, dc1394video_mode_t *video_mode);
dc1394error_t dc1394_video_set_mode(dc1394camera_t *camera, dc1394video_mode_t video_mode);
dc1394error_t dc1394_video_get_operation_mode(dc1394camera_t *camera, dc1394operation_mode_t *mode);
dc1394error_t dc1394_video_set_operation_mode(dc1394camera_t *camera, dc1394operation_mode_t mode);
dc1394error_t dc1394_video_get_iso_speed(dc1394camera_t *camera, dc1394speed_t *speed);
dc1394error_t dc1394_video_set_iso_speed(dc1394camera_t *camera, dc1394speed_t speed);
dc1394error_t dc1394_video_get_iso_channel(dc1394camera_t *camera, uint32_t * channel);
dc1394error_t dc1394_video_set_iso_channel(dc1394camera_t *camera, uint32_t channel);
/* Both "DATA DEPTH" and "BIT DEPTH" are used. This should be fixed. */
dc1394error_t dc1394_video_get_data_depth(dc1394camera_t *camera, uint32_t *depth);
 
/* start/stop isochronous data transmission */
dc1394error_t dc1394_video_set_transmission(dc1394camera_t *camera, dc1394switch_t pwr);
dc1394error_t dc1394_video_get_transmission(dc1394camera_t *camera, dc1394switch_t *pwr);
/* The following function is not necessary in general. You should only use it if you
   want a specific ISO channel. Usage: Call it before setting up capture and transmission */
dc1394error_t dc1394_video_specify_iso_channel(dc1394camera_t *camera, int iso_channel);

/* turn one shot mode on or off */
dc1394error_t dc1394_video_set_one_shot(dc1394camera_t *camera, dc1394switch_t pwr);
dc1394error_t dc1394_video_get_one_shot(dc1394camera_t *camera, dc1394bool_t *is_on);

/* turn multishot mode on or off */
dc1394error_t dc1394_video_set_multi_shot(dc1394camera_t *camera, uint32_t numFrames, dc1394switch_t pwr);
dc1394error_t dc1394_video_get_multi_shot(dc1394camera_t *camera, dc1394bool_t *is_on, uint32_t *numFrames);

/* Utility function */
dc1394error_t dc1394_video_get_bandwidth_usage(dc1394camera_t *camera, uint32_t *bandwidth);

/***************************************************************************
     Capture Functions
 ***************************************************************************/

/* setup/stop the capture */
dc1394error_t dc1394_capture_setup(dc1394camera_t *camera, uint32_t num_dma_buffers, uint32_t flags);
dc1394error_t dc1394_capture_stop(dc1394camera_t *camera);

/* Get a file descriptor to be used for select().  Must be called after
 * capture_setup(). */
int dc1394_capture_get_fileno (dc1394camera_t * camera);

/* capture video frames */
dc1394error_t dc1394_capture_dequeue(dc1394camera_t * camera, dc1394capture_policy_t policy, dc1394video_frame_t **frame);
dc1394error_t dc1394_capture_enqueue(dc1394camera_t * camera, dc1394video_frame_t * frame);

/* Set the DMA device filename manually. In most cases this is not necessary because the capture
   functions probe common filenames such as /dev/video1394/x or /dev/video1394. */
dc1394error_t dc1394_capture_set_device_filename(dc1394camera_t* camera, char *filename);

/***************************************************************************
     Format_7 (scalable image format)
 ***************************************************************************/

/* image size */
dc1394error_t dc1394_format7_get_max_image_size(dc1394camera_t *camera, dc1394video_mode_t video_mode, uint32_t *h_size,uint32_t *v_size);
dc1394error_t dc1394_format7_get_unit_size(dc1394camera_t *camera, dc1394video_mode_t video_mode, uint32_t *h_unit, uint32_t *v_unit);
dc1394error_t dc1394_format7_get_image_size(dc1394camera_t *camera, dc1394video_mode_t video_mode, uint32_t *width, uint32_t *height);
dc1394error_t dc1394_format7_set_image_size(dc1394camera_t *camera, dc1394video_mode_t video_mode, uint32_t width, uint32_t height);

/* image position */
dc1394error_t dc1394_format7_get_image_position(dc1394camera_t *camera, dc1394video_mode_t video_mode, uint32_t *left, uint32_t *top);
dc1394error_t dc1394_format7_set_image_position(dc1394camera_t *camera, dc1394video_mode_t video_mode, uint32_t left, uint32_t top);
dc1394error_t dc1394_format7_get_unit_position(dc1394camera_t *camera, dc1394video_mode_t video_mode, uint32_t *h_unit_pos, uint32_t *v_unit_pos);

/* color coding */
dc1394error_t dc1394_format7_get_color_coding(dc1394camera_t *camera, dc1394video_mode_t video_mode, dc1394color_coding_t *color_coding);
dc1394error_t dc1394_format7_get_color_codings(dc1394camera_t *camera, dc1394video_mode_t video_mode, dc1394color_codings_t *codings);
dc1394error_t dc1394_format7_set_color_coding(dc1394camera_t *camera, dc1394video_mode_t video_mode, dc1394color_coding_t color_coding);
dc1394error_t dc1394_format7_get_color_filter(dc1394camera_t *camera, dc1394video_mode_t video_mode, dc1394color_filter_t *color_filter);

/* packet */
dc1394error_t dc1394_format7_get_packet_para(dc1394camera_t *camera, dc1394video_mode_t video_mode, uint32_t *unit_bytes, uint32_t *max_bytes);
dc1394error_t dc1394_format7_get_byte_per_packet(dc1394camera_t *camera, dc1394video_mode_t video_mode, uint32_t *packet_bytes);
dc1394error_t dc1394_format7_set_byte_per_packet(dc1394camera_t *camera, dc1394video_mode_t video_mode, uint32_t packet_bytes);
dc1394error_t dc1394_format7_get_recommended_byte_per_packet(dc1394camera_t *camera, dc1394video_mode_t video_mode, uint32_t *bpp);
dc1394error_t dc1394_format7_get_packet_per_frame(dc1394camera_t *camera, dc1394video_mode_t video_mode, uint32_t *ppf);

/* other */
dc1394error_t dc1394_format7_get_data_depth(dc1394camera_t *camera, dc1394video_mode_t video_mode, uint32_t *data_depth);
dc1394error_t dc1394_format7_get_frame_interval(dc1394camera_t *camera, dc1394video_mode_t video_mode, float *interval);
dc1394error_t dc1394_format7_get_pixel_number(dc1394camera_t *camera, dc1394video_mode_t video_mode, uint32_t *pixnum);
dc1394error_t dc1394_format7_get_total_bytes(dc1394camera_t *camera, dc1394video_mode_t video_mode, uint64_t *total_bytes);

/* These functions get the properties of (one or all) format7 mode(s) */
dc1394error_t dc1394_format7_get_modeset(dc1394camera_t *camera, dc1394format7modeset_t *info);
dc1394error_t dc1394_format7_get_mode_info(dc1394camera_t *camera, dc1394video_mode_t video_mode, dc1394format7mode_t *f7_mode);

/* Joint function that fully sets a certain ROI taking all parameters into account
   Note that this function does not SWITCH to the video mode passed as argument, it mearly sets it */
dc1394error_t dc1394_format7_set_roi(dc1394camera_t *camera, dc1394video_mode_t video_mode, dc1394color_coding_t color_coding,
				     int bytes_per_packet, int left, int top, int width, int height);

dc1394error_t dc1394_format7_get_roi(dc1394camera_t *camera, dc1394video_mode_t video_mode, dc1394color_coding_t *color_coding,
				     uint32_t *bytes_per_packet, uint32_t *left, uint32_t *top, uint32_t *width, uint32_t *height);

/* This will have to be fixed or removed: it's ugly...*/
dc1394error_t dc1394_cleanup_iso_channels_and_bandwidth(dc1394camera_t *camera);


/***************************************************************************
     PIO, SIO and Strobe Functions
 ***************************************************************************/

/* PIO (Parallel Input-Output)*/
dc1394error_t dc1394_pio_set(dc1394camera_t *camera, uint32_t value);
dc1394error_t dc1394_pio_get(dc1394camera_t *camera, uint32_t *value);



#ifdef __cplusplus
}
#endif

#endif /* __DC1394_CAMERA_CONTROL_H__ */
