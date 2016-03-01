#include "glutil.h"

GLvoid CheckError( const char* const label ) {
    GLenum error;
    error = glGetError();
    while ( GL_NO_ERROR != error ) {
        std::cerr << "<GLERROR>" << label << ": " << gluErrorString(error) << "\n";
        error = glGetError();
    }
}

GLvoid CheckFrameBuffer() {
    switch(glCheckFramebufferStatus(GL_FRAMEBUFFER)) {
    case GL_FRAMEBUFFER_UNDEFINED:
        std::cerr<<"GL_FRAMEBUFFER_UNDEFINED: returned if target is the default framebuffer, but the default framebuffer does not exist.\n";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        std::cerr<<"GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: returned if any of the framebuffer attachment points are framebuffer incomplete.\n";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        std::cerr<<"GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: returned if the framebuffer does not have at least one image attached to it.\n";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        std::cerr<<"GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: returned if the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for any color attachment point(s) named by GL_DRAWBUFFERi.\n";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        std::cerr<<"GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: returned if GL_READ_BUFFER is not GL_NONE and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for the color attachment point named by GL_READ_BUFFER.\n";
        break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
        std::cerr<<"GL_FRAMEBUFFER_UNSUPPORTED: returned if the combination of internal formats of the attached images violates an implementation-dependent set of restrictions. GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE is also returned if the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not the same for all attached textures; or, if the attached images are a mix of renderbuffers and textures, the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not GL_TRUE for all attached textures.\n";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
        std::cerr<<"GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: returned if any framebuffer attachment is layered, and any populated attachment is not layered, or if all populated color attachments are not from textures of the same target.\n";
        break;
    case GL_FRAMEBUFFER_COMPLETE:
        std::cerr<<"GL_FRAMEBUFFER_COMPLETE: returned if everything is groovy!\n";
        break;
    default:
        std::cerr<<glCheckFramebufferStatus(GL_FRAMEBUFFER)<<": Undefined framebuffer return value: possible error elsewhere?\n";
        break;
    }
}
