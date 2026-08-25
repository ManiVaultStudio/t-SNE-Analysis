#include "hdi/utils/glad/glad.h" // Must be included before OpenGLContext

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

    auto loader = [](const char* name) -> GLADapiproc {
        QOpenGLContext* ctx = QOpenGLContext::currentContext();
        return ctx ? reinterpret_cast<GLADapiproc>(ctx->getProcAddress(name)) : nullptr;
    };

    if (!gladLoadGL(loader))
        qFatal("Failed to load OpenGL functions via Qt getProcAddress.");
        
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
