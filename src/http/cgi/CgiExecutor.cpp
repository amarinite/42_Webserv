#include "CgiExecutor.hpp"

CgiExecutor::CgiExecutor()
    : _pid(-1), _startTime(0), _bodyToWrite(), _outputBuffer(), _isFinished(false)
{
    _pipeIn[0] = -1;
    _pipeIn[1] = -1;
    _pipeOut[0] = -1;
    _pipeOut[1] = -1;
}

CgiExecutor::~CgiExecutor()
{
	closePipes();
	if (_pid > 0 && !_isFinished) {
		kill(_pid, SIGKILL);
		waitpid(_pid, NULL, 0);
	}
}

void CgiExecutor::closePipes()
{
	if (_pipeIn[0] != -1) { close(_pipeIn[0]); _pipeIn[0] = -1; }
	if (_pipeIn[1] != -1) { close(_pipeIn[1]); _pipeIn[1] = -1; }
	if (_pipeOut[0] != -1) { close(_pipeOut[0]); _pipeOut[0] = -1; }
	if (_pipeOut[1] != -1) { close(_pipeOut[1]); _pipeOut[1] = -1; }
}

void CgiExecutor::setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0)
		throw std::runtime_error("fcntl(GETFL) failed");

	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
		throw std::runtime_error("fcntl(SETFL) failed");
}


bool CgiExecutor::execute(char** envp, const std::string& scriptPath, const std::string& execPath, const std::string& body)
{
	// Opens two pipes via pipe().
	if	(pipe(_pipeIn) == -1)
		throw std::runtime_error("pipe creation failed");

	if (pipe(_pipeOut) == -1)
	{
		closePipes();
		throw std::runtime_error("pipe creation failed");
	}

	// Sets the parent ends (_pipeIn[1] and _pipeOut[0]) to O_NONBLOCK using fcntl().
	setNonBlocking(_pipeIn[1]);
	setNonBlocking(_pipeOut[0]);

	// Stores POST body and record start time
	_bodyToWrite = body;
	_startTime = time(NULL);

	// Fork process: execute script after redirect and set up envp
	_pid = fork();
	if (_pid < 0)
	{
		closePipes();
		throw std::runtime_error("fork failed");
	}
	if (_pid == 0)
	{
		if (dup2(_pipeIn[0], STDIN_FILENO) == -1)
			exit(1);
		if (dup2(_pipeOut[1], STDOUT_FILENO) == -1)
			exit(1);

		closePipes();
		char* argv[] = {
			const_cast<char*>(execPath.c_str()),
			const_cast<char*>(scriptPath.c_str()),
			NULL
		};
		execve(execPath.c_str(), argv, envp);
		exit(1);
	}
	else
	{
		close(_pipeIn[0]);
		close(_pipeOut[1]);
		_pipeIn[0] = -1;
		_pipeOut[1] = -1;

		// If there is no request body the script knows STDIN reaches EOF
		if (_bodyToWrite.empty())
		{
			close(_pipeIn[1]);
			_pipeIn[1] = -1;
		}
		return true;
	}
	return false;
}

void CgiExecutor::handleWriteEvent()
{
	if (_pipeIn[1] == -1 || _bodyToWrite.empty())
		return;

	ssize_t bytesWritten = write(_pipeIn[1], _bodyToWrite.c_str(), _bodyToWrite.size());

	if (bytesWritten > 0)
	{
		_bodyToWrite.erase(0, bytesWritten);

		// If all body data has been written, close write pipe to send EOF to CGI stdin
		if (_bodyToWrite.empty())
		{
			close(_pipeIn[1]);
			_pipeIn[1] = -1;
		}
	}
	else if (bytesWritten == -1)
	{
		close(_pipeIn[1]);
		_pipeIn[1] = -1;
		_isFinished = true;
	}
}

void CgiExecutor::handleReadEvent()
{
	if (_pipeOut[0] == -1)
		return;

	char buffer[4096];
	ssize_t bytesRead = read(_pipeOut[0], buffer, sizeof(buffer));

	if (bytesRead > 0)
	{
		_outputBuffer.append(buffer, bytesRead);
	}
	else if (bytesRead == 0) // EOF reached (CGI script finished writing)
	{
		close(_pipeOut[0]);
		_pipeOut[0] = -1;
		_isFinished = true;

		if (_pid > 0)
			waitpid(_pid, NULL, 0);
	}
	else // Error on read
	{
		close(_pipeOut[0]);
		_pipeOut[0] = -1;
		_isFinished = true;
	}
}

bool CgiExecutor::checkTimeout(time_t timeoutSeconds)
{
	if (_isFinished)
		return false;

	if (difftime(time(NULL), _startTime) >= timeoutSeconds)
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
	return _isFinished;
}

int CgiExecutor::getWriteFd() const
{
	return _pipeIn[1];
}

int CgiExecutor::getReadFd() const
{
	return _pipeOut[0];
}

const std::string& CgiExecutor::getOutput() const
{
	return _outputBuffer;
}
