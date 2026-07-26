#include "Http.hpp"

Http::Http(const ServerConfig &sc) :
	_rawBuff(""),
	_sConfig(sc),
	_status(READING_HEADERS),
    _request(sc.getClientMaxBodySize()),
    _response(),
    _processor(NULL) {}

Http::~Http() {
	if (_processor != NULL) {
		delete _processor;
		_processor = NULL;
	}
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

void Http::HttpRoutine(char *buff, size_t bytesRead) {
	switch(_status) {
		case READING_HEADERS: {
			handleBuffer(buff, bytesRead);
			if (_request.parseRequestHead()) {
				_status = READING_BODY;
				if (methodGetCase() && _request.parseRequestBody()) {
					_status = PROCESSING;
					if (_processor != NULL)
						delete _processor;
					_processor = new Processor(_request, _response, _sConfig.getLocationConfig(_request.getUri()));
					goto processing;
				}
			}
			break;
		}
		case READING_BODY: {
			handleBuffer(buff, bytesRead);
			if (_request.parseRequestBody()) {
				_status = PROCESSING;
				if (_processor != NULL)
						delete _processor;
				_processor = new Processor(_request, _response, _sConfig.getLocationConfig(_request.getUri()));
				goto processing;
			}
			break;
		}
		case PROCESSING: {
		processing:	
			_processor->processorRoutine();
			_status = WRITING_RESPONSE;	
		}
		// fall through
		case WRITING_RESPONSE: {
			_processor->prepareResponse();
			_status = FINISHED;
		}
		// fall through
		case FINISHED:
			break;
	}
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

