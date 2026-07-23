#pragma once
#include <string>

class CgiExecutor 
{
	private:
		pid_t		_pid;			// Process ID of the spawned CGI child
		int			_pipeIn[2];		// Pipe: Parent writes request body -> Child reads STDIN
		int			_pipeOut[2];	// Pipe: Child writes output STDOUT -> Parent reads
		time_t		_startTime;		// Timestamp when the child was spawned (for timeouts)
		
		std::string	_bodyToWrite;	// Holds remaining POST request body to send to CGI
		std::string	_outputBuffer;	// Accumulates raw output read from CGI
		bool		_isFinished;	// Set to true when CGI closes stdout or exits

	public:
		CgiExecutor();
		~CgiExecutor();

		bool execute(char** envp, const std::string& scriptPath, const std::string& execPath, const std::string& body);
		
		// Non-blocking I/O handlers called by SocketManager events
		void handleWriteEvent(); // Writes chunk of _bodyToWrite to _stdinPipe[1]
		void handleReadEvent();  // Reads chunk from _stdoutPipe[0] into _outputRead

		// Non-blocking process & timeout management
		bool checkTimeout(time_t timeoutSeconds);
		bool isFinished();

		// Getters for SocketManager registration
		int getWriteFd() const;
		int getReadFd() const;
		const std::string& getOutput() const;
};