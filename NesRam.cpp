#include "NesRam.h"

namespace _CPU
{
    namespace _6502
    {

        cNesRam::cNesRam()
        {
			m_cpuMinAddress = 0x0000;
			m_cpuMaxAddress = 0x1FFF;
            clearMem();
        }


        void cNesRam::clearMem()
        {
            memset(m_ram.data(), 0, sizeof(uint8_t) * m_ram.size());
        }       

        bool cNesRam::cpuWrite( uint16_t address, uint8_t val )
        {
            m_ram[address % 2048] = val; //ram is mirrored in 2k chunks up to 8kb address range
            return true;
        }      

        bool cNesRam::cpuRead(uint16_t address, uint8_t& val)
        {
            val = m_ram[address % 2048]; //ram is mirrored in 2k chunks up to 8kb address range            
            return true;
        }

    }
}

