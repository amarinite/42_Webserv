#include "HttpRequest.hpp"

Request::Request() : 
	_methodParsed(false),
	_uriParsed(false),
	_httpVerParsed(false),
	_parsedKey(false),
	_parsedValue(false),
	_incompleteEndLine(false),
	_bodyType(EMPTY),
	_maxBodySize(0),
	_chunkSize(false) {}

Request::Request(const Request &other) : _leftover(NULL) {
	*this = other;
}

Request &Request::operator=(const Request &other) {
	if (this != &other) {
		this->_headers = other._headers;
		this->_body = other._body;
		this->_stream = other._stream;
		this->_leftover = other._leftover;
	}
	return *this;
}

Request::~Request() {}

// Functs
static void tolowerStr(std::string &str) {
	for (size_t i = 0; i < str.size(); ++i) {
		unsigned char c = static_cast<char>(str[i]);
		str[i] = static_cast<char>(std::tolower(c));
	}
}

static void safeGetLine(std::string &stream, std::string &var, char delimiter, bool &done) {
	size_t pos = stream.find(delimiter);
	if (pos != std::string::npos) {
		var += stream.substr(0, pos);
		stream.erase(0, pos + 1);
		done = true;
		return;
	}
	var += stream;
	stream.clear();
	done = false;
}

bool Request::safeEnd() {
	if (_stream.size() == 0) {
		_incompleteEndLine = true;
		return false;
	}
	if (_stream[0] != '\n')
		throw HttpException(400, "Bad Request: Invalid Delimiters");
	_incompleteEndLine = false;
	_stream.erase(0, 1);
	return true;
}

bool Request::parseMethod() {
	if (!this->_methodParsed) {
		safeGetLine(_stream, _method, ' ', this->_methodParsed);
		if (!this->_methodParsed)
			return false;
		if (_method != "GET" && _method != "POST" && _method != "DELETE") // To be redone by server restrictions.
			throw HttpException(400, "Bad Request: Invalid Method");
	}

	if (!this->_uriParsed) {
		safeGetLine(_stream, _uriStr, ' ', this->_uriParsed);
		if (!this->_uriParsed)
			return false;
		parseUri(this->_uri, _uriStr);
		this->_uriStr.clear();
	}

	if (!this->_httpVerParsed) {
		safeGetLine(_stream, _httpVer, '\r', this->_httpVerParsed);
		if (!this->_httpVerParsed)
			return false;
		if (_httpVer != "HTTP/1.1")
			throw HttpException(400, "Bad Request: Incorrect HTTP Protocol");
		if (!safeEnd())
			return false;
	}
	if (_incompleteEndLine == true) {
		if (!safeEnd())
			return false;
	}
	return true;
}

bool Request::findKey() {
	if (!this->_parsedKey) {
		safeGetLine(_stream, _tmpKey, ':', this->_parsedKey);
		if (!this->_parsedKey)
			return false;
		else if (_tmpKey.find(' ') != std::string::npos)
				throw HttpException(400, "Bad Request: Space found in Header");
		else if (_tmpKey.size() == 0)
				throw HttpException(400, "Bad Request: Empty host key");
		tolowerStr(_tmpKey);
	}
	return true;
}

static void handleValueSpaces(std::string &value) {
	size_t first = value.find_first_not_of(' ');
	if (first == std::string::npos) { // todo espacios o vacío
		value.clear();
		return;
	}
	size_t last = value.find_last_not_of(' ');
	value.erase(last + 1);
	value.erase(0, first);
}

bool Request::findValue() {
	if (!this->_parsedValue) {
		safeGetLine(_stream, _tmpVal, '\r', this->_parsedValue);
		if (!this->_parsedValue)
			return false;
		if (!safeEnd())
			return false;
	}
	handleValueSpaces(_tmpVal);
	return true;
}

void Request::addHeader() {
	if (this->_parsedKey && this->_parsedValue) {
		if (this->_headers.count(_tmpKey)) {
			if (_tmpKey == "host" || _tmpKey == "content-length") {
				_tmpKey.clear();
				_tmpVal.clear();
				throw HttpException(400, "Bad Request: Invalid Header");
			}
			this->_headers[_tmpKey] += ", " + _tmpVal;
		} else
			this->_headers[_tmpKey] = _tmpVal;
	}
	_tmpKey.clear();
	_tmpVal.clear();
	this->_parsedKey = false;
	this->_parsedValue = false;
}

static void findColon(const std::string &stream, size_t headerEnd) {
	std::string header = stream.substr(0, headerEnd);
	size_t colon = header.find(":");
	if (colon == std::string::npos)
		throw HttpException(400, "Bad Request: No colon found in header.");
}

static void isBadlyClosed(const std::string &stream) {
	size_t posN = stream.find("\n");
	size_t posR = stream.find("\r");

	if (posN < posR)
		throw HttpException(400, "Bad Request: Bad Delimiter '\n'.");
}

bool Request::parseHeaders() {
	while (true) {
		isBadlyClosed(_stream);
		size_t del = _stream.find("\r\n");
		if (del == std::string::npos) {
			_leftover = _stream;
			_stream.clear();
			return false;
		}
		if (del == 0) {
			_stream.erase(0, 2);
			return true;
		}
		findColon(_stream, del);
		this->findKey();
		this->findValue();
		this->addHeader();
	}
	// Connection: Puede ser keep-alive (mantener el socket abierto para reutilizarlo en futuras peticiones) o close (cerrar el socket en cuanto envíes la respuesta).
}

void Request::checkInvalidHeaders() {
	if (_headers.count("host") <= 0)
		throw HttpException(400, "Bad Request: Header Host not present.");
	if (_headers.count("content-length")) {
		if (_headers["content-length"].find("-") != std::string::npos)
			throw HttpException(400, "Bad Request: Negative body length.");
	}

}

bool Request::parseRequestHead() {
	if (!parseMethod())
		return false;
	if (!parseHeaders())
		return false;
	checkInvalidHeaders();
			
	return true;
}

// Body Functs
bool isHexDigit(char c) {
	return std::isdigit(static_cast<unsigned char>(c)) ||
		   (c >= 'a' && c <= 'f') ||
		   (c >= 'A' && c <= 'F');
}

static size_t strToSize_t(const std::string &str, const int base) {
	if (base == 10) {
		for (size_t i = 0; i < str.size(); ++i) {
			if (!std::isdigit(static_cast<unsigned char>(str[i])))
				throw HttpException(400, "Bad Request: Invalid Body Size.");
		}
	} else if (base == 16) {
		for (size_t i = 0; i < str.size(); ++i) {
			if (!isHexDigit(str[i])) {
				throw HttpException(400, "Bad Request: Invalid Body Size.");
			}
		}
	} else 
		throw HttpException(400, "Bad Request: Unsuported Base.");
	return (static_cast<size_t>(std::strtoul(str.c_str(), NULL, base)));
}

void Request::setBodyType() {
	if (_bodyType != EMPTY)
		return;

	bool hasContentLength = _headers.count("content-length") > 0;
	bool hasTransferEncoding = _headers.count("transfer-encoding") > 0;

	if (hasContentLength && hasTransferEncoding)
		throw HttpException(400, "Bad Request: Repeated Headers.");
	else if (!hasContentLength && !hasTransferEncoding) {
		if (_stream.size() > 0)
			_leftover = _stream;
	}
	else if (hasContentLength) {
		_bodyType = FULL;
		_maxBodySize = strToSize_t(_headers.find("content-length")->second.c_str(), 10);
	} else if (hasTransferEncoding) 
		_bodyType = CHUNKED;
}

bool Request::fullBody() {
	size_t total = _body.size() + _stream.size();

	if (total < _maxBodySize) {
		_body += _stream;
		_stream.clear();
		return false;
	} else if (total == _maxBodySize) {
		_body += _stream;
		_maxBodySize = 0;
		_stream.clear();
		return true;
	} else {
		size_t remaining = _maxBodySize - _body.size();
		_leftover = _stream.substr(remaining);
		_body += _stream.substr(0, remaining);
		_stream.clear();		
		_maxBodySize = 0;
		return true;
	}
}

bool Request::chunkedBody() {
	
	const std::string delimiter = "\r\n";
	while (true) {
		if (!_leftoverBody.empty()) {
		_stream = _leftoverBody + _stream;
		_leftoverBody.clear();
		}
		size_t pos = _stream.find(delimiter);
		if (pos == std::string::npos) {
			_leftoverBody = _stream;
			_stream.clear();
			return false;
		}

		if (!_chunkSize) {
			std::string sizeStr = _stream.substr(0, pos);

			if (sizeStr == "0") {
				if (_stream.size() < pos + 4) {
					_leftoverBody = _stream;
					_stream.clear();
					return false;
				}
				if (_stream.substr(pos, 4) != "\r\n\r\n")
					throw HttpException(400, "Bad Request: Invalid Body.");
				_leftover = _stream.substr(pos + 4);
				_stream.clear();
				return true;
			} 
			_maxBodySize = strToSize_t(sizeStr, 16);
			_stream.erase(0, pos + 2);
			_chunkSize = true;
		} else {
			if (_stream.size() < _maxBodySize + 2) {
				_leftoverBody = _stream;
				_stream.clear();
				return false;
			}

			_body += _stream.substr(0, _maxBodySize);

			if (_stream.substr(_maxBodySize, 2) != "\r\n")
				throw HttpException(400, "Bad Request: Invalid Body.");
			_leftoverBody = _stream.substr(_maxBodySize + 2);
			_stream.clear();
			_chunkSize = false;
			_maxBodySize = 0;
		}
	}
}

bool Request::parseRequestBody() {
	setBodyType();
	if (_bodyType == FULL) {
		if(!fullBody())
			return false;
	} else if (_bodyType == CHUNKED) {
		if (!chunkedBody())
			return false;
	}
	return true;
}

//Getters
std::string	Request::getMethod() {
	return this->_method;
}

std::map<std::string, std::string>  Request::getHeaders() {
	return this->_headers;
}

std::string	Request::getBody() {
	return this->_body;
}

std::string Request::getPath() {
	return this->_uri.path;
}