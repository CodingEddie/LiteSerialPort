#if defined(_WIN32)

#include <Windows.h>
#include <CommCtrl.h>
#include <sstream>
#include <string>
#include <stdexcept>
#include "SerialPort.h"

#define SERIAL_RUNTIME_EXCEPTION(msg) \
	m_impl->throwRunTimeException(msg, __FILE__, __func__, __LINE__)

inline void Lite::SerialPort::Impl::throwRunTimeException(const char* msg,
	const char* fileName,
	const char* functionName,
	const std::size_t lineNumber)
{
	std::ostringstream os;
	os << "Exception: " << msg << ", file: " << fileName << " function: "
		<< functionName << " lineNumber: " << lineNumber;
	throw Lite::SerialPortRuntimeException(os.str().c_str());
}

struct Lite::SerialPort::Impl
{
	BYTE ParityValue(const SerialPort::PARITY parity);
	BYTE StopBitsValue(const SerialPort::STOPBITS stopBits);

	void throwRunTimeException(const char* msg,
		const char* fileName,
		const char* functionName,
		const std::size_t lineNumber);

	std::string GetCommError();
	HANDLE m_hCommPort;
};

std::string Lite::SerialPort::Impl::GetCommError()
{
	LPSTR lpMsgBuf;
	DWORD errCode = GetLastError();
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errCode,
		0, // Default language
		(LPTSTR)&lpMsgBuf,
		0,
		NULL
	);

	std::stringstream temp;
	temp << lpMsgBuf;
	return temp.str();
}

Lite::SerialPort::SerialPort(const std::string name, const uint32_t baudRate,
	const PARITY parity, const STOPBITS stopBit, const FLOWCONTROL flowControl,
	const uint8_t databits)
	: m_impl(std::make_unique <Impl>())
{
	m_impl->m_hCommPort = CreateFile(name.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		0,
		0,
		OPEN_EXISTING,
		0,
		0
	);
	if(m_impl->m_hCommPort == INVALID_HANDLE_VALUE) { 
		SERIAL_RUNTIME_EXCEPTION(m_impl->GetCommError().c_str());
	}
		//throw SerialPortRuntimeException(m_impl->GetCommError().c_str());

	DCB dcb;
	dcb.DCBlength = sizeof(DCB);

	if(!GetCommState(m_impl->m_hCommPort, &dcb))
		throw SerialPortRuntimeException(m_impl->GetCommError().c_str());

	dcb.BaudRate = baudRate;
	dcb.ByteSize = databits;
	dcb.Parity = m_impl->ParityValue(parity);
	dcb.StopBits = m_impl->StopBitsValue(stopBit);

	if (!SetCommState(m_impl->m_hCommPort, &dcb))
		throw SerialPortRuntimeException(m_impl->GetCommError().c_str());

	return;
}

Lite::SerialPort::~SerialPort()
{
	CloseHandle(m_impl->m_hCommPort);

	return;
}

std::vector<uint8_t> Lite::SerialPort::ReadBytes(std::vector<uint8_t>& buffer, 
	const size_t bytesToRead)
{
	if (buffer.size() > MAXDWORD)
		throw SerialPortRangeError("Size out of Range");

	if (bytesToRead > buffer.capacity())
		buffer.reserve(bytesToRead);

	DWORD bytesRead;
	
	ReadFile(m_impl->m_hCommPort, buffer.data(), bytesToRead, &bytesRead, NULL);

	return buffer;
}

size_t Lite::SerialPort::SendBytes(const std::vector<uint8_t>& data)
{
	if (data.size() > MAXDWORD)
		throw SerialPortRangeError("Size out of Range");

	DWORD sentSize;
	WriteFile(m_impl->m_hCommPort, data.data(), data.size(), &sentSize, NULL);

	return sentSize;
}

void Lite::SerialPort::SetRxTimeouts(std::chrono::milliseconds timeOut)
{
	COMMTIMEOUTS to;

	if (!GetCommTimeouts(m_impl->m_hCommPort, &to))
		throw SerialPortRuntimeException("get commTimeouts");

	to.ReadIntervalTimeout = 0;
	to.ReadTotalTimeoutMultiplier = 0;
	to.ReadTotalTimeoutConstant = timeOut.count();
	
	if(!SetCommTimeouts(m_impl->m_hCommPort, &to))
		throw SerialPortRuntimeException("get commTimeouts");

	return;
}

size_t Lite::SerialPort::CurrentRxQueueSize()
{
	COMMPROP cp;
	
	if (!GetCommProperties(m_impl->m_hCommPort, &cp))
		throw SerialPortRuntimeException(m_impl->GetCommError().c_str());

	return cp.dwCurrentRxQueue;
}

size_t Lite::SerialPort::CurrentTxQueueSize()
{
	COMMPROP cp;

	if (!GetCommProperties(m_impl->m_hCommPort, &cp))
		throw SerialPortRuntimeException(m_impl->GetCommError().c_str());

	return cp.dwCurrentTxQueue;
}

size_t Lite::SerialPort::CurrentRxInQueue()
{
	COMSTAT cs;
	DWORD error;

	if (!ClearCommError(m_impl->m_hCommPort, &error, &cs))
		throw SerialPortRuntimeException(m_impl->GetCommError().c_str());

	return cs.cbInQue;
}

size_t Lite::SerialPort::CurrentTxInQueue()
{
	COMSTAT cs;
	DWORD error;

	if (!ClearCommError(m_impl->m_hCommPort, &error, &cs))
		throw std::runtime_error(m_impl->GetCommError().c_str());

	return cs.cbOutQue;
}

BYTE Lite::SerialPort::Impl::ParityValue(const SerialPort::PARITY parity)
{
	BYTE temp;

	switch (parity)
	{
	case SerialPort::PARITY::NONE:
		temp = NOPARITY;
		break;
	case SerialPort::PARITY::ODD:
		temp = ODDPARITY;
		break;
	case SerialPort::PARITY::EVEN:
		temp = EVENPARITY;
		break;
	case SerialPort::PARITY::MARK:
		temp = MARKPARITY;
		break;
	case SerialPort::PARITY::SPACE:
		temp = SPACEPARITY;
		break;
	default:
		throw SerialPortRangeError("Enum out of range");
		break;
	}

	return temp;
}

BYTE Lite::SerialPort::Impl::StopBitsValue(const SerialPort::STOPBITS stopBits)
{
	BYTE temp;
	switch (stopBits)
	{
	case SerialPort::STOPBITS::ONE:
		temp = ONESTOPBIT;
		break;
	case SerialPort::STOPBITS::ONEPT5:
		temp = ONE5STOPBITS;
		break;
	case SerialPort::STOPBITS::TWO:
		temp = TWOSTOPBITS;
		break;
	default:
		throw SerialPortRangeError("Enum out of range");
		break;
	}

	return temp;
}

#endif