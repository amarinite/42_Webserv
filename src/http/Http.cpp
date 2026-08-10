#include "Http.hpp"

Http::Http(const ServerConfig &sc) :
	_rawBuff(""),
	_rawBuffSize(0),
	_status(READING_HEADERS),
	_request(sc.getClientMaxBodySize()),
	_response(),
	_sConfig(sc),
	_processor(NULL),
	_cgi(NULL)
{
}

Http::~Http() {
	if (_processor != NULL) {
		delete _processor;
		_processor = NULL;
	}
	delete _cgi;
}

//Functs
void Http::addLeftover(std::string &rawBuff, size_t &rawBufferSize) {
	const bool hasLeftover = !_request.getLeftover().empty();
	const bool hasRawBuff  = !rawBuff.empty();

	if (!hasLeftover && hasRawBuff)
		return;
	if (hasLeftover && !hasRawBuff) {
		rawBuff = _request.getLeftover();
		rawBufferSize = rawBuff.size();
		_request.clearLeftover();
		return;
	}
	if (!hasLeftover && !hasRawBuff)
		throw HttpException(400, "Bad Request: Empty Buffer.");
	rawBuff = _request.getLeftover() + rawBuff;
	rawBufferSize += _request.getLeftover().size();
	_request.clearLeftover();
}

void Http::handleBuffer(char *buff, size_t bytesRead) {
	std::string rawBuff;
	if (buff != NULL && bytesRead > 0)
		rawBuff.assign(buff, bytesRead);

	size_t rawBuffSize = bytesRead;
	addLeftover(rawBuff, rawBuffSize);

	_request.setStream(rawBuff);
}

bool Http::methodGetCase() {
	if (_request.getMethod() != "GET")
		return true;
	const std::map<std::string, std::string> &headers = _request.getHeaders();
	if (headers.count("content-length") > 0 || headers.count("transfer-encoding") > 0)
		throw HttpException(400, "Bad Request: Body Present in GET Method.");
	return true;
}

void Http::setClientIp(const std::string &ip) {
	_clientIp = ip;
}

// la_funct_del_isaac() {
// 	// Deberia ser algo asi:
// 	Http Request;
// 	char buffer[cantidad];
// 	size_t bytesRead = recv(something, &buffer, something);
// 	try {
// 		Request.httpRoutine(buffer, bytesRead);
// 		buildResponse();
// 	} catch (const HttpException& e) {
// 		Request._status = WRITING_RESPONSE;
// 		buildResponse();
// 	}
// }

void Http::startProcessing() {
	const LocationConfig &lc = _sConfig.getLocationConfig(_request.getUri());
	delete _processor;
	_processor = new Processor(_request, _response, lc);
}

void Http::startCgi() {
	char **envp = CgiHandler::buildCgiEnv(_request, _sConfig, _clientIp);
	_cgi = new CgiExecutor();
	try {
		_cgi->execute(envp, _processor->getCgiScriptPath(),
			_processor->getCgiExecPath(), _request.getBody());
	} catch (const std::exception &ex) {
		CgiHandler::freeCgiEnv(envp);
		delete _cgi;
		_cgi = NULL;
		throw HttpException(500,
			std::string("Internal Server Error: CGI failed to start (") + ex.what() + ")");
	}
	CgiHandler::freeCgiEnv(envp);
}

bool Http::checkCgiTimeout(double timeoutSeconds) {
	if (!_cgi)
		return false;
	if (_cgi->checkTimeout(timeoutSeconds)) {
		finishWithError(HttpException(504, "Gateway Timeout"));
		return true;
	}
	return false;
}

void Http::HttpRoutine(char *buff, size_t bytesRead) {
	try {
	switch (_status) {
			case READING_HEADERS: {
				handleBuffer(buff, bytesRead);
				if (_request.parseRequestHead()) {
					_status = READING_BODY;
					if (methodGetCase() && _request.parseRequestBody()) {
						_status = PROCESSING;
						goto processing;
					}
				}
				break;
			}
			case READING_BODY: {
				handleBuffer(buff, bytesRead);
				if (_request.parseRequestBody()) {
					_status = PROCESSING;
					goto processing;
				}
				break;
			}
			case PROCESSING: {
			processing:
				startProcessing();
				_processor->processorRoutine();
				if (_processor->wantsCgi()) {
					startCgi();
					_status = (_cgi->getWriteFd() == -1) ? CGI_READING : CGI_WRITING;
				} else {
					_status = WRITING_RESPONSE;
					goto wresponse;
				}
				break;
			}
			case CGI_WRITING:
			case CGI_READING:
				break;
			case WRITING_RESPONSE: {
			wresponse:
				_processor->prepareResponse();
				_status = FINISHED;
				break;
			}
			case FINISHED:
				break;
		}
	} catch (const HttpException &e) {
		finishWithError(e);
	}
}

void Http::finishWithError(const HttpException &e) {
	delete _cgi;
	_cgi = NULL;
	_response.prepareErrorResponse(_sConfig.getErrorPages(), e);
	_status = FINISHED;
}

bool Http::isWaitingOnCgi() const {
	return _status == CGI_WRITING || _status == CGI_READING;
}

void Http::onCgiWritable() {
	try {
		_cgi->handleWriteEvent();
		if (_cgi->getWriteFd() == -1)
			_status = CGI_READING;
	} catch (const HttpException &e) {
		finishWithError(e);
	}
}

void Http::onCgiReadable() {
	try {
		_cgi->handleReadEvent();
		if (_cgi->isFinished()) {
			_processor->consumeCgiOutput(_cgi->getOutput()); // parse CGI header block -> Response
			delete _cgi;
			_cgi = NULL;
			_status = WRITING_RESPONSE;
		}
	} catch (const HttpException &e) {
		finishWithError(e);
	}
}

int Http::getCgiWriteFd() const {
	return _cgi ? _cgi->getWriteFd() : -1;
}

int Http::getCgiReadFd()  const {
	return _cgi ? _cgi->getReadFd()  : -1;
}

void Http::buildResponse(const HttpException& e) {
	(void)e;
	_response.setStatusCode("200");
	_response.setMessage("OK");
	_response.setResponseBody("");
	_response.assignHeaders(".html", "close");
	_response.buildRawResponse();
}

// Getters
State Http::getStatus() const {
	return _status;
}

const Request &Http::getRequest() const {
	return _request;
}

Request &Http::getRequest() {
	return _request;
}

const Response &Http::getResponse() const {
	return _response;
}

Response &Http::getResponse() {
	return _response;
}
