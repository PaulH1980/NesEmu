#pragma once

#include <chrono>
#include <ctime>   
#include <string>

#define NES_PATTERN_TABLE_DIMS  (16 * 8)

#define NES_WIDTH		  256
#define NES_HEIGHT		  240
#define NES_PRG_BANK_SIZE 16384
#define NES_CHR_BANK_SIZE 8192

namespace Shared {

	template <class Resolution = std::chrono::nanoseconds>
	class ExecutionTimer {
	public:
		using Clock = std::conditional_t<std::chrono::high_resolution_clock::is_steady,
			std::chrono::steady_clock,
			std::chrono::steady_clock>;

	private:
		Clock::time_point mStart; 		

	public:
		ExecutionTimer() {
			reset();
		}
		~ExecutionTimer() {	
			
		}		

		inline void reset() {
			mStart = Clock::now();			
		}

		inline uint64_t elapsedTime( bool _reset = true) {				
			const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - mStart).count();
			if (_reset)
				reset();
			return duration;
		}

	};


	template <typename I> std::string n2hexstr(I w, size_t hex_len = sizeof(I) << 1) {
		static const char* digits = "0123456789ABCDEF";
		std::string rc(hex_len, '0');
		for (size_t i = 0, j = (hex_len - 1) * 4; i < hex_len; ++i, j -= 4)
			rc[i] = digits[(w >> j) & 0x0f];
		return rc;
	}

}