/**
 * @Author: pkalbers
 * @Date:   2017-06-21T23:06:32+01:00
 * @Last modified by:   pkalbers
 * @Last modified time: 2017-06-21T23:55:21+01:00
 */


#include <chrono>
#include <iostream>
#include <thread>

#include "progress.hpp"


int main(int argc, char const *argv[])
{
	Progress p1(1000);
	for (size_t i = 0; i < 1000; ++i)
	{
		++p1;
		std::this_thread::sleep_for(std::chrono::milliseconds(30));
	}
	p1.finish();

	Progress p2(1000);
	p2.show_prefix("Process");
	for (size_t i = 0; i < 1000; ++i)
	{
		++p2;
		std::this_thread::sleep_for(std::chrono::milliseconds(30));
	}
	p2.finish();

	Progress p3;
	for (size_t i = 0; i < 1000; ++i)
	{
		++p3;
		std::this_thread::sleep_for(std::chrono::milliseconds(30));
	}
	p3.finish();

	Progress p4(1e6);
	for (size_t i = 0; i < 1e6; ++i)
	{
		++p4;
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	p4.finish();

	return 0;
}
