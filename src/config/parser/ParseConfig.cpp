#include "ParseConfig.hpp"

ParseConfig::ParseConfig(const std::vector<Token> &tokens)
	: _treeHead(NULL), _tokens(tokens), _pos(0) {}

ParseConfig::~ParseConfig()
{
	delete _treeHead;
}

ParseConfig::ParseConfig(const ParseConfig &other) { (void)other; }

ParseConfig &ParseConfig::operator=(const ParseConfig &other) { (void)other; return (*this); }


const Token &ParseConfig::current() const
{
	return _tokens[_pos];
}

bool ParseConfig::accept(TokenType type)
{
	if (current().type == type)
	{
		if (_pos < _tokens.size() - 1)
			_pos++;
		return true;
	}
	return false;
}

const Token &ParseConfig::expect(TokenType type, const std::string& msg)
{
	const Token &tok = current();
	if (!accept(type))
		unexpected(tok, msg);
	return tok;
}

void ParseConfig::unexpected(const Token &token, const std::string& msg) const
{
	throw ConfigException(msg + " (got " + tokenTypeToString(token.type) + ")", token.line);
}

Node *ParseConfig::parseStatement(NodeContext ctxt)
{
	Token next = expect(TOKEN_WORD, "expected directive or block name");

	std::string name = next.value;
	int line = next.line;

	std::vector<std::string> args;
	while (current().type == TOKEN_WORD)
	{
		args.push_back(current().value);
		accept(TOKEN_WORD);
	}

	if (current().type == TOKEN_LBRACE)
		return parseBlock(name, args, line, ctxt);
	else if (current().type == TOKEN_SEMI)
		return parseDirective(name, args, line, ctxt);
	else
		unexpected(current(), "statement not followed by { or ;");
	return NULL;
}

Node *ParseConfig::parseBlock(const std::string &name, const std::vector<std::string> &args, int line, NodeContext ctxt)
{
	expect(TOKEN_LBRACE, "expected '{'");
	Node *newNode = new Node(NODE_BLOCK, name, ctxt, line);
	newNode->args = args;

	NodeContext childCtxt = (name == "server") ? SERVER_CTXT
							: (name == "location") ? LOCATION_CTXT 
							: ctxt;

	while(current().type != TOKEN_RBRACE)
	{
		if (current().type == TOKEN_EOF)
			unexpected(current(), "unclosed brace");
		newNode->children.push_back(parseStatement(childCtxt));
	}
	expect(TOKEN_RBRACE, "expected '}'");
	return newNode;
}

Node *ParseConfig::parseDirective(const std::string &name, const std::vector<std::string> &args, int line, NodeContext ctxt)
{
	expect(TOKEN_SEMI, "expected ';' after directive");
	Node *newNode = new Node(NODE_DIR, name, ctxt, line);
	newNode->args = args;
	return newNode;
}

Node *ParseConfig::parse()
{
	_treeHead = new Node(NODE_BLOCK, "main", MAIN_CTXT, 0);
	while (current().type != TOKEN_EOF)
		_treeHead->children.push_back(parseStatement(MAIN_CTXT));

	Node *result = _treeHead;
	_treeHead = NULL;
	return result;
}