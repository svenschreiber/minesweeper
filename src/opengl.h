#ifndef OPENGL_H
#define OPENGL_H

#define GL_PROC_LIST \
GLProc(glGenBuffers, GLGENBUFFERS) \
GLProc(glBindBuffer, GLBINDBUFFER) \
GLProc(glGenVertexArrays, GLGENVERTEXARRAYS) \
GLProc(glBindVertexArray, GLBINDVERTEXARRAY) \
GLProc(glCreateProgram, GLCREATEPROGRAM) \
GLProc(glCreateShader, GLCREATESHADER) \
GLProc(glShaderSource, GLSHADERSOURCE) \
GLProc(glCompileShader, GLCOMPILESHADER) \
GLProc(glGetShaderiv, GLGETSHADERIV) \
GLProc(glGetShaderInfoLog, GLGETSHADERINFOLOG) \
GLProc(glGetProgramiv, GLGETPROGRAMIV) \
GLProc(glGetProgramInfoLog, GLGETPROGRAMINFOLOG) \
GLProc(glAttachShader, GLATTACHSHADER) \
GLProc(glLinkProgram, GLLINKPROGRAM) \
GLProc(glValidateProgram, GLVALIDATEPROGRAM) \
GLProc(glDeleteShader, GLDELETESHADER) \
GLProc(glGetUniformLocation, GLGETUNIFORMLOCATION) \
GLProc(glGetAttribLocation, GLGETATTRIBLOCATION) \
GLProc(glEnableVertexAttribArray, GLENABLEVERTEXATTRIBARRAY) \
GLProc(glVertexAttribPointer, GLVERTEXATTRIBPOINTER) \
GLProc(glBufferData, GLBUFFERDATA) \
GLProc(glBufferSubData, GLBUFFERSUBDATA) \
GLProc(glBlendFuncSeparate, GLBLENDFUNCSEPARATE) \
GLProc(glUseProgram, GLUSEPROGRAM) \
GLProc(glUniform1i, GLUNIFORM1I) \
GLProc(glUniform1f, GLUNIFORM1F) \
GLProc(glUniform2f, GLUNIFORM2F) \
GLProc(glUniform3f, GLUNIFORM3F) \
GLProc(glUniform4f, GLUNIFORM4F) \
GLProc(glTexSubImage3D, GLTEXSUBIMAGE3D) \
GLProc(glTexImage3D, GLTEXIMAGE3D) \
GLProc(glActiveTexture, GLACTIVETEXTURE) \
GLProc(glGenerateMipmap, GLGENERATEMIPMAP)


#ifdef _WIN32
#include <windows.h>
#include <gl/gl.h>
#endif
#include "ext/glext.h"

#define GLProc(name, type) PFN##type##PROC name = 0;
GL_PROC_LIST
#undef GLProc

void load_gl_functions() {
#define GLProc(name, type) name = (PFN##type##PROC)platform_get_gl_proc_address(#name);
GL_PROC_LIST
#undef GLProc
}

#endif
