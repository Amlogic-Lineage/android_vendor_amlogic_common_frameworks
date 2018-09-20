/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef __SSM_ACTION_H__
#define __SSM_ACTION_H__

#include "PQType.h"
#include "SSMHandler.h"
#include "PQSettingCfg.h"

#define SSM_DATA_PATH             "/mnt/vendor/param/pq/ssm_data"
#define SSM_RGBOGO_FILE_PATH      "/dev/block/cri_data"
#define SSM_RGBOGO_FILE_OFFSET    (0)

#define SSM_CR_RGBOGO_LEN                           (256)
#define SSM_CR_RGBOGO_CHKSUM_LEN                    (2)
#define DEFAULT_BACKLIGHT_BRIGHTNESS                (10)


class SSMAction {
public:
    SSMAction();
    ~SSMAction();
    static SSMAction *getInstance();
    bool isFileExist(const char *file_name);
    int WriteBytes(int offset, int size, unsigned char *buf);
    int ReadBytes(int offset, int size, unsigned char *buf);
    int EraseAllData(void);
    int GetSSMActualAddr(int id);
    int GetSSMActualSize(int id);
    int GetSSMStatus(void);
    int SSMReadNTypes(int id, int data_len, int *data_buf, int offset = 0);
    int SSMWriteNTypes(int id, int data_len, int *data_buf, int offset = 0);

	bool SSMRecovery();
	int  SSMRestoreDefault(int id, bool resetAll = true);

    //PQ mode
    int SSMSavePictureMode(int offset, int rw_val);
    int SSMReadPictureMode(int offset, int *rw_val);
    //Color Temperature
    int SSMSaveColorTemperature(int offset, int rw_val);
    int SSMReadColorTemperature(int offset, int *rw_val);
	int SSMSaveColorDemoMode(unsigned char rw_val);
	int SSMReadColorDemoMode(unsigned char *rw_val);
	int SSMSaveColorBaseMode(unsigned char rw_val);
	int SSMReadColorBaseMode(unsigned char *rw_val);
	int SSMSaveRGBGainRStart(int offset, unsigned int rw_val);
	int SSMReadRGBGainRStart(int offset, unsigned int *rw_val);
	int SSMSaveRGBGainGStart(int offset, unsigned int rw_val);
	int SSMReadRGBGainGStart(int offset, unsigned int *rw_val);
	int SSMSaveRGBGainBStart(int offset, unsigned int rw_val);
	int SSMReadRGBGainBStart(int offset, unsigned int *rw_val);
	int SSMSaveRGBPostOffsetRStart(int offset, int rw_val);
    int SSMReadRGBPostOffsetRStart(int offset, int *rw_val);
    int SSMSaveRGBPostOffsetGStart(int offset, int rw_val);
    int SSMReadRGBPostOffsetGStart(int offset, int *rw_val);
    int SSMSaveRGBPostOffsetBStart(int offset, int rw_val);
    int SSMReadRGBPostOffsetBStart(int offset, int *rw_val);
    int SSMReadRGBOGOValue(int offset, int size, unsigned char data_buf[]);
    int SSMSaveRGBOGOValue(int offset, int size, unsigned char data_buf[]);
	int SSMSaveRGBValueStart(int offset, int8_t rw_val);
	int SSMReadRGBValueStart(int offset, int8_t *rw_val);
	int SSMSaveColorSpaceStart(unsigned char rw_val);
	int SSMReadColorSpaceStart(unsigned char *rw_val);
	int ReadDataFromFile(const char *file_name, int offset, int nsize, unsigned char data_buf[]);
    int SaveDataToFile(const char *file_name, int offset, int nsize, unsigned char data_buf[]);
    //Brightness
    int SSMSaveBrightness(int offset, int rw_val);
    int SSMReadBrightness(int offset, int *rw_val);
    //constract
    int SSMSaveContrast(int offset, int rw_val);
    int SSMReadContrast(int offset, int *rw_val);
    //saturation
    int SSMSaveSaturation(int offset, int rw_val);
    int SSMReadSaturation(int offset, int *rw_val);
    //hue
    int SSMSaveHue(int offset, int rw_val);
    int SSMReadHue(int offset, int *rw_val);
    //Sharpness
    int SSMSaveSharpness(int offset, int rw_val);
    int SSMReadSharpness(int offset, int *rw_val);
    //NoiseReduction
    int SSMSaveNoiseReduction(int offset, int rw_val);
    int SSMReadNoiseReduction(int offset, int *rw_val);

    int SSMSaveGammaValue(int rw_val);
    int SSMReadGammaValue(int *rw_val);

    int SSMSaveNonStandardValue(unsigned short rw_val);
    int SSMReadNonStandardValue(void);
    int SSMSaveEyeProtectionMode(int rw_val);
    int SSMReadEyeProtectionMode(int *rw_val);
    int SSMSaveTestPattern(unsigned char rw_val);
    int SSMReadTestPattern(unsigned char *rw_val);

    int SSMSaveDDRSSC(unsigned char rw_val);
    int SSMReadDDRSSC(unsigned char *rw_val);
    int SSMSaveLVDSSSC(unsigned char *rw_val);
	int SSMReadLVDSSSC(unsigned char *rw_val);

    int SSMSaveDisplayMode(int offset, int rw_val);
    int SSMReadDisplayMode(int offset, int *rw_val);

    int SSMSaveBackLightVal(int offset, int rw_val);
    int SSMReadBackLightVal(int offset, int *rw_val);

    int m_dev_fd;
    static SSMAction *mInstance;
    static SSMHandler *mSSMHandler;
};
#endif
