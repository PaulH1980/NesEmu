#include "DataBus.h"

namespace _CPU
{
    namespace _6502
    {

        cDataBus::cDataBus()
        {
          
        }

        bool cDataBus::cpuRead(cCpuBusDevice*sender, uint16_t address, uint8_t& val) const
        {
            val = 0;
            for (auto& device : m_cpuDevices) {                
                if ( device == sender ) //cannot read from self
                    continue;
                if (device->cpuAddressInRange(address) && device->cpuRead(address, val)) {
                    return true;
                }
            }
            return false;

        }

        bool cDataBus::cpuWrite(cCpuBusDevice*sender, uint16_t address, uint8_t val)
        {
            for (auto& device : m_cpuDevices)
            {
                if( device == sender )  //cannot write to self
                    continue;

                if (device->cpuAddressInRange(address) && device->cpuWrite(address, val)) {
                    return true;
                }
            }
            return false;
        }       

		bool cDataBus::ppuRead(cPpuBusDevice* sender, uint16_t address, uint8_t& val) const
		{
			address &= 0x3FFF;
            val = 0;
			for (auto& device : m_ppuDevices) {
				if (device == sender) //cannot read from self
					continue;
				if (device->ppuAddressInRange(address) && device->ppuRead(address, val)) {
                    return true;						
				}
			}
			return false;
		}

		bool cDataBus::ppuWrite(cPpuBusDevice* sender, uint16_t address, uint8_t val)
		{
            address &= 0x3FFF;
            for (auto& device : m_ppuDevices)
			{
				if (device == sender)  //cannot write to self
					continue;

				if (device->ppuAddressInRange(address) && device->ppuWrite(address, val)) {
                    return true;					
				}
			}
			return false;
		}

		bool cDataBus::addCpuDevice(cCpuBusDevice* device)
        {
            if (device->cpuAttachTo(this)) {
                m_cpuDevices.push_back(device);
                return true;
            } 
            return false;            
        }

		bool cDataBus::addPpuDevice(cPpuBusDevice* device)
		{
			if (device->ppuAttachTo(this)) {
                m_ppuDevices.push_back(device);
				return true;
			}          
            return false;            
		}

	}
}

