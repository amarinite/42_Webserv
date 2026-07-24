#include "HttpException.hpp"
#include "FileUtils.hpp"

static std::string errorPageBody(const int errorCode, std::map<int, std::string> &error_pages) {
	std::map<int, std::string>::iterator it = error_pages.begin();
	for (; it != error_pages.end(); ++it) {
		if (*it == errorCode)
			return readFile(it->second);
	}
	return "";
}

static std::string setConnection(const int code) {
	if (code == 400 || code == 413 || code > 499 || !exceptConnection)
		return "close";
	else 
		return "keep-alive";
}

void	HttpException::prepareErrorResponse(Response &res, ServerConfig &conf) {
	std::string strStatusCode = toStr(getStatusCode());

	res.setStatusCode(strStatusCode);
	res.setMessage(responseStatusMessage(strStatusCode));
	res.assignErrorBody(getStatusCode(), conf.getErrorPages());
	res.assignHeaders(".html", setConnection(getStatusCode()));
	if (getStatusCode() == 405)
		res.setAllowedMethodsHeader(_methods);
	res.buildRawResponse();
}

//////////////////////responseStatusMessage()

// catch {
// 	Response res();
// 	prepareErrorResponse(res, serverConfig.getErrorPages());
// 	res.buildRawResponse();
// }