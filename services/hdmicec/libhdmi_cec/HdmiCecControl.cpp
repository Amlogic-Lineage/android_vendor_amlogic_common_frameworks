/*
 * Copyright (C) 2016 The Android Open Source Project
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
 *  @version  1.0
 *  @date     2017/9/25
 *  @par function description:
 *  - 1 droidlogic hdmi cec real implementation
 */

#define LOG_TAG "hdmicecd"
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include "HdmiCecControl.h"

namespace android {

HdmiCecControl::HdmiCecControl() : mFirstEnableCec(true) {
    ALOGD("[hcc] HdmiCecControl");
    init();
}

HdmiCecControl::~HdmiCecControl(){}

/**
 * initialize some cec flags before opening cec deivce.
 */
void HdmiCecControl::init()
{
    char value[PROPERTY_VALUE_MAX] = {0};
    mCecDevice.isTvDeviceType = false;
    mCecDevice.mDeviceTypes = NULL;
    mCecDevice.mAddedPhyAddrs = NULL;
    mCecDevice.mTotalDevice = 0;
    getDeviceTypes();
    memset(value, 0, PROPERTY_VALUE_MAX);
    //property_get("persist.vendor.sys.hdmi.keep_awake", value, "true");
    //mCecDevice.mExtendControl = (!strcmp(value, "false")) ? 1 : 0;
    mCecDevice.mExtendControl= 1;
    ALOGD("[hcc] ext_control: %d", mCecDevice.mExtendControl);
    mSystemControl = new SystemControlClient();
}

void HdmiCecControl::getDeviceTypes() {
    int index = 0;
    char value[PROPERTY_VALUE_MAX] = {0};
    const char * split = ",";
    char * type;
    mCecDevice.mDeviceTypes = new int[DEV_TYPE_VIDEO_PROCESSOR];
    if (!mCecDevice.mDeviceTypes) {
        ALOGE("[hcc] alloc mDeviceTypes failed");
        return;
    }
    property_get("ro.vendor.platform.hdmi.device_type", value, "4");
    type = strtok(value, split);
    mCecDevice.mDeviceTypes[index] = atoi(type);
    while (type != NULL) {
        type = strtok(NULL,split);
        if (type != NULL)
            mCecDevice.mDeviceTypes[++index] = atoi(type);
    }
    mCecDevice.mTotalDevice = index + 1;
    index = 0;
    for (index = 0; index < mCecDevice.mTotalDevice; index++) {
        if (mCecDevice.mDeviceTypes[index] == DEV_TYPE_TV)
            mCecDevice.isTvDeviceType = true;
        ALOGD("[hcc] mCecDevice.mDeviceTypes[%d]: %d", index, mCecDevice.mDeviceTypes[index]);
    }
}
/**
 * close cec device, reset some cec flags, and recycle some resources.
 */
int HdmiCecControl::closeCecDevice()
{
    mCecDevice.mRun = false;
    while (!mCecDevice.mExited) {
        usleep(100 * 1000);
    }

    close(mCecDevice.mFd);
    delete mCecDevice.mpPortData;
    delete mCecDevice.mDeviceTypes;
    delete mCecDevice.mAddedPhyAddrs;
    ALOGD("[hcc] %s, cec has closed.", __FUNCTION__);
    return 0;
}

/**
 * initialize all cec flags when open cec devices, get the {@code fd} of cec devices,
 * and create a thread for cec working.
 * {@Return}  fd of cec device.
 */
int HdmiCecControl::openCecDevice()
{
    int index = 0;
    mCecDevice.mRun = true;
    mCecDevice.mExited = false;
    mCecDevice.mThreadId = 0;
    mCecDevice.mTotalPort = 0;
    mCecDevice.mConnectStatus = 0;
    mCecDevice.mpPortData = NULL;
    mCecDevice.mAddrBitmap = (1 << CEC_ADDR_BROADCAST);
    mCecDevice.isCecEnabled = true;
    mCecDevice.isCecControlled = false;
    mCecDevice.mFd = open(CEC_FILE, O_RDWR);
    if (mCecDevice.mFd < 0) {
        ALOGE("[hcc] can't open device. fd < 0");
        return -EINVAL;
    }
    for (index = 0; index < mCecDevice.mTotalDevice; index++) {
        int deviceType =  mCecDevice.mDeviceTypes[index];
        ALOGD("[hcc] set device type index : %d, type: %d", index, deviceType);
        int ret = ioctl(mCecDevice.mFd, CEC_IOC_SET_DEV_TYPE, deviceType);
    }
    getBootConnectStatus();
    mCecDevice.mAddedPhyAddrs = new int[ADDR_BROADCAST];
    pthread_create(&mCecDevice.mThreadId, NULL, __threadLoop, this);
    pthread_setname_np(mCecDevice.mThreadId, "hdmi_cec_loop");
    return mCecDevice.mFd;
}

/**
 * get connect status when boot
 */
void HdmiCecControl::getBootConnectStatus()
{
    unsigned int total, i;
    int port, connect, ret;

    ret = ioctl(mCecDevice.mFd, CEC_IOC_GET_PORT_NUM, &total);
    ALOGD("[hcc] total port:%d, ret:%d", total, ret);
    if (ret < 0)
        return ;

    if (total > MAX_PORT)
        total = MAX_PORT;
    hdmi_port_info_t  *portData = NULL;
    portData = new hdmi_port_info[total];
    if (!portData) {
        ALOGE("[hcc] alloc port_data failed");
        return;
    }
    ioctl(mCecDevice.mFd, CEC_IOC_GET_PORT_INFO, portData);
    mCecDevice.mConnectStatus = 0;
    for (i = 0; i < total; i++) {
        port = portData[i].port_id;
        connect = port;
        ret = ioctl(mCecDevice.mFd, CEC_IOC_GET_CONNECT_STATUS, &connect);
        if (ret) {
            ALOGD("[hcc] get port %d connected status failed, ret:%d", i, ret);
            continue;
        }
        mCecDevice.mConnectStatus |= ((connect ? 1 : 0) << port);
    }
    delete portData;
    ALOGD("[hcc] mConnectStatus: %d", mCecDevice.mConnectStatus);
}

void* HdmiCecControl::__threadLoop(void *user)
{
    HdmiCecControl *const self = static_cast<HdmiCecControl *>(user);
    self->threadLoop();
    return 0;
}

void HdmiCecControl::threadLoop()
{
    unsigned char msgBuf[CEC_MESSAGE_BODY_MAX_LENGTH];
    hdmi_cec_event_t event;
    int r = -1;

    //ALOGD("[hcc] threadLoop start.");
    while (mCecDevice.mFd < 0) {
        usleep(1000 * 1000);
        mCecDevice.mFd = open(CEC_FILE, O_RDWR);
    }
    ALOGD("[hcc] file open ok, fd = %d.", mCecDevice.mFd);

    while (mCecDevice.mRun) {
        if (!mCecDevice.isCecEnabled) {
            usleep(1000 * 1000);
            continue;
        }
        checkConnectStatus();

        memset(msgBuf, 0, sizeof(msgBuf));
        //try to get a message from dev.
        r = readMessage(msgBuf, CEC_MESSAGE_BODY_MAX_LENGTH);
        if (r <= 1)//ignore received ping messages
            continue;

        printCecMsgBuf((const char*)msgBuf, r);

        event.eventType = 0;
        memcpy(event.cec.body, msgBuf + 1, r - 1);
        event.cec.initiator = cec_logical_address_t((msgBuf[0] >> 4) & 0xf);
        event.cec.destination = cec_logical_address_t((msgBuf[0] >> 0) & 0xf);
        event.cec.length = r - 1;

        if (mCecDevice.isCecControlled) {
             event.eventType |= HDMI_EVENT_CEC_MESSAGE;
            /* call java method to process cec message for ext control */
            if (mCecDevice.mExtendControl) {
                event.eventType |= HDMI_EVENT_RECEIVE_MESSAGE;
            }
        } else {
            /* wakeup playback device */
            if (isWakeUpMsg((char*)msgBuf, r)) {
                memset(event.cec.body, 0, sizeof(event.cec.body));
                memcpy(event.cec.body, msgBuf + 1, r - 1);
                event.eventType |= HDMI_EVENT_CEC_MESSAGE;
                ALOGD("[hcc] receive wake up message for hdmi_tx");
            }
        }
        if (!messageValidate(&event)) {
            ALOGD("[hcc]  receive message is invalid.");
            continue;
        }
        if (mEventListener != NULL && event.eventType != 0) {
            mEventListener->onEventUpdate(&event);
        }
    }
    //ALOGE("thread end.");
    mCecDevice.mExited = true;
}

/**
 * Check if received a wakeup message if  mCecDevice.isCecControlled  is false
* @param msgBuf is a message Buf
*   msgBuf[1]: message type
*   msgBuf[2]-msgBuf[n]: message para
* @param len is message lenth
* @param deviceType is type of device
*/

bool HdmiCecControl::isWakeUpMsg(char *msgBuf, int len)
{
    bool ret = false;
    int index = 0;
    for (index = 0; index < mCecDevice.mTotalDevice; index++) {
        if (mCecDevice.mDeviceTypes[index] == DEV_TYPE_PLAYBACK) {
            switch (msgBuf[1]) {
                case CEC_MESSAGE_SET_STREAM_PATH:
                case CEC_MESSAGE_DECK_CONTROL:
                case CEC_MESSAGE_PLAY:
                case CEC_MESSAGE_ACTIVE_SOURCE:
                    ret = true;
                    break;
                case CEC_MESSAGE_ROUTING_CHANGE:
                    /* build <set stream path> message*/
                    msgBuf[1] = CEC_MESSAGE_SET_STREAM_PATH;
                    msgBuf[2] =  msgBuf[len - 2];
                    msgBuf[3] =  msgBuf[len - 1];
                    msgBuf[len - 2] &= 0x0;
                    msgBuf[len - 1] &= 0x0;
                    ret = true;
                    break;
                case CEC_MESSAGE_USER_CONTROL_PRESSED:
                    if (msgBuf[2] == CEC_KEYCODE_POWER
                    || msgBuf[2] == CEC_KEYCODE_ROOT_MENU
                    || msgBuf[2] == CEC_KEYCODE_POWER_ON_FUNCTION) {
                        ret = true;
                    }
                    break;
                default:
                    break;
            }
        } else if (mCecDevice.mDeviceTypes[index] == DEV_TYPE_TV) {
            switch (msgBuf[1]) {
                case CEC_MESSAGE_IMAGE_VIEW_ON:
                case CEC_MESSAGE_TEXT_VIEW_ON:
                    ret = true;
                    break;
                default:
                    break;
            }
        } else if (mCecDevice.mDeviceTypes[index] == DEV_TYPE_AUDIO_SYSTEM) {
            switch (msgBuf[1]) {
                case CEC_MESSAGE_SYSTEM_AUDIO_MODE_REQUEST:
                case CEC_MESSAGE_USER_CONTROL_PRESSED:
                    ret = true;
                    break;
                default:
                    break;
            }
        }
    }
    return ret;
}

/**
 * Check if received a valid message.
* @param msgBuf is a message Buf
*   msgBuf[1]: message type
*   msgBuf[2]-msgBuf[n]: message para
* @param len is message lenth
* @param deviceType is type of device
*/

bool HdmiCecControl::messageValidate(hdmi_cec_event_t* event)
{
    bool ret = true;
    cec_message_t message;
    int initiator = (int)(event->cec.initiator);
    int devPhyAddr = 0;
    if (mCecDevice.isTvDeviceType) {
        switch (event->cec.body[0]) {
            case CEC_MESSAGE_REPORT_PHYSICAL_ADDRESS:
                devPhyAddr = ((event->cec.body[1] & 0xff) << 8) +  (event->cec.body[2] & 0xff);
                if (event->cec.body[1] == 0) {
                    message.initiator = (cec_logical_address_t)DEV_TYPE_TV;
                    message.destination = (cec_logical_address_t)initiator;
                    message.body[0] = CEC_MESSAGE_GIVE_PHYSICAL_ADDRESS;
                    message.length = 1;
                    sendMessage(&message, false);
                    ret = false;
                } else {
                    mCecDevice.mAddedPhyAddrs[initiator] = devPhyAddr;
                }
                break;
            default:
                break;
        }
    }
    return ret;
}

void HdmiCecControl::checkConnectStatus()
{
    unsigned int index, prevStatus, bit;
    int i, port, connect, ret;
    hdmi_cec_event_t event;

    prevStatus = mCecDevice.mConnectStatus;
    for (i = 0; i < mCecDevice.mTotalPort && mCecDevice.mpPortData != NULL; i++) {
        port = mCecDevice.mpPortData[i].port_id;
        if (mCecDevice.mTotalPort == 1 && mCecDevice.mDeviceTypes[0] == DEV_TYPE_PLAYBACK) {
            //playback for tx hotplug para is always 0
            port = 0;
        }
        connect = port;
        ret = ioctl(mCecDevice.mFd, CEC_IOC_GET_CONNECT_STATUS, &connect);
        if (ret) {
            ALOGE("[hcc] get port %d connected status failed, ret:%d\n", mCecDevice.mpPortData[i].port_id, ret);
            continue;
        }
        bit = prevStatus & (1 << port);
        if (bit ^ ((connect ? 1 : 0) << port)) {//connect status has changed
            ALOGD("[hcc] port:%d, connect status changed, now:%d, prevStatus:%x\n",
                    mCecDevice.mpPortData[i].port_id, connect, prevStatus);
            if (mEventListener != NULL && mCecDevice.isCecEnabled && mCecDevice.isCecControlled) {
                event.eventType = HDMI_EVENT_HOT_PLUG;
                event.hotplug.connected = connect;
                event.hotplug.port_id = mCecDevice.mpPortData[i].port_id;
                mEventListener->onEventUpdate(&event);
            }
            if (connect == 0) {
                for (index = 0; index < ADDR_BROADCAST; index++) {
                    if (mCecDevice.mAddedPhyAddrs[index] == mCecDevice.mpPortData[i].physical_address) {
                        mCecDevice.mAddedPhyAddrs[index] = 0;
                        ALOGD("[hcc] mCecDevice.mAddedPhyAddrs[%d]: set zero.", index);
                        break;
                    }
                }
            }
            prevStatus &= ~(bit);
            prevStatus |= ((connect ? 1 : 0) << port);
            //ALOGD("[hcc] now mask:%x\n", prevStatus);
        }
    }
    mCecDevice.mConnectStatus = prevStatus;
}

int HdmiCecControl::readMessage(unsigned char *buf, int msgCount)
{
    if (msgCount <= 0 || !buf) {
        return 0;
    }

    int ret = -1;
    /* maybe blocked at driver */
    ret = read(mCecDevice.mFd, buf, msgCount);
    if (ret < 0) {
        ALOGE("[hcc] read :%s failed, ret:%d\n", CEC_FILE, ret);
        return -1;
    }

    return ret;
}

void HdmiCecControl::getPortInfos(hdmi_port_info_t* list[], int* total)
{
    if (assertHdmiCecDevice())
        return;

    ioctl(mCecDevice.mFd, CEC_IOC_GET_PORT_NUM, total);

    ALOGD("[hcc] total port:%d", *total);
    if (*total > MAX_PORT)
        *total = MAX_PORT;

    if (NULL != mCecDevice.mpPortData)
        delete mCecDevice.mpPortData;
    mCecDevice.mpPortData = new hdmi_port_info[*total];
    if (!mCecDevice.mpPortData) {
        ALOGE("[hcc] alloc port_data failed");
        *total = 0;
        return;
    }

    ioctl(mCecDevice.mFd, CEC_IOC_GET_PORT_INFO, mCecDevice.mpPortData);

    for (int i = 0; i < *total; i++) {
        ALOGD("[hcc] portId: %d, type:%s, cec support:%d, arc support:%d, physical address:%x",
                mCecDevice.mpPortData[i].port_id,
                mCecDevice.mpPortData[i].type ? "output" : "input",
                mCecDevice.mpPortData[i].cec_supported,
                mCecDevice.mpPortData[i].arc_supported,
                mCecDevice.mpPortData[i].physical_address);
    }

    *list = mCecDevice.mpPortData;
    mCecDevice.mTotalPort = *total;
}

int HdmiCecControl::addLogicalAddress(cec_logical_address_t address)
{
    if (assertHdmiCecDevice())
        return -EINVAL;

    if (address < CEC_ADDR_BROADCAST)
        mCecDevice.mAddrBitmap |= (1 << address);

    if (!mCecDevice.isTvDeviceType && mCecDevice.mExtendControl) {
        mCecDevice.mExtendControl |= (0x02);
        if (mEventListener != NULL) {
            hdmi_cec_event_t event;
            event.eventType = HDMI_EVENT_ADD_LOGICAL_ADDRESS;
            event.logicalAddress = address;
            mEventListener->onEventUpdate(&event);
        }
    }
    ALOGD("[hcc] addr:%x, bitmap:%x\n", address, mCecDevice.mAddrBitmap);
    return ioctl(mCecDevice.mFd, CEC_IOC_ADD_LOGICAL_ADDR, address);
}

void HdmiCecControl::clearLogicaladdress()
{
    if (assertHdmiCecDevice())
        return;

    mCecDevice.mAddrBitmap = (1 << CEC_ADDR_BROADCAST);
    ALOGD("bitmap:%x", mCecDevice.mAddrBitmap);
    if (mCecDevice.mExtendControl) {
        mCecDevice.mExtendControl &= ~(0x02);
    }

    ioctl(mCecDevice.mFd, CEC_IOC_CLR_LOGICAL_ADDR, 0);
}

void HdmiCecControl::setOption(int flag, int value)
{
    if (assertHdmiCecDevice())
        return;

    int ret = -1;
    switch (flag) {
        case HDMI_OPTION_ENABLE_CEC:
            //ret = ioctl(mCecDevice.mFd, CEC_IOC_SET_OPTION_ENALBE_CEC, value);
            mCecDevice.isCecEnabled = (value == 1) ? true : false;
            if (!mCecDevice.isCecEnabled) {
                mSystemControl->writeSysfs(HDMIRX_SYSFS, CEC_STATE_UNABLED);
            }
            break;

        case HDMI_OPTION_WAKEUP:
            ret = ioctl(mCecDevice.mFd, CEC_IOC_SET_OPTION_WAKEUP, value);
            break;

        case HDMI_OPTION_SYSTEM_CEC_CONTROL:
            ret = ioctl(mCecDevice.mFd, CEC_IOC_SET_OPTION_SYS_CTRL, value);
            mCecDevice.isCecEnabled = (value == 1 && !mCecDevice.isCecEnabled) ? true : mCecDevice.isCecEnabled;
            mCecDevice.isCecControlled = (value == 1) ? true : false;
            if (mFirstEnableCec && mCecDevice.isCecControlled)  {
                mSystemControl->writeSysfs(HDMIRX_SYSFS, CEC_STATE_BOOT_ENABLED);
            } else if (mCecDevice.isCecEnabled) {
                mSystemControl->writeSysfs(HDMIRX_SYSFS, CEC_STATE_ENABLED);
            } else {
                mSystemControl->writeSysfs(HDMIRX_SYSFS, CEC_STATE_UNABLED);
            }
            mFirstEnableCec = false;
            break;

        /* set device auto-power off by uboot */
        case HDMI_OPTION_CEC_AUTO_DEVICE_OFF:
            ret = ioctl(mCecDevice.mFd, CEC_IOC_SET_AUTO_DEVICE_OFF, value);
            break;

        case HDMI_OPTION_SET_LANG:
            ret = ioctl(mCecDevice.mFd, CEC_IOC_SET_OPTION_SET_LANG, value);
            break;

        default:
            break;
    }
    ALOGD("[hcc] %s, flag:0x%x, value:0x%x, ret:%d, isCecControlled:%x", __FUNCTION__, flag, value, ret, mCecDevice.isCecControlled);
}

void HdmiCecControl::setAudioReturnChannel(int port, bool flag)
{
    if (assertHdmiCecDevice())
        return;

    int ret = ioctl(mCecDevice.mFd, CEC_IOC_SET_ARC_ENABLE, flag);
    ALOGD("[hcc] %s, port id:%d, flag:%x, ret:%d\n", __FUNCTION__, port, flag, ret);
}

bool HdmiCecControl::isConnected(int port)
{
    if (assertHdmiCecDevice())
        return false;

    int status = -1, ret;
    /* use status pass port id */
    status = port;
    ret = ioctl(mCecDevice.mFd, CEC_IOC_GET_CONNECT_STATUS, &status);
    if (ret)
        return false;
    ALOGD("[hcc] %s, port:%d, connected:%s", __FUNCTION__, port, status ? "yes" : "no");
    return status;
}

int HdmiCecControl::getVersion(int* version)
{
    if (assertHdmiCecDevice())
        return -1;

    int ret = ioctl(mCecDevice.mFd, CEC_IOC_GET_VERSION, version);
    ALOGD("[hcc] %s, version:%x, ret = %d", __FUNCTION__, *version, ret);
    return ret;
}

int HdmiCecControl::getVendorId(uint32_t* vendorId)
{
    if (assertHdmiCecDevice())
        return -1;

    int ret = ioctl(mCecDevice.mFd, CEC_IOC_GET_VENDOR_ID, vendorId);
    ALOGD("[hcc] %s, vendorId: %x, ret = %d", __FUNCTION__, *vendorId, ret);
    return ret;
}

int HdmiCecControl::getPhysicalAddress(uint16_t* addr)
{
    if (assertHdmiCecDevice())
        return -EINVAL;

    int ret = ioctl(mCecDevice.mFd, CEC_IOC_GET_PHYSICAL_ADDR, addr);
    ALOGD("[hcc] %s, physical addr: %x, ret = %d", __FUNCTION__, *addr, ret);
    return ret;
}

int HdmiCecControl::sendMessage(const cec_message_t* message, bool isExtend)
{
    if (assertHdmiCecDevice())
        return -EINVAL;

    if (isExtend) {
        ALOGD("[hcc] isExtend = %d, mExtendControl = %d", isExtend, mCecDevice.mExtendControl);
        return sendExtMessage(message);
    }
    /* don't send message if controlled by extend */
    if (mCecDevice.mExtendControl == 0x03 && hasHandledByExtend(message)) {
        return HDMI_RESULT_SUCCESS;
    }
    if (preHandleBeforeSend(message) < 0) {
        return HDMI_RESULT_SUCCESS;
    }
    return send(message);
}

int HdmiCecControl::preHandleBeforeSend(const cec_message_t* message)
{
    int ret = 0;
    int opcode, para, value, avrAddr, index;
    hdmi_cec_event_t event;
    opcode = message->body[0] & 0xff;
    switch (opcode) {
        case CEC_MESSAGE_ROUTING_CHANGE:
        case CEC_MESSAGE_SET_STREAM_PATH:
            if (opcode == CEC_MESSAGE_ROUTING_CHANGE)
                para = ((message->body[3] & 0xff) << 8) + (message->body[4] & 0xff);
            else if (opcode == CEC_MESSAGE_SET_STREAM_PATH) {
                para = ((message->body[1] & 0xff) << 8) + (message->body[2] & 0xff);
            }
            avrAddr = mCecDevice.mAddedPhyAddrs[DEV_TYPE_AUDIO_SYSTEM];
            if (avrAddr == para) {
                ALOGD("[hcc] tv should not send Set Stream Path & Routing Change to avr which does not have sub device and connect to arc port.");
                for (index = 0; index < mCecDevice.mTotalPort && mCecDevice.mpPortData != NULL; index++) {
                    if (mCecDevice.mpPortData[index].physical_address == avrAddr && mCecDevice.mpPortData[index].arc_supported) {
                        ret = -1;
                        ALOGD("[hcc] Avr connect to arc port.");
                        break;
                    }
                }
                for (index = 0; index < ADDR_BROADCAST; index++) {
                    value = mCecDevice.mAddedPhyAddrs[index];
                    if ((index != DEV_TYPE_AUDIO_SYSTEM) && (value - avrAddr > 0) && ((value & 0xf000) == (avrAddr & 0xf000))) {
                        ret = 0;
                         ALOGD("[hcc] Avr has sub device.");
                        break;
                    }
                }
            }
            break;
        default:
            break;
    }
    return ret;
}

bool HdmiCecControl::hasHandledByExtend(const cec_message_t* message)
{
    bool ret = false;
    int opcode = message->body[0] & 0xff;
    switch (opcode) {
        case CEC_MESSAGE_SET_MENU_LANGUAGE:
        case CEC_MESSAGE_INACTIVE_SOURCE:
            ret = true;
            break;
        default:
            break;
    }
    return ret;
}

int HdmiCecControl::sendExtMessage(const cec_message_t* message)
{
    int i, addr;
    cec_message_t msg;
    if (!mCecDevice.isTvDeviceType) {
        addr = mCecDevice.mAddrBitmap;
        addr &= 0x7fff;
        for (i = 0; i < 15; i++) {
            if (addr & 1) {
                break;
            }
            addr >>= 1;
        }
        msg.initiator = (cec_logical_address_t) i;
    } else {
        msg.initiator = CEC_ADDR_TV; /* root for TV */
    }
    if (msg.initiator == CEC_ADDR_UNREGISTERED)//source can not be 0xf
        return -1;
    msg.destination = message->destination;
    msg.length = message->length;
    memcpy(msg.body, message->body, msg.length);
    return send(&msg);
}

int HdmiCecControl::send(const cec_message_t* message)
{
    unsigned char msgBuf[CEC_MESSAGE_BODY_MAX_LENGTH];
    int ret = -1;

    memset(msgBuf, 0, sizeof(msgBuf));
    msgBuf[0] = ((message->initiator & 0xf) << 4) | (message->destination & 0xf);
    memcpy(msgBuf + 1, message->body, message->length);
    ret = write(mCecDevice.mFd, msgBuf, message->length + 1);
    printCecMessage(message, ret);
    return ret;
}

bool HdmiCecControl::assertHdmiCecDevice()
{
    return mCecDevice.mFd < 0;
}

void HdmiCecControl::setEventObserver(const sp<HdmiCecEventListener> &eventListener)
{
    mEventListener = eventListener;
}

};//namespace android
