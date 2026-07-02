#include "ParseConfig.hpp"

ParseConfig::ParseConfig(const std::vector<Token> &tokens)
	: _tokens(tokens), _pos(0), _treeHead(NULL) {}

ParseConfig::~ParseConfig()
{
	delete _treeHead;
}

Node *ParseConfig::parse()
{
	_treeHead = parseStatement();
	return _treeHead;
}