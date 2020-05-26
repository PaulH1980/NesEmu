#pragma once
#include <memory>
#include <vector>
#include <map>
#include <unordered_map>
#include "BusDevice.h"
#include "DataBus.h"
#include "6502_Instruction.h"

namespace _CPU
{
	namespace _6502
	{
		using OpCodeCallBack = std::function<uint8_t(void)>;

		
		enum eStatusFlags : uint8_t{
			CARRY_MASK		= 1 << 0,
			ZERO_MASK		= 1 << 1,
			INTERRUPT_MASK	= 1 << 2,
			DECIMAL_MASK	= 1 << 3,
			BREAK_MASK		= 1 << 4,
			UNUSED_MASK		= 1 << 5,
			OVERFLOW_MASK	= 1 << 6,
			NEGATIVE_MASK	= 1 << 7
		};

        constexpr uint16_t SP_ADDRESS = 0x0100;
        constexpr uint8_t  HALTED = 0x0;

		class cCpu : public cCpuBusDevice
		{
		public:
			cCpu();
			cCpu(const cCpu&) = delete;
            ~cCpu();
			
			bool			cpuRead(uint16_t address, uint8_t& result) override;
            bool			cpuWrite(uint16_t address, uint8_t val) override;

            //set resetvector/entry pointer where the start of the program is
            //must be called before an 'reset()'
            void            setEntryPoint(uint16_t val);

            void			reset();

			void			tick();

            void            nmi();

            void            irq();

            bool            complete() const;

            bool            halted() const;

            uint16_t        programCounter() const;

            uint8_t			getStatus(eStatusFlags flag) const;
			void			setStatus(eStatusFlags flag, bool set);

			uint8_t			getStatusRegister() const;
			uint8_t			getAccumulator() const;
			uint8_t			getXRegister() const;
			uint8_t			getYRegister() const;


            std::map<uint16_t, std::string> disassemble(uint16_t nStart, uint16_t nStop);			
			

		private:
			
			std::unordered_map<std::string, OpCodeCallBack>		m_addressCallbackMap;
			std::unordered_map<std::string, OpCodeCallBack>		m_opCodeCallbackMap;
			std::vector<sInstruction> m_instructionTable;

			int8_t			m_accum			= 0;
			int8_t			m_xReg			= 0;
			int8_t			m_yReg			= 0;

			uint8_t			m_statusReg		= 0;
            //With the 6502, the stack is always on page one ($100-$1FF) and works top down.
			uint8_t			m_stackPtr		= 0xFF;     //decreasing stack pointer( add 0x100 when working with it )
			uint16_t		m_progCounter	= 0;	

			uint8_t			m_curOpCode		= 0;		//current opcode
			uint8_t			m_curOperand	= 0;		//current operand, fetched from memory or registers
			uint16_t		m_curAddress    = 0;
			uint16_t		m_relAddress    = 0;		//signed
			uint8_t			m_curCycle		= 0;	    //current cycle of this instruction, decrements

            bool            m_implied       = false;    //operand is implied
            bool            m_halted        = false;

			uint64_t		m_totalCycles	= 0;

			void			registerAddressModes();
			void			registerInstructions();
			void			createInstructionTable();
			bool			verifyInstruction(const sInstruction& inst) const;

            uint8_t			cpuRead(uint16_t& address); //read u8 increments address                     
            uint8_t			readOnly(uint16_t address); //readonly 
            uint16_t        readOnly2(uint16_t address); //read u16  
			
			//push value onto stack, stack values range from 0x100 to 0x1FF
			void			pushValue(uint8_t val);
			//pop value from stack, stack values range from 0x100 to 0x1FF
			uint8_t			popValue();          


			//internal helper function for branching
			void			branchInternal();
            void            pushProgramCounter();
            uint16_t        popProgramCounter();


			//address modes
			uint8_t			IMM();	// Immediate	 			OPC #$BB	 	operand is byte BB
			uint8_t			IMP();	// Implied 					OPC			 	operand is implied
					
			uint8_t			ABS();  // Absolute	 				OPC $LLHH	 	operand is address $HHLL *
			uint8_t			ABX(); // Absolute, X-indexed	 	OPC $LLHH,X	 	operand is address; effective address is address incremented by X with carry **
			uint8_t			ABY(); // Absolute, Y - indexed	OPC $LLHH, Y		operand is address; effective address is address incremented by Y with carry **

			uint8_t			IND();  // Indirect	 				OPC ($LLHH)	 	operand is address; effective address is contents of word at address: C.w($HHLL)
			uint8_t			IZX(); // X-indexed, indirect	 	OPC ($LL,X)	 	operand is zeropage address; effective address is word in (LL + X, LL + X + 1), inc. without carry: C.w($00LL + X)
			uint8_t			IZY(); // Indirect, Y-indexed	 	OPC ($LL),Y	 	operand is zeropage address; effective address is word in (LL, LL + 1) incremented by Y with carry: C.w($00LL) + Y

			uint8_t			REL();  // Relative	 				OPC $BB	 		branch target is PC + signed offset BB ***

			uint8_t			ZP();	// Zeropage	 				OPC $LL	 		operand is zeropage address (hi-byte is zero, address = $00LL)
			uint8_t			ZPX();	// Zeropage, X-indexed	 	OPC $LL,X	 	operand is zeropage address; effective address is address incremented by X without carry **
			uint8_t			ZPY();	// Zeropage, Y-indexed	 	OPC $LL,Y	 	operand is zeropage address; effective address is address incremented by Y without carry **
						

			//opcodes
			uint8_t			ADC();	//add with carry
			uint8_t			AND();	//and (with accumulator)
			uint8_t			ASL();	//arithmetic shift left
			uint8_t			BCC();	//branch on carry clear
			uint8_t			BCS();	//branch on carry set
			uint8_t			BEQ();	//branch on equal(zero set)
			uint8_t			BIT();	//bit test
			uint8_t			BMI();	//branch on minus(negative set)
			uint8_t			BNE();	//branch on not equal(zero clear)
			uint8_t			BPL();	//branch on plus(negative clear)
			uint8_t			BRK();	//break / interrupt
			uint8_t			BVC();	//branch on overflow clear
			uint8_t			BVS();	//branch on overflow set
			uint8_t			CLC();	//clear carry
			uint8_t			CLD();	//clear decimal
			uint8_t			CLI();	//clear interrupt disable
			uint8_t			CLV();	//clear overflow
			uint8_t			CMP();	//compare(with accumulator)
			uint8_t			CPX();	//compare with X
			uint8_t			CPY();	//compare with Y
			uint8_t			DEC();	//decrement
			uint8_t			DEX();	//decrement X
			uint8_t			DEY();	//decrement Y
			uint8_t			EOR();	//exclusive or (with accumulator)
			uint8_t			INC();	//increment
			uint8_t			INX();	//increment X
			uint8_t			INY();	//increment Y
			uint8_t			JMP();	//jump
			uint8_t			JSR();	//jump subroutine
			uint8_t			LDA();	//load accumulator
			uint8_t			LDX();	//load X
			uint8_t			LDY();	//load Y
			uint8_t			LSR();	//logical shift right
			uint8_t			NOP();	//no operation
			uint8_t			ORA();	//or with accumulator
			uint8_t			PHA();	//push accumulator
			uint8_t			PHP();	//push processor status(SR)
			uint8_t			PLA();	//pull accumulator
			uint8_t			PLP();	//pull processor status(SR)
			uint8_t			ROL();	//rotate left
			uint8_t			ROR();	//rotate right
			uint8_t			RTI();	//return from interrupt
			uint8_t			RTS();	//return from subroutine
			uint8_t			SBC();	//subtract with carry
			uint8_t			SEC();	//set carry
			uint8_t			SED();	//set decimal
			uint8_t			SEI();	//set interrupt disable
			uint8_t			STA();	//store accumulator
			uint8_t			STX();	//store X
			uint8_t			STY();	//store Y
			uint8_t			TAX();	//transfer accumulator to X
			uint8_t			TAY();	//transfer accumulator to Y
			uint8_t			TSX();	//transfer stack pointer to X
			uint8_t			TXA();	//transfer X to accumulator
			uint8_t			TXS();	//transfer X to stack pointer
			uint8_t			TYA();	//transfer Y to accumulator	

			uint8_t			UNK(); //unknown

			//not implemented 6510/8500/8502 chipset instructions
			uint8_t			AHX();
			uint8_t			ALR();
			uint8_t			ANC();
			uint8_t			ARR();			
			uint8_t			AXS();
			uint8_t			DCP();
			uint8_t			ISC();
			uint8_t			LAS();
			uint8_t			LAX();
			uint8_t			RLA();
			uint8_t			RRA();
			uint8_t			SAX();
			uint8_t			SHY();
			uint8_t			SHX();
			uint8_t			SLO();
			uint8_t			SRE();				
			uint8_t			TAS();	
			uint8_t			XAA();

		};
	}
}