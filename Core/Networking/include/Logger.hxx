#pragma once

#include <iostream>
#include <string>
#include <thread>
#include <sstream> 
#include <fstream>
#include <mutex>

namespace BrokenBytes::FastNet::Logger {
	static std::ofstream _logFile("fastnet.log", std::ofstream::binary);
	
	enum class Level { Info, Warning, Error };

	static std::mutex _lock;
	
	inline void Write(std::string msg, Level level = Level::Info) {
		std::scoped_lock lock{ _lock };
		std::string strLvl;
		switch (level)
		{
		case BrokenBytes::FastNet::Logger::Level::Info:
			strLvl = "Info";
			break;
		case BrokenBytes::FastNet::Logger::Level::Warning:
			strLvl = "Warning";
			break;
		case BrokenBytes::FastNet::Logger::Level::Error:
			strLvl = "Error";
			break;
		}

		std::stringstream str;

			str 
			<< "| "
			<< strLvl
			<< " | "
			<< msg 
			<< " | " 
			<< __TIME__ 
			<< " | "
			<< "THREAD_"
			<< std::this_thread::get_id()  
			<< " |" 
			<< std::endl;
			
			std::cout << str.str();
			_logFile.open("fastnet.log");
			_logFile << str.str();
			_logFile.close();
	}
}