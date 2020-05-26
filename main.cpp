#include <iostream>
#include "MainFrame.h"
#include "Nes.h"

using namespace _CPU::_6502;
using namespace UI;

inline void RunTestCode( cCpu& cpu, cNesRam& ram)
{
	const auto testCode = ParseProgram("TestApp.txt");
	const uint16_t progEntryPoint = 0x0400;
	memcpy(ram.m_ram.data() + progEntryPoint, testCode.data(), testCode.size());
	cpu.setEntryPoint(progEntryPoint);
	cpu.reset();
	auto dissasembled = cpu.disassemble(0x0000, 0xffff);


	std::string prevString;

	while (!cpu.halted()) {
		cpu.tick();
		if (dissasembled.count(cpu.programCounter()))
		{
			const auto curString = dissasembled[cpu.programCounter()];
			if (prevString != curString) {
				prevString = curString;
				std::cout << prevString << "\n";
			}
		}
	}
}

int main(int argc, char *argv[])
{
    cNes myNes;      
	myNes.loadRom("smb.nes");
    
    cMainFrame mainFrame(1600, 1200);
	mainFrame.setNes(&myNes);
    mainFrame.setPixelCallback( [&]() { return myNes.ppu.getFrameBuffer(); });
	mainFrame.setFrameCompleteCallback([&]() { return myNes.frameComplete(); });
	mainFrame.setNesUpdateCallback([&]() { return myNes.tick(); });
    mainFrame.mainLoop();

    return 0;
}

   