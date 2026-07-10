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
		DirectiveRule listen_rule = {1, -1, true};
		DirectiveRule error_page_rule = {2, 2, true};
		DirectiveRule max_body_rule = {1, 1, false};
		DirectiveRule root_rule = {1, 1, false};
		DirectiveRule index_rule = {1, 1, false};

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
		DirectiveRule root_rule = {1, 1, false};
		DirectiveRule index_rule = {1, 1, false};
		DirectiveRule methods_rule = {1, 3, false};
		DirectiveRule redirect_rule = {1, 1, false};
		DirectiveRule autoindex_rule = {1, 1, false};
		DirectiveRule upload_store_rule = {1, 1, false};
		DirectiveRule cgi_extension_rule = {2, -1, false};

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
	// FOR EACH NODE:
	// - dispatch to getAllowed funcs based on type/name
	(void)name;
	(void)type;
	(void)line;
	// TODO: implementation
}

void ConfigValidator::isContextCorrect(const Node* node)
{
	// FOR EACH NODE:
	// - no server inside server or location
	// - no location under main
	// - no directive under main
	(void)name;
	(void)type;
	(void)ctxt;
	(void)line;
	// TODO: implementation
}

void ConfigValidator::hasDuplicateListenAddr(const std::vector<Node*>& children, int line);
{
	(void)children;
	(void)line;
	// TODO: implementation
}

void ConfigValidator::isBlockEmpty(const std::vector<Node*>& children, int line)
{
	if (children.size() == 0)
		throw ConfigException("config block is empty", line);
}

void ConfigValidator::isListenSet(const std::vector<Node*>& children, int line)
{
	std::vector<Node*>::const_iterator it = 
		std::find_if(children.begin(), children.end(), NodeNameIs("server"));
	
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
	if (!node->args.empty())
		throw ConfigException("unexpected arguments", node->line);
}

void ConfigValidator::hasOneRoutePath(const Node* node)
{
	(void)node;
	// TODO: implementation
}

void ConfigValidator::hasNoDuplicateDirectives(std::vector<Node*> children, const std::map<std::string, DirectiveRule>& rules)
{
	std::sort(children.begin(), children.end(), CompareNodeByName());

	for (size_t i = 1; i < children.size(); i++)
	{
		if (children[i]->name == children[i - 1]->name)
		{
			std::map<std::string, DirectiveRule>::const_iterator it = rules.find(children[i]->name);
			if (it != rules.end())
			{
				const DirectiveRule& rule = it->second;
				if (!rule.repeatable)
					throw ConfigException("invalid duplicate directive: " + children[i]->name, children[i]->line);
			}
		}
	}
}

void ConfigValidator::validateDirective(const Node* node)
{
	(void)node;
	// TODO: dispatch to specific validators
}

void ConfigValidator::validateListen(const Node* node)
{
	(void)node;
	// TODO: implementation
}

void ConfigValidator::validateErrorPage(const Node* node)
{
	(void)node;
	// TODO: implementation
}

void ConfigValidator::validateClientMaxBodySize(const Node* node)
{
	(void)node;
	// TODO: implementation
}

void ConfigValidator::validateRoot(const Node* node)
{
	(void)node;
	// TODO: implementation
}

void ConfigValidator::validateIndex(const Node* node)
{
	(void)node;
	// TODO: implementation
}

void ConfigValidator::validateAllowedMethods(const Node* node)
{
	(void)node;
	// TODO: implementation
}

void ConfigValidator::validateRedirect(const Node* node)
{
	(void)node;
	// TODO: implementation
}

void ConfigValidator::validateAutoindex(const Node* node)
{
	(void)node;
	// TODO: implementation
}

void ConfigValidator::validateUploadStore(const Node* node)
{
	(void)node;
	// TODO: implementation
}

void ConfigValidator::validateCgiExtension(const Node* node)
{
	(void)node;
	// TODO: implementation
}

bool ConfigValidator::isDirectory(const t_uri& uri, std::string& path)
{
	try
	{
		parseUri(uri, path);
	} catch ()
	return false;
}

void ConfigValidator::validateNode(const Node* node)
{
	(void)node;
	// TODO: implementation
}

void ConfigValidator::validate(const Node* root)
{
	(void)root;
	// TODO: entry-point validation implementation
}