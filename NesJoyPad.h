#pragma once

#pragma once
#include <array>
#include "BusDevice.h"
namespace _CPU
{
	//https://wiki.nesdev.com/w/index.php/Standard_controller
	namespace _6502
	{
		enum eJoyPadButtons : std::uint8_t
		{
			BUTTON_A  = 0,
			BUTTON_B,
			BUTTON_SELECT,
			BUTTON_START,
			BUTTON_UP,
			BUTTON_DOWN,
			BUTTON_LEFT,
			BUTTON_RIGHT,				
		};
		
		
		class cNesJoyPad : public cCpuBusDevice {
		public:
			cNesJoyPad();

			bool        cpuWrite(uint16_t address, uint8_t val) override;
			bool        cpuRead(uint16_t address, uint8_t& result) override;
			void		reset();
			
			uint8_t		m_joyPads[2] = { 0 };
		};
	}
}