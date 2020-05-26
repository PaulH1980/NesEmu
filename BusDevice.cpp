#include "DataBus.h"
#include "BusDevice.h"

namespace _CPU
{
    namespace _6502
    {

        cCpuBusDevice::cCpuBusDevice(){

        }

        cCpuBusDevice::~cCpuBusDevice() = default;

		bool cCpuBusDevice::cpuAttachTo(cDataBus* bus)
		{
			m_cpuDataBus = bus;
			return m_cpuDataBus != nullptr;
		}

		bool cCpuBusDevice::cpuGetAddressRange(uint16_t& min, uint16_t& max) const
        {
            min = m_cpuMinAddress;
            max = m_cpuMaxAddress;
            return min <= max;
        }

        bool cCpuBusDevice::cpuAddressInRange(uint16_t address) const
        {
            return (address >= m_cpuMinAddress) && (address <= m_cpuMaxAddress);
        }

    //////////////////////////////////////////////////////////////////////////
	// cPpuDevice : Asbstract base class for ppu/gpu devices
	//////////////////////////////////////////////////////////////////////////

		cPpuBusDevice::cPpuBusDevice()
		{

		}

		cPpuBusDevice::~cPpuBusDevice()
		{

		}

		bool cPpuBusDevice::ppuAttachTo(cDataBus* bus)
		{
			m_ppuDataBus = bus;
			return m_ppuDataBus != nullptr;
		}

		bool cPpuBusDevice::ppuGetAddressRange(uint16_t& min, uint16_t& max) const
		{
			min = m_ppuMinAddress;
			max = m_ppuMaxAddress;
			return min <= max;
		}

		bool cPpuBusDevice::ppuAddressInRange(uint16_t address) const
		{
			return (address >= m_ppuMinAddress) && (address <= m_ppuMaxAddress);
		}

	}
}

