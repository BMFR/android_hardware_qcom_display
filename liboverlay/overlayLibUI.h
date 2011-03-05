/*
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (c) 2011, Code Aurora Forum. All rights reserved.
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
 */

#ifndef INCLUDE_OVERLAY_LIB_UI
#define INCLUDE_OVERLAY_LIB_UI

#include <errno.h>

#include "overlayLib.h"

namespace overlay {

enum channel_state_t { UP, CLOSED, PENDING_CLOSE };
enum status_t {
                  NO_ERROR,
                  INVALID_OPERATION = -ENOSYS,
                  BAD_VALUE = -EINVAL,
                  NO_INIT = -ENODEV,
                  ALREADY_EXISTS = -EEXIST
              };

/*
 * Display class provides following services
 * Open FB
 * FB information (Width, Height and Bpp)
 */

class Display {
    int mFD;
    int mFBWidth;
    int mFBHeight;
    int mFBBpp;
    Display(const Display& objDisplay);
    Display& operator=(const Display& objDisplay);

public:
    explicit Display() : mFD(NO_INIT) { };
    ~Display() { close(mFD); };
    int getFD() const { return mFD; };
    int getFBWidth() const { return mFBWidth; };
    int getFBHeight() const { return mFBHeight; };
    int getFBBpp() const { return mFBBpp; };
    status_t openDisplay(int fbnum);
    void closeDisplay() { close(mFD); mFD = NO_INIT; };
};

/*
 * OVHelper class, provides apis related to Overlay
 * It communicates with MDP driver, provides following services
 * Start overlay session
 * Set position of the destination on to display
 */

class OVHelper {
    int mSessionID;
    Display mobjDisplay;
    mdp_overlay mOVInfo;
    OVHelper(const OVHelper& objOVHelper);
    OVHelper& operator=(const OVHelper& objOVHelper);

public:
    explicit OVHelper() : mSessionID(NO_INIT) { };
    ~OVHelper() { closeOVSession(); };
    status_t startOVSession(mdp_overlay& ovInfo, int fbnum);
    status_t closeOVSession();
    status_t queueBuffer(msmfb_overlay_data ovData);
    int getFBWidth() const { return mobjDisplay.getFBWidth(); };
    int getFBHeight() const { return mobjDisplay.getFBHeight(); };
    int getFBBpp() const { return mobjDisplay.getFBBpp(); };
    status_t setPosition(int x, int y, int w, int h);
    status_t getOVInfo(mdp_overlay& ovInfo);
};

/*
 * Rotator class, manages rotation of the buffers
 * It communicates with Rotator driver, provides following services
 * Start rotator session
 * Rotate buffer
 */

class Rotator {
    int mFD;
    int mSessionID;
    int mPmemFD;
    void* mPmemAddr;
    int mRotOffset[max_num_buffers];
    int mCurrentItem;
    int mNumBuffers;
    int mSize;
    Rotator(const Rotator& objROtator);
    Rotator& operator=(const Rotator& objRotator);

public:
    explicit Rotator() : mFD(NO_INIT), mSessionID(NO_INIT), mPmemFD(-1) { };
    ~Rotator() { closeRotSession(); }
    status_t startRotSession(msm_rotator_img_info& rotInfo, int numBuffers = max_num_buffers);
    status_t closeRotSession();
    status_t rotateBuffer(msm_rotator_data_info& rotData);
};

/*
 * Overlay class for Comp. Bypass
 * We merge control and data channel classes.
 */

class OverlayUI {
    channel_state_t mChannelState;
    int mOrientation;
    int mFBNum;
    OVHelper mobjOVHelper;
    Rotator mobjRotator;

    OverlayUI(const OverlayUI& objOverlay);
    OverlayUI& operator=(const OverlayUI& objOverlay);

    status_t startChannel(int fbnum, mdp_overlay& ovInfo,
                               msm_rotator_img_info& rotInfo);
public:

    enum fbnum_t { FB0, FB1 };

    explicit OverlayUI() : mChannelState(CLOSED), mOrientation(NO_INIT), mFBNum(NO_INIT) { };
    ~OverlayUI() { closeChannel(); };
    status_t setSource(int w, int h, int format, int orientation,
                          bool useVGPipe = false, bool ignoreFB = true,
                          int fbnum = FB0, int zorder = NO_INIT);
    status_t setPosition(int x, int y, int w, int h) {
        return mobjOVHelper.setPosition(x, y, w, h);
    };
    status_t closeChannel();
    channel_state_t isChannelUP() const { return mChannelState; };
    int getFBWidth() const { return mobjOVHelper.getFBWidth(); };
    int getFBHeight() const { return mobjOVHelper.getFBHeight(); };
    status_t queueBuffer(buffer_handle_t buffer);
};

};
#endif
