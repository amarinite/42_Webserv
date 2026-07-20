#include "ConfigValidator.hpp"

const std::set<std::string>& ConfigValidator::getAllowedBlocks()
{
	static std::set<std::string> s;
	if (s.empty()) {
		s.insert("server");
		s.insert("location");
	}
	return s;
}

const std::map<std::string, DirectiveRule>& ConfigValidator::getServerRules()
{
	static std::map<std::string, DirectiveRule> rules;
	if (rules.empty()) {
		DirectiveRule listen_rule = {1, -1, true, &ConfigValidator::validateListen};
		DirectiveRule error_page_rule = {2, 2, true, &ConfigValidator::validateErrorPage};
		DirectiveRule max_body_rule = {1, 1, false, &ConfigValidator::validateClientMaxBodySize};
		DirectiveRule root_rule = {1, 1, false, &ConfigValidator::validateRoot};
		DirectiveRule index_rule = {1, -1, false, &ConfigValidator::validateIndex};

		rules["listen"] = listen_rule;
		rules["error_page"] = error_page_rule;
		rules["client_max_body_size"] = max_body_rule;
		rules["root"] = root_rule;
		rules["index"] = index_rule;
	}
	return rules;
}

const std::map<std::string, DirectiveRule>& ConfigValidator::getLocationRules()
{
	static std::map<std::string, DirectiveRule> rules;
	if (rules.empty()) {
		DirectiveRule root_rule = {1, 1, false, &ConfigValidator::validateRoot};
		DirectiveRule index_rule = {1, -1, false, &ConfigValidator::validateIndex};
		DirectiveRule methods_rule = {1, 3, false, &ConfigValidator::validateAllowedMethods};
		DirectiveRule redirect_rule = {1, 1, false, &ConfigValidator::validateRedirect};
		DirectiveRule autoindex_rule = {1, 1, false, &ConfigValidator::validateAutoindex};
		DirectiveRule upload_store_rule = {1, 1, false, &ConfigValidator::validateUploadStore};
		DirectiveRule cgi_extension_rule = {2, -1, false, &ConfigValidator::validateCgiExtension};

		rules["root"] = root_rule;
		rules["index"] = index_rule;
		rules["allowed_methods"] = methods_rule;
		rules["redirect"] = redirect_rule;
		rules["autoindex"] = autoindex_rule;
		rules["upload_store"] = upload_store_rule;
		rules["cgi_extension"] = cgi_extension_rule;
	}
	return rules;
}

struct NodeNameIs
{
	std::string target_name;
	
	NodeNameIs(const std::string& name) : target_name(name) {}
	
	bool operator()(const Node* node) const
	{
		return node && node->name == target_name;
	}
};

struct CompareNodeByName
{
	bool operator()(const Node* a, const Node* b) const
	{
		if (!a || !b) return false;
		return a->name < b->name;
	}
};


void ConfigValidator::isNameAllowed(const Node* node)
{
	bool allowed = false; 
	if (node->type == NODE_BLOCK)
	{
		const std::set<std::string>& blocks = getAllowedBlocks();
		if (blocks.find(node->name) != blocks.end())
			allowed = true;
	}
	else
	{
		const std::map<std::string, DirectiveRule>& dirs = 
			(node->context == SERVER_CTXT) ? getServerRules() : getLocationRules(); 

		if (dirs.find(node->name) != dirs.end())
			allowed = true;
	}

	if (!allowed)
		throw ConfigException("unknown block or directive", node->line);

}

void ConfigValidator::isContextCorrect(const Node* node)
{
	if (node->context == MAIN_CTXT)
	{
		if (node->name == "location" || node->type == NODE_DIR)
			throw ConfigException("location or directive outside server", node->line);
	}
	else if (node->context == SERVER_CTXT)
	{
		if (node->name == "server")
			throw ConfigException("server inside server", node->line);
	}
	else if (node->context == LOCATION_CTXT)
	{
		if (node->name == "server")
			throw ConfigException("server inside location", node->line);
	}
}

void ConfigValidator::hasDuplicateListenAddr(const Node* node, std::set<ListenAddr>& seenAddrs)
{
	for (size_t i = 0; i < node->args.size(); i++)
	{
		ListenAddr addr = parseListenArg(node->args[i]);
		if (seenAddrs.insert(addr).second == false)
			throw ConfigException("duplicate listen address", node->line);
	}
}

void ConfigValidator::isBlockEmpty(const std::vector<Node*>& children, int line)
{
	if (children.size() == 0)
		throw ConfigException("config block is empty", line);
}

void ConfigValidator::isListenSet(const std::vector<Node*>& children, int line)
{
	std::vector<Node*>::const_iterator it = 
		std::find_if(children.begin(), children.end(), NodeNameIs("listen"));
	
	if (it == children.end())
		throw ConfigException("missing listen directive inside server block", line);
}

void ConfigValidator::isRootSet(const std::vector<Node*>& children, int line)
{
	std::vector<Node*>::const_iterator it = 
		std::find_if(children.begin(), children.end(), NodeNameIs("root"));
	
	if (it == children.end())
		throw ConfigException("missing root directive inside server block", line);
}

void ConfigValidator::hasNoArgs(const Node* node)
{
	if (node->name == "server" && !node->args.empty())
		throw ConfigException("unexpected args", node->line);
}

void ConfigValidator::hasOneRoutePath(const Node* node)
{
	if (node->args.size() != 1)
		throw ConfigException("unexpected args", node->line);
	
	t_uri uri;

	try {
		parseUri(uri, node->args[0]);
	}
	catch (const std::exception& err) {
		throw ConfigException("invalid path: " + std::string(err.what()), node->line);
	}
}

void ConfigValidator::hasNoDuplicateDirectives(const Node* node)
{
	const std::map<std::string, DirectiveRule>& rules =
		(node->context == SERVER_CTXT) ? getServerRules() : getLocationRules();

	std::vector<Node*> children = node->children;
	std::sort(children.begin(), children.end(), CompareNodeByName());

	for (size_t i = 1; i < children.size(); i++)
	{
		if (children[i]->name == children[i - 1]->name)
		{
			std::map<std::string, DirectiveRule>::const_iterator it = rules.find(children[i]->name);
			if (it != rules.end() && !it->second.repeatable)
				throw ConfigException("invalid duplicate directive: " + children[i]->name, children[i]->line);
		}
	}
}

void ConfigValidator::validateDirective(const Node* node)
{
	const std::map<std::string, DirectiveRule>& rules =
		(node->context == SERVER_CTXT) ? getServerRules() : getLocationRules();

	std::map<std::string, DirectiveRule>::const_iterator it = rules.find(node->name);
	if (it != rules.end())
	{
		const DirectiveRule& rule = it->second;
		size_t argCount = node->args.size();

	if (rule.minArgs != -1 && argCount < static_cast<size_t>(rule.minArgs))
		throw ConfigException("too few arguments for directive " + node->name, node->line);

	if (rule.maxArgs != -1 && argCount > static_cast<size_t>(rule.maxArgs))
		throw ConfigException("too many arguments for directive " + node->name, node->line);
	
	if (rule.validator)
		(rule.validator)(node);
	}
}

void ConfigValidator::validateListen(const Node* node)
{
	for (size_t i = 0; i < node->args.size(); i++)
	{
		try {
			parseListenArg(node->args[i]);
		}
		catch (const std::exception& err) {
			throw ConfigException("invalid listen directive: " + std::string(err.what()), node->line);
		}
	}
}

void ConfigValidator::validateErrorPage(const Node* node)
{
	try {
		int code = parseNumber(node->args[0]);
		isHttpCodeValid(code);
	}
	catch (const std::invalid_argument& err) {
		throw ConfigException("invalid error code: " + std::string(err.what()), node->line);
	}
	if (node->args[1][0] != '/')
		throw ConfigException("invalid error page", node->line);
}

void ConfigValidator::validateClientMaxBodySize(const Node* node)
{
	try {
		parseNumber(node->args[0]);
	}
	catch (const std::exception& err) {
		throw ConfigException("invalid max body size directive", node->line);
	}
}

void ConfigValidator::validateRoot(const Node* node)
{
	try {
		parseFsPath(node->args[0]);
	}
	catch (const std::invalid_argument& err) {
		throw ConfigException("invalid root directive: " + std::string(err.what()), node->line);
	}
}

void ConfigValidator::validateIndex(const Node* node)
{
	std::set<std::string> seen;
	for (size_t i = 0; i < node->args.size(); i++)
	{
		if (!seen.insert(node->args[i]).second)
			throw ConfigException("duplicate index directive", node->line);
	}
}

void ConfigValidator::validateAllowedMethods(const Node* node)
{
	std::set<std::string> seen;

	for (size_t i = 0; i < node->args.size(); i++)
	{
		if (node->args[i] != "GET" && node->args[i] != "POST" && node->args[i] != "DELETE")
			throw ConfigException("invalid methods directive", node->line);

		if (!seen.insert(node->args[i]).second)
			throw ConfigException("duplicate methods directive", node->line);
	}
}

void ConfigValidator::validateRedirect(const Node* node)
{
	t_uri uri;

	try {
		parseUri(uri, node->args[0]);
	}
	catch (const std::exception& err) {
		throw ConfigException("invalid redirect directive", node->line);
	}
}

void ConfigValidator::validateAutoindex(const Node* node)
{
	if (node->args[0] != "on" && node->args[0] != "off")
		throw ConfigException("invalid autoindex directive", node->line);
}

void ConfigValidator::validateUploadStore(const Node* node)
{
	try {
		parseFsPath(node->args[0]);
	}
	catch (const std::invalid_argument& err) {
		throw ConfigException("invalid upload_store directive: " + std::string(err.what()), node->line);
	}
}

void ConfigValidator::validateCgiExtension(const Node* node)
{
	if (node->args[0][0] != '.')
		throw ConfigException("invalid cgi extension", node->line);
	try {
		parseFsPath(node->args[1]);
	}
	catch (const std::invalid_argument& err) {
		throw ConfigException("invalid cgi handler path: " + std::string(err.what()), node->line);
	}
}

void ConfigValidator::validateNode(const Node* node, std::set<ListenAddr>& seenAddrs)
{
	if (!node)
		return;

	isContextCorrect(node);
	isNameAllowed(node);

	if (node->type == NODE_BLOCK)
	{
		isBlockEmpty(node->children, node->line);
		hasNoDuplicateDirectives(node);

		if (node->name == "server")
		{
			isListenSet(node->children, node->line);
			isRootSet(node->children, node->line);
			hasNoArgs(node);
		}
		else if (node->name ==  "location")
			hasOneRoutePath(node);
	}
	else if (node->type == NODE_DIR)
	{
		validateDirective(node);
		if (node->name == "listen")
			hasDuplicateListenAddr(node, seenAddrs);
	}

	for (size_t i = 0; i < node->children.size(); i++)
		validateNode(node->children[i], seenAddrs);
}

void ConfigValidator::validate(const Node* root)
{
	std::set<ListenAddr> seenAddrs;

	for (size_t i = 0; i < root->children.size(); i++)
	{
		validateNode(root->children[i], seenAddrs);
	}
}