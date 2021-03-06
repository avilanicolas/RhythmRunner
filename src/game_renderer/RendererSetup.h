//	Joseph	Arhar



#ifndef	RENDERER_SETUP_H_

#define	RENDERER_SETUP_H_



#include	<GL/glew.h>

#include	<GLFW/glfw3.h>

#include	<imgui.h>

#include	<imgui_impl_glfw_gl3.h>



#define	TITLE	"Rhythm	Runner"



namespace	RendererSetup	{



GLFWwindow*	InitOpenGL();

void	Close(GLFWwindow*	window);

void	PreRender(GLFWwindow*	window);

void	PostRender(GLFWwindow*	window);



void	ImGuiCenterWindow(double	ratio);

void	ImGuiTopRightCornerWindow(double	ratio);

void	ImGuiTopLeftCornerWindow(double	ratio);



const	ImGuiWindowFlags	TITLE_WINDOW_FLAGS	=	ImGuiWindowFlags_NoCollapse;

}



#endif
