/**
 * @Author: pkalbers
 * @Date:   2017-06-21T23:06:32+01:00
 * @Last modified by:   pkalbers
 * @Last modified time: 2017-06-21T23:24:02+01:00
 */


#include <iostream>
#include <thread>

#include "progress.hpp"


int main(int argc, char const *argv[])
{
	Progress p(1000);

	for (size_t i = 0; i < 1000; ++i)
	{
		++p;
		std::this_thread::sleep_for(std::chrono::milliseconds(111));
	}

	p.finish();


	Progress q;

	for (size_t i = 0; i < 1000; ++i)
	{
		++q;
		std::this_thread::sleep_for(std::chrono::milliseconds(111));
	}

	return 0;
}
