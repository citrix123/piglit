/*
 * Copyright © 2013 Chris Forbes
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

    config.supports_gl_compat_version = 30;

    config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
    return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
    /* test some new error cases */

    GLuint fbo;
    GLuint tex[2];
    glGenFramebuffers(1, &fbo);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(2, tex);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, tex[0]);
    glTexImage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
            4, GL_RGBA, 64, 64, 2, GL_TRUE);

    if (!piglit_check_gl_error(GL_NO_ERROR)) {
        printf("should be no error so far\n");
        piglit_report_result(PIGLIT_FAIL);
    }

    /* binding a negative layer should fail */
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex[0], 0, -1);

    if (!piglit_check_gl_error(GL_INVALID_VALUE)) {
        printf("glFramebufferTextureLayer w/ negative layer must "
                "emit GL_INVALID_VALUE but did not\n");
        piglit_report_result(PIGLIT_FAIL);
    }

    /* An INVALID_VALUE error is generated if samples is zero. */
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, tex[1]);
    glTexImage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
            0, GL_RGBA, 64, 64, 2, GL_TRUE);

    if (!piglit_check_gl_error(GL_INVALID_VALUE)) {
        printf("glFramebufferTextureLayer w/ sampler == 0 must "
                "emit GL_INVALID_VALUE but did not\n");
        piglit_report_result(PIGLIT_FAIL);
    }

    piglit_report_result(PIGLIT_PASS);
}
