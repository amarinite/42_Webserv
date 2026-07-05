#pragma once
#include <string>
#include <utility>
#include <vector>
#include <iostream>
#include "Token.hpp"
#include "ConfigException.hpp"

enum NodeType { NODE_DIR, NODE_BLOCK };

enum NodeContext { MAIN_CTXT, SERVER_CTXT, LOCATION_CTXT };

struct Node
{
	NodeType					type;
	std::string					name;
	std::vector<std::string>	args;
	std::vector<Node*>			children;
	NodeContext					context;
	int							line;

	Node(NodeType type, const std::string& name, NodeContext context, int line)
		: type(type), name(name), context(context), line(line) {}
	~Node()
	{
		for (std::size_t i = 0; i < children.size(); ++i)
			delete children[i];
	}
};

class ParseConfig
{
	private:
		Node*				_treeHead;
		std::vector<Token>	_tokens;
		std::size_t			_pos;

		// disable copying
		ParseConfig(const ParseConfig&);
		ParseConfig &operator=(const ParseConfig&);
		
		const Token		&current() const;
		bool			accept(TokenType token);
		const Token&	expect(TokenType type, const std::string& msg);
		void 			unexpected(const Token &token, const std::string &msg) const; // throws

		Node			*parseStatement(NodeContext ctxt);
		Node			*parseBlock(const std::string &name, const std::vector<std::string> &args, int line, NodeContext ctxt);
		Node			*parseDirective(const std::string &name, const std::vector<std::string> &args, int line, NodeContext ctxt);
		
	public:
		ParseConfig(const std::vector<Token> &tokens);
		~ParseConfig();

		Node*	parse();
};