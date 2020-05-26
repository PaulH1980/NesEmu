#include "Nes.h"

namespace _CPU
{
	namespace _6502
	{

		cNes::cNes()
			: m_totalCycles( 0 )
		{
			
			cpuBus.addCpuDevice(&program);
			cpuBus.addCpuDevice(&cpu);
			cpuBus.addCpuDevice(&ppu);
			cpuBus.addCpuDevice(&ram);
			cpuBus.addCpuDevice(&joypads);

			ppuBus.addPpuDevice(&program);
			ppuBus.addPpuDevice(&ppu);
			ppuBus.addPpuDevice(&patternTables);
			ppuBus.addPpuDevice(&nameTables);
			ppuBus.addPpuDevice(&paletteData);

			


			//register known mappers
			m_mapperFactory.registerMapper(0, []() -> iMapper* { return new Mapper00; });
		}

		cNes::~cNes()
		{

		}

		void cNes::tick()
		{
			if (!program.romLoaded())
				return;
			
			ppu.tick(); //ppu runs 3 times as fast as cpu

			if (m_totalCycles % 3 == 0)
				cpu.tick();

			//interrupt request
			if ( ppu.nmi() )
				cpu.nmi();
			
			m_totalCycles++;
		}

		void cNes::reset()
		{
			m_totalCycles = 0;
			cpu.reset();
			ppu.reset();
		}

		bool cNes::loadRom(const std::string& fileName) {

			bool succeed = program.loadFromFile(fileName);
			if (!succeed)
				return false;
			
			auto crtMapperFun = m_mapperFactory.getMapperCreateFunc(program.getMapperId());
			if (!crtMapperFun) {
				throw std::runtime_error("Mapper Not Registered");
				return false;				
			}

			succeed &= program.setMapper(crtMapperFun());
			ppu.createPatternTableRgb(0);
			ppu.createPatternTableRgb(1);
			
			reset();
			return succeed;
		}

		bool cNes::frameComplete()
		{
			return true;
		}

	}
}

