// SHE library
// Copyright (C) 2012-2015  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef SHE_SKIA_SKIA_SURFACE_INCLUDED
#define SHE_SKIA_SKIA_SURFACE_INCLUDED
#pragma once

#include "she/common/locked_surface.h"

#include "base/unique_ptr.h"

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkImageInfo.h"
#include "SkRegion.h"

namespace she {

inline SkColor to_skia(gfx::Color c) {
  return SkPremultiplyARGBInline(gfx::geta(c), gfx::getr(c), gfx::getg(c), gfx::getb(c));
}

inline SkRect to_skia(const gfx::Rect& rc) {
  return SkRect::Make(SkIRect::MakeXYWH(rc.x, rc.y, rc.w, rc.h));
}

class SkiaSurface : public NonDisposableSurface
                  , public CommonLockedSurface {
public:
  SkiaSurface() {
  }

  void create(int width, int height) {
    m_bitmap.tryAllocPixels(
      SkImageInfo::MakeN32Premul(width, height));

    rebuild();
  }

  void createRgba(int width, int height) {
    m_bitmap.tryAllocPixels(
      SkImageInfo::MakeN32Premul(width, height));

    rebuild();
  }

  // Surface impl

  void dispose() override {
    delete this;
  }

  int width() const override {
    return m_bitmap.width();
  }

  int height() const override {
    return m_bitmap.height();
  }

  bool isDirectToScreen() const override {
    return false;
  }

  gfx::Rect getClipBounds() override {
    return m_clip;
  }

  void setClipBounds(const gfx::Rect& rc) override {
    m_clip = rc;
    m_canvas->clipRect(to_skia(m_clip), SkRegion::kReplace_Op);
  }

  bool intersectClipRect(const gfx::Rect& rc) override {
    m_clip &= rc;
    m_canvas->clipRect(to_skia(m_clip), SkRegion::kReplace_Op);
    return !m_clip.isEmpty();
  }

  void setDrawMode(DrawMode mode, int param) {
    // TODO
  }

  LockedSurface* lock() override {
    m_bitmap.lockPixels();
    return this;
  }

  void applyScale(int scaleFactor) override {
    SkBitmap result;
    result.tryAllocPixels(
      SkImageInfo::MakeN32Premul(width()*scaleFactor, height()*scaleFactor));

    SkCanvas canvas(result);
    SkRect srcRect = SkRect::Make(SkIRect::MakeXYWH(0, 0, m_bitmap.width(), m_bitmap.height()));
    SkRect dstRect = SkRect::Make(SkIRect::MakeXYWH(0, 0, result.width(), result.height()));
    canvas.drawBitmapRectToRect(m_bitmap, &srcRect, dstRect);

    swapBitmap(result);
  }

  void* nativeHandle() override {
    return (void*)this;
  }

  // LockedSurface impl

  int lockedWidth() const override {
    return m_bitmap.width();
  }

  int lockedHeight() const override {
    return m_bitmap.height();
  }

  void unlock() override {
    m_bitmap.unlockPixels();
  }

  void clear() override {
  }

  uint8_t* getData(int x, int y) const override {
    return (uint8_t*)m_bitmap.getAddr(x, y);
  }

  void getFormat(SurfaceFormatData* formatData) const override {
    formatData->format = kRgbaSurfaceFormat;
    formatData->bitsPerPixel = 8*m_bitmap.bytesPerPixel();

    switch (m_bitmap.colorType()) {
      case kRGB_565_SkColorType:
        formatData->redShift   = SK_R16_SHIFT;
        formatData->greenShift = SK_G16_SHIFT;
        formatData->blueShift  = SK_B16_SHIFT;
        formatData->alphaShift = 0;
        formatData->redMask    = SK_R16_MASK;
        formatData->greenMask  = SK_G16_MASK;
        formatData->blueMask   = SK_B16_MASK;
        formatData->alphaMask  = 0;
        break;
      case kARGB_4444_SkColorType:
        formatData->redShift   = SK_R4444_SHIFT;
        formatData->greenShift = SK_G4444_SHIFT;
        formatData->blueShift  = SK_B4444_SHIFT;
        formatData->alphaShift = SK_A4444_SHIFT;
        formatData->redMask    = (15 << SK_R4444_SHIFT);
        formatData->greenMask  = (15 << SK_G4444_SHIFT);
        formatData->blueMask   = (15 << SK_B4444_SHIFT);
        formatData->alphaMask  = (15 << SK_A4444_SHIFT);
        break;
      case kRGBA_8888_SkColorType:
        formatData->redShift   = SK_RGBA_R32_SHIFT;
        formatData->greenShift = SK_RGBA_G32_SHIFT;
        formatData->blueShift  = SK_RGBA_B32_SHIFT;
        formatData->alphaShift = SK_RGBA_A32_SHIFT;
        formatData->redMask    = ((1 << SK_RGBA_R32_SHIFT) - 1);
        formatData->greenMask  = ((1 << SK_RGBA_G32_SHIFT) - 1);
        formatData->blueMask   = ((1 << SK_RGBA_B32_SHIFT) - 1);
        formatData->alphaMask  = ((1 << SK_RGBA_A32_SHIFT) - 1);
        break;
      case kBGRA_8888_SkColorType:
        formatData->redShift   = SK_BGRA_R32_SHIFT;
        formatData->greenShift = SK_BGRA_G32_SHIFT;
        formatData->blueShift  = SK_BGRA_B32_SHIFT;
        formatData->alphaShift = SK_BGRA_A32_SHIFT;
        formatData->redMask    = ((1 << SK_BGRA_R32_SHIFT) - 1);
        formatData->greenMask  = ((1 << SK_BGRA_G32_SHIFT) - 1);
        formatData->blueMask   = ((1 << SK_BGRA_B32_SHIFT) - 1);
        formatData->alphaMask  = ((1 << SK_BGRA_A32_SHIFT) - 1);
        break;
      default:
        formatData->redShift   = 0;
        formatData->greenShift = 0;
        formatData->blueShift  = 0;
        formatData->alphaShift = 0;
        formatData->redMask    = 0;
        formatData->greenMask  = 0;
        formatData->blueMask   = 0;
        formatData->alphaMask  = 0;
        break;
    }
  }

  gfx::Color getPixel(int x, int y) const override {
    SkColor c = m_bitmap.getColor(x, y);
    return gfx::rgba(
      SkColorGetR(c),
      SkColorGetG(c),
      SkColorGetB(c),
      SkColorGetA(c));
  }

  void putPixel(gfx::Color color, int x, int y) override {
    SkPaint paint;
    paint.setColor(to_skia(color));
    m_canvas->drawPoint(SkIntToScalar(x), SkIntToScalar(y), paint);
  }

  void drawHLine(gfx::Color color, int x, int y, int w) override {
    SkPaint paint;
    paint.setColor(to_skia(color));
    m_canvas->drawLine(
      SkIntToScalar(x), SkIntToScalar(y),
      SkIntToScalar(x+w-1), SkIntToScalar(y), paint);
  }

  void drawVLine(gfx::Color color, int x, int y, int h) override {
    SkPaint paint;
    paint.setColor(to_skia(color));
    m_canvas->drawLine(
      SkIntToScalar(x), SkIntToScalar(y),
      SkIntToScalar(x), SkIntToScalar(y+h-1), paint);
  }

  void drawLine(gfx::Color color, const gfx::Point& a, const gfx::Point& b) override {
    SkPaint paint;
    paint.setColor(to_skia(color));
    m_canvas->drawLine(
      SkIntToScalar(a.x), SkIntToScalar(a.y),
      SkIntToScalar(b.x), SkIntToScalar(b.y), paint);
  }

  void drawRect(gfx::Color color, const gfx::Rect& rc) override {
    SkPaint paint;
    paint.setColor(to_skia(color));
    paint.setStyle(SkPaint::kStroke_Style);
    m_canvas->drawRect(to_skia(rc), paint);
  }

  void fillRect(gfx::Color color, const gfx::Rect& rc) override {
    SkPaint paint;
    paint.setColor(to_skia(color));
    paint.setStyle(SkPaint::kFill_Style);
    m_canvas->drawRect(to_skia(rc), paint);
  }

  void blitTo(LockedSurface* dest, int srcx, int srcy, int dstx, int dsty, int width, int height) const override {
    SkPaint paint;
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);

    SkRect srcRect = SkRect::Make(SkIRect::MakeXYWH(srcx, srcy, width, height));
    SkRect dstRect = SkRect::Make(SkIRect::MakeXYWH(dstx, dsty, width, height));
    ((SkiaSurface*)dest)->m_canvas->drawBitmapRectToRect(m_bitmap, &srcRect, dstRect, &paint);
  }

  void drawSurface(const LockedSurface* src, int dstx, int dsty) override {
    gfx::Clip clip(dstx, dsty, 0, 0,
      ((SkiaSurface*)src)->m_bitmap.width(),
      ((SkiaSurface*)src)->m_bitmap.height());

    if (!clip.clip(m_bitmap.width(), m_bitmap.height(), clip.size.w, clip.size.h))
      return;

    SkRect srcRect = SkRect::Make(SkIRect::MakeXYWH(clip.src.x, clip.src.y, clip.size.w, clip.size.h));
    SkRect dstRect = SkRect::Make(SkIRect::MakeXYWH(clip.dst.x, clip.dst.y, clip.size.w, clip.size.h));

    SkPaint paint;
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);

    m_canvas->drawBitmapRectToRect(
      ((SkiaSurface*)src)->m_bitmap, &srcRect, dstRect, &paint);
  }

  void drawRgbaSurface(const LockedSurface* src, int dstx, int dsty) override {
    gfx::Clip clip(dstx, dsty, 0, 0,
      ((SkiaSurface*)src)->m_bitmap.width(),
      ((SkiaSurface*)src)->m_bitmap.height());

    if (!clip.clip(m_bitmap.width(), m_bitmap.height(), clip.size.w, clip.size.h))
      return;

    SkRect srcRect = SkRect::Make(SkIRect::MakeXYWH(clip.src.x, clip.src.y, clip.size.w, clip.size.h));
    SkRect dstRect = SkRect::Make(SkIRect::MakeXYWH(clip.dst.x, clip.dst.y, clip.size.w, clip.size.h));

    SkPaint paint;
    paint.setXfermodeMode(SkXfermode::kSrcATop_Mode);

    m_canvas->drawBitmapRectToRect(
      ((SkiaSurface*)src)->m_bitmap, &srcRect, dstRect, &paint);
  }

  SkBitmap& bitmap() {
    return m_bitmap;
  }

  void swapBitmap(SkBitmap& other) {
    m_bitmap.swap(other);
    rebuild();
  }

private:
  void rebuild() {
    m_canvas.reset(new SkCanvas(m_bitmap));
    m_clip = gfx::Rect(0, 0, width(), height());
  }

  SkBitmap m_bitmap;
  base::UniquePtr<SkCanvas> m_canvas;
  gfx::Rect m_clip;
};

} // namespace she

#endif
