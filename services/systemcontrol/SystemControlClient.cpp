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
 *  @author   tellen
 *  @version  1.0
 *  @date     2017/10/18
 *  @par function description:
 *  - 1 system control apis for other vendor process
 */

#define LOG_TAG "SystemControlClient"
//#define LOG_NDEBUG 0

#include <utils/Log.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <android-base/logging.h>
#include <common.h>

#include <SystemControlClient.h>

namespace android {

SystemControlClient::SystemControlClient() {
    //mSysCtrl = ISystemControl::getService();
    sp<ISystemControl> ctrl = ISystemControl::tryGetService();
    while (ctrl == nullptr) {
         usleep(200*1000);//sleep 200ms
         ctrl = ISystemControl::tryGetService();
         ALOGE("tryGet system control daemon Service");
    };

    mDeathRecipient = new SystemControlDeathRecipient();
    Return<bool> linked = ctrl->linkToDeath(mDeathRecipient, /*cookie*/ 0);
    if (!linked.isOk()) {
        LOG(ERROR) << "Transaction error in linking to system service death: " << linked.description().c_str();
    } else if (!linked) {
        LOG(ERROR) << "Unable to link to system service death notifications";
    } else {
        LOG(INFO) << "Link to system service death notification successful";
    }

    mSysCtrl =ctrl ;
}

bool SystemControlClient::getProperty(const std::string& key, std::string& value) {
    mSysCtrl->getProperty(key, [&value](const Result &ret, const hidl_string& v) {
        if (Result::OK == ret) {
            value = v;
        }
    });

    return true;
}

bool SystemControlClient::getPropertyString(const std::string& key, std::string& value, std::string& def) {
    mSysCtrl->getPropertyString(key, def, [&value](const Result &ret, const hidl_string& v) {
        if (Result::OK == ret) {
            value = v;
        }
    });

    return true;
}

int32_t SystemControlClient::getPropertyInt(const std::string& key, int32_t def) {
    int32_t result;
    mSysCtrl->getPropertyInt(key, def, [&result](const Result &ret, const int32_t& v) {
        if (Result::OK == ret) {
            result = v;
        }
    });
    return result;
}

int64_t SystemControlClient::getPropertyLong(const std::string& key, int64_t def) {
    int64_t result;
    mSysCtrl->getPropertyLong(key, def, [&result](const Result &ret, const int64_t& v) {
        if (Result::OK == ret) {
            result = v;
        }
    });
    return result;
}

bool SystemControlClient::getPropertyBoolean(const std::string& key, bool def) {
    bool result;
    mSysCtrl->getPropertyBoolean(key, def, [&result](const Result &ret, const bool& v) {
        if (Result::OK == ret) {
            result = v;
        }
    });
    return result;
}

void SystemControlClient::setProperty(const std::string& key, const std::string& value) {
    mSysCtrl->setProperty(key, value);
}

bool SystemControlClient::readSysfs(const std::string& path, std::string& value) {
    mSysCtrl->readSysfs(path, [&value](const Result &ret, const hidl_string& v) {
        if (Result::OK == ret) {
            value = v;
        }
    });

    return true;
}

bool SystemControlClient::writeSysfs(const std::string& path, const std::string& value) {
    Result rtn = mSysCtrl->writeSysfs(path, value);
    if (rtn == Result::OK) {
        return true;
    }
    return false;
}

bool SystemControlClient::writeSysfs(const std::string& path, const char *value, const int size) {
    int i;
    hidl_array<int32_t, 4096> result;
    for (i = 0; i < size; ++i) {
        result[i] = value[i];
    }

    for (; i < 4096; ++i) {
        result[i] = 0;
    }

    Result rtn = mSysCtrl->writeSysfsBin(path, result, size);
    if (rtn == Result::OK) {
        return true;
    }
    return false;
}

bool SystemControlClient::writeUnifyKey(const std::string& key, const std::string& value) {
    Result rtn = mSysCtrl->writeUnifyKey(key, value);
    if (rtn == Result::OK) {
        return true;
    }
    return false;
}

bool SystemControlClient::readUnifyKey(const std::string& key, std::string& value) {
    mSysCtrl->readUnifyKey(key, [&value](const Result &ret, const hidl_string& v) {
        if (Result::OK == ret) {
            value = v;
        }
    });

    return true;
}

bool SystemControlClient::writePlayreadyKey(const std::string& key, const char *value, const int size) {
    int i;
    hidl_array<int32_t, 4096> result;
    for (i = 0; i < size; ++i) {
        result[i] = value[i];
    }

    for (; i < 4096; ++i) {
        result[i] = 0;
    }

    Result rtn = mSysCtrl->writePlayreadyKey(key, result, size);
    if (rtn == Result::OK) {
        return true;
    }
    return false;
}

int32_t SystemControlClient::readPlayreadyKey(const std::string& key, char *value, int size) {
    hidl_array<int32_t, 4096> result;
    int32_t len;
    int j;
    mSysCtrl->readPlayreadyKey(key, size, [&result, &len](const Result &ret, const hidl_array<int32_t, 4096> v, const int32_t& l) {
        if (Result::OK == ret) {
            for (int i = 0; i < l; ++i) {
                result[i] = v[i];
            }
            len = l;
        }
    });

    for (j = 0; j < len; ++j) {
        value[j] = result[j];
    }

    return len;
}

int32_t SystemControlClient::readAttestationKey(const std::string& node, const std::string& name, char *value, int size) {
    hidl_array<int32_t, 10240> result;
    int32_t len;
    int j;
    mSysCtrl->readAttestationKey(node, name, size, [&result, &len](const Result &ret, const hidl_array<int32_t, 10240> v, const int32_t& l) {
        if (Result::OK == ret) {
            for (int i = 0; i < l; ++i) {
                result[i] = v[i];
            }
            len = l;
        }
    });

    for (j = 0; j < len; ++j) {
        value[j] = result[j];
    }

    return len;
}

bool SystemControlClient::writeAttestationKey(const std::string& node, const std::string& name, const char *buff, const int size) {
    int i;
    hidl_array<int32_t, 10240> result;

    for (i = 0; i < size; ++i) {
        result[i] = buff[i];
    }

    for (; i < 10240; ++i) {
        result[i] = 0;
    }

    Result rtn = mSysCtrl->writeAttestationKey(node, name, result);
    if (rtn == Result::OK) {
        return true;
    }
    return false;
}

int32_t SystemControlClient::readHdcpRX22Key(char *value, int size) {
    hidl_array<int32_t, 4096> result;
    int32_t len;
    int j;
    mSysCtrl->readHdcpRX22Key(size, [&result, &len](const Result &ret, const hidl_array<int32_t, 4096> v, const int32_t& l) {
        if (Result::OK == ret) {
            for (int i = 0; i < l; ++i) {
                result[i] = v[i];
            }
            len = l;
        }
    });

    for (j = 0; j < len; ++j) {
        value[j] = result[j];
    }

    return len;
}

bool SystemControlClient::writeHdcpRX22Key(const char *value, const int size) {
    int i;
    hidl_array<int32_t, 4096> result;
    for (i = 0; i < size; ++i) {
        result[i] = value[i];
    }

    for (; i < 4096; ++i) {
        result[i] = 0;
    }

    Result rtn = mSysCtrl->writeHdcpRX22Key(result, size);
    if (rtn == Result::OK) {
        return true;
    }
    return false;
}

int32_t SystemControlClient::readHdcpRX14Key(char *value, int size) {
    hidl_array<int32_t, 4096> result;
    int32_t len;
    int j;
    mSysCtrl->readHdcpRX14Key(size, [&result, &len](const Result &ret, const hidl_array<int32_t, 4096> v, const int32_t& l) {
        if (Result::OK == ret) {
            for (int i = 0; i < l; ++i) {
                result[i] = v[i];
            }
            len = l;
        }
    });

    for (j = 0; j < len; ++j) {
        value[j] = result[j];
    }

    return len;
}

bool SystemControlClient::writeHdcpRX14Key(const char *value, const int size) {
    int i;
    hidl_array<int32_t, 4096> result;
    for (i = 0; i < size; ++i) {
        result[i] = value[i];
    }

    for (; i < 4096; ++i) {
        result[i] = 0;
    }
    Result rtn = mSysCtrl->writeHdcpRX14Key(result, size);
    if (rtn == Result::OK) {
        return true;
    }
    return false;
}

bool SystemControlClient::writeHdcpRXImg(const std::string& path) {
    Result rtn = mSysCtrl->writeHdcpRXImg(path);
    if (rtn == Result::OK) {
        return true;
    }
    return false;
}

bool SystemControlClient::getBootEnv(const std::string& key, std::string& value) {
    mSysCtrl->getBootEnv(key, [&value](const Result &ret, const hidl_string& v) {
        if (Result::OK == ret) {
            value = v;
        }
    });
    return true;
}

void SystemControlClient::setBootEnv(const std::string& key, const std::string& value) {
    mSysCtrl->setBootEnv(key, value);
}

void SystemControlClient::getDroidDisplayInfo(int &type __unused, std::string& socType __unused, std::string& defaultUI __unused,
    int &fb0w __unused, int &fb0h __unused, int &fb0bits __unused, int &fb0trip __unused,
    int &fb1w __unused, int &fb1h __unused, int &fb1bits __unused, int &fb1trip __unused) {

    /*mSysCtrl->getDroidDisplayInfo([&](const Result &ret, const DroidDisplayInfo& info) {
        if (Result::OK == ret) {
            type = info.type;
            socType = info.socType;
            defaultUI = info.defaultUI;
            fb0w = info.fb0w;
            fb0h = info.fb0h;
            fb0bits = info.fb0bits;
            fb0trip = info.fb0trip;
            fb1w = info.fb1w;
            fb1h = info.fb1h;
            fb1bits = info.fb1bits;
            fb1trip = info.fb1trip;
        }
    });*/
}

void SystemControlClient::loopMountUnmount(int &isMount, const std::string& path)  {
    mSysCtrl->loopMountUnmount(isMount, path);
}

void SystemControlClient::setMboxOutputMode(const std::string& mode) {
    mSysCtrl->setSourceOutputMode(mode);
}

void SystemControlClient::setSinkOutputMode(const std::string& mode) {
    mSysCtrl->setSinkOutputMode(mode);
}

void SystemControlClient::setDigitalMode(const std::string& mode) {
    mSysCtrl->setDigitalMode(mode);
}

void SystemControlClient::setOsdMouseMode(const std::string& mode) {
    mSysCtrl->setOsdMouseMode(mode);
}

void SystemControlClient::setOsdMousePara(int x, int y, int w, int h) {
    mSysCtrl->setOsdMousePara(x, y, w, h);
}

void SystemControlClient::setPosition(int left, int top, int width, int height)  {
    mSysCtrl->setPosition(left, top, width, height);
}

void SystemControlClient::getPosition(const std::string& mode, int &outx, int &outy, int &outw, int &outh) {
    mSysCtrl->getPosition(mode, [&outx, &outy, &outw, &outh](const Result &ret,
        const int32_t& x, const int32_t& y, const int32_t& w, const int32_t& h) {
        if (Result::OK == ret) {
            outx = x;
            outy = y;
            outw = w;
            outh = h;
        }
    });
}

void SystemControlClient::saveDeepColorAttr(const std::string& mode, const std::string& dcValue) {
    mSysCtrl->saveDeepColorAttr(mode, dcValue);
}

void SystemControlClient::getDeepColorAttr(const std::string& mode, std::string& value) {
    mSysCtrl->getDeepColorAttr(mode, [&value](const Result &ret, const hidl_string& v) {
        if (Result::OK == ret) {
            value = v;
        }
    });
}

void SystemControlClient::setDolbyVisionEnable(int state) {
    mSysCtrl->setDolbyVisionState(state);
}

bool SystemControlClient::isTvSupportDolbyVision(std::string& mode) {
    bool supported = false;
    mSysCtrl->sinkSupportDolbyVision([&mode, &supported](const Result &ret, const hidl_string& sinkMode, const bool &isSupport) {
        if (Result::OK == ret) {
            mode = sinkMode;
            supported = isSupport;
        }
    });

    return supported;
}

int32_t SystemControlClient::getDolbyVisionType() {
    int32_t result;
    mSysCtrl->getDolbyVisionType([&result](const Result &ret, const int32_t& v) {
        if (Result::OK == ret) {
            result = v;
        }
    });
    return result;
}

void SystemControlClient::setGraphicsPriority(const std::string& mode) {
   mSysCtrl->setGraphicsPriority(mode);
}

void SystemControlClient::getGraphicsPriority(std::string& mode) {
    mSysCtrl->getGraphicsPriority([&mode](const Result &ret, const hidl_string& tempmode) {
        if (Result::OK == ret)
            mode = tempmode.c_str();
        else
            mode.clear();
    });

    if (mode.empty()) {
        LOG(ERROR) << "system control client getGraphicsPriority FAIL.";
    }
}

int64_t SystemControlClient::resolveResolutionValue(const std::string& mode) {
    int64_t value = 0;
    mSysCtrl->resolveResolutionValue(mode, [&value](const Result &ret, const int64_t &v) {
        if (Result::OK == ret) {
            value = v;
        }
    });
    return value;
}

void SystemControlClient::setHdrMode(const std::string& mode) {
    mSysCtrl->setHdrMode(mode);
}

void SystemControlClient::setSdrMode(const std::string& mode) {
    mSysCtrl->setSdrMode(mode);
}

void SystemControlClient::setListener(const sp<ISystemControlCallback> callback) {
    Return<void> ret = mSysCtrl->setCallback(callback);
}

bool SystemControlClient::getSupportDispModeList(std::vector<std::string>& supportDispModes) {
    mSysCtrl->getSupportDispModeList([&supportDispModes](const Result &ret, const hidl_vec<hidl_string> list) {
        if (Result::OK == ret) {
            for (size_t i = 0; i < list.size(); i++) {
                supportDispModes.push_back(list[i]);
            }
        } else {
            supportDispModes.clear();
        }
    });

    if (supportDispModes.empty()) {
        LOG(ERROR) << "syscontrol::readEdidList FAIL.";
        return false;
    }

    return true;
}

bool SystemControlClient::getActiveDispMode(std::string& activeDispMode) {
    mSysCtrl->getActiveDispMode([&activeDispMode](const Result &ret, const hidl_string& mode) {
        if (Result::OK == ret)
            activeDispMode = mode.c_str();
        else
            activeDispMode.clear();
    });

    if (activeDispMode.empty()) {
        LOG(ERROR) << "system control client getActiveDispMode FAIL.";
        return false;
    }

    return true;
}

bool SystemControlClient::setActiveDispMode(std::string& activeDispMode) {
    Result rtn = mSysCtrl->setActiveDispMode(activeDispMode);
    if (rtn == Result::OK) {
        return true;
    }
    return false;
}

void SystemControlClient::isHDCPTxAuthSuccess(int &status) {
    Result rtn = mSysCtrl->isHDCPTxAuthSuccess();
    if (rtn == Result::OK) {
        status = 1;
    }
    else {
        status = 0;
    }
}

//3D
int32_t SystemControlClient::set3DMode(const std::string& mode3d) {
    mSysCtrl->set3DMode(mode3d);
    return 0;
}

void SystemControlClient::init3DSetting(void) {
    mSysCtrl->init3DSetting();
}

int SystemControlClient::getVideo3DFormat(void) {
    int32_t value = 0;
    mSysCtrl->getVideo3DFormat([&value](const Result &ret, const int32_t &v) {
        if (Result::OK == ret) {
            value = v;
        }
    });
    return value;
}

int SystemControlClient::getDisplay3DTo2DFormat(void) {
    int32_t value = 0;
    mSysCtrl->getDisplay3DTo2DFormat([&value](const Result &ret, const int32_t &v) {
        if (Result::OK == ret) {
            value = v;
        }
    });
    return value;
}

bool SystemControlClient::setDisplay3DTo2DFormat(int format) {
    mSysCtrl->setDisplay3DTo2DFormat(format);
    return true;
}

bool SystemControlClient::setDisplay3DFormat(int format) {
    mSysCtrl->setDisplay3DFormat(format);
    return true;
}

int SystemControlClient::getDisplay3DFormat(void) {
    int32_t value = 0;
    mSysCtrl->getDisplay3DFormat([&value](const Result &ret, const int32_t &v) {
        if (Result::OK == ret) {
            value = v;
        }
    });
    return value;
}

bool SystemControlClient::setOsd3DFormat(int format) {
    mSysCtrl->setOsd3DFormat(format);
    return true;
}

bool SystemControlClient::switch3DTo2D(int format) {
    mSysCtrl->switch3DTo2D(format);
    return true;
}

bool SystemControlClient::switch2DTo3D(int format) {
    mSysCtrl->switch2DTo3D(format);
    return true;
}

void SystemControlClient::autoDetect3DForMbox() {
    mSysCtrl->autoDetect3DForMbox();
}
//3D end

//PQ
int SystemControlClient::loadPQSettings(source_input_param_t srcInputParam) {
    SourceInputParam hidlSrcInput;
    memcpy(&hidlSrcInput, &srcInputParam, sizeof(source_input_param_t));
    return mSysCtrl->loadPQSettings(hidlSrcInput);
}

int SystemControlClient::loadCpqLdimRegs(void) {
    return mSysCtrl->loadCpqLdimRegs();
}

int SystemControlClient::setPQmode(int mode, int isSave, int is_autoswitch) {
    return mSysCtrl->setPQmode(mode, isSave, is_autoswitch);
}

int SystemControlClient::getPQmode(void) {
    return mSysCtrl->getPQmode();
}

int SystemControlClient::savePQmode(int mode) {
    return mSysCtrl->savePQmode(mode);
}

int SystemControlClient::setColorTemperature(int mode, int isSave) {
    return mSysCtrl->setColorTemperature(mode, isSave);
}

int SystemControlClient::getColorTemperature(void) {
    return mSysCtrl->getColorTemperature();
}

int SystemControlClient::saveColorTemperature(int mode) {
    return mSysCtrl->saveColorTemperature(mode);
}

int SystemControlClient::setColorTemperatureParam(int mode, tcon_rgb_ogo_t params) {
    TconRgbOgo hidlParam;
    memcpy(&hidlParam, &params, sizeof(tcon_rgb_ogo_t));
    return mSysCtrl->setColorTemperatureParam(mode, hidlParam);
}

int SystemControlClient::getColorTemperatureParam(int mode, int id) {
    return mSysCtrl->getColorTemperatureParam(mode, id);
}

int SystemControlClient::saveColorTemperatureParam(int mode, tcon_rgb_ogo_t params) {
    TconRgbOgo hidlParam;
    memcpy(&hidlParam, &params, sizeof(tcon_rgb_ogo_t));
    return mSysCtrl->saveColorTemperatureParam(mode, hidlParam);
}

int SystemControlClient::setBrightness(int value, int isSave) {
    return mSysCtrl->setBrightness(value, isSave);
}

int SystemControlClient::getBrightness(void) {
    return mSysCtrl->getBrightness();
}

int SystemControlClient::saveBrightness(int value) {
    return mSysCtrl->saveBrightness(value);
}

int SystemControlClient::setContrast(int value, int isSave) {
    return mSysCtrl->setContrast(value, isSave);
}

int SystemControlClient::getContrast(void) {
    return mSysCtrl->getContrast();
}

int SystemControlClient::saveContrast(int value) {
    return mSysCtrl->saveContrast(value);
}

int SystemControlClient::setSaturation(int value, int isSave) {
    return mSysCtrl->setSaturation(value, isSave);
}

int SystemControlClient::getSaturation(void) {
    return mSysCtrl->getSaturation();
}

int SystemControlClient::saveSaturation(int value) {
    return mSysCtrl->saveSaturation(value);
}

int SystemControlClient::setHue(int value, int isSave) {
    return mSysCtrl->setHue(value, isSave);
}

int SystemControlClient::getHue(void) {
    return mSysCtrl->getHue();
}

int SystemControlClient::saveHue(int value) {
    return mSysCtrl->saveHue(value);
}

int SystemControlClient::setSharpness(int value, int is_enable, int isSave) {
    return mSysCtrl->setSharpness(value, is_enable, isSave);
}

int SystemControlClient::getSharpness(void) {
    return mSysCtrl->getSharpness();
}

int SystemControlClient::saveSharpness(int value) {
    return mSysCtrl->saveSharpness(value);
}

int SystemControlClient::setNoiseReductionMode(int nr_mode, int isSave) {
    return mSysCtrl->setNoiseReductionMode(nr_mode, isSave);
}

int SystemControlClient::getNoiseReductionMode(void) {
    return mSysCtrl->getNoiseReductionMode();
}

int SystemControlClient::saveNoiseReductionMode(int nr_mode) {
    return mSysCtrl->saveNoiseReductionMode(nr_mode);
}

int SystemControlClient::setEyeProtectionMode(int source_input, int enable, int isSave) {
    return mSysCtrl->setEyeProtectionMode(source_input, enable, isSave);
}

int SystemControlClient::getEyeProtectionMode(int source_input) {
    return mSysCtrl->getEyeProtectionMode(source_input);
}

int SystemControlClient::setGammaValue(int gamma_curve, int isSave) {
    return mSysCtrl->setGammaValue(gamma_curve, isSave);
}

int SystemControlClient::getGammaValue(void) {
    return mSysCtrl->getGammaValue();
}

int SystemControlClient::setDisplayMode(int source_input, int mode, int isSave)
{
    return mSysCtrl->setDisplayMode(source_input, mode, isSave);
}

int SystemControlClient::getDisplayMode(int source_input) {
    return mSysCtrl->getDisplayMode(source_input);
}

int SystemControlClient::saveDisplayMode(int source_input, int mode)
{
    return mSysCtrl->saveDisplayMode(source_input, mode);
}

int SystemControlClient::setBacklight(int source_input, int value, int isSave)
{
    return mSysCtrl->setBacklight(source_input, value, isSave);
}

int SystemControlClient::getBacklight(int source_input)
{
    return mSysCtrl->getBacklight(source_input);
}

int SystemControlClient::saveBacklight(int source_input, int value)
{
    return mSysCtrl->saveBacklight(source_input, value);
}

int SystemControlClient::factoryResetPQMode(void) {
    return mSysCtrl->factoryResetPQMode();
}

int SystemControlClient::factoryResetColorTemp(void) {
    return mSysCtrl->factoryResetColorTemp();
}

int SystemControlClient::factorySetPQParam(source_input_param_t srcInputParam, int mode, int id, int value) {
    SourceInputParam hidlSrcInput;
    memcpy(&hidlSrcInput, &srcInputParam, sizeof(source_input_param_t));
    return mSysCtrl->factorySetPQParam(hidlSrcInput, mode, id, value);
}

int SystemControlClient::factoryGetPQParam(source_input_param_t srcInputParam, int mode, int id) {
    SourceInputParam hidlSrcInput;
    memcpy(&hidlSrcInput, &srcInputParam, sizeof(source_input_param_t));
    return mSysCtrl->factoryGetPQParam(hidlSrcInput, mode, id);
}

int SystemControlClient::factorySetColorTemperatureParam(int colortemperature_mode, int id, int value) {
    return mSysCtrl->factorySetColorTemperatureParam(colortemperature_mode, id, value);
}

int SystemControlClient::factoryGetColorTemperatureParam(int colortemperature_mode, int id) {
    return mSysCtrl->factoryGetColorTemperatureParam(colortemperature_mode, id);
}

int SystemControlClient::factorySaveColorTemperatureParam(int colortemperature_mode, int id, int value) {
    return mSysCtrl->factorySaveColorTemperatureParam(colortemperature_mode, id, value);
}

int SystemControlClient::factorySetOverscan(source_input_param_t srcInputParam, int he_value, int hs_value, int ve_value, int vs_value) {
    SourceInputParam hidlSrcInput;
    memcpy(&hidlSrcInput, &srcInputParam, sizeof(source_input_param_t));
    return mSysCtrl->factorySetOverscan(hidlSrcInput, he_value, hs_value, ve_value, vs_value);
}

int SystemControlClient::factoryGetOverscan(source_input_param_t srcInputParam, int id) {
    SourceInputParam hidlSrcInput;
    memcpy(&hidlSrcInput, &srcInputParam, sizeof(source_input_param_t));
    return mSysCtrl->factoryGetOverscan(hidlSrcInput, id);
}

int SystemControlClient::factorySetNolineParams(source_input_param_t srcInputParam, int type, int osd0_value, int osd25_value,
                            int osd50_value, int osd75_value, int osd100_value) {
    SourceInputParam hidlSrcInput;
    memcpy(&hidlSrcInput, &srcInputParam, sizeof(source_input_param_t));
    return mSysCtrl->factorySetNolineParams(hidlSrcInput, type, osd0_value, osd25_value,
                            osd50_value, osd75_value, osd100_value);
}

int SystemControlClient::factoryGetNolineParams(source_input_param_t srcInputParam, int type, int id) {
    SourceInputParam hidlSrcInput;
    memcpy(&hidlSrcInput, &srcInputParam, sizeof(source_input_param_t));
    return mSysCtrl->factoryGetNolineParams(hidlSrcInput, type, id);
}

int SystemControlClient::factorySetParamsDefault(void) {
    return mSysCtrl->factorySetParamsDefault();
}

int SystemControlClient::factorySSMRestore(void) {
    return mSysCtrl->factorySSMRestore();
}

int SystemControlClient::factoryResetNonlinear(void) {
    return mSysCtrl->factoryResetNonlinear();
}

int SystemControlClient::factorySetGamma(int gamma_r, int gamma_g, int gamma_b) {
    return mSysCtrl->factorySetGamma(gamma_r, gamma_g, gamma_b);
}

int SystemControlClient::sysSSMReadNTypes(int id, int data_len, int offset) {
    return mSysCtrl->sysSSMReadNTypes(id, data_len, offset);
}

int SystemControlClient::sysSSMWriteNTypes(int id, int data_len, int data_buf, int offset) {
    return mSysCtrl->sysSSMWriteNTypes(id, data_len, data_buf, offset);
}

int SystemControlClient::getActualAddr(int id) {
    return mSysCtrl->getActualAddr(id);
}

int SystemControlClient::getActualSize(int id) {
    return mSysCtrl->getActualSize(id);
}

int SystemControlClient::SSMRecovery(void) {
    return mSysCtrl->SSMRecovery();
}

int SystemControlClient::setPLLValues(source_input_param_t srcInputParam) {
    SourceInputParam hidlSrcInput;
    memcpy(&hidlSrcInput, &srcInputParam, sizeof(source_input_param_t));
    return mSysCtrl->setPLLValues(hidlSrcInput);
}

int SystemControlClient::setCVD2Values(source_input_param_t srcInputParam) {
    SourceInputParam hidlSrcInput;
    memcpy(&hidlSrcInput, &srcInputParam, sizeof(source_input_param_t));
    return mSysCtrl->setCVD2Values(hidlSrcInput);
}

int SystemControlClient::setPQConfig(Set_Flag_Cmd_t id, int value) {
    return mSysCtrl->setPQConfig(id, value);
}

int SystemControlClient::getSSMStatus(void) {
    return mSysCtrl->getSSMStatus();
}

int SystemControlClient::resetLastPQSettingsSourceType(void) {
    return mSysCtrl->resetLastPQSettingsSourceType();
}

int SystemControlClient::setCurrentSourceInfo(source_input_param_t srcInputParam) {
    SourceInputParam hidlSrcInput;
    memcpy(&hidlSrcInput, &srcInputParam, sizeof(source_input_param_t));
    return mSysCtrl->setCurrentSourceInfo(hidlSrcInput);
}

void SystemControlClient::getCurrentSourceInfo(int32_t &sourceInput, int32_t &sourceType, int32_t &sourcePort,
                                                       int32_t &sigFmt, int32_t &transFmt, int32_t &is3d)
{
    mSysCtrl->getCurrentSourceInfo([&](const Result &ret, const SourceInputParam &hidlSrcInput) {
        if (Result::OK == ret) {
            sourceInput = hidlSrcInput.sourceInput;
            sourceType = hidlSrcInput.sourceType;
            sourcePort = hidlSrcInput.sourcePort;
            sigFmt = hidlSrcInput.sigFmt;
            transFmt = hidlSrcInput.transFmt;
            is3d = hidlSrcInput.is3d;
        }
    });
}

int SystemControlClient::getAutoSwitchPCModeFlag(void) {
    return mSysCtrl->getAutoSwitchPCModeFlag();
}
//PQ end

int SystemControlClient::setwhiteBalanceGainRed(int32_t inputSrc, int32_t colortemp_mode, int32_t value) {
    return mSysCtrl->setwhiteBalanceGainRed(inputSrc, colortemp_mode, value);
}

int SystemControlClient::setwhiteBalanceGainGreen(int32_t inputSrc, int32_t colortemp_mode, int32_t value) {
    return mSysCtrl->setwhiteBalanceGainGreen(inputSrc, colortemp_mode, value);
}

int SystemControlClient::setwhiteBalanceGainBlue(int32_t inputSrc, int32_t colortemp_mode, int32_t value) {
    return mSysCtrl->setwhiteBalanceGainBlue(inputSrc, colortemp_mode, value);
}

int SystemControlClient::setwhiteBalanceOffsetRed(int32_t inputSrc, int32_t colortemp_mode, int32_t value) {
    return mSysCtrl->setwhiteBalanceOffsetRed(inputSrc, colortemp_mode, value);
}

int SystemControlClient::setwhiteBalanceOffsetGreen(int32_t inputSrc, int32_t colortemp_mode, int32_t value) {
    return mSysCtrl->setwhiteBalanceOffsetGreen(inputSrc, colortemp_mode, value);
}

int SystemControlClient::setwhiteBalanceOffsetBlue(int32_t inputSrc, int32_t colortemp_mode, int32_t value) {
    return mSysCtrl->setwhiteBalanceOffsetBlue(inputSrc, colortemp_mode, value);
}

int SystemControlClient::getwhiteBalanceGainRed(int32_t inputSrc, int32_t colortemp_mode) {
    return mSysCtrl->getwhiteBalanceGainRed(inputSrc, colortemp_mode);
}

int SystemControlClient::getwhiteBalanceGainGreen(int32_t inputSrc, int32_t colortemp_mode) {
    return mSysCtrl->getwhiteBalanceGainGreen(inputSrc, colortemp_mode);
}

int SystemControlClient::getwhiteBalanceGainBlue(int32_t inputSrc, int32_t colortemp_mode) {
    return mSysCtrl->getwhiteBalanceGainBlue(inputSrc, colortemp_mode);
}

int SystemControlClient::getwhiteBalanceOffsetRed(int32_t inputSrc, int32_t colortemp_mode) {
    return mSysCtrl->getwhiteBalanceOffsetRed(inputSrc, colortemp_mode);
}

int SystemControlClient::getwhiteBalanceOffsetGreen(int32_t inputSrc, int32_t colortemp_mode) {
    return mSysCtrl->getwhiteBalanceOffsetGreen(inputSrc, colortemp_mode);
}

int SystemControlClient::getwhiteBalanceOffsetBlue(int32_t inputSrc, int32_t colortemp_mode) {
    return mSysCtrl->getwhiteBalanceOffsetBlue(inputSrc, colortemp_mode);
}

int SystemControlClient:: saveWhiteBalancePara(int32_t sourceType, int32_t colorTemp_mode, int32_t r_gain, int32_t g_gain, int32_t b_gain, int32_t r_offset, int32_t g_offset, int32_t
b_offset) {
    return mSysCtrl->saveWhiteBalancePara(sourceType, colorTemp_mode, r_gain, g_gain, b_gain, r_offset, g_offset, b_offset);
}

int SystemControlClient::getRGBPattern() {
    return mSysCtrl->getRGBPattern();
}

int SystemControlClient::setRGBPattern(int32_t r, int32_t g, int32_t b) {
    return mSysCtrl->setRGBPattern(r, g, b);
}

int SystemControlClient::factorySetDDRSSC(int32_t step) {
    return mSysCtrl->factorySetDDRSSC(step);
}

int SystemControlClient::factoryGetDDRSSC() {
    return mSysCtrl->factoryGetDDRSSC();
}

int SystemControlClient::factorySetLVDSSSC(int32_t step) {
    return mSysCtrl->factorySetLVDSSSC(step);
}

int SystemControlClient::factoryGetLVDSSSC() {
    return mSysCtrl->factoryGetLVDSSSC();
}

int SystemControlClient::whiteBalanceGrayPatternClose() {
    return mSysCtrl->whiteBalanceGrayPatternClose();
}

int SystemControlClient::whiteBalanceGrayPatternOpen() {
    return mSysCtrl->whiteBalanceGrayPatternOpen();
}

int SystemControlClient::whiteBalanceGrayPatternSet(int32_t value) {
    return mSysCtrl->whiteBalanceGrayPatternSet(value);
}

int SystemControlClient::whiteBalanceGrayPatternGet() {
    return mSysCtrl->whiteBalanceGrayPatternGet();
}

void SystemControlClient::SystemControlDeathRecipient::serviceDied(uint64_t cookie,
        const ::android::wp<::android::hidl::base::V1_0::IBase>& who) {
    LOG(ERROR) << "system control service died. need release some resources";
}

}; // namespace android
