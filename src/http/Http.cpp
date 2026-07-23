#include "Http.hpp"

Http::Http(ServerConfig &sc) : _rawBuff(""),
	_rawBuffSize(0),
	_status(READING_HEADERS),
	_request(sc.getClientMaxBodySize();),
	_config(sc)
{}

Http::Http(const ServerConfig &conf) : _config(conf), 
	_rawBuff(""),
	_rawBuffSize(0),
	_status(READING_HEADERS),
	_request()
{}

// Http::Http(const HandleSocket &socket) : _status(READING_HEADERS), _socket(socket) {}

Http::Http(const Http &other) {
	*this = other;
}

Http &Http::operator=(const Http &other) {
	if (this != &other) {
		_rawBuff = other._rawBuff;
		_rawBuffSize = other._rawBuffSize;
		_status = other._status;
		_request = other._request;
		// _response = other._response;
	}
	return *this;
}

Http::~Http() {}

//Functs
void Http::addLeftover(std::string &rawBuff, size_t &rawBufferSize) {
	const bool hasLeftover = !_request._leftover.empty();
	const bool hasRawBuff  = !rawBuff.empty();

	if (!hasLeftover && hasRawBuff)
		return;
	if (hasLeftover && !hasRawBuff) {
		rawBuff = _request._leftover;
		rawBufferSize = rawBuff.size();
		_request._leftover.clear();
		return;
	}
	if (!hasLeftover && !hasRawBuff)
		throw HttpException(400, "Bad Request: Empty Buffer.");
	rawBuff = _request._leftover + rawBuff;
	rawBufferSize += _request._leftover.size();
	_request._leftover.clear();
}

void Http::handleBuffer(char *buff, size_t bytesRead) {
	std::string rawBuff;
	if (buff != NULL && bytesRead > 0)
		rawBuff.assign(buff, bytesRead);

	size_t rawBuffSize = bytesRead;
	addLeftover(rawBuff, rawBuffSize);

	_request._stream = rawBuff;
}

bool Http::methodGetCase() {
	if (_request._method != "GET") 
		return true;
	if (_request._headers.count("content-length") > 0
		|| _request._headers.count("transfer-encoding") > 0)
		throw HttpException(400, "Bad Request: Body Present in GET Method.");
	return true;
}


la_funct_del_isaac() {
	// Deberia ser algo asi:
	Http Request; 
	char buffer[cantidad];
	size_t bytesRead = recv(something, &buffer, something);
	try {
		Request.httpRoutine(buffer, bytesRead);
		buildResponse()
	} catch (const HttpException& e) {
		Request._status = WRITING_RESPONSE;
		buildResponse();
	}
}

void Http::HttpRoutine(char *buff, size_t bytesRead) {
	switch(_status) {
		case READING_HEADERS: {
			handleBuffer(buff, bytesRead);
			if (_request.parseRequestHead()) {
				_status = READING_BODY;
				if (methodGetCase() && _request.parseRequestBody())
					_status = PROCESSING;
			}
			break;
		}
		case READING_BODY: {
			handleBuffer(buff, bytesRead);
			if (_request.parseRequestBody())
				_status = PROCESSING;
			break;
		}
		case PROCESSING: {
			//Alba
			break;
		}
		case WRITING_RESPONSE: {
			break;
		}
		case FINISHED: {
			// Segurament no fa falta.
			break;
		}
	}
}

void Http::buildResponse(const HttpException& e) {
	_response.assignHead(e);
	_response.assignHeaders(_request._headers);
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
////////////////////////
// Revisa si inicialitzes correctament la request i la response. 
// - Maybe el constructor per defecte de request hauria de fer inicialitzacio basica. 
