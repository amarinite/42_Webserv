#pragma once
#include <string>
#include <set>
#include <vector>
#include <map>
#include <algorithm>
#include "ParseConfig.hpp"
#include "ParseUtils.hpp"
#include "UriParser.hpp"
#include "ConfigException.hpp"

typedef void (*DirValidatorFn)(const Node*);

struct DirectiveRule
{
	int				minArgs;
	int				maxArgs;
	bool			repeatable;
	DirValidatorFn	validator;
};

class ConfigValidator
{
	private:
		static const std::set<std::string>& getAllowedBlocks();
		static const std::map<std::string, DirectiveRule>& getServerRules();
		static const std::map<std::string, DirectiveRule>& getLocationRules();

		// FOR EACH NODE:
		static void isNameAllowed(const Node* node);
		static void isContextCorrect(const Node* node);

		// FOR BLOCKS
		static void isBlockEmpty(const std::vector<Node*>& children, int line);
		static void hasNoDuplicateDirectives(const Node* node);

		// FOR SERVER
		static void isListenSet(const std::vector<Node*>& children, int line);
		static void isRootSet(const std::vector<Node*>& children, int line);
		static void hasNoArgs(const Node* node);

		// FOR LOCATION
		static void hasOneRoutePath(const Node* node);

		// FOR DIRECTIVES
		static void validateDirective(const Node* node); // dispatch to specific validators

		static void hasDuplicateListenAddr(const Node* node, std::set<ListenAddr>& seenAddrs);
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

		// VALIDATION ENFORCER
		static void validateNode(const Node* node, std::set<ListenAddr>& seenAddrs);

	public:
		static void validate(const Node* root);
};