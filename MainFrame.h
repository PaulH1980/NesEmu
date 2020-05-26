#pragma once

#include <functional>
#include "Shared.h"
struct SDL_Renderer;
struct SDL_Window;
struct SDL_Texture;
typedef struct _TTF_Font TTF_Font;
/*
	Emulator UI is layout like this:
*/


#define PATTERN_PANEL_DIMS   320 
#define FONT_SIZE            16
#define TEXTFIELD_HEIGHT     (FONT_SIZE + 4)

#define RIGHT_PANEL_WIDTH  (PATTERN_PANEL_DIMS + 20)

namespace _CPU
{
	namespace _6502
	{
		class cNes;
	}
}

namespace UI
{
	

	using GetScreenPixelsCb = std::function<uint32_t*()>;
	using UpdateNesCb	    = std::function<void()>;
	using FrameCompleteCb   = std::function<bool()>;
	
	class cMainFrame
	{
	public:
		cMainFrame(int w, int h);
		~cMainFrame();

		void mainLoop();

		void drawFrame();
		void setNes(_CPU::_6502::cNes* nes);

		void setPixelCallback(GetScreenPixelsCb cb);
		void setFrameCompleteCallback(FrameCompleteCb cb);
		void setNesUpdateCallback(UpdateNesCb cb );
		

	private:

		/*
			@brief: Copy pixels from ppu backend to framebuffer
		*/
		bool update( float elapsedTime );
		void drawPatternTables();
		void drawCpuRegisterInfo();

		int m_width, m_height;
		float m_remainingTime = 0.0f;

		SDL_Renderer *m_renderer				= nullptr;
		SDL_Window *m_window					= nullptr;
		SDL_Texture* m_nesFrameBufferTexture    = nullptr;	
		SDL_Texture* m_nesLeftPatternTexture	= nullptr;
		SDL_Texture* m_nesRightPatternTexture   = nullptr;
		TTF_Font*    m_displayFont				= nullptr;

		//callbacks below
		FrameCompleteCb   m_frameComplete		= nullptr;
		UpdateNesCb	      m_nesTick				= nullptr;
		GetScreenPixelsCb m_getPixels			= nullptr;

		_CPU::_6502::cNes* m_nesPtr				= nullptr;
	};
}