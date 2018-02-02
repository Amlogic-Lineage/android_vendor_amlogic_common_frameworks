/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 *  @author   Tellen Yu
 *  @version  2.0
 *  @date     2015/06/18
 *  @par function description:
 *  - 1 show picture in video layer
 */

#ifndef ANDROID_IMAGEPLAYERSERVICE_H
#define ANDROID_IMAGEPLAYERSERVICE_H

#include <utils/KeyedVector.h>
#include <utils/String8.h>
#include <utils/String16.h>
#include <utils/Vector.h>
//#include <utils/threads.h>
//#include <utils/Timers.h>
//#include <utils/RefBase.h>
#include <media/MediaPlayerInterface.h>
#include <SkBitmap.h>
#include <SkStream.h>
#include <SkMovie.h>
//#include <binder/MemoryDealer.h>
#include <IImagePlayerService.h>
#include "ISystemControlService.h"
#include <binder/Binder.h>

#define MAX_FILE_PATH_LEN           1024
#define MAX_PIC_SIZE                8000

namespace android {

class MovieThread;
class DeathNotifier;

typedef struct {
    char* pBuff;
    int frame_width;
    int frame_height;
    int format;
    int rotate;
}FrameInfo_t;

struct InitParameter {
    float degrees;
    float scaleX;
    float scaleY;
    int cropX;
    int cropY;
    int cropWidth;
    int cropHeight;
};

enum RetType {
    RET_OK_OPERATION_SCALE_MIN      = 2,
    RET_OK_OPERATION_SCALE_MAX      = 1,
    RET_OK                          = 0,
    RET_ERR_OPEN_SYSFS              = -1,
    RET_ERR_OPEN_FILE               = -2,
    RET_ERR_INVALID_OPERATION       = -3,
    RET_ERR_DECORDER                = -4,
    RET_ERR_PARAMETER               = -5,
    RET_ERR_BAD_VALUE               = -6,
    RET_ERR_NO_MEMORY               = -7
};

enum ScaleDirect {
    SCALE_NORMAL                    = 0,
    SCALE_UP                        = 1,
    SCALE_DOWN                      = 2,
};

enum TranslateDirect {
    TRANSLATE_NORMAL                    = 0,
    TRANSLATE_LEFT                      = 1,
    TRANSLATE_RIGHT                     = 2,
    TRANSLATE_UP                        = 3,
    TRANSLATE_DOWN                      = 4,
    TRANSLATE_LEFTUP                    = 5,
    TRANSLATE_LEFTDOWN                  = 6,
    TRANSLATE_RIGHTUP                   = 7,
    TRANSLATE_RIGHTDOWN                 = 8,
};

/*
enum ParameterKey {
    KEY_PARAMETER_SET_IMAGE_SAMPLESIZE_SURFACESIZE,
    KEY_PARAMETER_ROTATE,
    KEY_PARAMETER_SCALE,
    KEY_PARAMETER_ROTATE_SCALE,
    KEY_PARAMETER_CROP_RECT,
    KEY_PARAMETER_DECODE_NEXT,
    KEY_PARAMETER_SHOW_NEXT
};
*/

class ImagePlayerService :  public BnImagePlayerService {
  public:
    ImagePlayerService();
    virtual ~ImagePlayerService();

    virtual int init();
    virtual int setDataSource(const char* uri);
    virtual int setDataSource (
            const sp<IMediaHTTPService> &httpService,
            const char *srcUrl);
    virtual int setDataSource(int fd, int64_t offset, int64_t length);
    virtual int setSampleSurfaceSize(int sampleSize, int surfaceW, int surfaceH);
    virtual int setRotate(float degrees, int autoCrop) ;
    virtual int setScale(float sx, float sy, int autoCrop);
    virtual int setHWScale(float sc);
    virtual int setTranslate(float tx, float ty);
    virtual int setRotateScale(float degrees, float sx, float sy, int autoCrop);
    virtual int setCropRect(int cropX, int cropY, int cropWidth, int cropHeight);
    virtual int prepareBuf(const char *uri);
    virtual int showBuf();
    virtual int start();
    virtual int prepare();
    virtual int show();
    virtual int release();
    static void instantiate();
    virtual int notifyProcessDied(const sp<IBinder> &binder);

    //use to show gif etc. images
    bool MovieInit(const char path[]);
    bool MovieShow();
    void MovieRenderPost(SkBitmap *bitmap);
    int MovieThreadStart();
    int MovieThreadStop();

    virtual status_t dump(int fd, const Vector<String16>& args);

  private:
    void initVideoAxis();
    int convertRGBA8888toRGB(void *dst, const SkBitmap *src);
    int convertARGB8888toYUYV(void *dst, const SkBitmap *src);
    int convertRGB565toYUYV(void *dst, const SkBitmap *src);
    int convertIndex8toYUYV(void *dst, const SkBitmap *src);

    int post();
    int render(int format, SkBitmap *bitmap);
    SkBitmap* decode(SkStreamRewindable *stream, InitParameter *parameter);
    SkBitmap* decodeTiff(const char *filePath);
    SkBitmap* scale(SkBitmap *srcBitmap, float sx, float sy);
    SkBitmap* rotate(SkBitmap *srcBitmap, float degrees);
    SkBitmap* rotateAndScale(SkBitmap *srcBitmap, float degrees, float sx, float sy);
    bool renderAndShow(SkBitmap *bitmap);
    bool showBitmapRect(SkBitmap *bitmap, int cropX, int cropY, int cropWidth, int cropHeight);
    void resetRotateScale();
    void resetTranslate();
    void resetHWScale();
    void isTranslateToEdge(SkBitmap *srcBitmap, int dstWidth, int dstHeight, int tx, int ty);
    SkBitmap* scaleStep(SkBitmap *srcBitmap, float sx, float sy);
    SkBitmap* scaleAndCrop(SkBitmap *srcBitmap, float sx, float sy);
    SkBitmap* fillSurface(SkBitmap *bitmap);
    bool isSupportFromat(const char *uri, SkBitmap **bitmap);

    mutable Mutex mLock;
    int mWidth, mHeight;
    SkBitmap *mBitmap;
    SkBitmap *mBufBitmap;
    // sample-size, if set to > 1, tells the decoder to return a smaller than
    // original bitmap, sampling 1 pixel for every size pixels. e.g. if sample
    // size is set to 3, then the returned bitmap will be 1/3 as wide and high,
    // and will contain 1/9 as many pixels as the original.
    int mSampleSize;

    char mImageUrl[MAX_FILE_PATH_LEN];
    int mFileDescription;
    //bool isAutoCrop;
    int surfaceWidth, surfaceHeight;

    //0:normal 1: scale up 2:scale down
    int mScalingDirect;
    float mScalingStep;
    SkBitmap *mScalingBitmap;
    SkBitmap *mRotateBitmap;
    SkMovie *mSkMovie;
    bool mMovieImage;
    int mMovieTime;
    int mMovieDegree;
    bool mNeedResetHWScale;
    //0:normal 1: translate left 2:translate right 3: translate up 4:translate down
    int mTranslatingDirect;
    bool mTranslateImage;
    bool mTranslateToXLEdge;  //Translate To X Left Edge
    bool mTranslateToXREdge;  //Translate To X Right Edge
    bool mTranslateToYTEdge;  //Translate To Y Top Edge
    bool mTranslateToYBEdge;  //Translate To Y Bottom Edge
    int mTx;
    int mTy;
    float mMovieScale;
    sp<MovieThread> mMovieThread;

    //sp<ALooper> mLooper;

    InitParameter *mParameter;
    int mDisplayFd;

    sp<IMediaHTTPService> mHttpService;
    sp<DeathNotifier> mDeathNotifier;
    sp<ISystemControlService> mSystemControl;
};

class MovieThread : public Thread {
public:
    MovieThread(const sp<ImagePlayerService>& player);
    virtual ~MovieThread();

private:
    sp<ImagePlayerService> mPlayer;

    virtual status_t readyToRun();
    virtual bool threadLoop();
};

#if 0
struct MovieImageHandler : public AHandler {
    MovieImageHandler(const sp<ImagePlayerService>& player);

    void startServer(unsigned localPort);
    void startClient(const char *remoteHost, unsigned remotePort);

protected:
    virtual ~MovieImageHandler();

    virtual void onMessageReceived(const sp<AMessage> &msg);

private:
    enum {
        kWhatInit,
        kWhatShow,
        kWhatStop,
    };

    sp<ImagePlayerService> mPlayer;

    DISALLOW_EVIL_CONSTRUCTORS(MovieImageHandler);
};
#endif

}  // namespace android

#endif // ANDROID_IMAGEPLAYERSERVICE_H
