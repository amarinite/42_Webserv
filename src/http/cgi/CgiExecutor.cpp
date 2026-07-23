#include "CgiExecutor.hpp"

CgiExecutor::CgiExecutor()
	: _pid(-1), _startTime(0), _isFinished(false),
	  _bodyToWrite(), _outputRead()
{
	_stdinPipe[0] = -1;
	_stdinPipe[1] = -1;
	_stdoutPipe[0] = -1;
	_stdoutPipe[1] = -1;
}

CgiExecutor::~CgiExecutor()
{
	closePipes();
	if (_pid > 0 && !_isFinished) {
		kill(_pid, SIGKILL);
		waitpid(_pid, NULL, WNOHANG);
	}
}

bool CgiExecutor::execute(char** envp, const std::string& scriptPath, const std::string& execPath, const std::string& body)
{
	(void)envp;
	(void)scriptPath;
	(void)execPath;
	(void)body;
	// TODO
	return false;
}

void CgiExecutor::handleWriteEvent()
{
	// TODO
}

void CgiExecutor::handleReadEvent()
{
	// TODO
}

bool CgiExecutor::checkTimeout(double timeoutSeconds)
{
	if (_isFinished)
		return false;

	if (difftime(time(NULL), _startTime) > timeoutSeconds)
	{
		kill(_pid, SIGKILL);
		waitpid(_pid, NULL, WNOHANG); // Clean up zombie entry
		closePipes();
		_isFinished = true;
		return true;
	}
	return false;
}

bool CgiExecutor::isFinished()
{
	// TODO
	return _isFinished;
}

int CgiExecutor::getWriteFd() const
{
	// TODO
	return _stdinPipe[1];
}

int CgiExecutor::getReadFd() const
{
	// TODO
	return _stdoutPipe[0];
}

const std::string& CgiExecutor::getOutput() const
{
	return _outputRead;
}