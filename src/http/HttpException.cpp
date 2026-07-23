#include "HttpException.hpp"
#include "FileUtils.hpp"

static std::string errorPageBody(const int errorCode, std::map<int, std::string> &error_pages) {
	std::map<int, std::string>::iterator it = error_pages.begin();
	for (; it != error_pages.end(); ++it) {
		if (*it == errorCode)
			return readFile(it->second);
	}
}

static std::string intToStr(int &num) {
	char[4] buff;
	sprintf(buff, "%d", num);
	std::string str(buff);
	return str;
}
static std::string setConnection(const int code) {
	if (code == 400 || code == 413 || code > 499 || !exceptConnection)
		return "close";
	else 
		return "keep-alive";
}

void	HttpException::prepareErrorResponse(Response &res, std::map<int, std::string> &error_pages) {
	Response res();
	std::string strStatusCode = intToString(getStatusCode());

	res.setStatusCode(strStatusCode);
	res.setMessage(responseStatusMessage(strStatusCode));
	std::string body = errorPageBody(getStatusCode(), error_pages);
	res.setResponseBody(body);
	res.assignHeaders("text/html", setConnection(getStatusCode()));
}

//////////////////////responseStatusMessage()

catch {
	Response res();
	prepareErrorResponse(res, serverConfig.getErrorPages());
	res.buildRawResponse();
}