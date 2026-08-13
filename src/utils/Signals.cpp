#include "Signals.hpp"
#include <cstddef>

volatile sig_atomic_t g_shutdown = 0;

static void handleSigint(int signum)
{
	(void)signum;
	g_shutdown = 1;
}

void setupSignalHandlers()
{
	signal(SIGINT, handleSigint);
	signal(SIGTERM, handleSigint);
}
