#pragma once
#include <string>
#include <utility>
#include "Token.hpp"

enum NodeType { NODE_DIR, NODE_BLOCK };

enum NodeContext { SERVER_CTXT, LOCATION_CTXT };

struct Node
{
	enum NodeType	type;
	struct Node		*left;
	struct Node		*right;

	Node(NodeType t) : type(t), left(NULL), right(NULL) {}
	virtual ~Node() { delete left; delete right; }
};

struct DirectiveNode : public Node
{
	std::pair<std::string, std::string> directive;

	DirectiveNode() : Node(NODE_DIR) {}
};

struct BlockNode : public Node
{
	NodeContext ctxt;
	Node *reparse;

	BlockNode(NodeContext c) : Node(NODE_BLOCK), ctxt(c), reparse(NULL) {}
	~BlockNode() { delete reparse; }
};

class ParseConfig
{
	private:
		Node*				_treeHead;
		std::vector<Token>	_tokens;
    	std::size_t			_pos;
		
		const Token	&current() const;
		bool		accept(TokenType token);
		bool		expect(TokenType token);
		void		unexpected(const Token &token) const; // throws

		Node	*parseStatement();
		Node	parseBlock(vector<Token> tokens);
		Node	parseDirective(vector<Token> tokens);

		// disable copying
		ParseConfig(const ParseConfig&);
		ParseConfig &operator=(const ParseConfig&);
		
	public:
		ParseConfig(const std::vector<Token> &tokens);
		~ParseConfig();

		Node*	parse();
		// Config*	validate();
};