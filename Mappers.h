#pragma once
#include <functional>
#include <unordered_map>
#include "iMapper.h"
namespace _CPU
{
	namespace _6502
	{
		
		//https://wiki.nesdev.com/w/index.php/Mapper
		
		using CreateMapper = std::function<iMapper*()>;
		
		/*
			@brief: Simple mapper factory
		*/
		class MapperFactory
		{
		public:
			/*
				@brief: Register new mapper where key = mapperid
			*/
			bool	registerMapper(int key, CreateMapper mapper) {
				if (m_createMapperMap.count(key)) //duplicate
					return false;
				m_createMapperMap[key] = mapper;
				return true;
			}
			/*
				@brief: Return function pointer to create mapper function
			*/
			CreateMapper getMapperCreateFunc(int key) {
				if (!m_createMapperMap.count(key))
					return nullptr;
				return m_createMapperMap[key];
			}

		private:
			std::unordered_map<int, CreateMapper> m_createMapperMap;
		};
		
		
		class Mapper00 : public iMapper {
		public:
			Mapper00();

			virtual  bool    cpuRead(uint16_t address, uint32_t& result);
			virtual  bool    cpuWrite(uint16_t address, uint32_t& result);

			virtual  bool    ppuRead(uint16_t address, uint32_t& result);
			virtual  bool    ppuWrite(uint16_t address, uint32_t& result);
		};

		
		
	}
}