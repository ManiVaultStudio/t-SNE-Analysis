#pragma once

#include <QWindow>
#include <QOpenGLContext>
#include <QOffscreenSurface>

class OffscreenBuffer : public QWindow
{
public:
    OffscreenBuffer();

    QOpenGLContext* getContext() { return _context; }

    /** The surface the context is made current on. A QOffscreenSurface (rather
     *  than the QWindow itself) is required so the context can be made current
     *  on a worker thread - a never-shown QWindow has no valid native surface
     *  off the main thread on macOS, making makeCurrent succeed but leave no
     *  usable context. */
    QOffscreenSurface* getSurface() { return _surface; }

    /** Initialize and bind the OpenGL context associated with this buffer */
    void initialize();

    /** Bind the OpenGL context associated with this buffer */
    void bindContext();

    /** Release the OpenGL context associated with this buffer */
    void releaseContext();

private:
    QOpenGLContext* _context;
    QOffscreenSurface* _surface;
};
