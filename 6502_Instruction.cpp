#include <filesystem>
#include <fstream>
#include <sstream>
#include <assert.h>
#include "6502_Instruction.h"

constexpr std::size_t INSTRUCTION_SIZE = 256;

inline std::vector<std::string> split(const std::string& s, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter))
	{
		tokens.push_back(token);
	}
	return tokens;
}



namespace _CPU
{
	namespace _6502
	{

	
		sInstruction ParseInstruction(const std::string& instructionString) {
			const auto tokens = split( instructionString, ' ' ); //space separated
			//addressmode is implied if tokenCount == 2
			const auto tokCount = tokens.size();
			std::string opCode, 
						addressMode, 
						numCycles;
			if (tokCount == 1 && tokens[0] == "KIL") {
				return UNKNOWN;
			}
			else if (tokCount == 2) {
				addressMode = "IMP"; //implied
				opCode = tokens[0];				
				numCycles = tokens[1];
			}
			else if (tokCount == 3)
			{
				opCode = tokens[0];
				addressMode = tokens[1];
				numCycles = tokens[2];
			}
			else
				assert(false && "Invalid TokenCount");

			std::replace(numCycles.begin(), numCycles.end(), '*', ' '); // replace all 'x' to 'y'
			int iCycles = std::stoi(numCycles, nullptr );

			return { opCode, addressMode, iCycles };

		}


		std::vector<sInstruction> ParseInstructions(const std::string& fileName)
		{
			if (!std::filesystem::exists(fileName)) {
				return {};
			}
			std::vector<std::string> allTokens;

			std::ifstream inStream(fileName);
			std::string line;
			while (std::getline(inStream, line))
			{
				const auto tokens = split(line, '\t'); //tab separated
				allTokens.insert(std::end(allTokens), std::begin(tokens), std::end(tokens));
			}
			if (allTokens.size() != INSTRUCTION_SIZE)
				return {};

			std::vector<sInstruction> result;
			result.reserve(INSTRUCTION_SIZE);
			for (const auto& val : allTokens)
			{
				auto instr = ParseInstruction(val);
				result.push_back(instr);
			}
			return result;
		}

        std::vector<uint8_t> ParseProgram(const std::string& fileName)
        {
            if (!std::filesystem::exists(fileName)) {
                return {};
            }
            std::vector<uint8_t> result;
            std::ifstream inStream(fileName);
            std::string line;
            while (std::getline(inStream, line))
            {
                const auto tokens = split(line, ' '); //space separated
                for (const auto& val : tokens) {
                    uint32_t bc = std::strtoul(val.c_str(), 0, 16);
                   // inStream >> std::hex >> bc;
                    result.push_back((uint8_t)bc);

                }               
            }
            return result;
        }

    }
}