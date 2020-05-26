#pragma once
#include <vector>
#include "BusDevice.h"


namespace _CPU
{
    namespace _6502
    {
        class cDataBus
        {
        public:
            cDataBus();

            bool     cpuRead(cCpuBusDevice*sender, uint16_t address, uint8_t& val) const;
            bool     cpuWrite(cCpuBusDevice*sender, uint16_t address, uint8_t val);

			bool     ppuRead(cPpuBusDevice* sender, uint16_t address, uint8_t& val) const;
			bool     ppuWrite(cPpuBusDevice* sender, uint16_t address, uint8_t val);

            bool     addCpuDevice(cCpuBusDevice* device);          
            bool     addPpuDevice(cPpuBusDevice* device);

            std::vector<cCpuBusDevice*> m_cpuDevices; //connected cpu devices 
            std::vector<cPpuBusDevice*> m_ppuDevices; //connected ppu devices 
        };

    }
}
