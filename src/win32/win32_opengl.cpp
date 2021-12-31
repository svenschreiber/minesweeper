#include <gl/gl.h>
#include "ext/wglext.h"

PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
PFNWGLMAKECONTEXTCURRENTARBPROC wglMakeContextCurrentARB;
PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;

static void *
platform_get_gl_proc_address(char *function_name) {
    void *p = (void *)wglGetProcAddress(function_name);
    if (p == 0 || p == (void *)0x1 || p == (void *)0x2 || p == (void *)0x3 || p == (void *)-1) {
        return (void *)0;
    }
    return p;
}

static void
win32_load_wgl_functions() {
    wglChoosePixelFormatARB    = (PFNWGLCHOOSEPIXELFORMATARBPROC)platform_get_gl_proc_address("wglChoosePixelFormatARB");
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)platform_get_gl_proc_address("wglCreateContextAttribsARB");
    wglMakeContextCurrentARB   = (PFNWGLMAKECONTEXTCURRENTARBPROC)platform_get_gl_proc_address("wglMakeContextCurrentARB");
    wglSwapIntervalEXT         = (PFNWGLSWAPINTERVALEXTPROC)platform_get_gl_proc_address("wglSwapIntervalEXT");
}

static void
win32_create_opengl_context(HGLRC *gl_rc, HDC window_dc) {
    PIXELFORMATDESCRIPTOR dummy_pfd = {};
    {
        dummy_pfd.nSize        = sizeof(PIXELFORMATDESCRIPTOR);
        dummy_pfd.nVersion     = 1;
        dummy_pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        dummy_pfd.iPixelType   = PFD_TYPE_RGBA;
        dummy_pfd.cColorBits   = 32;
        dummy_pfd.cDepthBits   = 24;
        dummy_pfd.cStencilBits = 8;
    }

    s32 pixel_format = ChoosePixelFormat(window_dc, &dummy_pfd);
    if (pixel_format) {
        SetPixelFormat(window_dc, pixel_format, &dummy_pfd);
        HGLRC dummy_rc = wglCreateContext(window_dc);
        wglMakeCurrent(window_dc, dummy_rc);

        win32_load_wgl_functions();

        s32 pixel_format_int_attribs[] = {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            0 // must be null-terminated
        };

        u32 num_formats = 0;
        wglChoosePixelFormatARB(window_dc, pixel_format_int_attribs, 0, 1, &pixel_format, &num_formats);
        if (pixel_format) {
            s32 context_attribs[] = {
                WGL_CONTEXT_MAJOR_VERSION_ARB, 3, // opengl version
                WGL_CONTEXT_MINOR_VERSION_ARB, 3,
                WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
                0 // also null-terminated
            };

            *gl_rc = wglCreateContextAttribsARB(window_dc, dummy_rc, context_attribs);
            if (*gl_rc) {
                wglMakeCurrent(window_dc, 0);
                wglDeleteContext(dummy_rc);
                wglMakeCurrent(window_dc, *gl_rc);
                wglSwapIntervalEXT(1); // enable v-sync
            }
        }
    }
}
