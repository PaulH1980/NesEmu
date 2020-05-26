#include "NesJoyPad.h"

namespace _CPU
{
	namespace _6502
	{

		cNesJoyPad::cNesJoyPad() {
			m_cpuMinAddress = 0x4016;
			m_cpuMaxAddress = 0x4017;
		}

		bool cNesJoyPad::cpuWrite(uint16_t address, uint8_t val)
		{
			m_joyPads[address & 0x1] = val; 	//0x4016-0x4017
			return true;
		}

		bool cNesJoyPad::cpuRead(uint16_t address, uint8_t& result)
		{
			result = m_joyPads[address & 0x1]; //0x4016-0x4017
			return true;
		}

		void cNesJoyPad::reset()
		{
			m_joyPads[0] = m_joyPads[1] = 0;
		}

	}
}

