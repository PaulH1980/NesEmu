#include <fstream>
#include "NesRom.h"


namespace _CPU
{
    namespace _6502
    {
        static_assert(sizeof(sINesHeader) == 16);
        
        cNesRom::cNesRom()
        {
			m_cpuMinAddress = 0x8000;
			m_cpuMaxAddress = 0xFFFF;
            m_ppuMinAddress = 0x0000;
            m_ppuMaxAddress = 0xFFFF;
            m_progData.resize(8192);
        }       

		cNesRom::~cNesRom()
		{

		}

		bool cNesRom::cpuWrite(uint16_t address, uint8_t val)
        {
            if (m_mapper && m_loaded) {
				uint32_t mappedAddress;
                if (m_mapper->cpuWrite(address, mappedAddress)) {
                    m_progData[mappedAddress] = val;
                    return true;
                }
			}
            
            return false;
        }

        bool cNesRom::cpuRead(uint16_t address, uint8_t& val)
        {
	        if (m_mapper && m_loaded)
			{
                uint32_t mappedAddress;
                if (m_mapper->cpuRead(address, mappedAddress)) {
                    val = m_progData[mappedAddress];
                    return true;
                }
			}    
            return false;
        }

		bool cNesRom::ppuRead(uint16_t address, uint8_t& val)
		{
		    if (m_mapper && m_loaded)
            {
				uint32_t mappedAddress;
                if (m_mapper->ppuRead(address, mappedAddress)) {
                    val = m_chrData[mappedAddress];
                    return true;
                }
            }
            return false;
		}

		bool cNesRom::ppuWrite(uint16_t address, uint8_t val)
		{
			if (m_mapper && m_loaded)
			{
				uint32_t mappedAddress;
                if (m_mapper->ppuWrite(address, mappedAddress)) {
                    m_chrData[mappedAddress] = val;
                    return true;
                }
			}
            return false;
		}

		bool cNesRom::romLoaded() const
		{
			return m_loaded;
		}

		int cNesRom::getMapperId() const
		{
            return m_mapperId;
		}

		bool cNesRom::setMapper(iMapper* mapper)
		{
            if (!mapper)
                return false;            
            m_mapper = std::unique_ptr<iMapper>(mapper);           
            m_mapper->setBankInfo(m_numChrBanks, m_numProgBank);
            return true;
		}

		bool cNesRom::loadFromFile(const std::string& fileName)
		{
            std::ifstream fileIn(fileName, std::ios::binary);
            fileIn.read((char*)(&m_nesHeader), sizeof(sINesHeader));
            //skip past trainer data
            if (m_nesHeader.romInfo1AsStruct.hasTrainer) {
                fileIn.seekg(512, std::ios_base::cur);
            }
            if (m_nesHeader.romInfo2AsStruct.nesFormat2 == 2)
                return false;

            //extract mapper id
            m_mapperId    = (m_nesHeader.romInfo2AsStruct.mapperHi << 4) | m_nesHeader.romInfo1AsStruct.mapperLo;
            m_numProgBank = m_nesHeader.m_numPrgBanks;
            m_numChrBanks = m_nesHeader.m_numChrBanks;

            m_progData.resize((m_numProgBank * NES_PRG_BANK_SIZE));
            fileIn.read((char*)m_progData.data(), m_progData.size());

            m_chrData.resize(m_numChrBanks * NES_CHR_BANK_SIZE);           
            fileIn.read((char*)m_chrData.data(), m_chrData.size());

            m_loaded = true;
            return true;
		}

	}
}