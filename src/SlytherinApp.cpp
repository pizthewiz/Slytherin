
#include "cinder/app/AppNative.h"
#include "cinder/gl/Fbo.h"
#include "cinder/Capture.h"

using namespace ci;
using namespace ci::app;
using namespace ci::gl;
using namespace std;

class SlytherinApp : public AppNative {
public:
    void prepareSettings(Settings* settings);
    void setup();
    void update();
    void draw();

private:
    CaptureRef mCapture;
    gl::Fbo mFBO;
    uint32_t mLastUpdateFrame;
    float mLinesPerFrame;
    uint32_t mLineIndex;
};

void SlytherinApp::prepareSettings(Settings* settings) {
    settings->enableHighDensityDisplay();
    settings->setWindowSize(640, 480);
    settings->prepareWindow(Window::Format().fullScreenButton());
}

void SlytherinApp::setup() {
    // setup webcam
    try {
        mCapture = Capture::create(640, 480);
        mCapture->start();
    } catch(...) {
        console() << "ERROR - failed to initialize capture" << endl;
        quit();
    }

    // setup webcam FBO
    gl::Fbo::Format format;
    format.enableColorBuffer(true);
    format.enableDepthBuffer(false);
    format.setWrap(GL_CLAMP, GL_CLAMP);

    mFBO = gl::Fbo(mCapture->getWidth(), mCapture->getHeight(), format);
    mFBO.bindFramebuffer();
        gl::setViewport(mFBO.getBounds());
        gl::clear();
    mFBO.unbindFramebuffer();

    setFrameRate(60.0f);

    mLastUpdateFrame = UINT32_MAX;
    mLinesPerFrame = 2.0f; // 1 line every 2 frames (at getFrameRate())
    mLineIndex = 0;
}

void SlytherinApp::update() {
    uint32_t elapsedFrames = getElapsedFrames();
    bool needsFrame = mLastUpdateFrame == UINT32_MAX || (elapsedFrames - mLastUpdateFrame) * mLinesPerFrame >= 1.0f;
    if (mCapture && needsFrame && mCapture->checkNewFrame()) {
        uint32_t lineCount = (uint32_t)floorf((elapsedFrames - mLastUpdateFrame) * mLinesPerFrame);

        Surface8u surface = mCapture->getSurface();
        Texture texture = Texture(surface);

        if (mLineIndex + lineCount < mFBO.getHeight()) {
            // single segment
//            mLineIndex to mLineIndex + lineCount
            console() << "process (" << mLineIndex << "-" << mLineIndex+lineCount << ") on frame " << elapsedFrames << endl;
            mLineIndex += lineCount;
        } else {
            // two segments
//            mLineIndex to mFBO.getHeight() - 1
            uint32_t overflowLineCount = mLineIndex + lineCount - mFBO.getHeight() + 1;
//            0 to overflowLineCount
            console() << "process (" << mLineIndex << "-" << mFBO.getHeight() - 1 << ") and (0-" << overflowLineCount << ") on frame " << elapsedFrames << endl;
            mLineIndex = overflowLineCount;
        }

        mLastUpdateFrame = elapsedFrames;
    }
}

void SlytherinApp::draw() {
    gl::clear(Color::black());

    // TODO FBO - shit
}

CINDER_APP_NATIVE(SlytherinApp, RendererGl)
