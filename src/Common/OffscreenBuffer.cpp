#include "hdi/dimensionality_reduction/gradient_descent_tsne_texture.h" // Included for glad, must be included before OpenGLContext

#include "OffscreenBuffer.h"

OffscreenBuffer::OffscreenBuffer() :
    _context(nullptr),
    _surface(nullptr)
{
    setSurfaceType(QWindow::OpenGLSurface);

    create();
}

void OffscreenBuffer::initialize()
{
    QOpenGLContext* globalContext = QOpenGLContext::globalShareContext();
    _context = new QOpenGLContext(this);
    _context->setFormat(globalContext->format());

    if (!_context->create())
        qFatal("Cannot create requested OpenGL context.");

    // Create an offscreen surface (on the GUI thread) matching the context
    // format. The context is later made current on this surface from the worker
    // thread; using the QWindow itself as the surface fails on macOS.
    _surface = new QOffscreenSurface();
    _surface->setFormat(_context->format());
    _surface->create();

    bindContext();

#ifdef __APPLE__
    // On macOS, Qt 6 provides OpenGL over Metal and exposes it only through the
    // context's getProcAddress; the system OpenGL framework symbols are not
    // bound to it. Load glad via Qt so HDILib's (glad-based) GL calls reach the
    // live context. Must be done with the context current (bindContext above).
    if (!gladLoadGLLoader((GLADloadproc)[](const char* name) -> void* {
            QOpenGLContext* ctx = QOpenGLContext::currentContext();
            return ctx ? reinterpret_cast<void*>(ctx->getProcAddress(name)) : nullptr;
        }))
        qFatal("Failed to load OpenGL functions via Qt getProcAddress on macOS.");
#else
    if (!gladLoadGL()) {
        qFatal("No OpenGL context is currently bound, therefore OpenGL function loading has failed.");
    }
#endif

    releaseContext();
}

void OffscreenBuffer::bindContext()
{
    _context->makeCurrent(_surface);
}

void OffscreenBuffer::releaseContext()
{
    _context->doneCurrent();
}
