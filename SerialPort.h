#pragma once
#include <memory>
#include <vector>
#include <string>
#include <chrono>

namespace Lite
{
	class SerialPort
	{
	public:
		enum class PARITY { NONE, ODD, EVEN, MARK, SPACE };
		enum class STOPBITS { ONE, ONEPT5, TWO };
		enum class FLOWCONTROL { SW, HW, NONE };

		SerialPort(const std::string port = "COM1",
			const uint32_t baudRate = 9600,
			const PARITY parity = PARITY::NONE,
			const STOPBITS stopBit = STOPBITS::ONE,
			const FLOWCONTROL flowControl = FLOWCONTROL::NONE,
			const uint8_t databits = 8);
		~SerialPort();

		SerialPort(const SerialPort&) = delete;
		SerialPort(SerialPort&&) = delete;

		SerialPort& operator=(const SerialPort&) = delete;
		SerialPort& operator=(SerialPort&&) = delete;

		std::vector<uint8_t>ReadBytes(std::vector<uint8_t>& buffer, const size_t bytesToRead);
		size_t SendBytes(const std::vector<uint8_t>& data);

		void SetRxTimeouts(std::chrono::milliseconds timeOut);

		size_t CurrentRxQueueSize();
		size_t CurrentTxQueueSize();

		size_t CurrentRxInQueue();
		size_t CurrentTxInQueue();

	private:
		struct Impl;
		std::unique_ptr<Impl> m_impl;
	};

	class SerialPortRuntimeException
		: public std::runtime_error
	{
	public:
		SerialPortRuntimeException(const char *message)
			: std::runtime_error(message) {}
	};

	class SerialPortRangeError
		: public std::range_error
	{
	public:
		SerialPortRangeError(const char *message)
			: std::range_error(message) {}
	};
}

