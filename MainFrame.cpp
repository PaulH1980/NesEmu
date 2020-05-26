
#include <assert.h>
#include "..\include\sdl\sdl.h"
#include "..\include\sdl\sdl_ttf.h"
#include "Nes.h"
#include "Shared.h"
#include "MainFrame.h"

namespace UI
{

	SDL_Color White = { 0xff, 0xff, 0xff, 0 };
	SDL_Color Black = { 0,    0,    0, 0 };
	SDL_Color Red = { 0xff,    0,    0, 0 };
	SDL_Color Green = { 0, 0xff,    0, 0 };
	SDL_Color Blue = { 0,    0, 0xff, 0 };
	SDL_Color Yellow = { 0xff, 0xff,    0, 0 };
	SDL_Color Purple = { 0xff,    0, 0xff, 0 };
	SDL_Color Cyan = { 0, 0xff, 0xff, 0 };
	SDL_Color Transparent = { 0, 0, 0, 0 };



	struct sTextHelper
	{
		SDL_Renderer* renderer;
		SDL_Color       color;
		TTF_Font* font;
		std::string		text;
		int x;
		int y;
	};




	inline SDL_Rect CreateRectangle(int x, int y, int w, int h) {
		SDL_Rect rect;
		rect.x = x;
		rect.y = y;
		rect.w = w;
		rect.h = h;
		return rect;
	}




	inline SDL_Rect drawText(sTextHelper& _textInfo)
	{
		auto surface = TTF_RenderText_Solid(_textInfo.font, _textInfo.text.c_str(), _textInfo.color);
		auto texture = SDL_CreateTextureFromSurface(_textInfo.renderer, surface);
		auto rectangle = CreateRectangle(_textInfo.x, _textInfo.y, surface->w, surface->h);

		SDL_FreeSurface(surface);
		SDL_RenderCopy(_textInfo.renderer, texture, nullptr, &rectangle);
		SDL_DestroyTexture(texture);
		return rectangle;
	}


	cMainFrame::cMainFrame(int w, int h)
		: m_width(w)
		, m_height(h)
	{

		SDL_Init(SDL_INIT_VIDEO);
		SDL_CreateWindowAndRenderer(w, h, 0, &m_window, &m_renderer);
		TTF_Init();
		m_displayFont = TTF_OpenFont("OpenSans-Bold.ttf", FONT_SIZE);
		m_nesFrameBufferTexture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, NES_WIDTH, NES_HEIGHT);
		m_nesLeftPatternTexture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, NES_PATTERN_TABLE_DIMS, NES_PATTERN_TABLE_DIMS);
		m_nesRightPatternTexture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, NES_PATTERN_TABLE_DIMS, NES_PATTERN_TABLE_DIMS);
		assert(m_nesFrameBufferTexture != nullptr);
	}

	cMainFrame::~cMainFrame()
	{
		SDL_DestroyTexture(m_nesFrameBufferTexture);
		SDL_DestroyRenderer(m_renderer);
		SDL_DestroyWindow(m_window);
		
		SDL_Quit();
	}


	void cMainFrame::drawFrame()
	{
		SDL_RenderClear(m_renderer);
		drawPatternTables();
		drawCpuRegisterInfo();

		{
			//nes frame buffer
			const auto rect = CreateRectangle(0, 0, m_width - RIGHT_PANEL_WIDTH, m_height);
			SDL_RenderCopy(m_renderer, m_nesFrameBufferTexture, NULL, &rect);
		}

		SDL_RenderPresent(m_renderer);
	}

	void cMainFrame::setNes(_CPU::_6502::cNes* nes)
	{
		m_nesPtr = nes;
	}

	void cMainFrame::setPixelCallback(GetScreenPixelsCb cb)
	{
		m_getPixels = cb;
	}

	void cMainFrame::setFrameCompleteCallback(FrameCompleteCb cb)
	{
		m_frameComplete = cb;
	}

	void cMainFrame::setNesUpdateCallback(UpdateNesCb cb)
	{
		m_nesTick = cb;
	}

	bool cMainFrame::update(float elapsedTime)
	{
		//validate callbacks
		if (!m_getPixels || !m_frameComplete || !m_nesTick)
			return false;

		if (m_remainingTime > 0.0f)
			m_remainingTime -= elapsedTime;
		else
		{
			m_remainingTime += (1.0f / 60.0f) - elapsedTime;
			do {
				m_nesPtr->ppu.createPatternTableRgb(0);
				m_nesPtr->ppu.createPatternTableRgb(1);

				m_nesTick();
			} while (!m_frameComplete());


			int  rowPitch = 0;
			int* texPixels = nullptr;
			{
				SDL_LockTexture(m_nesFrameBufferTexture, nullptr, (void**)&texPixels, &rowPitch);
				const int numBytes = NES_WIDTH * NES_HEIGHT * 4;
				memcpy(texPixels, m_getPixels(), numBytes);
				SDL_UnlockTexture(m_nesFrameBufferTexture);
			}

			{   //left pattern table
				SDL_LockTexture(m_nesLeftPatternTexture, nullptr, (void**)&texPixels, &rowPitch);
				const int numBytes = NES_PATTERN_TABLE_DIMS * NES_PATTERN_TABLE_DIMS * 4;
				memcpy(texPixels, m_nesPtr->ppu.m_patternTablesRgb[0].data(), numBytes);
				SDL_UnlockTexture(m_nesLeftPatternTexture);
			}

			{   //left pattern table
				SDL_LockTexture(m_nesRightPatternTexture, nullptr, (void**)&texPixels, &rowPitch);
				const int numBytes = NES_PATTERN_TABLE_DIMS * NES_PATTERN_TABLE_DIMS * 4;
				memcpy(texPixels, m_nesPtr->ppu.m_patternTablesRgb[1].data(), numBytes);
				SDL_UnlockTexture(m_nesRightPatternTexture);
			}



		}
		return true;
	}

	void cMainFrame::drawPatternTables()
	{
		{   //pattern tables

			int xStartOffset = m_width - RIGHT_PANEL_WIDTH + 10;
			int yOffset = 10;

			//left pattern table
			{
				const auto rect = CreateRectangle(xStartOffset, yOffset, PATTERN_PANEL_DIMS, PATTERN_PANEL_DIMS);
				SDL_RenderCopy(m_renderer, m_nesLeftPatternTexture, NULL, &rect);
			}
			//right pattern table
			{
				const auto rect = CreateRectangle(xStartOffset, yOffset + PATTERN_PANEL_DIMS + 10, PATTERN_PANEL_DIMS, PATTERN_PANEL_DIMS);
				SDL_RenderCopy(m_renderer, m_nesRightPatternTexture, NULL, &rect);
			}
		}
	}

	void cMainFrame::drawCpuRegisterInfo()
	{
		{
			int xStartOffset = m_width - RIGHT_PANEL_WIDTH + FONT_SIZE;
			int yStartOffset = PATTERN_PANEL_DIMS * 2 + FONT_SIZE * 2;


			//text
			sTextHelper helper;
			helper.renderer = m_renderer;
			helper.x = xStartOffset;
			helper.y = yStartOffset;
			helper.font = m_displayFont;
			helper.color = White;

			{   //status
				const char* Flags[8] = {
					"C", "Z", "I", "D", "B", "U", "V", "N"
				};
				helper.text = "Status: ";
				helper.x = xStartOffset + FONT_SIZE;
				const auto dims = drawText(helper);
				helper.x += 64;

				for (int i = 0; i < 8; i++)
				{
					helper.x += FONT_SIZE * 1.25;
					helper.text = Flags[i];
					if (m_nesPtr->cpu.getStatusRegister() & (1 << i))
						helper.color = Green;
					else
						helper.color = Red;
					drawText(helper);
				}
			}
			{ //accumulator
				helper.x = xStartOffset + FONT_SIZE;
				helper.y += FONT_SIZE + 4;
				helper.text = "A:          0x" + Shared::n2hexstr(m_nesPtr->cpu.getAccumulator());
				helper.color = White;
				const auto dims = drawText(helper);
			}

			{ //x Reg
				helper.x = xStartOffset + FONT_SIZE;
				helper.y += FONT_SIZE + 4;
				helper.text = "X:           0x" + Shared::n2hexstr(m_nesPtr->cpu.getXRegister());;
				helper.color = White;
				const auto dims = drawText(helper);

			}

			{ //y Reg
				helper.x = xStartOffset + FONT_SIZE;
				helper.y += FONT_SIZE + 4;
				helper.text = "Y:           0x" + Shared::n2hexstr(m_nesPtr->cpu.getYRegister());;;
				helper.color = White;
				const auto dims = drawText(helper);
			}
		}
	}

	void cMainFrame::mainLoop()
	{
		Shared::ExecutionTimer<>  timer;
		while (1) //forever
		{
			auto deltaTime = timer.elapsedTime(true);
			if (!deltaTime)
				deltaTime += 1; //#TODO account for zero delta times??
			update(deltaTime * 0.001f);
			drawFrame();

			SDL_Event event;
			if (SDL_PollEvent(&event) && event.type == SDL_QUIT) {
				break;
			}
		}
	}
}