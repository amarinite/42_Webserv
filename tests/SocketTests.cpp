#include "SocketManager.hpp"
#include "TestUtils.hpp"

bool testSocketCreation()
{
	try
	{
		HandleSocket nSock = HandleSocket("127.0.0.1", 4343);
		nSock.createSocket();
		nSock.setReuseAddr();
		nSock.bindSocket();
		nSock.listenSocket();
		nSock.setNonBlocking();
		std::cout << "Server escuchando en el puerto " << nSock.getPort() << " con fd " << nSock.getFD() << std::endl;
		int flags = fcntl(nSock.getFD(), F_GETFL, 0);
		ASSERT(flags != -1);
		ASSERT(flags & O_NONBLOCK);
		// SOLO PARA TESTEO MANUAL - quitar después
		std::cout << "Esperando 10 segundos, prueba con nc..." << std::endl;
		sleep(10);
		return true;
	}
	catch(const std::exception& e)
	{
		std::cout << RED << "  [EXCEPTION] " << e.what() << "\n" << RESET;
		return false;
	}
}

void runSocketTests(int& passed, int& failed)
{
	Test tests[] = {
		{ "Assign a valid FD to a Socket",	testSocketCreation}
	};

	int localPassed = 0;
	int localFailed = 0;

	for (int i = 0; i < (int)ARRAY_SIZE(tests); i++)
	{
		if (tests[i].fn()) {
			localPassed++;
		} else {
			std::cout << RED << "  [FAIL] " << tests[i].name << "\n" << RESET;
			localFailed++;
		}
	}

	printTestSummary("Sockets", localPassed, localFailed);

	passed += localPassed;
	failed += localFailed;
}
