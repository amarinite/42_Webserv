#include "Http.hpp"

Http::Http() {
	std::cout << "Http class: I shouldn't exist, yet here I am." << std::endl;
}

Http::Http(const HandleSocket &socket) : _status(READING_HEADERS), _socket(socket) {}

Http::Http(const Http &other) {
	*this = other;
}

Http &Http::operator=(const Http &other) {
	if (this != &other) {
		_socket = other._socket;
		_state = other._state;
		_request = other._request;
		_response = other_response;
	}
	return *this;
}

Http::~Http() {}

//Functs
void Http::addLeftover(std::string &rawBuffer, size_t &rawBufferSize) {
	if (_request._leftover == NULL && rawBuff != NULL)
		return;
	if (_request._leftover != NULL && rawBuff == NULL) {
		buff = _request._leftover;
		return;
	}
	if (_request._leftover == NULL && rawBuff == NULL)
		// Hauria de ser un exit de la func de parseig de request. De fet no hi ha ni info rebuda ni heredada.
	else {
		rawBuff = _leftover + rawBuff;
		rawBuffSize += _leftover.size();
		_leftover.clear();		
	}
}

void Http::handleBuffer(char *buff, size_t bytesRead) {
	if (!buff || bytesRead == 0)
		// Error no buff.
	std::string rawBuff(buff, bytesRead);
	size_t rawBuffSize = bytesRead;
	addLeftover(rawBuff);
	
	std::string head(rawBuff, rawBuffSize);
	size_t bodyStart = 0;

	if (_status == READING_HEADERS) {
		size_t end = head.find("\r\n\r\n");
		if (end == std::string::npos) {
			_request._stream = head;
			return;
		}
		_request._stream = head.substr(0, end + 4);
		bodyStart = end + 4;
	}
	size_t bodyLen = head.size() - bodyStart;
	_request._streamBody = head.substr(bodyStart, head.end());
}

void Http::methodGetCase() {
	if (_request._method == "GET") {
		if (_headers.count("content-length") > 0 || _headers.count("transfer-encoding") > 0)
			throw HttpException(400, "Bad Request: Body Present in Get Method.");
		else {
			_status = PROCESSING;
			return 1;
		}
		return 0;
	}
}

void Http::HttpRoutine(char *buff, size_t bytesRead) {
	switch(_status) {
		case READING_HEADERS: {
			handleBuffer(buff, bytesRead);
			if (_request.parseRequestHead()) {
				_status = READING_BODY;
				if (methodGetCase())
					continue;
				if (_request.parseRequestBody())
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

//Getters
HandleSocket	Http::getSocket {
	return this->_socket;
}

////////////////////////
// Revisa si inicialitzes correctament la request i la response. 
// - Maybe el constructor per defecte de request hauria de fer inicialitzacio basica. 