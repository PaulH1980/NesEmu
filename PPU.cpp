#include "Shared.h"
#include "DataBus.h"
#include "PPU.h"



namespace _CPU
{
    namespace _6502
    {

        cPPU::cPPU()
        {
            m_cpuMinAddress = 0x2000;
            m_cpuMaxAddress = 0x3FFF;  
			m_cpuMask       = 0x0007; 
			m_ppuMinAddress = 0x0000;
			m_ppuMinAddress = 0xFFFF;
            m_frameBufferRgb.resize(NES_WIDTH * NES_HEIGHT);

			createPalette(); 
			
        }

        cPPU::~cPPU()
        {
        }
       

        bool cPPU::cpuWrite(uint16_t address, uint8_t val)
        {
		    const uint16_t remapAddress = address % 8; //ppu addressed are mirrored every 8 bytes
            switch (remapAddress)
            {
            case PPU_CONTROL:
				writeControl(val);
                break;
            case PPU_MASK: 
				writeRenderControl(val);
                break;
            case PPU_STATUS:
				writeStatus(val);
                break;
            case PPU_OAMADDR:
				writeOamAddress(val);
                break;
            case PPU_OAMDATA:
				writeOamData(val);
                break;
            case PPU_SCROLL:	
				writeScroll(val);
                break;
			case PPU_ADDR:
				writePpuAddress(val);
				break;
			case PPU_DATA:
				writePpuData(val);
				break;
            case PPU_OAMDMA:
				////writeOamDma(value);
                break;
            };

            return true;
        }

        bool cPPU::cpuRead(uint16_t address, uint8_t& val)
        {
			val = 0;
            const uint16_t remapAddress = address % 8; //ppu addressed are mirrored every 8 bytes
			switch (remapAddress)
			{
			case PPU_CONTROL:
				break;
			case PPU_MASK:
				break;
			case PPU_STATUS:
				val = readStatus();				
				break;
			case PPU_OAMADDR:
				break;
			case PPU_OAMDATA:
				break;
			case PPU_SCROLL:
				break;
			case PPU_ADDR:
				break;
			case PPU_DATA:
				val = readPpuData();
				break;				
			};


            return true;
        }

		bool cPPU::ppuWrite(uint16_t address, uint8_t val)
		{
			return m_ppuDataBus->ppuWrite( this, address, val );
		}

		bool cPPU::ppuRead(uint16_t address, uint8_t& val)
		{
			return m_ppuDataBus->ppuRead( this, address, val );			
		}

	

		uint32_t* cPPU::getFrameBuffer()
		{
            
            return m_frameBufferRgb.data();
		}

		void cPPU::tick()
		{
			//#todo
			if (m_cycle >= 341)
			{
				m_cycle = 0;
				m_scanLine++;
			}
			if (m_scanLine == 241)
			{
				m_nmi = true;
			}
			if (m_scanLine > 261)
				m_scanLine = 0;
			m_cycle++;

		}

		void cPPU::reset()
		{
            m_cycle = 0;
            m_scanLine = 0;
			m_writeToggle = 0;
		}

		bool cPPU::nmi()
		{
			if (m_nmi) {
				m_nmi = false;
				return true;
			}			
			return false;
		}

		void cPPU::plotPixel()
		{

		}

		void cPPU::createPalette()
		{
            m_paletteRgb = {
				0x666666, 0x002A88, 0x1412A7, 0x3B00A4, 0x5C007E, 0x6E0040, 0x6C0600, 0x561D00,
				0x333500, 0x0B4800, 0x005200, 0x004F08, 0x00404D, 0x000000, 0x000000, 0x000000,
				0xADADAD, 0x155FD9, 0x4240FF, 0x7527FE, 0xA01ACC, 0xB71E7B, 0xB53120, 0x994E00,
				0x6B6D00, 0x388700, 0x0C9300, 0x008F32, 0x007C8D, 0x000000, 0x000000, 0x000000,
				0xFFFEFF, 0x64B0FF, 0x9290FF, 0xC676FF, 0xF36AFF, 0xFE6ECC, 0xFE8170, 0xEA9E22,
				0xBCBE00, 0x88D800, 0x5CE430, 0x45E082, 0x48CDDE, 0x4F4F4F, 0x000000, 0x000000,
				0xFFFEFF, 0xC0DFFF, 0xD3D2FF, 0xE8C8FF, 0xFBC2FF, 0xFEC4EA, 0xFECCC5, 0xF7D8A5,
				0xE4E594, 0xCFEF96, 0xBDF4AB, 0xB3F3CC, 0xB5EBF2, 0xB8B8B8, 0x000000, 0x000000 
            };
		}

		void cPPU::randomizePixels()
		{
			for (int i = 0; i < m_frameBufferRgb.size(); ++i) {
				int idx = rand() % 63;
				m_frameBufferRgb[i] = m_paletteRgb[idx];
			}
		}

		//https://wiki.nesdev.com/w/index.php/PPU_pattern_tables
		void cPPU::createPatternTableRgb(int idx)
		{
			const auto& patternTable = getPatternTable()->m_patternTables[idx];
			const auto rowStride = 256; //16 bytes * 16 columns
			const auto bytesPerTile = 16;
			
			m_patternTablesRgb[idx].resize(128 * 128);
			
			for (int y = 0; y < 16; y++) {
				for (int x = 0; x < 16; x++)
				{
					int startIdx = x * bytesPerTile + (y * rowStride);
					for (int row = 0; row < 8; row++)
					{
						int yLoc = y * 8 + row;			
						uint8_t bitPlane0, bitPlane1;
						const auto baseAddress = idx * 0x1000 + startIdx + row;
						bool succeed = ppuRead( baseAddress + 0, bitPlane0 );
						succeed		&= ppuRead( baseAddress + 8, bitPlane1 );
						
						if (succeed) {
							
							int xLoc = x * 8 + 7; // +7 because we're iterating backwards in bit-positions
							for (int col = 0; col < 8; col++)
							{
								uint8_t pixel = (bitPlane0 & 0x01) + ((bitPlane1& 0x01) * 2) + 0;
								bitPlane0 >>= 1;
								bitPlane1 >>= 1;
								const auto tblIdx = yLoc * 128 + xLoc;	
								m_patternTablesRgb[idx][tblIdx] = m_paletteRgb[pixel];   //#todo correct palette lookup getColorFromPalette(3, pixel); //
								xLoc--; 
							}
						}						
					}
				}
			}
		}

		uint32_t cPPU::getColorFromPalette(uint8_t palette, uint8_t pixel)
		{
			uint8_t val;
			bool succeed = ppuRead( 0x3F00 + ( palette << 2 ) + pixel, val);			
			return m_paletteRgb[val & 0x3f];
		}

		

		uint8_t cPPU::readStatus()
		{
			m_ppuStatus.m_vblankStarted = 0;
			m_writeToggle				= 0;
			return m_ppuStatus.m_packedData;
		}

		uint8_t cPPU::readOamData()
		{
			return m_oamData.m_packedData[m_oamAddress];
		}

		uint8_t cPPU::readPpuData()
		{
			//uint8_t data = 0;
			//if (m_ppuAddress > 0x3F00)
			//{
			//	//data = 
			//}
			//else {
			//	data = m_patternTables.m_packedData[m_ppuAddress];	
			//}
			//if (m_ppuController.m_vramIncrement)
			//	m_ppuAddress += 32;
			//else
			//	m_ppuAddress += 1;
			//return data;
			return 0;
		}

		void cPPU::writeControl(uint8_t val)
		{
			m_ppuController.m_packedData = val;
		}

		void cPPU::writeRenderControl(uint8_t val)
		{
			m_ppuRenderControl.m_packedData = val;
		}

		void cPPU::writeStatus(uint8_t val)
		{
			m_ppuStatus.m_packedData = val;
		}

		void cPPU::writeScroll(uint8_t val)
		{
			if (m_writeToggle)
			{
				uint16_t hi = val;
				uint16_t lo = m_dataBuffer;
				m_ppuScroll.m_packedData = (hi << 8) | lo;
				m_writeToggle = 0;
			}
			{
				m_dataBuffer = val;
				m_writeToggle = 1;
			}
		}

		void cPPU::writePpuAddress(uint8_t val)
		{
			if (m_writeToggle)
			{
				uint16_t hi   = val;
				uint16_t lo   = m_dataBuffer;
				m_ppuAddress  = (hi << 8) | lo;
				m_writeToggle = 0;
			}
			else
			{
				m_dataBuffer = val;
				m_writeToggle = 1;
			}
		}

		void cPPU::writePpuData(uint8_t val)
		{
			////#todo don't do this during rendering
			//m_patternTables.m_packedData[m_ppuAddress] = val;
			//if (m_ppuController.m_vramIncrement)
			//	m_ppuAddress += 32;
			//else
			//	m_ppuAddress += 1;

		}

		void cPPU::writeOamData(uint8_t val)
		{
			m_oamData.m_packedData[m_oamAddress++] = val;
		}

		void cPPU::writeOamAddress(uint8_t val)
		{
			m_oamAddress = val;
		}

		void cPPU::writeOamDma(uint8_t val)
		{
			
		}

		eFrameStatus cPPU::getFrameStatus() const
		{
			if (m_scanLine >= 0 && m_scanLine <= 239)
				return FRAME_VISIBLE;
			else if (m_scanLine == 240)
				return FRAME_POST;
			else if (m_scanLine >= 241 && m_scanLine <= 260)
				return FRAME_VBLANK;
			else if (m_scanLine == 261)
				return FRAME_PRE;
			else
				throw std::runtime_error("Invalid ScanLine");
		}


		cPatternTable* cPPU::getPatternTable() const
		{
			//#FIXME slow!!
			for (auto device : m_ppuDataBus->m_ppuDevices)
			{
				if (auto result = dynamic_cast<cPatternTable*>(device))
					return result;
			}
			return nullptr;
		}

		cNameTables* cPPU::getNameTable() const
		{
			//#FIXME slow!!
			for (auto device : m_ppuDataBus->m_ppuDevices)
			{
				if (auto result = dynamic_cast<cNameTables*>(device))
					return result;
			}
			return nullptr;
		}

		cPaletteData* cPPU::getPalette() const
		{
			//#FIXME slow!!
			for (auto device : m_ppuDataBus->m_ppuDevices)
			{
				if (auto result = dynamic_cast<cPaletteData*>(device))
					return result;
			}
			return nullptr;
		}

		//////////////////////////////////////////////////////////////////////////
		//cPatternTable
		//////////////////////////////////////////////////////////////////////////
		cPatternTable::cPatternTable()
		{
			m_ppuMinAddress = 0x0000;
			m_ppuMaxAddress = 0x1FFF;
		    m_ppuMask       = 0x001F;
		}

		bool cPatternTable::ppuWrite(uint16_t address, uint8_t val)
		{
			m_packedData[address & m_ppuMask] = val;
			return true;
		}

		bool cPatternTable::ppuRead(uint16_t address, uint8_t& val)
		{
			val = m_packedData[address & m_ppuMask];
			return true;
		}

		//////////////////////////////////////////////////////////////////////////
		//\cNameTables
		//////////////////////////////////////////////////////////////////////////
		cNameTables::cNameTables()
		{
			m_ppuMinAddress = 0x2000;
			m_ppuMaxAddress = 0x3EFF;
			m_ppuMask       = 0x0FFF;
		}

		bool cNameTables::ppuWrite(uint16_t address, uint8_t val)
		{
			return false;
		}

		bool cNameTables::ppuRead(uint16_t address, uint8_t& val)
		{
			return false;
		}

		cPaletteData::cPaletteData()
		{
			m_ppuMinAddress = 0x3F00;
			m_ppuMaxAddress = 0x3FFF;
			m_ppuMask       = 0x001F;
		}

		bool cPaletteData::ppuWrite(uint16_t address, uint8_t val)
		{
			address &= m_ppuMask;
			if (address == 0x0010) address = 0x0000;
			if (address == 0x0014) address = 0x0004;
			if (address == 0x0018) address = 0x0008;
			if (address == 0x001C) address = 0x000C;
			m_paletteData[address] = val;
			return true;
		}

		bool cPaletteData::ppuRead(uint16_t address, uint8_t& val)
		{
			address &= m_ppuMask;
			if (address == 0x0010) address = 0x0000;
			if (address == 0x0014) address = 0x0004;
			if (address == 0x0018) address = 0x0008;
			if (address == 0x001C) address = 0x000C;	
			val = m_paletteData[address];
			return true;
		}

	}
}