/**
 * Copyright 2017 Intel Corporation
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

/** @file named-framebuffer-draw-buffers-errors.c
 *
 * Test that NamedFramebufferDrawBuffers() returns correct error
 * message for different values.
 *
 * All spec quotes come from OpenGL 4.5 spec, section 17.4.1
 * "Selecting Buffers for Writint", page 492 (515 on PDF).
 *
 * Note that for 4.5, they are the same errors that for DrawBuffers,
 * so we are also testing that method although somewhat
 * indirectly. Also some spec quotes could mention DrawBuffers too.
 *
 * From OpenGL 4.5, section 17.4.1 "Selecting Buffers for Writing",
 * page 492:
 *
 * "void DrawBuffers( sizei n, const enum *bufs );
 *  void NamedFramebufferDrawBuffers(uint framebuffer,
 *                                   sizei n, const enum *bufs );
 *
 * [...]
 *
 * For DrawBuffers, the framebuffer object is that bound to the DRAW_-
 * FRAMEBUFFER binding. For NamedFramebufferDrawBuffers, framebuffer
 * is the name of the framebuffer object. If framebuffer is zero, then
 * the default framebuffer is affected.  n specifies the number of
 * buffers in bufs. bufs is a pointer to an array of values specifying
 * the buffer to which each fragment color is written."
 *
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 45;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA |
		PIGLIT_GL_VISUAL_DOUBLE;

	config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

/*
 * Table 17.6 from 4.5 spec, page 492 (512 on PDF)
 */
static const GLenum table_17_6[] = {
	GL_NONE,
	GL_FRONT_LEFT,
	GL_FRONT_RIGHT,
	GL_BACK_LEFT,
	GL_BACK_RIGHT,
};

/*
 * Table 17.6 from 4.5 spec, plus BACK, as BACK is allowed for default
 * framebuffer since 4.5 under some conditions.
 */
static const GLenum table_17_6_and_back[] = {
	GL_NONE,
	GL_FRONT_LEFT,
	GL_FRONT_RIGHT,
	GL_BACK_LEFT,
	GL_BACK_RIGHT,
	GL_BACK,
};

static const GLenum multiple_buffers[] = {
	GL_FRONT,
	GL_LEFT,
	GL_RIGHT,
	GL_FRONT_AND_BACK,
};

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLuint framebuffer;
	GLuint default_framebuffer = 0;
	GLenum bufs[2] = {GL_BACK_LEFT, GL_BACK};
	GLenum one_buf;
	bool subtest_pass;
	int i;

	glCreateFramebuffers(1, &framebuffer);
	piglit_check_gl_error(GL_NO_ERROR);

	/*
	 * "An INVALID_OPERATION error is generated by
	 *  NamedFramebufferDrawBuffers if framebuffer is not zero or
	 *  the name of an existing framebuffer"
	 */
	glNamedFramebufferDrawBuffers(5, 2, bufs);
	PIGLIT_SUBTEST_ERROR(GL_INVALID_OPERATION, pass, "INVALID_OPERATION if "
			     "framebuffer is not zero or the name of an existing "
			     "framebuffer");

	/*
	 * "An INVALID_VALUE error is generated if n is negative, or greater than the
	 *  value of MAX_DRAW_BUFFERS."
	 */
	int max_draw_buffers;
	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &max_draw_buffers);

	subtest_pass = true;
	glNamedFramebufferDrawBuffers(0, -1, bufs);
	subtest_pass = subtest_pass & piglit_check_gl_error(GL_INVALID_VALUE);

	glNamedFramebufferDrawBuffers(0, max_draw_buffers + 1, bufs);
	subtest_pass = subtest_pass & piglit_check_gl_error(GL_INVALID_VALUE);

	PIGLIT_SUBTEST_CONDITION(subtest_pass, pass, "INVALID_VALUE error is "
				 "generated if n is negative, or greater than "
				 "the value of MAX_DRAW_BUFFERS.");

	/*
	 * From OpenGL 4.5 spec
	 *   "An INVALID_ENUM error is generated if any value in bufs
	 *    is not one of the values in tables 17.5 or 17.6."
	 */
	subtest_pass = true;
	one_buf = GL_RED;
	glNamedFramebufferDrawBuffers(default_framebuffer, 1, &one_buf);
	subtest_pass = subtest_pass && piglit_check_gl_error(GL_INVALID_ENUM);
	glNamedFramebufferDrawBuffers(framebuffer, 1, &one_buf);
	subtest_pass = subtest_pass && piglit_check_gl_error(GL_INVALID_ENUM);

	PIGLIT_SUBTEST_CONDITION(subtest_pass, pass, "INVALID_ENUM error is "
				 "generated if any value in bufs is not one of "
				 "the values in tables 17.5 or 17.6.");
	/*
	* (cont on error out of 17.5 or 17.6) Specifically for the default
	 * framebuffer:
	 * From OpenGL 4.5 spec, page 492 (515 PDF)
	 * "If the default framebuffer is affected, then each of the
	 *  constants must be one of the values listed in table 17.6
	 *  or the special value BACK .""
	 *
	 * And:
	 *"An INVALID_OPERATION error is generated if the default
	 *  framebuffer is affected and any value in bufs is a
	 *  constant (other than NONE or BACK) that does not indicate
	 *  one of the color buffers allocated to the default
	 *  framebuffer."
	 *
	 * So for the default framebuffer, and that table, we expect
	 * GL_NO_ERROR or GL_INVALID_OPERATION.
	 */
	subtest_pass = true;
	for (i = 0; i < ARRAY_SIZE(table_17_6_and_back); i++) {
		GLenum err = 0;

		glNamedFramebufferDrawBuffers(default_framebuffer, 1,
					      &table_17_6_and_back[i]);

		/* We manually check glGetError instead of relying on
		 * piglit_check_gl_error like in other subtests
		 * because for subtests that checks several enums, we
		 * are interested on getting which one failed. That
		 * makes debugging easier.
		 */
		err = glGetError();
		if (err != GL_NO_ERROR && err != GL_INVALID_OPERATION) {
			printf("Expected GL_NO_ERROR or GL_INVALID_OPERATION "
			       "with %s but received: %s\n",
			       piglit_get_gl_enum_name(table_17_6_and_back[i]),
			       piglit_get_gl_error_name(err));
			subtest_pass = false;
		}
	}

	/* For that spec paragraph, we also test enums from table
	 * 17.5. They should return INVALID_OPERATION, as after all,
	 * they are not allocated to the default framebuffer. */
	one_buf = GL_COLOR_ATTACHMENT0;
	glNamedFramebufferDrawBuffers(default_framebuffer, 1, &one_buf);
	subtest_pass = subtest_pass &&
	  piglit_check_gl_error(GL_INVALID_OPERATION);

	PIGLIT_SUBTEST_CONDITION(subtest_pass, pass, "If the default framebuffer"
				 " is affected, then each of the constants must "
				 "be one of the values listed in table 17.6 or "
				 "the special value BACK. INVALID_OPERATION error"
				 " is generated if the default framebuffer is "
				 "affected and any value in bufs is a constant "
				 "(other than NONE or BACK ) that does not indicate "
				 "one of the color buffers allocated to the default"
				 " framebuffer.");

	/* (cont default framebuffer)
	 * From OpenGL 4.5 spec:
	 * "When BACK is used, n must be 1 and color values are
	 *  written into the left buffer for single-buffered
	 *  contexts, or into the back left buffer for
	 *  double-buffered contexts"
	 *
	 * From the error table:
	 * "An INVALID_OPERATION error is generated if any value in
	 *   bufs is BACK , and n is not one."
	 *
	 */
	glNamedFramebufferDrawBuffers(default_framebuffer, 2, bufs);
	PIGLIT_SUBTEST_ERROR(GL_INVALID_OPERATION, pass, "(default framebuffer)"
			     " An INVALID_OPERATION error is generated if any "
			     "value in bufs is BACK, and n is not one.");

	/*
	 * Now, specifically for a framebuffer object:
	 * "If a framebuffer object is affected, then each of the
	 *  constants must be one of the values listed in table 17.5."
	 *
	 * 17.5 is GL_NONE, and COLOR_ATTACHMENTi, where i <
	 * MAX_COLOR_ATTACHMENTS - 1
	 */
	int max_attachments;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &max_attachments);

	subtest_pass = true;
	for (i = 0; i < max_attachments; i++) {
		 one_buf = GL_COLOR_ATTACHMENT0 + i;
		 glNamedFramebufferDrawBuffers(framebuffer, 1, &one_buf);
		 subtest_pass = subtest_pass &&
		   piglit_check_gl_error(GL_NO_ERROR);
	}
	PIGLIT_SUBTEST_CONDITION(subtest_pass, pass, "If a framebuffer object is "
			 "affected, then each of constants must be one of the "
			 "values listed in table 17.5.");

	/*
	 * "An INVALID_OPERATION error is generated if a framebuffer
	 *  object is affected and any value in bufs is a constant
	 *  from table 17.6, or COLOR_ATTACHMENTm where m is greater
	 *  than or equal to the value of MAX_COLOR_ATTACHMENTS."
	 */
	subtest_pass = true;
	/* Starting at 1, as GL_NONE is valid */
	for (i = 1; i < ARRAY_SIZE(table_17_6); i++) {
		GLenum err = 0;

		glNamedFramebufferDrawBuffers(framebuffer, 1, &table_17_6[i]);
		err = glGetError();
		if (err != GL_INVALID_OPERATION) {
			printf("Expected GL_INVALID_OPERATION with"
			       " %s but received: %s\n",
			       piglit_get_gl_enum_name(table_17_6[i]),
			       piglit_get_gl_error_name(err));
			subtest_pass = false;
		}
	}

	one_buf = GL_COLOR_ATTACHMENT0 + max_attachments;
	glNamedFramebufferDrawBuffers(framebuffer, 1, &one_buf);
	subtest_pass = subtest_pass &&
	  piglit_check_gl_error(GL_INVALID_OPERATION);
	PIGLIT_SUBTEST_CONDITION(subtest_pass, pass, "INVALID_OPERATION error is "
			 "generated if a framebuffer object is affected and any "
			 "value in bufs is a constant from table 17.6, or "
			 "COLOR_ATTACHMENTm where m is greater than or equal "
			 "to the value of MAX_COLOR_ATTACHMENTS.");

	/*
	 * "An INVALID_OPERATION error is generated if a buffer other
	 *  than NONE is specified more than once in the array pointed
	 *  to by bufs"
	 */
	bufs[0] = bufs[1] = GL_FRONT_LEFT;
	glNamedFramebufferDrawBuffers(framebuffer, 2, bufs);
	PIGLIT_SUBTEST_ERROR(GL_INVALID_OPERATION, pass, "INVALID_OPERATION error "
			     "is generated if a buffer other than NONE is specified "
			     "more than once in the array pointed to by bufs.");

	/*
	 * From OpenGL 4.5 spec:
	 * "An INVALID_ENUM error is generated if any value in bufs is
	 *  FRONT, LEFT, RIGHT, or FRONT_AND_BACK . This restriction
	 *  applies to both the default framebuffer and framebuffer
	 *  objects, and exists because these constants may themselves
	 *  refer to multiple buffers, as shown in table 17.4."
	 */
	subtest_pass = true;
	for (i = 0; i < ARRAY_SIZE(multiple_buffers); i++) {
		GLenum err = 0;
		bool local_pass = true;

		glNamedFramebufferDrawBuffers(default_framebuffer, 1,
					      &multiple_buffers[i]);
		err = glGetError();
		local_pass = local_pass && (err == GL_INVALID_ENUM);

		glNamedFramebufferDrawBuffers(framebuffer, 1,
					      &multiple_buffers[i]);
		err = glGetError();
		local_pass = local_pass && (err == GL_INVALID_ENUM);

		if (!local_pass)
			printf("Expected GL_INVALID_ENUM with"
			       " %s but received: %s\n",
			       piglit_get_gl_enum_name(table_17_6_and_back[i]),
			       piglit_get_gl_error_name(err));

		subtest_pass = subtest_pass && local_pass;

	}
	PIGLIT_SUBTEST_CONDITION(subtest_pass, pass, "INVALID_ENUM error is "
				 "generated if any value in bufs is FRONT, LEFT,"
				 " RIGHT, or FRONT_AND_BACK ");

	/* clean up */
	glDeleteFramebuffers(1, &framebuffer);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}
