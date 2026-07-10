#pragma once
#include <string>
#include <set>
#include <vector>
#include <map>
#include <algorithm>
#include "ParseConfig.hpp"
#include "UriParser.hpp"
#include "ConfigException.hpp"

struct DirectiveRule
{
	int		minArgs;
	int		maxArgs;
	bool	repeatable;
};

class ConfigValidator
{
	private:
		static const std::set<std::string>& getAllowedBlocks();
		static const std::set<std::string>& getAllowedServerDirs();
		static const std::set<std::string>& getAllowedLocationDirs();

		// FOR EACH NODE:
		static void isNameAllowed(const Node* node);
		static void isContextCorrect(const Node* node);

		// FOR MAIN
		static void hasDuplicateListenAddr(const std::vector<Node*>& children, int line);

		// FOR BLOCKS
		static void isBlockEmpty(const std::vector<Node*>& children, int line);

		// FOR SERVER
		static void isListenSet(const std::vector<Node*>& children, int line);
		static void isRootSet(const std::vector<Node*>& children, int line);
		static void hasNoArgs(const Node* node);

		// FOR LOCATION
		static void hasOneRoutePath(const Node* node);

		// FOR DIRECTIVES
		static void hasNoDuplicateDirectives(std::vector<Node*> children, 
			const std::map<std::string, DirectiveRule>& rules);
		static void validateDirective(const Node* node); // dispatch to specific validators

		static void validateListen(const Node* node);
		static void validateErrorPage(const Node* node);
		static void validateClientMaxBodySize(const Node* node);
		static void validateRoot(const Node* node);
		static void validateIndex(const Node* node);
		static void validateAllowedMethods(const Node* node);
		static void validateRedirect(const Node* node);
		static void validateAutoindex(const Node* node);
		static void validateUploadStore(const Node* node);
		static void validateCgiExtension(const Node* node);

		// UTILS
		static bool isDirectory(const t_uri& uri);

		// VALIDATION ENFORCER
		static void validateNode(const Node* node);

	public:
		static void validate(const Node* root);
};