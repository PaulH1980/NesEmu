#pragma once
#include <cstdint>

namespace _CPU
{
    namespace _6502
    {
        /*
            @brief: Interface for mappers e.g. indirection tables/bank switching
        */
        class iMapper
        {
        public:

           virtual ~iMapper() = default;

           void             setBankInfo(int numChrBanks, int numPrgBanks) {
               m_numChrBanks = numChrBanks;
               m_numPrgBank  = numPrgBanks;
           }

           //cpu indirection
           virtual  bool    cpuRead(uint16_t address, uint32_t& result) = 0;
           virtual  bool    cpuWrite(uint16_t address, uint32_t& result) = 0;
           //ppu indirection
           virtual  bool    ppuRead(uint16_t address, uint32_t& result) = 0;
           virtual  bool    ppuWrite(uint16_t address, uint32_t& result) = 0;

        protected:
            int m_numChrBanks = 0;
            int m_numPrgBank = 0;
        };
    }
}