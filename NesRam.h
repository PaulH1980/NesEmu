#pragma once
#include <array>
#include "BusDevice.h"
namespace _CPU
{
    namespace _6502
    {
        class cNesRam : public cCpuBusDevice {
        public:
            
            cNesRam();

            void        clearMem();  

            bool        cpuWrite(uint16_t address, uint8_t val) override;
           
            bool        cpuRead(uint16_t address, uint8_t& val) override;

            std::array<uint8_t, 2048> m_ram;
        };
    }
}