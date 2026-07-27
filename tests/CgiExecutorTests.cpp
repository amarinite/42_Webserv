#include "TestUtils.hpp"
#include "CgiExecutor.hpp"
#include <fstream>
#include <cstdio>
#include <unistd.h>
#include <sys/select.h>
#include <cstring>
#include <sys/stat.h>

static void writeScript(const std::string &path, const std::string &content)
{
	std::ofstream f(path.c_str());
	f << content;
	f.close();
	chmod(path.c_str(), 0755);
}

// Drives handleWriteEvent()/handleReadEvent() using select() until the CGI
// finishes or the given wall-clock timeout elapses. Returns false on timeout.
static bool driveUntilFinished(CgiExecutor &cgi, int timeoutSeconds)
{
	time_t start = time(NULL);
	while (!cgi.isFinished()) {
		if (difftime(time(NULL), start) > timeoutSeconds)
			return false;

		fd_set readSet, writeSet;
		FD_ZERO(&readSet);
		FD_ZERO(&writeSet);
		int maxFd = -1;

		int rfd = cgi.getReadFd();
		int wfd = cgi.getWriteFd();
		if (rfd != -1) { FD_SET(rfd, &readSet); maxFd = rfd; }
		if (wfd != -1) { FD_SET(wfd, &writeSet); if (wfd > maxFd) maxFd = wfd; }

		if (maxFd == -1)
			break; // both fds closed but isFinished() not yet set — shouldn't happen, safety net

		struct timeval tv;
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		int ready = select(maxFd + 1, &readSet, &writeSet, NULL, &tv);
		if (ready < 0)
			return false;

		if (wfd != -1 && FD_ISSET(wfd, &writeSet))
			cgi.handleWriteEvent();
		if (rfd != -1 && FD_ISSET(rfd, &readSet))
			cgi.handleReadEvent();
	}
	return true;
}

static char** makeEmptyEnv()
{
	char **envp = static_cast<char**>(malloc(sizeof(char*)));
	envp[0] = NULL;
	return envp;
}

static bool testExecute_simpleGetProducesOutput()
{
	std::string scriptPath = "/tmp/webserv_cgi_echo.sh";
	writeScript(scriptPath,
		"#!/bin/sh\n"
		"echo \"Content-Type: text/plain\"\n"
		"echo \"\"\n"
		"echo \"hello from cgi\"\n");

	char **envp = makeEmptyEnv();
	CgiExecutor cgi;
	bool started = cgi.execute(envp, scriptPath, "/bin/sh", "");
	free(envp[0]); free(envp);

	ASSERT(started);
	ASSERT(driveUntilFinished(cgi, 5));
	ASSERT(cgi.getOutput().find("hello from cgi") != std::string::npos);
	ASSERT(cgi.getOutput().find("Content-Type: text/plain") != std::string::npos);

	remove(scriptPath.c_str());
	return true;
}

static bool testExecute_postBodyReachesScript()
{
	// cat echoes stdin straight back to stdout — perfect for checking the
	// body actually made it through the write pipe.
	std::string body = "field=value&another=thing";

	char **envp = makeEmptyEnv();
	CgiExecutor cgi;
	bool started = cgi.execute(envp, "/bin/cat", "/bin/cat", body);
	free(envp[0]); free(envp);

	ASSERT(started);
	ASSERT(driveUntilFinished(cgi, 5));
	ASSERT(cgi.getOutput() == body);
	return true;
}

static bool testExecute_emptyBodyClosesStdinImmediately()
{
	char **envp = makeEmptyEnv();
	CgiExecutor cgi;
	bool started = cgi.execute(envp, "/bin/cat", "/bin/cat", "");
	free(envp[0]); free(envp);

	ASSERT(started);
	// With no body, execute() should already have closed the write end.
	ASSERT(cgi.getWriteFd() == -1);
	ASSERT(driveUntilFinished(cgi, 5));
	ASSERT(cgi.getOutput().empty());
	return true;
}

static bool testCheckTimeout_killsHungScript()
{
	std::string scriptPath = "/tmp/webserv_cgi_hang.sh";
	writeScript(scriptPath, "#!/bin/sh\nsleep 30\n");

	char **envp = makeEmptyEnv();
	CgiExecutor cgi;
	bool started = cgi.execute(envp, scriptPath, "/bin/sh", "");
	free(envp[0]); free(envp);

	ASSERT(started);
	// timeoutSeconds = 0 -> should immediately be considered timed out
	ASSERT(cgi.checkTimeout(0) == true);
	ASSERT(cgi.isFinished() == true);

	remove(scriptPath.c_str());
	return true;
}

static bool testCheckTimeout_doesNotFireBeforeDeadline()
{
	std::string scriptPath = "/tmp/webserv_cgi_quick.sh";
	writeScript(scriptPath,
		"#!/bin/sh\n"
		"echo \"\"\n"
		"echo \"done\"\n");

	char **envp = makeEmptyEnv();
	CgiExecutor cgi;
	bool started = cgi.execute(envp, scriptPath, "/bin/sh", "");
	free(envp[0]); free(envp);

	ASSERT(started);
	ASSERT(cgi.checkTimeout(60) == false); // 60s deadline, shouldn't fire immediately
	ASSERT(driveUntilFinished(cgi, 5));

	remove(scriptPath.c_str());
	return true;
}

static bool testExecute_nonexistentInterpreterStillReturnsTrue()
{
	// execve failure happens in the child; the parent's execute() has no way
	// to know synchronously — it should still report "started" and the
	// child's exit(1) should surface as empty/short output + finished state.
	char **envp = makeEmptyEnv();
	CgiExecutor cgi;
	bool started = cgi.execute(envp, "/nonexistent/script.py", "/nonexistent/interpreter", "");
	free(envp[0]); free(envp);

	ASSERT(started);
	ASSERT(driveUntilFinished(cgi, 5));
	ASSERT(cgi.getOutput().empty());
	return true;
}

void runCgiExecutorTests(int& passed, int& failed)
{
	Test tests[] = {
		{ "execute: simple GET produces output",			testExecute_simpleGetProducesOutput },
		{ "execute: POST body reaches script",				testExecute_postBodyReachesScript },
		{ "execute: empty body closes stdin immediately",	testExecute_emptyBodyClosesStdinImmediately },
		{ "checkTimeout: kills hung script",				testCheckTimeout_killsHungScript },
		{ "checkTimeout: does not fire before deadline",	testCheckTimeout_doesNotFireBeforeDeadline },
		{ "execute: bad interpreter path handled",			testExecute_nonexistentInterpreterStillReturnsTrue }
	};
	int localPassed = 0;
	int localFailed = 0;
	for (int i = 0; i < (int)ARRAY_SIZE(tests); i++) {
		if (tests[i].fn()) {
			localPassed++;
		} else {
			std::cout << RED << "  [FAIL] " << tests[i].name << "\n" << RESET;
			localFailed++;
		}
	}
	printTestSummary("CgiExecutor", localPassed, localFailed);
	passed += localPassed;
	failed += localFailed;
}