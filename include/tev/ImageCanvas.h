// This file was developed by Thomas Müller <thomas94@gmx.net>.
// It is published under the BSD 3-Clause License within the LICENSE file.

#pragma once

#include <tev/UberShader.h>
#include <tev/Image.h>
#include <tev/Lazy.h>

#include <nanogui/canvas.h>

#include <memory>

TEV_NAMESPACE_BEGIN

struct CanvasStatistics {
    float mean;
    float maximum;
    float minimum;
    std::vector<float> histogram;
    int nChannels;
    int histogramZero;
};

class ImageCanvas : public nanogui::Canvas {
public:
    ImageCanvas(nanogui::Widget* parent, float pixelRatio);

    bool scroll_event(const nanogui::Vector2i& p, const nanogui::Vector2f& rel) override;

    void draw_contents() override;

    void draw(NVGcontext *ctx) override;

    void translate(const nanogui::Vector2f& amount);
    void scale(float amount, const nanogui::Vector2f& origin);
    float extractScale() const {
        float det = mTransform.m[0][0] * mTransform.m[1][1] - mTransform.m[0][1] * mTransform.m[1][0];
        return std::sqrt(det);
    }

    void setExposure(float exposure) {
        mExposure = exposure;
    }

    void setOffset(float offset) {
        mOffset = offset;
    }

    void setGamma(float gamma) {
        mGamma = gamma;
    }

    float applyExposureAndOffset(float value) const;

    void setImage(std::shared_ptr<Image> image) {
        mImage = image;
    }

    void setReference(std::shared_ptr<Image> reference) {
        mReference = reference;
    }

    void setRequestedChannelGroup(const std::string& groupName) {
        mRequestedChannelGroup = groupName;
    }

    nanogui::Vector2i getImageCoords(const Image& image, nanogui::Vector2i mousePos);

    void getValuesAtNanoPos(nanogui::Vector2i nanoPos, std::vector<float>& result, const std::vector<std::string>& channels);
    std::vector<float> getValuesAtNanoPos(nanogui::Vector2i nanoPos, const std::vector<std::string>& channels) {
        std::vector<float> result;
        getValuesAtNanoPos(nanoPos, result, channels);
        return result;
    }

    ETonemap tonemap() const {
        return mTonemap;
    }

    void setTonemap(ETonemap tonemap) {
        mTonemap = tonemap;
    }

    static nanogui::Vector3f applyTonemap(const nanogui::Vector3f& value, float gamma, ETonemap tonemap);
    nanogui::Vector3f applyTonemap(const nanogui::Vector3f& value) const {
        return applyTonemap(value, mGamma, mTonemap);
    }

    EMetric metric() const {
        return mMetric;
    }

    void setMetric(EMetric metric) {
        mMetric = metric;
    }

    static float applyMetric(float value, float reference, EMetric metric);
    float applyMetric(float value, float reference) const {
        return applyMetric(value, reference, mMetric);
    }

    const nanogui::Color& backgroundColor() {
        return mShader->backgroundColor();
    }

    void setBackgroundColor(const nanogui::Color& color) {
        mShader->setBackgroundColor(color);
    }

    void fitImageToScreen(const Image& image);
    void resetTransform();

    void setClipToLdr(bool value) {
        mClipToLdr = value;
    }

    bool clipToLdr() const {
        return mClipToLdr;
    }

    std::vector<float> getHdrImageData(bool divideAlpha, int priority) const;
    std::vector<char> getLdrImageData(bool divideAlpha, int priority) const;

    void saveImage(const filesystem::path& filename) const;

    std::shared_ptr<Lazy<std::shared_ptr<CanvasStatistics>>> canvasStatistics();

private:
    static std::vector<Channel> channelsFromImages(
        std::shared_ptr<Image> image,
        std::shared_ptr<Image> reference,
        const std::string& requestedChannelGroup,
        EMetric metric,
        int priority
    );

    static Task<std::shared_ptr<CanvasStatistics>> computeCanvasStatistics(
        std::shared_ptr<Image> image,
        std::shared_ptr<Image> reference,
        const std::string& requestedChannelGroup,
        EMetric metric,
        int priority
    );

    nanogui::Vector2f pixelOffset(const nanogui::Vector2i& size) const;

    // Assembles the transform from canonical space to
    // the [-1, 1] square for the current image.
    nanogui::Matrix3f transform(const Image* image);
    nanogui::Matrix3f textureToNanogui(const Image* image);

    float mPixelRatio = 1;
    float mExposure = 0;
    float mOffset = 0;
    float mGamma = 2.2f;

    bool mClipToLdr = false;

    std::shared_ptr<Image> mImage;
    std::shared_ptr<Image> mReference;

    std::string mRequestedChannelGroup = "";

    nanogui::Matrix3f mTransform = nanogui::Matrix3f::scale(nanogui::Vector3f(1.0f));

    std::unique_ptr<UberShader> mShader;

    ETonemap mTonemap = SRGB;
    EMetric mMetric = Error;

    std::map<std::string, std::shared_ptr<Lazy<std::shared_ptr<CanvasStatistics>>>> mMeanValues;
};

TEV_NAMESPACE_END
