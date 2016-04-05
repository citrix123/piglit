/*
 * Copyright © 2016 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

/**
 * \file zero-vertex-attrib.c
 *
 * Test based on this paragraph of ARB_vertex_attrib_64bit spec:
 * "  void GetVertexAttribLdv(uint index, enum pname, double *params);
 * <skip>
 *  The error INVALID_OPERATION is generated if index
 *  is zero, as there is no current value for generic attribute zero."
 *
 * Although the paragraph is focused on GetVertexAtribLdv, taking into
 * account the explanation, INVALID_OPERATION should be raised for any
 * or the *L* methods defined by the spec.
 *
 * This test is similar to max-vertex-attrib, but using index 0
 * (instead of GL_MAX_VERTEX_ATTRIB) and checking agains
 * INVALID_OPERATION instead of INVALID_VALUE.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 33;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static int test = 0;

#define CHECK_GL_INVALID_OPERATION \
	if (glGetError() != GL_INVALID_OPERATION) return PIGLIT_FAIL; \
	else printf("max-vertex-attrib test %d passed\n", ++test);

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

static GLboolean
run_test(void)
{
	GLdouble doublev[] = { 1.0, 1.0, 1.0, 1.0 };
	GLdouble quad[] = { -1.0, 1.0, 1.0, 1.0, -1.0, -1.0, 1.0, -1.0 };

	GLuint zeroIndex = 0; /* using a variable to make easier to read the method calls */

	glVertexAttribL1d(zeroIndex, doublev[0]);
	CHECK_GL_INVALID_OPERATION;

	glVertexAttribL2d(zeroIndex, doublev[0], doublev[1]);
	CHECK_GL_INVALID_OPERATION;

	glVertexAttribL3d(zeroIndex, doublev[0], doublev[1], doublev[2]);
	CHECK_GL_INVALID_OPERATION;

	glVertexAttribL4d(zeroIndex, doublev[0], doublev[1], doublev[2],
			 doublev[3]);
	CHECK_GL_INVALID_OPERATION;

	glVertexAttribL1dv(zeroIndex, doublev);
	CHECK_GL_INVALID_OPERATION;

	glVertexAttribL2dv(zeroIndex, doublev);
	CHECK_GL_INVALID_OPERATION;

	glVertexAttribL3dv(zeroIndex, doublev);
	CHECK_GL_INVALID_OPERATION;

	glVertexAttribL4dv(zeroIndex, doublev);
	CHECK_GL_INVALID_OPERATION;

	glVertexAttribLPointer(zeroIndex, 2, GL_DOUBLE, 0, quad);
	CHECK_GL_INVALID_OPERATION;

	glGetVertexAttribLdv(zeroIndex, GL_CURRENT_VERTEX_ATTRIB, doublev);
	CHECK_GL_INVALID_OPERATION;

        if (piglit_is_extension_supported("GL_EXT_direct_state_access")) {
                uint vaobj;

                glGenVertexArrays(1, &vaobj);
                glBindVertexArray(vaobj);

                glVertexArrayVertexAttribLOffsetEXT(vaobj, 0, zeroIndex, 3,
                                                    GL_DOUBLE, 0, 0);
                glDeleteVertexArrays(1, &vaobj);
                CHECK_GL_INVALID_OPERATION;
        }

        return PIGLIT_PASS;
}

void piglit_init(int argc, char **argv)
{
	piglit_require_gl_version(20);

	piglit_require_extension("GL_ARB_vertex_attrib_64bit");

        piglit_report_result(run_test());
}
