#pragma once
#include <array>
#include <vector>
#include "BusDevice.h"
namespace _CPU
{
    
    /*
     	$0000-1FFF is normally mapped by the cartridge to a CHR-ROM or CHR-RAM, often with a bank switching mechanism.
        $2000-2FFF is normally mapped to the 2kB NES internal VRAM, providing 2 nametables with a mirroring configuration controlled by the cartridge, but it can be partly or fully remapped to RAM on the cartridge, allowing up to 4 simultaneous nametables.
        $3000-3EFF is usually a mirror of the 2kB region from $2000-2EFF. The PPU does not render from this address range, so this space has negligible utility.
        $3F00-3FFF is not configurable, always mapped to the internal palette control.    
    */
    //https://wiki.nesdev.com/w/index.php/PPU_programmer_reference
    namespace _6502
    {
#pragma pack(push, 1)
        enum ePPU : uint32_t
        {
            PPU_CONTROL = 0,
            PPU_MASK,
            PPU_STATUS,
            PPU_OAMADDR,
            PPU_OAMDATA,
            PPU_SCROLL,
            PPU_ADDR,
            PPU_DATA,
            PPU_OAMDMA  = 0x4014
        };

		enum eCurScroll : std::uint8_t
		{
			SCROLL_X = 0,
			SCROLL_Y
		};

        enum ePpuAddress : std::uint8_t
        {
            WRITE_LO = 0, 
            WRITE_HI
        };

        enum eMirroring : std::uint8_t
        {
            MIRROR_HORIZONTAL = 0,
            MIRROR_VERTICAL   = 1
        };


        //0x2000
        struct sPPUController {

            union {
                uint8_t     m_packedData;
                struct {

                    uint8_t m_nameTableAddress          : 2; //Base nametable address
                    uint8_t m_vramIncrement             : 1; //VRAM address increment per CPU read/write of PPUDATA (0: add 1, going across; 1: add 32, going down)
                    uint8_t m_spritePatternTable8x8     : 1;
                    uint8_t m_backgroundTableAddres     : 1;
                    uint8_t m_spriteSize                : 1;
                    uint8_t m_ppuMasterSlaveSelected    : 1;
                    uint8_t m_generateNMIAtVBlanck      : 1;
                };
            };
        };

        //0x2001
        struct sPPURenderControl
        {
            union {
                uint8_t     m_packedData;
                struct {
                    uint8_t m_greyScale                 : 1;
                    uint8_t m_showBackgroundLeftMost    : 1;
                    uint8_t m_showSpritesLeftMost       : 1;
                    uint8_t m_showBackground            : 1;
                    uint8_t m_showSprites               : 1;
                    uint8_t m_emphasizeRed              : 1;
                    uint8_t m_empasizeGreen             : 1;
                    uint8_t m_emphasizeBlue             : 1;
                };
            };
        };

        // 0x2002
		struct sPPUStatus
		{
            union {
                uint8_t     m_packedData;
                struct {
                    uint8_t m_leastSigBitsPrevWritten   : 5;
                    uint8_t m_spriteOverflow            : 1;
                    uint8_t m_spriteZeroHit             : 1;
                    uint8_t m_vblankStarted             : 1;
                };
            };
		};

        //0x2005
		struct sPPUScroll
		{
			union
			{
				uint16_t m_packedData;
				struct {
					uint16_t m_coarseScrollX    : 5;
					uint16_t m_coarseScrollY    : 5;
					uint16_t m_nameTable        : 2;
					uint16_t m_fineScrollY      : 3;
					uint16_t m_unused           : 1;
				};
			};
		};


        
        //object attribute memory
        struct sOAM
        {
            uint8_t     m_yPosition;
            union 
            {
                uint8_t m_tileIndex;
                struct {
                    uint8_t m_tileBank   : 1;
                    uint8_t m_tileNumber : 7;
                };
            };
            union 
            {
                uint8_t  m_attributes;
                struct {
                    uint8_t m_palette        : 2;
                    uint8_t m_unused         : 3;
                    uint8_t m_priority       : 1;
                    uint8_t m_flipHorizontal : 1;
                    uint8_t m_flipVertical   : 1;
                };
            };
            uint8_t     m_xPosition;
        };

        struct sTileOffset
        {
            union
            {
                uint16_t m_packedData;
                struct {
                    uint16_t m_fineYoffset      : 3;
                    uint16_t m_bitPlane         : 1;
                    uint16_t m_tileColumn       : 4;
                    uint16_t m_tileRow          : 4;
                    uint16_t m_halfSpriteTable  : 1;
                    uint16_t m_tableLocation    : 1;
                    uint16_t m_padding          : 2;
                };
            };
        };



        /*
            @brief: 2 patterns addressed at 0x0000-0x0FFF and 0x1000-0x1FFF
            This contains 2 pattern tables each consisting of 8*8 pixels 
        */
        class cPatternTable : public cPpuBusDevice
        {
        public:
            cPatternTable();

			bool                        ppuWrite(uint16_t address, uint8_t val) override;
			bool                        ppuRead(uint16_t address, uint8_t& val) override;

            
            union {
				uint8_t m_packedData[2 * 1024];
				struct
				{
					uint8_t m_patternTables[2][1024];
				};
			};

        };

        /*
            @brief: 
        */
        class cNameTables : public cPpuBusDevice
        {
        public:
            cNameTables();
            
			bool                        ppuWrite(uint16_t address, uint8_t val) override;
			bool                        ppuRead(uint16_t address, uint8_t& val) override;

            union {
                uint8_t m_packedData[4 * 1024];
                struct
                {
                    uint8_t m_nameTables[4][1024];
                };
            };
        };


		class cPaletteData : public cPpuBusDevice
		{
        public:       
            cPaletteData();

			bool                        ppuWrite(uint16_t address, uint8_t val) override;
			bool                        ppuRead(uint16_t address, uint8_t& val) override;

            uint8_t                     m_paletteData[32];
		};


        struct sOamData
        {
            union
            {
                uint8_t m_packedData[256];
                struct {
                    sOAM m_tileSets[64];
                };
            };
        };

       

        enum eFrameStatus : std::uint8_t
        {
            FRAME_VISIBLE = 0, //scanline [0-239]   //currently rendering
            FRAME_POST    = 1, //scanline [240]
            FRAME_VBLANK  = 2, //scanline [241-260]
            FRAME_PRE     = 3  //scanline [261]
        };

#pragma pack(pop) 

        //verify sizes	
		static_assert(sizeof(sOamData) == 256);
		static_assert(sizeof(sOAM) == 4);
        static_assert(sizeof(sPPURenderControl) == 1);
        static_assert(sizeof(sPPUController) == 1);
        static_assert(sizeof(sPPUStatus) == 1);   
        
        /*
            @brief: PPU the gpu of the nes, address range is from 0x2000 to 0x3FFF in cpu space
        */
        class cPPU 
            : public cCpuBusDevice
            , public cPpuBusDevice {
        public:
            cPPU();
            ~cPPU();

            //read/write data to cpu bus
            bool                        cpuWrite(uint16_t address, uint8_t val) override;
            bool                        cpuRead(uint16_t address, uint8_t& val) override;

			bool                        ppuWrite(uint16_t address, uint8_t val) override;
			bool                        ppuRead(uint16_t address, uint8_t& val) override;

            void                        createPatternTableRgb(int idx);
            uint32_t                    getColorFromPalette(uint8_t palette, uint8_t pixel);

                       
            /*
                @brief: Retrieve frame buffer
            */
            uint32_t*                   getFrameBuffer();

            void                        tick();
            void                        reset();
            bool                        nmi();

        private:
            void                        plotPixel();

            void                        createPalette();
            void                        randomizePixels();           

            uint8_t                     readStatus();
            uint8_t                     readOamData();
            uint8_t                     readPpuData();

            void                        writeControl(uint8_t val);
            void                        writeRenderControl(uint8_t val);
            void                        writeStatus(uint8_t val);
            void                        writeScroll( uint8_t val);
            void                        writePpuAddress(uint8_t val);
            void                        writePpuData(uint8_t val);
            void                        writeOamData(uint8_t val);
            void                        writeOamAddress(uint8_t val);
            void                        writeOamDma(uint8_t val);

            eFrameStatus                getFrameStatus() const;         
            
        public://public for now

            cPatternTable*              getPatternTable() const;
            cNameTables*                getNameTable() const;
            cPaletteData*               getPalette() const;

            cNameTables*                m_nameTable    = nullptr;
            cPatternTable*              m_patternTable = nullptr;
            cPaletteData*               m_paletteData  = nullptr;

            sOamData                    m_oamData;             
            uint8_t                     m_oamAddress   = 0;

            uint32_t                    m_cycle;        //0-340
            uint32_t                    m_scanLine;     //0-261 0-239=visible, 240=post, 241-260=vblank, 261=pre
           
            sPPUController              m_ppuController;
            sPPURenderControl           m_ppuRenderControl;
            sPPUStatus                  m_ppuStatus; 
            sPPUScroll                  m_ppuScroll; // 0x2005

            uint8_t                     m_writeToggle = 0; //if '1' write to hi word else write lo word
            uint8_t                     m_dataBuffer  = 0;        
            uint16_t                    m_ppuAddress  = 0;
            
            //output
            std::vector<uint32_t>       m_frameBufferRgb;       //rgb pixels
            std::vector<uint32_t>       m_paletteRgb;			
            std::vector<uint32_t>       m_patternTablesRgb[2];

			bool                        m_nmi       = false;
            bool                        m_pre       = false;
            bool                        m_post      = false;
            bool                        m_visible   = false;
            bool                        m_vblank    = false;

        };
    }
}