/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *  @author   Tellen Yu
 *  @version  2.0
 *  @date     2014/10/23
 *  @par function description:
 *  - 1 set display mode
 */

#ifndef ANDROID_DISPLAY_MODE_H
#define ANDROID_DISPLAY_MODE_H

#include "SysWrite.h"
#include "common.h"
#include "HDCP/HDCPTxAuth.h"
#include "HDCP/HDCPRxAuth.h"
#include "HDCP/HDCPRx22ImgKey.h"
#include "HDCP/HDCPRxKey.h"
#include "FrameRateAutoAdaption.h"
#include <FormatColorDepth.h>
#include <map>
#include <cmath>
#include <string>
#include <pthread.h>
#include <linux/fb.h>
#include <semaphore.h>

#ifndef RECOVERY_MODE
#include "ISystemControlNotify.h"

using namespace android;
#endif

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

//frame rate auto adatper feature
#define FRAME_RATE_AUTO_ADAPTER

#define TEST_UBOOT_MODE

#define DEVICE_STR_MID                  "MID"
#define DEVICE_STR_MBOX                 "MBOX"
#define DEVICE_STR_TV                   "TV"

#define DESITY_720P                     "160"
#define DESITY_1080P                    "240"
#define DESITY_2160P                    "480"

#define DEFAULT_EDID_CRCHEAD            "checkvalue: "
#define DEFAULT_OUTPUT_MODE             "480p60hz"
#define DISPLAY_CFG_FILE                "/system/etc/mesondisplay.cfg"
#define DISPLAY_FB0                     "/dev/graphics/fb0"
#define DISPLAY_FB1                     "/dev/graphics/fb1"
#define SYSFS_DISPLAY_MODE              "/sys/class/display/mode"
#define SYSFS_DISPLAY_MODE2             "/sys/class/display2/mode"
//when close freescale, will enable display axis, cut framebuffer output
//when open freescale, will enable window axis, scale framebuffer output
#define SYSFS_DISPLAY_AXIS              "/sys/class/display/axis"
#define SYSFS_VIDEO_AXIS                "/sys/class/video/axis"
#define SYSFS_BOOT_TYPE                 "/sys/power/boot_type"
#define SYSFS_VIDEO_LAYER_STATE         "/sys/class/video/video_layer1_state"
#define DISPLAY_FB0_BLANK               "/sys/class/graphics/fb0/blank"
#define DISPLAY_FB1_BLANK               "/sys/class/graphics/fb1/blank"
#define DISPLAY_LOGO_INDEX              "/sys/module/fb/parameters/osd_logo_index"
#define SYS_DISABLE_VIDEO               "/sys/class/video/disable_video"

#define DISPLAY_FB0_FREESCALE           "/sys/class/graphics/fb0/free_scale"
#define DISPLAY_FB1_FREESCALE           "/sys/class/graphics/fb1/free_scale"
#define DISPLAY_FB0_FREESCALE_MODE      "/sys/class/graphics/fb0/freescale_mode"
#define DISPLAY_FB1_FREESCALE_MODE      "/sys/class/graphics/fb1/freescale_mode"
#define DISPLAY_FB0_SCALE_AXIS          "/sys/class/graphics/fb0/scale_axis"
#define DISPLAY_FB1_SCALE_AXIS          "/sys/class/graphics/fb1/scale_axis"
#define DISPLAY_FB1_SCALE               "/sys/class/graphics/fb1/scale"

#define DISPLAY_FB0_FREESCALE_AXIS      "/sys/class/graphics/fb0/free_scale_axis"
#define DISPLAY_FB0_WINDOW_AXIS         "/sys/class/graphics/fb0/window_axis"
#define DISPLAY_FB0_FREESCALE_SWTICH    "/sys/class/graphics/fb0/free_scale_switch"

#define DISPLAY_HDMI_HDCP14_STOP        "stop14" //stop HDCP1.4 authenticate
#define DISPLAY_HDMI_HDCP22_STOP        "stop22" //stop HDCP2.2 authenticate
#define DISPLAY_HDMI_HDCP_14            "1"
#define DISPLAY_HDMI_HDCP_22            "2"
#define DISPLAY_HDMI_HDCP_VER           "/sys/class/amhdmitx/amhdmitx0/hdcp_ver"//RX support HDCP version
#define DISPLAY_HDMI_HDCP_MODE          "/sys/class/amhdmitx/amhdmitx0/hdcp_mode"//set HDCP mode
#define DISPLAY_HDMI_HDCP_AUTH          "/sys/module/hdmitx20/parameters/hdmi_authenticated"//HDCP Authentication
#define DISPLAY_HDMI_HDCP_CONF          "/sys/class/amhdmitx/amhdmitx0/hdcp_ctrl" //HDCP config
#define DISPLAY_HDMI_HDCP_KEY           "/sys/class/amhdmitx/amhdmitx0/hdcp_lstore"//TX have 22 or 14 or none key
#define DISPLAY_HDMI_HDCP_POWER         "/sys/class/amhdmitx/amhdmitx0/hdcp_pwr"//write to 1, force hdcp_tx22 quit safely

#define DISPLAY_HPD_STATE               "/sys/class/amhdmitx/amhdmitx0/hpd_state"
#define DISPLAY_HDMI_EDID               "/sys/class/amhdmitx/amhdmitx0/disp_cap"//RX support display mode
#define DISPLAY_HDMI_DISP_CAP_3D        "/sys/class/amhdmitx/amhdmitx0/disp_cap_3d"//RX support display 3d mode
#define DISPLAY_HDMI_DEEP_COLOR         "/sys/class/amhdmitx/amhdmitx0/dc_cap"//RX supoort deep color
#define DISPLAY_HDMI_HDR                "/sys/class/amhdmitx/amhdmitx0/hdr_cap"
#define DISPLAY_HDMI_HDR_MODE           "/sys/module/am_vecm/parameters/hdr_mode"
#define DISPLAY_HDMI_SDR_MODE           "/sys/module/am_vecm/parameters/sdr_mode"
#define DISPLAY_HDMI_AUDIO              "/sys/class/amhdmitx/amhdmitx0/aud_cap"
#define DISPLAY_HDMI_AUDIO_MUTE         "/sys/class/amhdmitx/amhdmitx0/aud_mute"
#define DISPLAY_HDMI_VIDEO_MUTE         "/sys/class/amhdmitx/amhdmitx0/vid_mute"
#define DISPLAY_HDMI_MODE_PREF          "/sys/class/amhdmitx/amhdmitx0/preferred_mode"
#define DISPLAY_HDMI_SINK_TYPE          "/sys/class/amhdmitx/amhdmitx0/sink_type"
#define DISPLAY_HDMI_VIC                "/sys/class/amhdmitx/amhdmitx0/vic"//if switch between 8bit and 10bit, clear mic first

#define DISPLAY_HDMI_AVMUTE             "/sys/devices/virtual/amhdmitx/amhdmitx0/avmute"
#define DISPLAY_EDID_VALUE              "/sys/class/amhdmitx/amhdmitx0/edid"
#define DISPLAY_EDID_STATUS             "/sys/class/amhdmitx/amhdmitx0/edid_parsing"
#define DISPLAY_EDID_RAW                "/sys/class/amhdmitx/amhdmitx0/rawedid"
#define DISPLAY_HDMI_PHY                "/sys/class/amhdmitx/amhdmitx0/phy"

#define AUDIO_DSP_DIGITAL_RAW           "/sys/class/audiodsp/digital_raw"
#define AV_HDMI_CONFIG                  "/sys/class/amhdmitx/amhdmitx0/config"
#define AV_HDMI_3D_SUPPORT              "/sys/class/amhdmitx/amhdmitx0/support_3d"

#define HDMI_TX_PLUG_UEVENT             "DEVPATH=/devices/virtual/switch/hdmi"//hdmi hot plug event
#define HDMI_TX_POWER_UEVENT            "DEVPATH=/devices/virtual/switch/hdmi_power"
#define HDMI_TX_PLUG_STATE              "/sys/devices/virtual/switch/hdmi/state"
#define HDMI_TX_HDR_UEVENT              "DEVPATH=/devices/virtual/switch/hdmi_hdr"
#define HDMI_TX_HDCP_UEVENT             "DEVPATH=/devices/virtual/switch/hdcp"
#define HDMI_TX_HDCP14_LOG_UEVENT       "DEVPATH=/devices/virtual/switch/hdcp_log"
#define HDMI_TX_HDCP14_LOG_SYS          "/sys/kernel/debug/hdcp/log"
#define HDMI_TX_SWITCH_HDR              "/sys/class/switch/hdmi_hdr/state"

//dolby vision sysfs
#define DOLBY_VISION_POLICY             "/sys/module/am_vecm/parameters/dolby_vision_policy"
#define DOLBY_VISION_HDR10_POLICY       "/sys/module/am_vecm/parameters/dolby_vision_hdr10_policy"
#define DOLBY_VISION_ENABLE             "/sys/module/am_vecm/parameters/dolby_vision_enable"
#define DOLBY_VISION_MODE               "/sys/class/amvecm/dv_mode"
#define DOLBY_VISION_IS_SUPPORT         "/sys/class/amhdmitx/amhdmitx0/dv_cap"
#define DOLBY_VISION_GRAPHICS_PRIORITY  "/sys/module/am_vecm/parameters/dolby_vision_graphics_priority"
#define DOLBY_VISION_LL_POLICY          "/sys/module/am_vecm/parameters/dolby_vision_ll_policy"

#define DOLBY_VISION_SET_ENABLE_LL_RGB  3
#define DOLBY_VISION_SET_ENABLE_LL_YUV  2
#define DOLBY_VISION_SET_ENABLE         1
#define DOLBY_VISION_SET_DISABLE        0

#define DV_ENABLE                       "Y"
#define DV_DISABLE                      "N"

#define DV_POLICY_FOLLOW_SINK           "0"
#define DV_POLICY_FOLLOW_SOURCE         "1"
#define DV_POLICY_FORCE_MODE            "2"
#define DV_HDR10_POLICY                 "3"

#define DV_MODE_BYPASS                  "0x0"
#define DV_MODE_IPT_TUNNEL              "0x2"

#define HDMI_UEVENT_HDMI                "hdmi"
#define HDMI_UEVENT_HDMI_POWER          "hdmi_power"
#define HDMI_UEVENT_HDMI_HDR            "hdmi_hdr"
#define HDMI_UEVENT_HDCP                "hdcp"
#define HDMI_UEVENT_HDCP_LOG            "hdcp_log"

#define HDMI_TX_PLUG_OUT                "0"
#define HDMI_TX_PLUG_IN                 "1"
#define HDMI_TX_SUSPEND                 "0"
#define HDMI_TX_RESUME                  "1"

//HDCP RX
#define HDMI_RX_PLUG_UEVENT             "DEVPATH=/devices/virtual/switch/hdmirx_hpd"//1:plugin 0:plug out
#define HDMI_RX_AUTH_UEVENT             "DEVPATH=/devices/virtual/switch/hdmirx_hdcp_auth"//0:FAIL 1:HDCP14 2:HDCP22

#define HDMI_RX_PLUG_OUT                "0"
#define HDMI_RX_PLUG_IN                 "1"
#define HDMI_RX_AUTH_FAIL               "0"
#define HDMI_RX_AUTH_HDCP14             "1"
#define HDMI_RX_AUTH_HDCP22             "2"

#define HDMI_RX_HPD_STATE               "/sys/module/tvin_hdmirx/parameters/hpd_to_esm"
#define HDMI_RX_KEY_COMBINE             "/sys/module/tvin_hdmirx/parameters/hdcp22_firmware_ok_flag"

#define H265_DOUBLE_WRITE_MODE          "/sys/module/amvdec_h265/parameters/double_write_mode"

#define VIDEO_LAYER1_UEVENT             "DEVPATH=/devices/virtual/switch/video_layer1"
#define VIDEO_LAYER_ENABLE              "0"
#define VIDEO_LAYER_DISABLE             "1"
#define VIDEO_LAYER_AUTO_ENABLE         "2"//2:enable video layer when first frame data come

#define PROP_TVSOC_AS_MBOX              "ro.tvsoc.as.mbox"

#define PROP_HDMIONLY                   "ro.platform.hdmionly"
#define PROP_SUPPORT_4K                 "ro.platform.support.4k"
#define PROP_SUPPORT_OVER_4K30          "ro.platform.support.over.4k30"
#define PROP_LCD_DENSITY                "ro.sf.lcd_density"
#define PROP_WINDOW_WIDTH               "const.window.w"
#define PROP_WINDOW_HEIGHT              "const.window.h"
#define PROP_HAS_CVBS_MODE              "ro.platform.has.cvbsmode"
#define PROP_BEST_OUTPUT_MODE           "ro.platform.best_outputmode"
#define PROP_BOOTANIM                   "init.svc.bootanim"
#define PROP_FS_MODE                    "const.filesystem.mode"
#define PROP_BOOTANIM_DELAY             "const.bootanim.delay"
#define PROP_BOOTVIDEO_SERVICE          "service.bootvideo"
#define PROP_DEEPCOLOR                  "sys.open.deepcolor" //default close this function, when reboot
#define PROP_BOOTCOMPLETE               "dev.bootcomplete"
#define PROP_DOLBY_VISION_ENABLE        "persist.sys.dolbyvision.enable"
#define PROP_DOLBY_VISION_TYPE         "persist.sys.dolbyvision.type"
#define PROP_DOLBY_VISION_PRIORITY      "persist.sys.graphics.priority"
#define PROP_HDR_MODE_STATE             "persist.sys.hdr.state"
#define PROP_SDR_MODE_STATE             "persist.sys.sdr.state"

#define HDR_MODE_OFF                    "0"
#define HDR_MODE_ON                     "1"
#define HDR_MODE_AUTO                   "2"

#define SDR_MODE_OFF                    "0"
#define SDR_MODE_AUTO                   "2"

#define SUFFIX_10BIT                    "10bit"
#define SUFFIX_12BIT                    "12bit"
#define SUFFIX_14BIT                    "14bit"
#define SUFFIX_RGB                      "rgb"

#define DEFAULT_DEEP_COLOR_ATTR         "444,8bit"
#define DEFAULT_420_DEEP_COLOR_ATTR     "420,8bit"


#define UBOOTENV_DIGITAUDIO             "ubootenv.var.digitaudiooutput"
#define UBOOTENV_HDMIMODE               "ubootenv.var.hdmimode"
#define UBOOTENV_TESTMODE               "ubootenv.var.testmode"
#define UBOOTENV_CVBSMODE               "ubootenv.var.cvbsmode"
#define UBOOTENV_OUTPUTMODE             "ubootenv.var.outputmode"
#define UBOOTENV_ISBESTMODE             "ubootenv.var.is.bestmode"
#define UBOOTENV_EDIDCRCVALUE           "ubootenv.var.edid.crcvalue"

#define FULL_WIDTH_480                  720
#define FULL_HEIGHT_480                 480
#define FULL_WIDTH_576                  720
#define FULL_HEIGHT_576                 576
#define FULL_WIDTH_720                  1280
#define FULL_HEIGHT_720                 720
#define FULL_WIDTH_768                  1366
#define FULL_HEIGHT_768                 768
#define FULL_WIDTH_1080                 1920
#define FULL_HEIGHT_1080                1080
#define FULL_WIDTH_4K2K                 3840
#define FULL_HEIGHT_4K2K                2160
#define FULL_WIDTH_4K2KSMPTE            4096
#define FULL_HEIGHT_4K2KSMPTE           2160

enum {
    EVENT_OUTPUT_MODE_CHANGE            = 0,
    EVENT_DIGITAL_MODE_CHANGE           = 1,
};

enum {
    DISPLAY_TYPE_NONE                   = 0,
    DISPLAY_TYPE_TABLET                 = 1,
    DISPLAY_TYPE_MBOX                   = 2,
    DISPLAY_TYPE_TV                     = 3,
    DISPLAY_TYPE_REPEATER               = 4
};

#define MODE_480I                       "480i60hz"
#define MODE_480P                       "480p60hz"
#define MODE_480CVBS                    "480cvbs"
#define MODE_576I                       "576i50hz"
#define MODE_576P                       "576p50hz"
#define MODE_576CVBS                    "576cvbs"
#define MODE_720P50HZ                   "720p50hz"
#define MODE_720P                       "720p60hz"
#define MODE_768P                       "768p60hz"
#define MODE_1080P24HZ                  "1080p24hz"
#define MODE_1080I50HZ                  "1080i50hz"
#define MODE_1080P50HZ                  "1080p50hz"
#define MODE_1080I                      "1080i60hz"
#define MODE_1080P                      "1080p60hz"
#define MODE_4K2K24HZ                   "2160p24hz"
#define MODE_4K2K25HZ                   "2160p25hz"
#define MODE_4K2K30HZ                   "2160p30hz"
#define MODE_4K2K50HZ                   "2160p50hz"
#define MODE_4K2K60HZ                   "2160p60hz"
#define MODE_4K2KSMPTE                  "smpte24hz"
#define MODE_4K2KSMPTE30HZ              "smpte30hz"
#define MODE_4K2KSMPTE50HZ              "smpte50hz"
#define MODE_4K2KSMPTE60HZ              "smpte60hz"

#define MODE_480I_PREFIX                "480i"
#define MODE_480P_PREFIX                "480p"
#define MODE_576I_PREFIX                "576i"
#define MODE_576P_PREFIX                "576p"
#define MODE_720P_PREFIX                "720p"
#define MODE_768P_PREFIX                "768p"
#define MODE_1080I_PREFIX               "1080i"
#define MODE_1080P_PREFIX               "1080p"
#define MODE_4K2K_PREFIX                "2160p"
#define MODE_4K2KSMPTE_PREFIX           "smpte"

enum {
    DISPLAY_MODE_480I                   = 0,
    DISPLAY_MODE_480P                   = 1,
    DISPLAY_MODE_480CVBS                = 2,
    DISPLAY_MODE_576I                   = 3,
    DISPLAY_MODE_576P                   = 4,
    DISPLAY_MODE_576CVBS                = 5,
    DISPLAY_MODE_720P50HZ               = 6,
    DISPLAY_MODE_720P                   = 7,
    DISPLAY_MODE_1080P24HZ              = 8,
    DISPLAY_MODE_1080I50HZ              = 9,
    DISPLAY_MODE_1080P50HZ              = 10,
    DISPLAY_MODE_1080I                  = 11,
    DISPLAY_MODE_1080P                  = 12,
    DISPLAY_MODE_4K2K24HZ               = 13,
    DISPLAY_MODE_4K2K25HZ               = 14,
    DISPLAY_MODE_4K2K30HZ               = 15,
    DISPLAY_MODE_4K2K50HZ               = 16,
    DISPLAY_MODE_4K2K60HZ               = 17,
    DISPLAY_MODE_4K2KSMPTE              = 18,
    DISPLAY_MODE_4K2KSMPTE30HZ          = 19,
    DISPLAY_MODE_4K2KSMPTE50HZ          = 20,
    DISPLAY_MODE_4K2KSMPTE60HZ          = 21,
    DISPLAY_MODE_768P                   = 22,
    DISPLAY_MODE_TOTAL                  = 23
};

typedef enum {
    OUPUT_MODE_STATE_INIT               = 0,
    OUPUT_MODE_STATE_POWER              = 1,//hot plug
    OUPUT_MODE_STATE_SWITCH             = 2,//user switch the mode
    OUPUT_MODE_STATE_SWITCH_ADAPTER     = 3,//video auto switch the mode
    OUPUT_MODE_STATE_RESERVE            = 4,
    OUPUT_MODE_STATE_ADAPTER_END        = 5 //end hint video auto switch the mode
}output_mode_state;

typedef enum {
    HDMI_SINK_TYPE_NONE                 = 0,
    HDMI_SINK_TYPE_SINK                 = 1,
    HDMI_SINK_TYPE_REPEATER             = 2,
    HDMI_SINK_TYPE_RESERVE              = 3
}hdmi_sink_type;

typedef struct hdmi_data {
    char edid[MAX_STR_LEN];
    int sinkType;
    char current_mode[MODE_LEN];
    char ubootenv_hdmimode[MODE_LEN];
}hdmi_data_t;

typedef struct axis_s {
    int x;
    int y;
    int w;
    int h;
} axis_t;

typedef struct resolution {
//       resolution       standard frequency deepcolor
//          2160             p       50hz      420   //2160p50hz420
//          1080             p       60hz        0   //1080p60hz
//0x00 0000 0000 0000 0000 | 0 | 0000 0000 | 0 0000 0000 //resolution_num
    int resolution;
    char standard;
    int frequency;
    int deepcolor;
    int64_t resolution_num;
} resolution_t;

// ----------------------------------------------------------------------------

class DisplayMode : public HDCPTxAuth::TxUevntCallbak,
                                      private FrameRateAutoAdaption::Callbak
{
public:
    DisplayMode(const char *path);
    ~DisplayMode();

    void init();
    void reInit();

    void getDisplayInfo(int &type, char* socType, char* defaultUI);
    void getFbInfo(int &fb0w, int &fb0h, int &fb0bits, int &fb0trip,
        int &fb1w, int &fb1h, int &fb1bits, int &fb1trip);

    void setLogLevel(int level);
    int dump(char *result);
    void setSourceOutputMode(const char* outputmode);
    void setSinkOutputMode(const char* outputmode);
    void setDigitalMode(const char* mode);
    void setOsdMouse(const char* curMode);
    void setOsdMouse(int x, int y, int w, int h);
    void setPosition(int left, int top, int width, int height);
    void getPosition(const char* curMode, int *position);
    void setDolbyVisionEnable(int state);
    bool isDolbyVisionEnable();
    bool isTvSupportDolbyVision(char *mode);
    void DetectDolbyVisionOutputMode(output_mode_state state, char* outputmode);
    void setGraphicsPriority(const char* mode);
    void getGraphicsPriority(char* mode);
    void getDeepColorAttr(const char* mode, char *value);
    void saveDeepColorAttr(const char* mode, const char* dcValue);
    int64_t resolveResolutionValue(const char *mode);
    void setHdrMode(const char* mode);
    void setSdrMode(const char* mode);
    void isHDCPTxAuthSuccess( int *status);
    static void* bootanimDetect(void *data);
    static void* bootvideoDetect(void *data);

    void setSourceDisplay(output_mode_state state);

    void setVideoPlayingAxis();
    void getHdmiData(hdmi_data_t* data);

    int readHdcpRX22Key(char *value, int size);
    bool writeHdcpRX22Key(const char *value, const int size);
    int readHdcpRX14Key(char *value, int size);
    bool writeHdcpRX14Key(const char *value, const int size);
    bool writeHdcpRXImg(const char *path);

    HDCPTxAuth *geTxAuth();
#ifndef RECOVERY_MODE
    void notifyEvent(int event);
    void setListener(const sp<ISystemControlNotify>& listener);
#endif

    virtual void onTxEvent (char* hpdstate, int outputState);
    virtual void onDispModeSyncEvent (const char* outputmode, int state);
    void hdcpSwitch();

    void setBootanimStatus(int status);
    void getBootanimStatus(int *status);

private:

    bool getBootEnv(const char* key, char* value);
    void setBootEnv(const char* key, char* value);

    int parseConfigFile();
    void setTabletDisplay();
    void getBestHdmiMode(char * mode, hdmi_data_t* data);
    void getHighestHdmiMode(char* mode, hdmi_data_t* data);
    void getHighestPriorityMode(char* mode, hdmi_data_t* data);
    void filterHdmiMode(char * mode, hdmi_data_t* data);
    void getHdmiOutputMode(char *mode, hdmi_data_t* data);
    void resolveResolution(const char *mode, resolution_t* resol_t);
    void setAutoSwitchFrameRate(int state);
    void updateDefaultUI();
    void updateDeepColor(bool cvbsMode, output_mode_state state, const char* outputmode);
    void updateFreeScaleAxis();
    void updateWindowAxis(const char* outputmode);
    void initGraphicsPriority();
    void initHdrSdrMode();
    bool isEdidChange();
    bool isBestOutputmode();
    bool modeSupport(char *mode, int sinkType);
    void setSourceOutputMode(const char* outputmode, output_mode_state state);
    void setSinkOutputMode(const char* outputmode, bool initState);
    int modeToIndex(const char *mode);
    void startHdmiPlugDetectThread();
    void startBootanimDetectThread();
    void startBootvideoDetectThread();
    static void* HdmiUenventThreadLoop(void* data);
    void setSinkDisplay(bool initState);
    void setFbParameter(const char* fbdev, struct fb_var_screeninfo var_set);
    int getBootenvInt(const char* key, int defaultVal);
    void dumpCap(const char * path, const char * hint, char *result);
    void dumpCaps(char *result=NULL);

    const char* pConfigPath;
    int mDisplayType;
    int mFb0Width;
    int mFb0Height;
    int mFb0FbBits;
    bool mFb0TripleEnable;//Triple Buffer enable or not

    int mFb1Width;
    int mFb1Height;
    int mFb1FbBits;
    bool mFb1TripleEnable;//Triple Buffer enable or not
    bool mVideoPlaying;

    int mDisplayWidth;
    int mDisplayHeight;

    char mSocType[64];
    char mDefaultUI[64];//this used for mbox
    int mLogLevel;
    SysWrite *pSysWrite = NULL;
    FrameRateAutoAdaption *pFrameRateAutoAdaption = NULL;

    HDCPTxAuth *pTxAuth = NULL;
    HDCPRxAuth *pRxAuth = NULL;

    // bootAnimation flag
    int mBootanimStatus;
#ifndef RECOVERY_MODE
    sp<ISystemControlNotify> mNotifyListener;
#endif
};

#endif // ANDROID_DISPLAY_MODE_H
