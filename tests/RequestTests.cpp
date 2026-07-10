#include "TestUtils.hpp"
#include "HttpRequest.hpp"
#include <iostream>

static bool testSimpleGetRequest()
{
	Request req;

	req.feedStream("GET /index.html HTTP/1.1\r\nHost: localhost\r\nUser-Agent: Mozilla\r\n\r\n");

	ASSERT(req.parseRequestHead() == true);
	
	ASSERT(req.getMethod() == "GET"); 
	
	std::map<std::string, std::string> headers = req.getHeaders();
	ASSERT(headers.count("host") == 1);
	ASSERT(headers["host"] == "localhost");
	ASSERT(headers.count("user-agent") == 1);
	ASSERT(headers["user-agent"] == "Mozilla");
	
	return true;
}

static bool testMissingHostHeaderThrows()
{
	Request req;
	req.feedStream("GET /index.html HTTP/1.1\r\nUser-Agent: Mozilla\r\n\r\n");

	try {
		req.parseRequestHead();
		ASSERT(false);
	}
	catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	catch (...) {
		ASSERT(false);
	}
	return true;
}

static bool testHeaderValueTrimmingWithInternalSpaces()
{
	Request req;
	req.feedStream("POST /api HTTP/1.1\r\nHost:   localhost:8080   \r\nContent-Type:  text/html; charset=utf-8  \r\n\r\n");

	ASSERT(req.parseRequestHead() == true);
	
	std::map<std::string, std::string> headers = req.getHeaders();
	ASSERT(headers["host"] == "localhost:8080");
	ASSERT(headers["content-type"] == "text/html; charset=utf-8");

	return true;
}

static bool testHeaderKeysAreCaseInsensitive()
{
	Request req;
	req.feedStream("GET / HTTP/1.1\r\nHOsT: example.com\r\nCONTENT-leNGTH: 0\r\n\r\n");

	ASSERT(req.parseRequestHead() == true);
	
	std::map<std::string, std::string> headers = req.getHeaders();
	ASSERT(headers.count("host") == 1);
	ASSERT(headers["host"] == "example.com");
	ASSERT(headers.count("content-length") == 1);
	ASSERT(headers["content-length"] == "0");

	return true;
}

static bool testDuplicateCriticalHeadersThrows()
{
	Request req;
	req.feedStream("GET / HTTP/1.1\r\nHost: localhost\r\nHost: duplicate.com\r\n\r\n");

	try {
		req.parseRequestHead();
		ASSERT(false);
	}
	catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}

	Request req2;
	req2.feedStream("POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\nContent-Length: 10\r\n\r\n");
	try {
		req2.parseRequestHead();
		ASSERT(false);
	}
	catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testFullBodyParsing()
{
	Request req;
	req.feedStream("POST /submit HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\n\r\n");
	
	ASSERT(req.parseRequestHead() == true);
	
	// Alimentamos el buffer de body
	req.feedBody("Hello");
	
	ASSERT(req.parseRequestBody() == true);
	
	std::string bodyVec = req.getBody();
	std::string bodyStr(bodyVec.begin(), bodyVec.end());
	ASSERT(bodyStr == "Hello");

	return true;
}

static bool testChunkedBodyParsing()
{
	Request req;
	req.feedStream("POST /stream HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n");
	
	ASSERT(req.parseRequestHead() == true);

	req.feedBody("5\r\nHello\r\n");
	ASSERT(req.parseRequestBody() == false);


	req.feedBody("0\r\n\r\n");
	ASSERT(req.parseRequestBody() == true);

	std::string bodyVec = req.getBody();
	std::string bodyStr(bodyVec.begin(), bodyVec.end());
	ASSERT(bodyStr == "Hello");

	return true;
}

void runHttpRequestTests(int& passed, int& failed)
{
	Test tests[] = {
		{ "Simple valid GET request head",           testSimpleGetRequest },
		{ "Missing Host header throws 400",          testMissingHostHeaderThrows },
		{ "Header value trimming & internal spaces", testHeaderValueTrimmingWithInternalSpaces },
		{ "Header keys are case insensitive",        testHeaderKeysAreCaseInsensitive },
		{ "Duplicate critical headers throw 400",    testDuplicateCriticalHeadersThrows },
		{ "Full body parsing via Content-Length",    testFullBodyParsing },
		{ "Chunked body parsing",                    testChunkedBodyParsing }
	};

	int localPassed = 0;
	int localFailed = 0;

	for (int i = 0; i < (int)ARRAY_SIZE(tests); i++)
	{
		if (tests[i].fn()) {
			localPassed++;
		} else {
			std::cout << RED << "  [FAIL] " << tests[i].name << "\n" << RESET;
			localFailed++;
		}
	}

	printTestSummary("HttpRequest", localPassed, localFailed);

	passed += localPassed;
	failed += localFailed;
}