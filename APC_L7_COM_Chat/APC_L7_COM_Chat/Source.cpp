#include <iostream>
#include <Windows.h>

#include <string>



int main()
{
	std::string portName("\\.\\COM");

	int portNum;
	std::cout << "Select serial port number to connect to: ";
	std::cin >> portNum;
	portName += std::to_string(portNum);

	HANDLE portHandle = CreateFileA(portName.data(), GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_EXISTING, NULL, NULL);
	if (portHandle == INVALID_HANDLE_VALUE)
	{
		std::cout << "Can't open port! Code: " << GetLastError() << std::endl;
		return -1;
	}


	//Define some parameters of serial port
	DCB serialConfig;
	serialConfig.DCBlength = sizeof(serialConfig);
	if (!GetCommState(portHandle, &serialConfig))
	{
		std::cout << "Can't get port settings!" << std::endl;
	}
	serialConfig.BaudRate = CBR_2400;
	serialConfig.ByteSize = 8;
	serialConfig.StopBits = ONESTOPBIT;
	serialConfig.Parity = NOPARITY;
	if (!SetCommState(portHandle, &serialConfig))
	{
		std::wcout << "Can't set port settings!" << std::endl;
	}


	int opt;
	std::cout << "Select mode:" << std::endl;
	std::cout << "1 - Receiver mode" << std::endl;
	std::cout << "2 - Transmitter mode" << std::endl;
	std::cin >> opt;

	switch (opt)
	{
	case 1:
	{
		std::cout << "Entered read mode. Displays all incoming messages. Close program to exit" << std::endl;

		const int BufSize = 512;

		char buf[BufSize + 1];
		DWORD bytesRead;


		//Mask out all signals except "character received". If it triggers, means data is incoming - can read it.
		if (!SetCommMask(portHandle, EV_RXCHAR))
		{
			std::cout << "Failed to set signal mask. Code: " << GetLastError() << std::endl;
		}

		while (1)
		{
			//Wait for start of input - event EV_RXCHAR.
			DWORD signalRes;
			bool res = WaitCommEvent(portHandle, &signalRes, NULL);

			if (!ReadFile(portHandle, buf, BufSize, &bytesRead, NULL))
			{
				std::cout << "FAILED TO READ. CODE: " << GetLastError() << std::endl;
				break;
			}
			//Doesn't cap off string by itself
			buf[bytesRead] = '\0';

			std::cout << "<" << buf << std::endl;

			//RXCHAR event will trip once more, maybe from last received character. Fix that
			if(bytesRead > 1)	//(single character can't cause 2 events, so skip)
				WaitCommEvent(portHandle, &signalRes, NULL);
		}

		break;
	}
	case 2:
	{
		std::cout << "Write mode. Can send lines of text through serial port. Empty input to exit" << std::endl;

		std::string buf;
		DWORD bytesWritten;

		std::cin.ignore();
		while (1)
		{
			std::cout << ">";
			std::getline(std::cin, buf);
			if (buf.empty())
				break;

			WriteFile(portHandle, buf.data(), buf.size(), &bytesWritten, NULL);
		}

		break;
	}
	default:
		break;	
	}

	CloseHandle(portHandle);
	std::cout << "Closed port handle. Done!" << std::endl;

	return 0;
}