#pragma once
#include "Shared.h"
#include "PPU.h"
#include "NesRam.h"
#include "NesRom.h"
#include "Mappers.h"
#include "NesJoyPad.h"
#include "6502.h"

namespace _CPU
{
	namespace _6502
	{
		class cNes
		{
		public:
			cNes();
			~cNes();

			void tick();
			void reset();
			bool loadRom(const std::string& fileName);
			bool frameComplete();
			
			
			cDataBus		cpuBus;			
			cDataBus		ppuBus;

			cCpu			cpu;       //6502
			cPPU			ppu;       //rp2c02
			//cpu
			cNesJoyPad		joypads;   //controllers
			cNesRom			program;   //program rom( code/graphics )do nothing yet
			cNesRam			ram;       //nes ram
			//ppu
			cNameTables		nameTables;
			cPaletteData	paletteData;
			cPatternTable   patternTables;

			uint64_t		m_totalCycles;

			MapperFactory m_mapperFactory;
	

		};
	}
}