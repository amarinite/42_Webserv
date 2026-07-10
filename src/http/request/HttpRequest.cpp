#include "HttpRequest.hpp"

Request::Request() : _headRead(false), _bodyRead(false), _leftover(NULL) {}

Request::Request(const Request &other) {
	this = other;
}

Request &Request::operator=(const Request &other) {
	if (this != &other) {
		this->_rawRequest = other._rawRequest;
		this->_methodHeader = other._methodHeader;
		this->_headers = other._headers;
		this->_body = other._body;
	}
	return *this;
}

Request::~Request() {
}

// Functs
void Request::assignLeftover(std::vector<char> &vec) {
	delete[] _leftover;
	_leftover = new char[vec.size() + 1];
	for (size_t i = 0; i < vec.size(); ++i) {
		_leftover[i] = vec[i];
	}
	_leftover[vec.size()] = '\0';
}

void Request::assignLeftover(std::string &str) {
	delete[] _leftover;
	_leftover = new char[str.size() + 1];
	for (size_t i = 0; i < str.size(); ++i) {
		_leftover[i] = str[i];
	}
	_leftover[str.size()] = '\0';
}

static void tolowerStr(std::string &str) {
	for (size_t i = 0; i < str.size(); ++i) {
		unsigned char c = static_cast<char>(str[i]);
		str[i] = static_cast<char>(std::tolower(c));
	}
}

static void safeGetLine(std::string &stream, std::string &var, char delimiter, bool &done) {
	size_t pos = stream.find(delimiter);
	if (pos != std::string::npos) {
		var = stream.substr(0, pos);
		stream.erase(0, pos + 1);
		done = true;
		return;
	}
	var += stream;
	stream.clear();
	done = false;
}

static bool Request::safeEnd() {
	if (_stream.size() == 0) {
		_incompleteEndLine = true;
		return false;
	}
	if (_stream[0] != '\n')
		// 400 Bad Request.
	_incompleteEndLine = false;
	_stream.erase(0, 1);
	return true;
}

bool Request::parseMethod() {
	if (!this->_methodParsed) {
		safeGetLine(_stream, _method, ' ', this->_methodParsed);
		if (!this->_methodParsed)
			return false;
		if (_method != "GET" && methodStr != "POST" && methodStr != "DELETE") // To be redone by server restrictions.
			// 400 Bad Request.
	}

	if (!this->_uriParsed) {
		safeGetLine(_stream, _uriStr, ' ', this->_uriParsed);
		if (!this._uriParsed)
			return false;
		parseUri(this->_uri, _uriStr);
		this->_uriStr.clear();
	}

	if (!this->_httpVerParsed) {
		safeGetLine(_stream, _httpVer, '\r', this->_httpVerParsed);
		if (!this->_httpVerParsed)
			return false;
		if (_httpVer != "HTTP/1.1")
			// 400 Bad Request.
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
				// 400 Bad Request;
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

bool Request::findValue(std::string &_tmpValue) {
	if (!this->_parsedValue) {
		safeGetLine(_stream, _tmpValue, '\r', this->_parsedValue);
		if (!this->_parsedValue)
			return false;
		if (!safeEnd())
			return false;
	}
	handleValueSpaces(_tmpValue);
	return true;
}

void Request::addHeader() {
	if (this->_parsedKey && this->_parsedValue) {
		if (this->_headers.count(_tmpKey) {
			if (_tmpKey == "host" || _tmpKey == "content-length") {
				_tmpKey.clear();
				_tmpVal.clear();
				// 400 Bad Request;
			}
			this->_headers[_tmpKey] += ", " + _tmpValue;
		} else
			this->_headers[_tmpKey] = _tmpValue;
	}
	_tmpKey.clear();
	_tmpVal.clear();
}

bool Request::parseHeaders() {
	if (!this->findKey())
		return false;

	if (!this->findValue())
		return false;

	this->addHeader();

	size_t pos = _stream.find("\r\n"); 
	if (pos == std::string::npos || pos != 0) {
		this->_parsedKey = false;
		this->_parsedValue = false;
		assignLeftover(_stream);
		_stream.clear();
		return false;
	}
	_stream.erase(0, 2);
	return true;


	// El header Host hi ha de ser sempre. -> Func is there a header.
	// Connection: Puede ser keep-alive (mantener el socket abierto para reutilizarlo en futuras peticiones) o close (cerrar el socket en cuanto envíes la respuesta).
}

bool Request::hasHeader(std::string &str) {
	bool hasHeader = _headers.count(str);
	if (hasHeader)
		return 1;
	return 0;
}

bool Request::parseRequestHead() {
	// addLeftover();
	if (!parseMethod())
		return false;
	while (true) {
		if (_stream.size() >= 2 && _stream[0] == '\r' && _stream[1] == '\n') {
			_stream.erase(0, 2);
			return true;
		}
		if (!parseHeaders())
			return false;
		if (!hasHeader("Host"))
			// Error
	}
	return true;
}

// Body Functs
bool isHexDigit(char c) {
	return std::isdigit(static_cast<unsigned char>(c)) ||
		   (c >= 'a' && c <= 'f') ||
		   (c >= 'A' && c <= 'F');
}

static size_t strToSize_t(std::string &str, const int base) {
	if (base == 10) {
		for (size_t i = 0; i < str.size(); ++i) {
			if (!std::isdigit(static_cast<unsigned char>(str[i])))
				// Error -> non digit present in expected decimal num.
		}
	} else if (base == 16) {
		for (size_t i = 0; i < str.size(); ++i) {
			if (!isHexDigit(str[i])) {
				// Error -> non digit present in expected hex num.
			}
		}
	} else 
		//  unsuported base.
	return static_cast<size_t>(std::strtoul(str.c_str(), NULL, base));
}

void Request::setBodyType() {
	if (_bodyType != EMPTY)
		return;

	bool hasContentLength = _headers.count("content-length") > 0;
	bool hasTransferEncoding = _headers.count("transfer-encoding") > 0;

	if (hasContentLength && hasTransferEncoding)
		// Error -> ambas cabeceras presentes.
	else if (hasContentLength) {
			_bodyType = FULL;
			_maxBodySize = strToSize_t(_headers.find("content-length")->second.c_str(), 10);
	} else if (hasTransferEncoding) 
		_bodyType = CHUNKED;
}

bool Request::fullBody() {
	size_t total = _body.size() + _streamBody.size();

	if (total < _maxBodySize) {
		_body += _streamBody;
		_streamBody.clear();
		return false;
	} else if (total == _maxBodySize) {
		_body += _streamBody;
		_maxBodySize = 0;
		_streamBody.clear();
		return true;
	} else {
		size_t remaining = _maxBodySize - _body.size();
		_leftover = _streamBody.substr(remaining);
		_body += _streamBody.substr(0, remaining);
		_streamBody.clear();		
		_maxBodySize = 0;
		return true;
	}
}

bool Request::chunkedBody() {
	
	const std::string delimiter = "\r\n";

	while (true) {
		if (!_leftoverBody.empty()) {
		_streamBody = _leftoverBody + _streamBody;
		_leftoverBody.clear();
		}

		size_t pos = _streamBody.find(delimiter);
		if (pos == std::string::npos) {
			_leftoverBody = _streamBody;
			_streamBody.clear();
			return false;
		}

		if (!_chunkSize) {
			std::string sizeStr = _streamBody.substr(0, pos);

			if (sizeStr == "0") {
				if (_streamBody.size() < pos + 4) {
					_leftoverBody = _streamBody;
					_streamBody.clear();
					return false;
				}
				if (_streamBody.substr(pos, 4) != "\r\n\r\n")
					// Error -> invalid final chunk.
				_leftover = _streamBody.substr(pos + 4);
				_streamBody.clear();
				return true;
			} 
			_maxBodySize = strToSize_t(sizeStr, 16);
			_streamBody.erase(0, pos + 2);
			_chunkSize = true;
		} else {
			if (_streamBody.size() < _maxBodySize + 2) {
				_leftoverBody = _streamBody;
				_streamBody.clear();
				return false;
			}

			_body += _streamBody.substr(0, _maxBodySize);

			if (_streamBody.substr(_maxBodySize, 2) != "\r\n")
				// Error -> invalid chunk ending.

			_leftoverBody = _streamBody.substr(_maxBodySize + 2);
			_streamBody.clear();
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
t_method	Request::getMethod {
	return this->_method;
}

std::map<std::string, std::string>  Request::getHeaders {
	return this->_headers;
}

std::vector<char>	Request::getBody {
	return this->body;
}

pal tester:
- Headers en mayuscula
- Espacio no eliminados en headers value despues de los : y al final del value
- Espacio dentro del contenido del value de los headers.

////////////////
// Corregeix Request -> Revisa si cal refactorizar Body, en principi no