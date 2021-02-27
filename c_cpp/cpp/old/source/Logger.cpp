
#include <iostream>
#include <ctime>
#include <iomanip>
#include "Logger.h"


using namespace perri;



Logger& Logger::operator<<(const TON &t)
{
	long long int ms = t.since() % 1000;
	*this << "\n[" << t.since() / 1000 << "." << std::setfill('0') << std::setw(3) << ms << "] ";
	return *this;
}



void Logger::SwitchMode(Logger::MODE mode)
{
	switch (mode) {
	case Logger::MODE::OFF:
		this->toNull();
		break;
	case Logger::MODE::CONSOLE:
		this->toConsole();
		break;
	case Logger::MODE::FILE:
		this->toFile();
		break;
	}
}



void Logger::toNull()
{
	this->basic_ios<char>::rdbuf( 0 );
}



void Logger::toConsole()
{
	this->basic_ios<char>::rdbuf( std::cout.rdbuf() );
}



void Logger::toFile()
{
	std::string filename;
	filename.resize(64);

	time_t seconds = time(NULL);
	tm *timeinfo = localtime(&seconds);
	strftime(&(filename[0]), filename.size(), "%Y_%b_%d_%H.%M.%S", timeinfo);
	filename = "/var/log/" + filename + ".log";
	this->toFile(filename);
}



void Logger::toFile(const std::string &filename)
{
	this->open(filename.c_str(), std::ios_base::out);
}



const TON &Logger::GetTimer() const
{
	return this->Time;
}



Logger::Logger() : std::fstream()
{
	this->Time.start(0);
}



Logger::~Logger()
{
	if (this->is_open())
		this->close();
}


