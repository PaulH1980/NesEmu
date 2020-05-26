#pragma once
#pragma once
#include <array>
#include <vector>
#include "Shared.h"
#include "BusDevice.h"
#include "iMapper.h"

namespace _CPU
{
    namespace _6502
    {

		// https://wiki.nesdev.com/w/index.php/INES
#pragma pack(push, 1)

		struct sINesHeader
		{
			uint8_t  m_magicWord[4];    //'NES' null terminated
			uint8_t  m_numPrgBanks;     //Size of PRG ROM in 16 KB units
			uint8_t  m_numChrBanks;     //Size of CHR ROM in 8 KB units (Value 0 means the board uses CHR RAM)

			union {
				uint8_t  m_romInfo1;        //Mapper, mirroring, battery, trainer
				struct {
					uint8_t mirroring : 1;      //0: horizontal (vertical arrangement) (CIRAM A10 = PPU A11), 1: vertical (horizontal arrangement) (CIRAM A10 = PPU A10)
					uint8_t hasBattery : 1;     //Cartridge contains battery-backed PRG RAM ($6000-7FFF) or other persistent memory
					uint8_t hasTrainer : 1;    // 512 - byte trainer at $7000 - $71FF(stored before PRG data)
					uint8_t ignoreMirroring : 1;  //Ignore mirroring control or above mirroring bit; instead provide four-screen VRAM
					uint8_t mapperLo : 4; //Lower nybble of mapper numbe
				} romInfo1AsStruct;
			};

			union {
				uint8_t  m_romInfo2;        //Mapper, VS/Playchoice, NES 2.0
				struct {
					uint8_t vsUniSystem : 1;  //VS Unisystem
					uint8_t playChoice10 : 1; //PlayChoice-10 (8KB of Hint Screen data stored after CHR data)
					uint8_t nesFormat2 : 2;  //If equal to 2, flags 8-15 are in NES 2.0 format
					uint8_t mapperHi : 4;   // Upper nybble of mapper number
				}romInfo2AsStruct;
			};

			uint8_t  m_prgRamSize;      //PRG-RAM size (rarely used extension)
			uint8_t  m_tvSystem1;       //TV system (rarely used extension)
			uint8_t  m_tvSystem2;       // TV system, PRG-RAM presence (unofficial, rarely used extension)
			uint8_t  m_padding[5];
		};
#pragma pack(pop) 

		/*
            @brief: cNesRom this is where the program and graphics live that makes up a nes game/program
        */
        class cNesRom 
			: public cCpuBusDevice
			, public cPpuBusDevice 
		{
        public:

            cNesRom();
			~cNesRom();

			bool        loadFromFile(const std::string& fileName);
			//assumes heap memory, takes ownership
			bool        setMapper(iMapper* mapper);			

            bool        cpuWrite(uint16_t address, uint8_t val) override;

            bool        cpuRead(uint16_t address, uint8_t& val) override;

			bool		ppuRead(uint16_t address, uint8_t& val) override;

			bool        ppuWrite(uint16_t address, uint8_t val) override;

			bool		romLoaded() const;

			int			getMapperId() const;

		private:

			sINesHeader  m_nesHeader = {};

            std::vector<uint8_t>	 m_progData; //(cpu)program data
            std::vector<uint8_t>	 m_chrData;  //(ppu)bitmaps etc

            int						 m_numChrBanks = 0;
            int						 m_numProgBank = 0;
			int						 m_mapperId = -1;   
			std::unique_ptr<iMapper> m_mapper;            
			bool					 m_loaded = false;
        };
    }
}

