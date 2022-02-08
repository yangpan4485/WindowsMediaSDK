#include "video_render_opengl.h"

#include <iostream>

#define ATTRIB_VERTEX 3
#define ATTRIB_TEXTURE 4

VideoRenderOpenGL::VideoRenderOpenGL() {}

VideoRenderOpenGL::~VideoRenderOpenGL() {}

void VideoRenderOpenGL::RendFrameI420(uint8_t* y_data, uint32_t y_stride, uint8_t* u_data,
                                      uint32_t u_stride, uint8_t* v_data, uint32_t v_stride,
                                      uint32_t width, uint32_t height) {
    if (CheckWindowSizeChange()) {
        init_ = false;
    }
    if (!init_) {
        Init();
        init_ = true;
    }
    if (frame_width_ != width || frame_height_ != height) {
        frame_width_ = width;
        frame_height_ = height;
    }
    glClearColor(0.0, 0.0, 0.0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glShadeModel(GL_FLAT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_y_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, y_stride, frame_height_, 0, GL_RED, GL_UNSIGNED_BYTE, y_data);
    glUniform1i(uniform_y_, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture_u_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, u_stride, frame_height_ / 2, 0, GL_RED, GL_UNSIGNED_BYTE, u_data);
    glUniform1i(uniform_u_, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture_v_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, v_stride, frame_height_ / 2, 0, GL_RED, GL_UNSIGNED_BYTE, v_data);
    glUniform1i(uniform_v_, 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glFlush();
    SwapBuffers(hdc_);
}

void VideoRenderOpenGL::RendFrameNV12(uint8_t* y_data, uint32_t y_stride, uint8_t* uv_data,
                                      uint32_t uv_stride, uint32_t width, uint32_t height) {}

void VideoRenderOpenGL::RendFrameNV21(uint8_t* y_data, uint32_t y_stride, uint8_t* vu_data,
                                      uint32_t vu_stride, uint32_t width, uint32_t height) {}

void VideoRenderOpenGL::Init() {
    hdc_ = GetDC(render_window_);
    DestroyContext();
    InitContext();
    InitShaders();
    InitTexture();
}

void VideoRenderOpenGL::InitContext() {
    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0x00, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cRedBits = 8;
    pfd.cGreenBits = 8;
    pfd.cBlueBits = 8;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    // pfd.cColorBits = (BYTE)GetDeviceCaps(hdc_, BITSPIXEL);
    int nPixelFormat = ChoosePixelFormat(hdc_, &pfd);
    if (nPixelFormat == 0) {
        std::cout << "pixel format not found" << std::endl;
        return;
    }
    bool bResult = SetPixelFormat(hdc_, nPixelFormat, &pfd);
    if (!bResult) {
        return;
    }
    HGLRC tmp = wglCreateContext(hdc_);

    wglMakeCurrent(hdc_, tmp);

    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();
    if (GlewInitResult != GLEW_OK) {
        return;
    }
#if 0
    int attribs[] = { WGL_CONTEXT_MAJOR_VERSION_ARB,
                     3,
                     WGL_CONTEXT_MINOR_VERSION_ARB,
                     3,
                     WGL_CONTEXT_FLAGS_ARB,
                     0,
                     0 };

    if (wglewIsSupported("WGL_ARB_create_context") == 1) {
        hrc_ = wglCreateContextAttribsARB(hdc_, 0, attribs);
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(tmp);
        wglMakeCurrent(hdc_, hrc_);
    }
    else {
        hrc_ = tmp;
    }
    wglMakeCurrent(hdc_, hrc_);

    hrc_ = wglCreateContext(hdc_);
    int pixelformat = ChoosePixelFormat(hdc_, &pfd);
    SetPixelFormat(hdc_, pixelformat, &pfd);
    wglMakeCurrent(hdc_, hrc_);
#endif
}

void VideoRenderOpenGL::DestroyContext() {
    wglDeleteContext(hrc_);
    wglMakeCurrent(NULL, NULL);
}

void VideoRenderOpenGL::InitShaders() {
    std::string vsh_str = "attribute vec4 vertexIn;\n"
                          "attribute vec2 textureIn;\n"
                          "varying vec2 textureOut;\n"
                          "void main(void)\n"
                          "{\n"
                          "    gl_Position = vertexIn;\n"
                          "    textureOut = textureIn;\n"
                          "}\n";

    std::string fsh_str = "varying vec2 textureOut;\n"
                          "uniform sampler2D tex_y;\n"
                          "uniform sampler2D tex_u;\n"
                          "uniform sampler2D tex_v;\n"
                          "void main(void)\n"
                          "{\n"
                          "    vec3 yuv;\n"
                          "    vec3 rgb;\n"
                          "    yuv.x = texture2D(tex_y, textureOut).r;\n"
                          "    yuv.y = texture2D(tex_u, textureOut).r - 0.5;\n"
                          "    yuv.z = texture2D(tex_v, textureOut).r - 0.5;\n"
                          "    rgb = mat3( 1,       1,         1,\n"
                          "                0,       -0.39465,  2.03211,\n"
                          "                1.13983, -0.58060,  0) * yuv;\n"
                          "    gl_FragColor = vec4(rgb, 1);\n"
                          "}\n";
    GLint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertex_shader_source = vsh_str.c_str();
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);

    GLint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragment_shader_source = fsh_str.c_str();
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);

    shader_program_ = glCreateProgram();
    glAttachShader(shader_program_, vertex_shader);
    glAttachShader(shader_program_, fragment_shader);
    glBindAttribLocation(shader_program_, ATTRIB_VERTEX, "vertexIn");
    glBindAttribLocation(shader_program_, ATTRIB_TEXTURE, "textureIn");

    glLinkProgram(shader_program_);
    glUseProgram(shader_program_);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}
void VideoRenderOpenGL::CalcRenderPos() {
    static const GLfloat vertexVertices[] = {
        -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
    };

    static const GLfloat textureVertices[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    };

    glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, vertexVertices);
    // Enable it
    glEnableVertexAttribArray(ATTRIB_VERTEX);
    glVertexAttribPointer(ATTRIB_TEXTURE, 2, GL_FLOAT, 0, 0, textureVertices);
    glEnableVertexAttribArray(ATTRIB_TEXTURE);
}
void VideoRenderOpenGL::InitTexture() {
    CalcRenderPos();
    uniform_y_ = glGetUniformLocation(shader_program_, "tex_y");
    uniform_u_ = glGetUniformLocation(shader_program_, "tex_u");
    uniform_v_ = glGetUniformLocation(shader_program_, "tex_v");

    glGenTextures(1, &texture_y_);
    glBindTexture(GL_TEXTURE_2D, texture_y_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &texture_u_);
    glBindTexture(GL_TEXTURE_2D, texture_u_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &texture_v_);
    glBindTexture(GL_TEXTURE_2D, texture_v_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}