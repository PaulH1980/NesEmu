#pragma once
#include <functional>
#include <string>
#include <vector>

namespace _CPU
{
	namespace _6502
	{
		struct sInstruction {
			std::string		opCode;	//OpCode
			std::string		addressMode; 			
			int				numCycles = 0;		//initial amount of cycles

			
		};

		const sInstruction UNKNOWN = { "UNK", "IMP", 2 };

		std::vector<sInstruction> ParseInstructions( const std::string& fileName );

        std::vector<uint8_t>      ParseProgram(const std::string& fileName);


		
	}
}