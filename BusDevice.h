#pragma once
#include <cstdint>

namespace _CPU
{
    namespace _6502
    {
        class cDataBus;

        /*
            @brief: Interface for devices connected to the CPU
        */
        class iCpuBusDevice
        {
        public:
            virtual ~iCpuBusDevice() = default;
            virtual bool		cpuAttachTo(cDataBus*) = 0;           
            virtual bool        cpuWrite(uint16_t address, uint8_t val) = 0;            
            virtual bool        cpuRead(uint16_t address, uint8_t& result) = 0;
        };

        /*
            @brief: Interface for devices connected to PPU
        */
        class iPpuBusDevice
        {
        public:
            virtual ~iPpuBusDevice() = default;
            virtual bool		ppuAttachTo(cDataBus*) = 0;
            virtual bool        ppuWrite(uint16_t address, uint8_t val) = 0;
            virtual bool        ppuRead(uint16_t address, uint8_t& result) = 0;
        };        

        /*
            @brief: Abstract Base class for devices on the cpu databus
        */
        class cCpuBusDevice : public iCpuBusDevice {
        public:

            cCpuBusDevice();

            virtual ~cCpuBusDevice();

            /*
                @brief: Attach this device to the databus
            */
            virtual bool		cpuAttachTo(cDataBus* bus);

            /*
                @brief: Get mappable address range, wherein this device listens
            */
            bool                cpuGetAddressRange(uint16_t& min, uint16_t& max) const;

            /*
                @brief: Indicates if the address fits in this device it's mappable
                address space
            */
            bool                cpuAddressInRange(uint16_t address) const;

        protected:
            cDataBus*       m_cpuDataBus    = nullptr;
            uint16_t        m_cpuMinAddress = 0xFFFF; //set to invalid range
            uint16_t        m_cpuMaxAddress = 0;
            uint32_t        m_cpuMask       = 0xFFFF;
        };

        /*
           @brief: Abstract Base class for devices on the ppu databus
        */
        class cPpuBusDevice : public iPpuBusDevice {
        public:

            cPpuBusDevice();

            virtual ~cPpuBusDevice();

            /*
                @brief: Attach this device to the databus
            */
            virtual bool		ppuAttachTo(cDataBus* bus);

            /*
                @brief: Get mappable address range, wherein this device listens
            */
            bool                ppuGetAddressRange(uint16_t& min, uint16_t& max) const;

            /*
                @brief: Indicates if the address fits in this device it's mappable
                address space
            */
            bool                ppuAddressInRange(uint16_t address) const;


        protected:
            cDataBus*       m_ppuDataBus    = nullptr;
			uint32_t        m_ppuMinAddress = 0xFFFF; //set to invalid range
			uint32_t        m_ppuMaxAddress = 0;
            uint32_t        m_ppuMask       = 0xFFFF;
        };
    }
    
}