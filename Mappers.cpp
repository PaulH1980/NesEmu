#include "Mappers.h"

namespace _CPU
{
	namespace _6502
	{


		//////////////////////////////////////////////////////////////////////////
		//\Mapper00
		//////////////////////////////////////////////////////////////////////////
		Mapper00::Mapper00()
		{

		}

		bool Mapper00::cpuRead(uint16_t address, uint32_t& result)
		{
			address &= m_numPrgBank > 1 ? 0x7FFF : 0x3FFF;
			result = address;
			return true;
		}

		bool Mapper00::cpuWrite(uint16_t address, uint32_t& result)
		{
			address &= m_numPrgBank > 1 ? 0x7FFF : 0x3FFF;
			result = address;
			return true;
		}

		bool Mapper00::ppuRead(uint16_t address, uint32_t& result)
		{
			if (address >= 0x0000 && address <= 0x1FFF) {
				result = address;
				return true;
			}
			return false;
		}

		bool Mapper00::ppuWrite(uint16_t address, uint32_t& result)
		{
			if (address >= 0x0000 && address <= 0x1FFF) {
				result = address;
				return true;
			}
			return false;			
		}

	}
}

