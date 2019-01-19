#include <iostream>
#include "SerialPort.h"

int main()
{
	try
	{
		Lite::SerialPort sp{ "COM4", 4800 };

		std::cout << "rx queue size: " << sp.CurrentRxQueueSize() << '\n';
		std::cout << "rx in queue size: " << sp.CurrentRxInQueue() << '\n';

		sp.SetRxTimeouts(std::chrono::milliseconds(1000));
		std::vector<uint8_t> test(sp.CurrentRxQueueSize());

		sp.ReadBytes(test, 100);
	}
	catch (Lite::SerialPortRuntimeException& e)
	{
		std::cerr << e.what();
		1;
	}
	catch (Lite::SerialPortRangeError& e)
	{
		std::cerr << e.what();
		1;
	}
	std::cin.get();
}