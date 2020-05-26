#include <assert.h>
#include <iostream>
#include "6502.h"

namespace _CPU
{

	namespace _6502
	{
        //references:
        //https://www.pagetable.com/?p=410
        //https://wiki.nesdev.com/w/index.php/CPU_unofficial_opcodes
        //https://www.masswerk.at/6502/6502_instruction_set.html#BRK
        //https://wiki.nesdev.com/w/index.php/Status_flags



        //helper function that returns if 2 memory addresses live on the
        //same page
        inline bool SamePage(const uint16_t& a, const uint16_t& b) {
            return (a & 0xFF00) == (b & 0xFF00);
        }

       
        /*
            @brief: helper function to avoid repetition/errors
            will N(egative) and Z(ero) flags for a given value
            nb. will only check the lower 8 bits
        */
        inline void SetN_Z(const uint16_t val, cCpu* cpu) {
            cpu->setStatus(ZERO_MASK, (val&0x00FF) == 0);
            cpu->setStatus(NEGATIVE_MASK, (val & 0x0080) != 0);
        }
        
        
        cCpu::cCpu()			
		{
			m_cpuMinAddress = 0x0000;
			m_cpuMaxAddress = 0xFFFF;            
            
            registerAddressModes();
			registerInstructions();
			createInstructionTable();
			setStatus(UNUSED_MASK, true);
		}

        cCpu::~cCpu()
        {

        }

		uint8_t cCpu::getStatus(eStatusFlags flag) const
		{
			return m_statusReg & flag;
		}

		void cCpu::setStatus(eStatusFlags flag, bool set)
		{
			if (set) {
				m_statusReg |= flag;
			}
			else {
				m_statusReg &= ~flag;
			}
		}


		uint8_t cCpu::getStatusRegister() const
		{
			return m_statusReg;
		}

		uint8_t cCpu::getAccumulator() const
		{
			return m_accum;
		}

		uint8_t cCpu::getXRegister() const
		{
			return m_xReg;
		}

		uint8_t cCpu::getYRegister() const
		{
			return m_yReg;
		}

		std::map<uint16_t, std::string> cCpu::disassemble(uint16_t nStart, uint16_t nStop)
        {
            uint32_t addr = nStart;
            uint8_t value = 0x00, lo = 0x00, hi = 0x00;
            std::map<uint16_t, std::string> mapLines;
            uint16_t line_addr = 0;

            // A convenient utility to convert variables into
            // hex strings because "modern C++"'s method with 
            // streams is atrocious
            auto hex = [](uint32_t n, uint8_t d)
            {
                std::string s(d, '0');
                for (int i = d - 1; i >= 0; i--, n >>= 4)
                    s[i] = "0123456789ABCDEF"[n & 0xF];
                return s;
            };

            // Starting at the specified address we read an instruction
            // byte, which in turn yields information from the lookup table
            // as to how many additional bytes we need to read and what the
            // addressing mode is. I need this info to assemble human readable
            // syntax, which is different depending upon the addressing mode

            // As the instruction is decoded, a std::string is assembled
            // with the readable output
            while (addr <= (uint32_t)nStop)
            {
                line_addr = addr;

                // Prefix line with instruction address
                std::string sInst = "$" + hex(addr, 4) + ": ";

                uint8_t opcode, lo, hi, value;

                // Read instruction, and get its readable name
				m_cpuDataBus->cpuRead(this, addr, opcode); addr++;
                const auto& instruction = m_instructionTable[opcode];
                sInst += instruction.opCode+ " ";

                // Get operands from desired locations, and form the
                // instruction based upon its addressing mode. These
                // routines mimmick the actual fetch routine of the
                // 6502 in order to get accurate data as part of the
                // instruction
                if (instruction.addressMode == "IMP")
                {
                    sInst += " {IMP}";
                }
                else if (instruction.addressMode == "IMM")
                {
					m_cpuDataBus->cpuRead(this, addr, value); addr++;
                    sInst += "#$" + hex(value, 2) + " {IMM}";
                }
                else if (instruction.addressMode == "ZP")
                {
					m_cpuDataBus->cpuRead(this, addr, lo); addr++;
                    hi = 0x00;
                    sInst += "$" + hex(lo, 2) + " {ZP0}";
                }
                else if (instruction.addressMode == "ZPX")
                {
					m_cpuDataBus->cpuRead(this, addr, lo); addr++;
                    hi = 0x00;
                    sInst += "$" + hex(lo, 2) + ", X {ZPX}";
                }
                else if (instruction.addressMode == "ZPY")
                {
					m_cpuDataBus->cpuRead(this, addr, lo); addr++;
                    hi = 0x00;
                    sInst += "$" + hex(lo, 2) + ", Y {ZPY}";
                }
                else if (instruction.addressMode == "IZX")
                {
					m_cpuDataBus->cpuRead(this, addr, lo); addr++;
                    hi = 0x00;
                    sInst += "($" + hex(lo, 2) + ", X) {IZX}";
                }
                else if (instruction.addressMode == "IZY")
                {
					m_cpuDataBus->cpuRead(this, addr, lo); addr++;
                    hi = 0x00;
                    sInst += "($" + hex(lo, 2) + "), Y {IZY}";
                }
                else if (instruction.addressMode == "ABS")
                {
                    m_cpuDataBus->cpuRead(this, addr, lo); addr++;
                    m_cpuDataBus->cpuRead(this, addr, hi); addr++;
                    sInst += "$" + hex((uint16_t)(hi << 8) | lo, 4) + " {ABS}";
                }
                else if (instruction.addressMode == "ABX")
                {
                    m_cpuDataBus->cpuRead(this, addr, lo); addr++;
                    m_cpuDataBus->cpuRead(this, addr, hi); addr++;
                    sInst += "$" + hex((uint16_t)(hi << 8) | lo, 4) + ", X {ABX}";
                }
                else if (instruction.addressMode == "ABY")
                {
                    m_cpuDataBus->cpuRead(this, addr, lo); addr++;
                    m_cpuDataBus->cpuRead(this, addr, hi); addr++;
                    sInst += "$" + hex((uint16_t)(hi << 8) | lo, 4) + ", Y {ABY}";
                }
                else if (instruction.addressMode == "IND")
                {
                    m_cpuDataBus->cpuRead(this, addr, lo); addr++;
                    m_cpuDataBus->cpuRead(this, addr, hi); addr++;
                    sInst += "($" + hex((uint16_t)(hi << 8) | lo, 4) + ") {IND}";
                }
                else if (instruction.addressMode == "REL")
                {
					m_cpuDataBus->cpuRead(this, addr, value); addr++;
                    sInst += "$" + hex(value, 2) + " [$" + hex(addr + value, 4) + "] {REL}";
                }

                // Add the formed string to a std::map, using the instruction's
                // address as the key. This makes it convenient to look for later
                // as the instructions are variable in length, so a straight up
                // incremental index is not sufficient.
                mapLines[line_addr] = sInst;
            }

            return mapLines;
        }

        uint8_t cCpu::cpuRead(uint16_t& address)
		{
            uint8_t val;
            if (!m_cpuDataBus->cpuRead(this, address++, val))
                throw std::runtime_error("Nothing Was Read");
		    return val;			
		} 		

		bool cCpu::cpuRead(uint16_t address, uint8_t& result)
		{
			return m_cpuDataBus->cpuRead(this, address, result);
		}

		uint8_t cCpu::readOnly(uint16_t address)
		{
            uint8_t val;
			if (!m_cpuDataBus->cpuRead(this, address, val)) {
				 throw std::runtime_error("Nothing Was Read");
			}
            return val;
		}

        uint16_t cCpu::readOnly2(uint16_t address)
        {
            const uint16_t lo = readOnly(address + 0);
            const uint16_t hi = readOnly(address + 1);
            return (hi << 8) | lo;
        }

        void cCpu::pushProgramCounter()
        {
            const auto lo = (uint8_t)(m_progCounter & 0x00FF);
            const auto hi = (uint8_t)((m_progCounter >> 8) & 0x00FF);
            pushValue(hi);
            pushValue(lo);
        }

        uint16_t cCpu::popProgramCounter()
        {
            uint16_t lo = popValue();
            uint16_t hi = popValue();
            return (hi << 8) | lo;
        }

        bool cCpu::cpuWrite(uint16_t address, uint8_t val)
		{
			m_cpuDataBus->cpuWrite( this, address, val );
            return true;
		}

        void cCpu::setEntryPoint(uint16_t val)
        {
            cpuWrite(0xFFFD, (val >> 8) & 0xFF);
            cpuWrite(0xFFFC, (val) & 0xFF);
        }


        void cCpu::reset()
        {
            //fetch entry point
            m_curAddress = 0xFFFC;
            m_progCounter = readOnly2(m_curAddress);
            //registers & stack
            m_accum = 0;
            m_xReg = 0;
            m_yReg = 0;            
            m_statusReg = UNUSED_MASK;
            m_stackPtr = 0xFD;
			//misc stuff
            m_curOperand = 0;
            m_curAddress = 0;
            m_relAddress = 0;
            m_totalCycles = 0;
			m_halted = false;
            m_curCycle = 8; //adjust cycles
        }

        void cCpu::tick()
        {
            if (m_curCycle == 0 && !m_halted) 
            {
                setStatus(UNUSED_MASK, true);
                m_implied = false;
                m_curOpCode = cpuRead(m_progCounter);
                if (m_curOpCode == HALTED) {
                    m_halted = true;
                    return;
                }				

                const auto& curInst = m_instructionTable[m_curOpCode];

				std::cout << curInst.opCode << "\n";
				

		        //store num cycles because this may be altered by addressmode
                m_curCycle = curInst.numCycles;
                m_curAddress = m_progCounter;
                //execute addressing mode
                uint8_t cycles1 = m_addressCallbackMap[curInst.addressMode]();
                if (!m_implied)
                    m_curOperand = readOnly(m_curAddress); //fetch operand from stream
                else
                    m_curOperand = m_accum;
                //execute opcode
                uint8_t cycles2 = m_opCodeCallbackMap[curInst.opCode]();

                m_curCycle += (cycles1 & cycles2);
                setStatus(UNUSED_MASK, true);
            }
            m_totalCycles++;
            m_curCycle--;
        }



        void cCpu::nmi()
        {
            pushProgramCounter();
            setStatus(BREAK_MASK, false);
            setStatus(INTERRUPT_MASK, true);
            setStatus(UNUSED_MASK, true);
            pushValue(m_statusReg);
            m_curAddress = 0xFFFA;
            m_progCounter = readOnly2(m_curAddress);
            m_curCycle = 8;  //adjust cycles           
        }

        void cCpu::irq()
        {
            if (getStatus(INTERRUPT_MASK) == false) {
                pushProgramCounter();
                setStatus(BREAK_MASK, false);
                setStatus(INTERRUPT_MASK, true);
                setStatus(UNUSED_MASK, true);
                pushValue(m_statusReg);
                m_curAddress = 0xFFFE;
                m_progCounter = readOnly2(m_curAddress);
                m_curCycle = 7; //adjust cycles
            }
        }

        bool cCpu::complete() const
        {
            return m_curCycle == 0;
        }

        bool cCpu::halted() const
        {
            return m_halted;
        }

        uint16_t cCpu::programCounter() const
        {
            return m_progCounter;
        }

        void cCpu::registerAddressModes()
		{
			m_addressCallbackMap["IMP"] = [&]()-> uint8_t {return this->IMP(); };
			m_addressCallbackMap["IMM"]  = [&]()-> uint8_t {return this->IMM(); };
			m_addressCallbackMap["ABS"]  = [&]()-> uint8_t {return this->ABS(); };
			m_addressCallbackMap["ABX"] = [&]()-> uint8_t {return this->ABX(); };
			m_addressCallbackMap["ABY"] = [&]()-> uint8_t {return this->ABY(); };
			m_addressCallbackMap["IND"]  = [&]()-> uint8_t {return this->IND(); };
			m_addressCallbackMap["IZX"] = [&]()-> uint8_t {return this->IZX(); };
			m_addressCallbackMap["IZY"] = [&]()-> uint8_t {return this->IZY(); };
			m_addressCallbackMap["REL"]  = [&]()-> uint8_t {return this->REL(); };
			m_addressCallbackMap["ZP"]  = [&]()-> uint8_t {return this->ZP(); };
			m_addressCallbackMap["ZPX"] = [&]()-> uint8_t {return this->ZPX(); };
			m_addressCallbackMap["ZPY"] = [&]()-> uint8_t {return this->ZPY(); };			
		}

		void cCpu::registerInstructions()
		{
			m_opCodeCallbackMap["ADC"]=	[&]()-> uint8_t {return this->ADC();};
			m_opCodeCallbackMap["AND"]=	[&]()-> uint8_t {return this->AND();};
			m_opCodeCallbackMap["ASL"]=	[&]()-> uint8_t {return this->ASL();};
			m_opCodeCallbackMap["BCC"]=	[&]()-> uint8_t {return this->BCC();};
			m_opCodeCallbackMap["BCS"]=	[&]()-> uint8_t {return this->BCS();};
			m_opCodeCallbackMap["BEQ"]=	[&]()-> uint8_t {return this->BEQ();};
			m_opCodeCallbackMap["BIT"]=	[&]()-> uint8_t {return this->BIT();};
			m_opCodeCallbackMap["BMI"]=	[&]()-> uint8_t {return this->BMI();};
			m_opCodeCallbackMap["BNE"]=	[&]()-> uint8_t {return this->BNE();};
			m_opCodeCallbackMap["BPL"]=	[&]()-> uint8_t {return this->BPL();};
			m_opCodeCallbackMap["BRK"]=	[&]()-> uint8_t {return this->BRK();};
			m_opCodeCallbackMap["BVC"]=	[&]()-> uint8_t {return this->BVC();};
			m_opCodeCallbackMap["BVS"]=	[&]()-> uint8_t {return this->BVS();};
			m_opCodeCallbackMap["CLC"]=	[&]()-> uint8_t {return this->CLC();};
			m_opCodeCallbackMap["CLD"]=	[&]()-> uint8_t {return this->CLD();};
			m_opCodeCallbackMap["CLI"]=	[&]()-> uint8_t {return this->CLI();};
			m_opCodeCallbackMap["CLV"]=	[&]()-> uint8_t {return this->CLV();};
			m_opCodeCallbackMap["CMP"]=	[&]()-> uint8_t {return this->CMP();};
			m_opCodeCallbackMap["CPX"]=	[&]()-> uint8_t {return this->CPX();};
			m_opCodeCallbackMap["CPY"]=	[&]()-> uint8_t {return this->CPY();};
			m_opCodeCallbackMap["DEC"]=	[&]()-> uint8_t {return this->DEC();};
			m_opCodeCallbackMap["DEX"]=	[&]()-> uint8_t {return this->DEX();};
			m_opCodeCallbackMap["DEY"]=	[&]()-> uint8_t {return this->DEY();};
			m_opCodeCallbackMap["EOR"]=	[&]()-> uint8_t {return this->EOR();};
			m_opCodeCallbackMap["INC"]=	[&]()-> uint8_t {return this->INC();};
			m_opCodeCallbackMap["INX"]=	[&]()-> uint8_t {return this->INX();};
			m_opCodeCallbackMap["INY"]=	[&]()-> uint8_t {return this->INY();};
			m_opCodeCallbackMap["JMP"]=	[&]()-> uint8_t {return this->JMP();};
			m_opCodeCallbackMap["JSR"]=	[&]()-> uint8_t {return this->JSR();};
			m_opCodeCallbackMap["LDA"]=	[&]()-> uint8_t {return this->LDA();};
			m_opCodeCallbackMap["LDX"]=	[&]()-> uint8_t {return this->LDX();};
			m_opCodeCallbackMap["LDY"]=	[&]()-> uint8_t {return this->LDY();};
			m_opCodeCallbackMap["LSR"]=	[&]()-> uint8_t {return this->LSR();};
			m_opCodeCallbackMap["NOP"]=	[&]()-> uint8_t {return this->NOP();};
			m_opCodeCallbackMap["ORA"]=	[&]()-> uint8_t {return this->ORA();};
			m_opCodeCallbackMap["PHA"]=	[&]()-> uint8_t {return this->PHA();};
			m_opCodeCallbackMap["PHP"]=	[&]()-> uint8_t {return this->PHP();};
			m_opCodeCallbackMap["PLA"]=	[&]()-> uint8_t {return this->PLA();};
			m_opCodeCallbackMap["PLP"]=	[&]()-> uint8_t {return this->PLP();};
			m_opCodeCallbackMap["ROL"]=	[&]()-> uint8_t {return this->ROL();};
			m_opCodeCallbackMap["ROR"]=	[&]()-> uint8_t {return this->ROR();};
			m_opCodeCallbackMap["RTI"]=	[&]()-> uint8_t {return this->RTI();};
			m_opCodeCallbackMap["RTS"]=	[&]()-> uint8_t {return this->RTS();};
			m_opCodeCallbackMap["SBC"]=	[&]()-> uint8_t {return this->SBC();};
			m_opCodeCallbackMap["SEC"]=	[&]()-> uint8_t {return this->SEC();};
			m_opCodeCallbackMap["SED"]=	[&]()-> uint8_t {return this->SED();};
			m_opCodeCallbackMap["SEI"]=	[&]()-> uint8_t {return this->SEI();};
			m_opCodeCallbackMap["STA"]=	[&]()-> uint8_t {return this->STA();};
			m_opCodeCallbackMap["STX"]=	[&]()-> uint8_t {return this->STX();};
			m_opCodeCallbackMap["STY"]=	[&]()-> uint8_t {return this->STY();};
			m_opCodeCallbackMap["TAX"]=	[&]()-> uint8_t {return this->TAX();};
			m_opCodeCallbackMap["TAY"]=	[&]()-> uint8_t {return this->TAY();};
			m_opCodeCallbackMap["TSX"]=	[&]()-> uint8_t {return this->TSX();};
			m_opCodeCallbackMap["TXA"]=	[&]()-> uint8_t {return this->TXA();};
			m_opCodeCallbackMap["TXS"]=	[&]()-> uint8_t {return this->TXS();};
			m_opCodeCallbackMap["TYA"]=	[&]()-> uint8_t {return this->TYA();};
			m_opCodeCallbackMap["UNK"] = [&]()-> uint8_t {return this->UNK();};

		    //not implemented 6510/8500/8502 chipset instructions
			m_opCodeCallbackMap["AHX"] = [&]()-> uint8_t {return this->AHX(); };
			m_opCodeCallbackMap["ALR"] = [&]()-> uint8_t {return this->ALR(); };
			m_opCodeCallbackMap["ANC"] = [&]()-> uint8_t {return this->ANC(); };
			m_opCodeCallbackMap["ARR"] = [&]()-> uint8_t {return this->ARR(); };
			m_opCodeCallbackMap["AXS"] = [&]()-> uint8_t {return this->AXS(); };
			m_opCodeCallbackMap["DCP"] = [&]()-> uint8_t {return this->DCP(); };
			m_opCodeCallbackMap["ISC"] = [&]()-> uint8_t {return this->ISC(); };
			m_opCodeCallbackMap["LAS"] = [&]()-> uint8_t {return this->LAS(); };
			m_opCodeCallbackMap["LAX"] = [&]()-> uint8_t {return this->LAX(); };
			m_opCodeCallbackMap["RLA"] = [&]()-> uint8_t {return this->RLA(); };
			m_opCodeCallbackMap["RRA"] = [&]()-> uint8_t {return this->RRA(); };
			m_opCodeCallbackMap["SAX"] = [&]()-> uint8_t {return this->SAX(); };
			m_opCodeCallbackMap["SHY"] = [&]()-> uint8_t {return this->SHY(); };
			m_opCodeCallbackMap["SHX"] = [&]()-> uint8_t {return this->SHX(); };
			m_opCodeCallbackMap["SLO"] = [&]()-> uint8_t {return this->SLO(); };
			m_opCodeCallbackMap["SRE"] = [&]()-> uint8_t {return this->SRE(); };
			m_opCodeCallbackMap["TAS"] = [&]()-> uint8_t {return this->TAS(); };
			m_opCodeCallbackMap["XAA"] = [&]()-> uint8_t {return this->XAA(); };
		}

		void cCpu::createInstructionTable()
		{
			m_instructionTable = ParseInstructions("InstructionSet.txt");
			for (int i = 0; i < m_instructionTable.size(); ++i)
			{
				const auto& inst = m_instructionTable[i];
				if (!verifyInstruction(inst)) {
					m_instructionTable[i] = UNKNOWN;
				}
			}
		}

		bool cCpu::verifyInstruction(const sInstruction& inst) const
		{
			if (m_opCodeCallbackMap.count(inst.opCode) == 0)
				return false;
			if (m_addressCallbackMap.count(inst.addressMode) == 0)
				return false;
			if (inst.numCycles == 0)
				return false;
			return true;
		}

		void cCpu::pushValue(uint8_t val) {
            const auto stackAddress = SP_ADDRESS + (uint16_t)(m_stackPtr--);
            cpuWrite(stackAddress, val);
		}

		uint8_t cCpu::popValue()
		{
            m_stackPtr = m_stackPtr + 1;
            const auto stackAddress = SP_ADDRESS + (uint16_t)m_stackPtr;
            return readOnly(stackAddress);
		}

		void cCpu::branchInternal() {
			m_curAddress = m_progCounter + m_relAddress;
			m_curCycle++;
			if (!SamePage(m_progCounter, m_curAddress))
				m_curCycle++;
			m_progCounter = m_curAddress;
		}

		uint8_t cCpu::IMM()
		{
			m_curAddress = m_progCounter++;
			return 0;
		}

		uint8_t cCpu::IMP()
		{
			m_implied = true;
			return 0;
		}

		uint8_t cCpu::ABS()
		{
			uint16_t lo = cpuRead(m_progCounter);
            uint16_t hi = cpuRead(m_progCounter);
			m_curAddress = (hi << 8) | lo ;			
			return 0;
		}

		uint8_t cCpu::ABX()
		{
			uint16_t lo = cpuRead(m_progCounter);
			uint16_t hi = cpuRead(m_progCounter);
			m_curAddress = (hi << 8) | lo;
			m_curAddress += m_xReg;
			if ((m_curAddress & 0xFF00) != (hi << 8))
				return 1;
			else
				return 0;
		}

		uint8_t cCpu::ABY()
		{
			uint16_t lo = cpuRead(m_progCounter);
			uint16_t hi = cpuRead(m_progCounter);
			m_curAddress = (hi << 8) | lo;
			m_curAddress += m_yReg;
			if ((m_curAddress & 0xFF00) != (hi << 8))
				return 1;
			else
				return 0;
		}

		uint8_t cCpu::IND()
		{
			uint16_t lo = cpuRead(m_progCounter);
			uint16_t hi = cpuRead(m_progCounter);
			uint16_t ptr = (hi << 8) | lo;
			if (lo & 0xFF) 
			{
				lo = readOnly(ptr & 0xFF00) << 8;
				hi = readOnly(ptr + 1);
			}
			else
			{
				lo = readOnly(ptr + 0);
				hi = readOnly(ptr + 1);
			}
			m_curAddress = (hi << 8) | lo;			
			return 0;
		}

		uint8_t cCpu::IZX()
		{
			uint16_t t = cpuRead(m_progCounter);			
			uint16_t readPos = t + m_xReg;
			uint16_t lo = readOnly(readPos + 0);
			uint16_t hi = readOnly(readPos + 1);
			m_curAddress = (hi << 8) | lo;
			return 0;
		}

		uint8_t cCpu::IZY()
		{
			uint16_t t = cpuRead(m_progCounter);
			uint16_t lo = readOnly(t + 1);
			uint16_t hi = readOnly(t + 0);
			m_curAddress = (hi << 8) | lo;
			m_curAddress += m_yReg;
			if ((m_curAddress & 0xFF00) != (hi << 8))
				return 1;
			else
				return 0;
		}

		uint8_t cCpu::REL()
		{
			m_relAddress = cpuRead(m_progCounter); //signed	
			if (m_relAddress & 0x80)
				m_relAddress |= 0xFF00;
			return 0;
		}

		uint8_t cCpu::ZP()
		{
			m_curAddress = cpuRead(m_progCounter);
			m_curAddress &= 0x00FF;
			return 0;
		}

		uint8_t cCpu::ZPX()
		{
			m_curAddress = cpuRead(m_progCounter);
			m_curAddress += m_xReg;
			m_curAddress &= 0x00FF;
			return 0;
		}

		uint8_t cCpu::ZPY()
		{
			m_curAddress = cpuRead(m_progCounter);
			m_curAddress += m_yReg;
			m_curAddress &= 0x00FF;
			return 0;
		}


        /*
            addition with carry
            a = a + m + c
        
        */
		uint8_t cCpu::ADC()
		{
	        const uint16_t carry = getStatus(CARRY_MASK) ? 1 : 0;            
            uint16_t tmp = (uint16_t)m_accum + (uint16_t)m_curOperand + carry;            
            setStatus(CARRY_MASK, tmp > 255);
            SetN_Z(tmp & 0x00FF, this);
            //determine overflow
            setStatus(OVERFLOW_MASK, (~((uint16_t)m_accum ^ (uint16_t)m_curOperand) & ((uint16_t)m_accum ^ (uint16_t)tmp)) & 0x0080);
            m_accum = tmp & 0x00FF;
            return 1; //add a cycle
		}

		uint8_t cCpu::AND()
		{
			m_accum  = m_accum & m_curOperand;
            SetN_Z(m_accum, this);
			return 0;
		}

        //Shift Left One Bit (Memory or Accumulator)
		uint8_t cCpu::ASL()
		{
            const uint16_t result = (uint16_t)m_curOperand << 1;
            const uint8_t lo = (uint8_t)(result & 0x00FF);
            const uint8_t hi = (uint8_t) ((result >> 8) & 0x00FF);

            setStatus(CARRY_MASK, hi > 0);
            SetN_Z(lo, this);
                       
            if (m_implied) 
                m_accum = lo;
            else
                cpuWrite(m_curAddress, lo);            
            
            return 0;
		}

		//branch on carry clear
		uint8_t cCpu::BCC()
		{
			if (getStatus(CARRY_MASK) == false) {
				branchInternal();
			}
			return 0;
		}

		//branch on carry set
		uint8_t cCpu::BCS()
		{
			if (getStatus(CARRY_MASK)) {
				branchInternal();
			}
			return 0;
		}

		//Branch on Result Zero/equals
		uint8_t cCpu::BEQ()
		{
			if (getStatus(ZERO_MASK)) {
				branchInternal();
			}			
			return 0;
		}		

		//branch on result minus
		uint8_t cCpu::BMI()
		{
			if (getStatus(NEGATIVE_MASK)) {
				branchInternal();
			}			
			return 0;
		}

		//branch on not equal
		uint8_t cCpu::BNE()
		{
			if (getStatus(ZERO_MASK) == false) {
				branchInternal();
			}
			return 0;
		}

		//branch on result plus
		uint8_t cCpu::BPL()
		{
			if (getStatus(NEGATIVE_MASK) == false) {
				branchInternal();
			}
			return 0;
		}

		
		//branch on overflow clear
		uint8_t cCpu::BVC()
		{
			if (getStatus(OVERFLOW_MASK) == false) {
				branchInternal();
			}			
			return 0;
		}
		
		//branch on overflow set
		uint8_t cCpu::BVS()
		{
			if (getStatus(OVERFLOW_MASK) ) {
				branchInternal();
			}
			return 0;
		}

		//force break
        //push pc, push status set I flag
		uint8_t cCpu::BRK()
		{
            pushProgramCounter();
            setStatus(BREAK_MASK, true);
            setStatus(INTERRUPT_MASK, true);
            pushValue(m_statusReg);
            setStatus(BREAK_MASK, false);

            m_progCounter =(uint16_t)readOnly(0xFFFE) | ((uint16_t)readOnly(0xFFFF) << 8);
            
            return 0;
		}


		//Test Bits in Memory with Accumulator
		//bits 7 and 6 of operand are transfered to bit 7 and 6 of SR(N, V);
		//the zeroflag is set to the result of operand AND accumulator.
		uint8_t cCpu::BIT()
		{
			auto result = m_curOperand & m_accum;
			setStatus(NEGATIVE_MASK, (m_curOperand & NEGATIVE_MASK) != 0);
			setStatus(OVERFLOW_MASK, (m_curOperand & OVERFLOW_MASK) != 0);
			setStatus(ZERO_MASK, result != 0);
			return 0;
		}

		//clear carry
		uint8_t cCpu::CLC()
		{
			setStatus(CARRY_MASK, false);
			return 0;
		}

		//clear decimal
		uint8_t cCpu::CLD()
		{
			setStatus(DECIMAL_MASK, false);
			return 0;
		}

		//clear interrupt
		uint8_t cCpu::CLI()
		{
			setStatus(INTERRUPT_MASK, false);
			return 0;
		}

		//clear overflow
		uint8_t cCpu::CLV()
		{
			setStatus(OVERFLOW_MASK, false);
			return 0;
		}

        //compare accumulator with memory address
		uint8_t cCpu::CMP()
		{
            const uint16_t result = (uint16_t)m_accum - (uint16_t)m_curOperand;
            setStatus(CARRY_MASK, m_accum >= m_curOperand );        
            SetN_Z(result, this);

            return 0;
		}

        //compare accumulator with x register
		uint8_t cCpu::CPX()
		{
            const uint16_t result = (uint16_t)m_xReg - (uint16_t)m_curOperand;
            setStatus(CARRY_MASK, m_xReg >= m_curOperand);
            SetN_Z(result, this);
            return 0;
		}

        //compare accumulator with y register
		uint8_t cCpu::CPY()
		{
            const auto result = (uint16_t)m_yReg - (uint16_t)m_curOperand;
            setStatus(CARRY_MASK, m_yReg >= m_curOperand);
            SetN_Z(result, this);
            return 0;
		}

		//decrement memory location
		uint8_t cCpu::DEC()
		{
			uint16_t val = readOnly(m_curAddress);
			val = val - 1;
            SetN_Z(val, this);
			cpuWrite(m_curAddress,  (uint8_t)(val & 0x00FF));
			return 0;
		}

		//decrement x reg
		uint8_t cCpu::DEX()
		{
			m_xReg = m_xReg - 1;
            SetN_Z(m_xReg, this);
			return 0;
		}

		//decrement y reg 
		uint8_t cCpu::DEY()
		{
			m_yReg = m_yReg - 1;
            SetN_Z(m_yReg, this);
			return 0;			
		}

        //bitwise Exclusive OR
		uint8_t cCpu::EOR()
		{
            m_accum = m_accum ^ m_curOperand;
            SetN_Z(m_accum, this);
			return 0;
		}

		//increment memory location
		uint8_t cCpu::INC()
		{
			uint16_t val = readOnly(m_curAddress); //read current address
			val = val + 1;
            SetN_Z(val, this);
			cpuWrite(m_curAddress, (uint8_t)(val & 0x00FF));
			return 0;
		}

		//increment x reg
		uint8_t cCpu::INX()
		{
			m_xReg = m_xReg + 1;
            SetN_Z(m_xReg, this);
			return 0;
		}

		//increment y reg
		uint8_t cCpu::INY()
		{
			m_yReg = m_yReg + 1;
            SetN_Z(m_yReg, this);
			return 0;
		}

		//set program counter to current address
		uint8_t cCpu::JMP()
		{
			m_progCounter = m_curAddress;
			return 0;
		}

        /*
            JSR pushes the address-1 of the next operation on to the stack before transferring program control 
            to the following address. Subroutines are normally terminated by a RTS op code.
        */
		uint8_t cCpu::JSR()
		{
            m_progCounter--;   
            pushProgramCounter();
            m_progCounter = m_curAddress;
            return 0;
		}

		//load operand into accumulator
		uint8_t cCpu::LDA()
		{
			m_accum = m_curOperand;
            SetN_Z(m_accum, this);		
			return 0;
		}

		//load operand into x reg
		uint8_t cCpu::LDX()
		{
			m_xReg = m_curOperand;
            SetN_Z(m_xReg, this);
			return 0;
		}

		//load operand into y reg
		uint8_t cCpu::LDY()
		{
			m_yReg = m_curOperand;
            SetN_Z(m_yReg, this);
			return 0;
		}

        //Shift bit right(memory or accumulator )
		uint8_t cCpu::LSR()
		{
            setStatus(CARRY_MASK, (m_curOperand & 0x1) != 0);          
            uint8_t result = m_curOperand >> 1;
            SetN_Z(result, this);
            if (m_implied)
                m_accum = result;
            else
                cpuWrite(m_curAddress, result);
            return 0;
		}

        //do nothing eat cycles
		uint8_t cCpu::NOP()
		{
			return 0;
		}

        //bitwise or with accumulator
		uint8_t cCpu::ORA()
		{
            m_accum = m_accum | m_curOperand;
            SetN_Z(m_accum, this);
            return 0;
		}

        //push accumulator onto stack
		uint8_t cCpu::PHA()
		{
            pushValue(m_accum);            
            return 0;
		}

        //push status register onto stack      
		uint8_t cCpu::PHP()
		{
            pushValue(m_statusReg | BREAK_MASK | UNUSED_MASK);
            setStatus( UNUSED_MASK, false);      
            setStatus( BREAK_MASK, false);
            return 0;
		}

        //pull accumulator from stack
		uint8_t cCpu::PLA()
		{
            m_accum = popValue();
            SetN_Z(m_accum, this);                
            return 0;
		}

        //pull status register from stack
		uint8_t cCpu::PLP() {
	         m_statusReg = popValue();
             setStatus(UNUSED_MASK, true);
             return 0;
		}

        //ROL shifts all bits left one position. The Carry is shifted into bit 0 
        //and the original bit 7 is shifted into the Carry.
		uint8_t cCpu::ROL()
		{
            uint8_t tmp = m_curOperand << 1;
            tmp |= getStatus(CARRY_MASK);
            setStatus(CARRY_MASK, ( m_curOperand >> 7) & 0x01);
            if (m_implied)
                m_accum = tmp;
            else
                cpuWrite(m_curAddress, tmp);
            return 0;
		}
        //ROR shifts all bits right one position. The Carry is shifted into bit 7 
        //and the original bit 0 is shifted into the Carry.
		uint8_t cCpu::ROR()
		{
            uint8_t tmp = m_curOperand >> 1;
            tmp |= getStatus(CARRY_MASK) << 7;
            setStatus(CARRY_MASK, m_curOperand & 0x01);
            if (m_implied)
                m_accum = tmp;
            else
                cpuWrite(m_curAddress, tmp);            
            return 0;
		}
        
        /*
            RTI retrieves the Processor Status Word (flags) and the Program Counter from the stack in that order 
            (interrupts push the PC first and then the PSW).
            Note that unlike RTS, the return address on the stack is the actual address rather than the address-1.
        */
		uint8_t cCpu::RTI()
		{
            m_statusReg = popValue();            
            setStatus(BREAK_MASK, false);
            setStatus(CARRY_MASK, false);
            uint16_t pc = popProgramCounter();
            m_progCounter = pc;
            return 0;
		}

        /*
            RTS pulls the top two bytes off the stack (low byte first) and transfers program control to that address+1. 
            It is used, as expected, to exit a subroutine invoked via JSR which pushed the address-1.
            RTS is frequently used to implement a jump table where addresses-1 are pushed onto the stack and accessed via 
            RTS eg. to access the second of four routines:
        */
		uint8_t cCpu::RTS()
		{
            uint16_t pc = popProgramCounter();
            m_progCounter = pc + 1;            
            return 0;
		}
        //subtract from accumulator with borrow
        //A = A - M - (1 - C)
		uint8_t cCpu::SBC()
		{
            const uint16_t carry = getStatus(CARRY_MASK) ? 0 : 1;
            uint16_t result = (uint16_t)m_accum - (uint16_t)m_curOperand - carry;
            m_statusReg &= ~(CARRY_MASK | ZERO_MASK | OVERFLOW_MASK | NEGATIVE_MASK);
            if (result == 0)
                m_statusReg |= (ZERO_MASK | CARRY_MASK);
            else if (result > 0)
                m_statusReg |= CARRY_MASK;
            SetN_Z(result & 0x00FF, this);
            m_accum = result & 0x00FF;
            return 1; //add a cycle
		}

        //set carry flag
		uint8_t cCpu::SEC()
		{
            setStatus(CARRY_MASK, true);            
            return 0;
		}

        //set decimal flag
		uint8_t cCpu::SED()
		{
            setStatus(DECIMAL_MASK, true);
            return 0;
		}
        //set interrupt flag
		uint8_t cCpu::SEI()
		{
            setStatus(INTERRUPT_MASK, true);
            return 0;
		}
        //store accumulator into memory
		uint8_t cCpu::STA()
		{
            cpuWrite(m_curAddress, m_accum);    
            return 0;
		}
        //store x register into memory
		uint8_t cCpu::STX()
		{
            cpuWrite(m_curAddress, m_xReg);
			return 0;
		}
        //store y register into memory
		uint8_t cCpu::STY()
		{
            cpuWrite(m_curAddress, m_yReg);
            return 0;
		}
        //Transfer accumulator to x-reg
		uint8_t cCpu::TAX()
		{
            m_xReg = m_accum;
            SetN_Z(m_xReg, this); 
            return 0;
		}

        //Transfer accumulator to y-reg
		uint8_t cCpu::TAY()
		{
            m_yReg = m_accum;
            SetN_Z(m_yReg, this);
            return 0;
		}
        //Transfer stack pointer to x-reg
		uint8_t cCpu::TSX()
		{
            m_xReg = popValue();
            SetN_Z(m_xReg, this);
            return 0;
		}
        //Transfer x-reg to accumulator
		uint8_t cCpu::TXA()
		{
            m_accum = m_xReg;
            SetN_Z(m_accum, this);
            return 0;
		}
        //Transfer x-reg to stack register
		uint8_t cCpu::TXS()
		{
            pushValue(m_xReg);            
            return 0;
		}
        //Transfer y-reg to accumulator
		uint8_t cCpu::TYA()
		{
            m_accum = m_yReg;
            SetN_Z(m_accum, this);
            return 0;
		}

		//////////////////////////////////////////////////////////////////////////
		//Not implemented, 
		//////////////////////////////////////////////////////////////////////////
		uint8_t cCpu::SLO()
		{
            throw std::runtime_error("Not Implemented");
            
            return 0;
		}

		uint8_t cCpu::SRE()
		{
            throw std::runtime_error("Not Implemented");

            return 0;
		}

		uint8_t cCpu::TAS()
		{
            throw std::runtime_error("Not Implemented");

            return 0;
		}

		uint8_t cCpu::XAA()
		{
            throw std::runtime_error("Not Implemented");

            return 0;
		}

		uint8_t cCpu::UNK()
		{
            throw std::runtime_error("Not Implemented");

            return 0;
		}

		uint8_t cCpu::AHX()
		{
            throw std::runtime_error("Not Implemented");

            return 0;
		}

		uint8_t cCpu::ALR()
		{
            throw std::runtime_error("Not Implemented");

            return 0;
		}

		uint8_t cCpu::ANC()
		{
            throw std::runtime_error("Not Implemented");

            return 0;
		}

		uint8_t cCpu::ARR()
		{
            throw std::runtime_error("Not Implemented");

            return 0;
		}

		uint8_t cCpu::AXS()
		{
            throw std::runtime_error("Not Implemented");

            return 0;
		}

		uint8_t cCpu::DCP()
		{
            throw std::runtime_error("Not Implemented");

            return 0;
		}

		uint8_t cCpu::ISC()
		{
            throw std::runtime_error("Not Implemented");

            return 0;
		}

		uint8_t cCpu::LAS()
		{
            throw std::runtime_error("Not Implemented");

            return 0;
		}

		uint8_t cCpu::LAX()
		{
            throw std::runtime_error("Not Implemented");

            return 0;
		}

		uint8_t cCpu::RLA()
		{
            throw std::runtime_error("Not Implemented");

            return 0;
		}

		uint8_t cCpu::RRA()
		{
            throw std::runtime_error("Not Implemented");

            return 0;
		}

		uint8_t cCpu::SAX()
		{
            throw std::runtime_error("Not Implemented");

            return 0;
		}

		uint8_t cCpu::SHY()
		{
            throw std::runtime_error("Not Implemented");

            return 0;
		}

		uint8_t cCpu::SHX()
		{
            throw std::runtime_error("Not Implemented");

            return 0;
		}

	}
}

