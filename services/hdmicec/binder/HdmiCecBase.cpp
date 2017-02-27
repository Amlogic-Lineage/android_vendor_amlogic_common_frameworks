#define LOG_NDEBUG 0
#define LOG_CEE_TAG "HdmiCecBase"

#include "HdmiCecBase.h"

int mLogLevel = LOG_LEVEL_1;

namespace android {

void HdmiCecBase::printCecMsgBuf(const char *msg_buf, int len)
{
    if (mLogLevel < LOG_LEVEL_2)
        return;

    char buf[64] = { };
    int i, size = 0;
    memset(buf, 0, sizeof(buf));
    for (i = 0; i < len; i++) {
        size += sprintf(buf + size, " %02x", msg_buf[i]);
    }
    LOGD("%s, msg:%s", __FUNCTION__, buf);
}

void HdmiCecBase::printCecEvent(const hdmi_cec_event_t *event)
{
    if (mLogLevel < LOG_LEVEL_2)
        return;

    if (((event->eventType & HDMI_EVENT_CEC_MESSAGE) != 0)
            || ((event->eventType & HDMI_EVENT_RECEIVE_MESSAGE) != 0)) {
        LOGD("%s, eventType: %d", __FUNCTION__, event->eventType);
        printCecMessage(&event->cec);
    } else if ((event->eventType & HDMI_EVENT_HOT_PLUG) != 0) {
        LOGD("%s, hotplug, connected:%d, port_id:%d", __FUNCTION__, event->hotplug.connected,
                event->hotplug.port_id);
    } else if ((event->eventType & HDMI_EVENT_ADD_LOGICAL_ADDRESS) != 0) {
        LOGD("%s, add logical address, logicalAddress:%x", __FUNCTION__, event->logicalAddress);
    }
}

void HdmiCecBase::printCecMessage(const cec_message_t* message)
{
    if (mLogLevel < LOG_LEVEL_2)
        return;

    char buf[64];
    int i, size = 0;
    memset(buf, 0, sizeof(buf));
    for (i = 0; i < message->length; i++) {
        size += sprintf(buf + size, " %02x", message->body[i]);
    }
    LOGD("%s, [%x -> %x] len: %d, body:%s", __FUNCTION__, message->initiator, message->destination,
            message->length, buf);
}

void HdmiCecBase::printCecMessage(const cec_message_t* message, int result)
{
    if (mLogLevel < LOG_LEVEL_2)
        return;

    char buf[64];
    int i, size = 0;
    memset(buf, 0, sizeof(buf));
    for (i = 0; i < message->length; i++) {
        size += sprintf(buf + size, " %02x", message->body[i]);
    }
    LOGD("%s, [%x -> %x] len: %d, body:%s, result: %s", __FUNCTION__,
            message->initiator, message->destination, message->length, buf, getResult(result));
}

const char *HdmiCecBase::getResult(int result)
{
    switch (result) {
        case HDMI_RESULT_SUCCESS:
            return "success";
        case HDMI_RESULT_NACK:
            return "no ack";
        case HDMI_RESULT_BUSY:
            return "busy";
        case HDMI_RESULT_FAIL:
            return "fail other";
        default:
            return "unknown fail code";
    }
}

const char* HdmiCecBase::getEventType(int eventType)
{
    switch (eventType) {
        case HDMI_EVENT_CEC_MESSAGE:
            return "cec message";
        case HDMI_EVENT_HOT_PLUG:
            return "hotplug message";
        case HDMI_EVENT_ADD_LOGICAL_ADDRESS:
            return "add logical address for extend";
        case HDMI_EVENT_RECEIVE_MESSAGE:
            return "cec message for extend";
        case (HDMI_EVENT_CEC_MESSAGE | HDMI_EVENT_RECEIVE_MESSAGE):
            return "cec message for system and extend";
        default:
            return "unknown message";
    }
}

void HdmiCecBase::setLogLevel(int level)
{
    mLogLevel = level;
}

int HdmiCecBase::getLogLevel()
{
    return mLogLevel;
}

int __unit_log_print(int prio, const char *tag, const char *cec_tag, const char *fmt, ...)
{
    char buf[DEFAULT_LOG_BUFFER_LEN];
    sprintf(buf, "[%s]:", cec_tag);
    int cec_tag_len = strlen(buf);

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf + cec_tag_len, DEFAULT_LOG_BUFFER_LEN - cec_tag_len, fmt, ap);

    return __android_log_write(prio, tag, buf);
}

};//namespace android
