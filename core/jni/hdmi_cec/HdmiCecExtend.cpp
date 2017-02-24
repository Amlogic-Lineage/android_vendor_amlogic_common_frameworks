#define LOG_NDEBUG 0
#define LOG_TAG "JHdmiCecExtend"

#include <jni.h>
#include <JNIHelp.h>
#include <ScopedPrimitiveArray.h>
#include <sys/param.h>
#include <android_runtime/AndroidRuntime.h>

#include <HdmiCecBase.h>
#include <HdmiCecClient.h>

namespace android {

#define FIND_CLASS(var, className) \
        var = env->FindClass(className); \
        LOG_FATAL_IF(! var, "Unable to find class " className)

#define GET_METHOD_ID(var, clazz, methodName, methodDescriptor) \
        var = env->GetMethodID(clazz, methodName, methodDescriptor); \
        LOG_FATAL_IF(! var, "Unable to find method " methodName);

static struct {
    jmethodID onCecMessageRx;
    jmethodID onAddAddress;
} gHdmiCecExtendClassInfo;

class JHdmiCecExtend : public HdmiCecBase, public HdmiCecEventListener {
public:
    JHdmiCecExtend(jobject callbacksObj);
    ~JHdmiCecExtend();

    void init();

    virtual int getVersion(int* version);
    virtual int getVendorId(uint32_t* vendorId);
    virtual int getPhysicalAddress(uint16_t* addr);
    virtual int sendMessage(const cec_message_t* message, bool isExtend);

    virtual int openCecDevice() {return -1;}
    virtual int closeCecDevice() {return -1;}
    virtual void getPortInfos(hdmi_port_info_t* list[] __unused, int* total __unused) {}
    virtual int addLogicalAddress(cec_logical_address_t address __unused) {return -1;}
    virtual void clearLogicaladdress() {}
    virtual void setOption(int flag __unused, int value __unused) {}
    virtual void setAudioReturnChannel(int port __unused, bool flag __unused) {}
    virtual bool isConnected(int port __unused) {return false;}

    virtual void onEventUpdate(const hdmi_cec_event_t* event);

private:
    sp<HdmiCecClient> mHdmiCecClient;
    jobject mCallbacksObj;
};

JHdmiCecExtend::JHdmiCecExtend(jobject callbacksObj) :
        mCallbacksObj(callbacksObj) {
    mHdmiCecClient = HdmiCecClient::connect();
    mHdmiCecClient->setEventObserver(this);
}

JHdmiCecExtend::~JHdmiCecExtend() {
    mHdmiCecClient.clear();
}

void JHdmiCecExtend::init() {
}

int JHdmiCecExtend::getPhysicalAddress(uint16_t* addr) {
    if (mHdmiCecClient != NULL)
        return mHdmiCecClient->getPhysicalAddress(addr);
    return 0;
}

int JHdmiCecExtend::getVendorId(uint32_t* vendorId) {
    if (mHdmiCecClient != NULL)
        return mHdmiCecClient->getVendorId(vendorId);
    return 0;
}

int JHdmiCecExtend::getVersion(int* version) {
    if (mHdmiCecClient != NULL)
        return mHdmiCecClient->getVersion(version);
    return 0;
}

int JHdmiCecExtend::sendMessage(const cec_message_t* message, bool isExtend) {
    if (mHdmiCecClient != NULL)
        return mHdmiCecClient->sendMessage(message, isExtend);
    return 0;
}

void JHdmiCecExtend::onEventUpdate(const hdmi_cec_event_t* event)
{
    printCecEvent(event);

    JNIEnv* env = AndroidRuntime::getJNIEnv();
    if ((event->eventType & HDMI_EVENT_RECEIVE_MESSAGE) != 0) {
        char msg_buf[CEC_MESSAGE_BODY_MAX_LENGTH];
        memset(msg_buf, 0, sizeof(msg_buf));
        memcpy(msg_buf + 1, event->cec.body, event->cec.length);
        msg_buf[0] = ((event->cec.initiator << 4) & 0x0f) | (event->cec.destination & 0x0f);
        printCecMsgBuf(msg_buf, event->cec.length);

        jbyteArray array = env->NewByteArray(event->cec.length + 1);
        const jbyte* bodyPtr = reinterpret_cast<const jbyte *>(msg_buf);
        env->SetByteArrayRegion(array, 0, event->cec.length + 1, bodyPtr);
        env->CallVoidMethod(mCallbacksObj, gHdmiCecExtendClassInfo.onCecMessageRx, array);
        env->DeleteLocalRef(array);
    } else if ((event->eventType & HDMI_EVENT_ADD_LOGICAL_ADDRESS) != 0) {
        env->CallVoidMethod(mCallbacksObj, gHdmiCecExtendClassInfo.onAddAddress, event->logicalAddress);
    }
}

//----------------------------------------------------------------------------------------

static jint nativeSendMessage(JNIEnv* env, jclass clazz __unused, jlong extendPtr, jint dest, jbyteArray body)
{
    JHdmiCecExtend* extend = reinterpret_cast<JHdmiCecExtend*>(extendPtr);
    cec_message_t message;

    message.destination = static_cast<cec_logical_address_t>(dest);

    jsize len = env->GetArrayLength(body);
    message.length = MIN(len, CEC_MESSAGE_BODY_MAX_LENGTH);

    ScopedByteArrayRO bodyPtr(env, body);
    std::memcpy(message.body, bodyPtr.get(), message.length);

    return extend->sendMessage(&message, true);
}

static jlong nativeInit(JNIEnv *env, jobject thiz __unused, jobject obj)
{
    JHdmiCecExtend *extend = new JHdmiCecExtend(env->NewGlobalRef(obj));
    extend->init();
    return reinterpret_cast<jlong>(extend);
}

static jint nativeGetPhysicalAddress(JNIEnv* env __unused, jclass clazz __unused, jlong extendPtr)
{
    JHdmiCecExtend* extend = reinterpret_cast<JHdmiCecExtend*>(extendPtr);
    unsigned short addr = -1;
    extend->getPhysicalAddress(&addr);
    return addr;
}

static jint nativeGetVendorId(JNIEnv* env __unused, jclass clazz __unused, jlong extendPtr)
{
    JHdmiCecExtend* extend = reinterpret_cast<JHdmiCecExtend*>(extendPtr);
    unsigned int id = 0;
    extend->getVendorId(&id);
    return id;
}

static jint nativeGetVersion(JNIEnv* env __unused, jclass clazz __unused, jlong extendPtr)
{
    JHdmiCecExtend* extend = reinterpret_cast<JHdmiCecExtend*>(extendPtr);
    int version = 0;
    extend->getVersion(&version);
    return version;
}

static JNINativeMethod hdmiExtend_method[] = {
    {"nativeSendCecMessage", "(JI[B)I", (void *)nativeSendMessage},
    {"nativeInit", "(Lcom/droidlogic/HdmiCecExtend;)J", (void *)nativeInit},
    {"nativeGetPhysicalAddr", "(J)I", (void *)nativeGetPhysicalAddress},
    {"nativeGetVendorId", "(J)I", (void *)nativeGetVendorId},
    {"nativeGetCecVersion", "(J)I", (void *)nativeGetVersion},
};

#define CLASS_PATH "com/droidlogic/HdmiCecExtend"

int register_droidlogic_HdmiCecExtend(JNIEnv* env) {
    int res = jniRegisterNativeMethods(env, CLASS_PATH, hdmiExtend_method, NELEM(hdmiExtend_method));

    jclass clazz;
    FIND_CLASS(clazz, CLASS_PATH);

    GET_METHOD_ID(gHdmiCecExtendClassInfo.onCecMessageRx, clazz, "onCecMessageRx", "([B)V");
    GET_METHOD_ID(gHdmiCecExtendClassInfo.onAddAddress, clazz, "onAddAddress", "(I)V");
    return res;
}

}//end namespace android

using namespace android;

extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved __unused)
{
    JNIEnv* env = NULL;
    jint result = -1;

    LOGD("load hdmi cec extend jni...\n");

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGD("GetEnv failed!\n");
        return result;
    }

    int res = register_droidlogic_HdmiCecExtend(env);
    LOG_FATAL_IF(res < 0, "Unable to register native methods.");

    return JNI_VERSION_1_4;
}

