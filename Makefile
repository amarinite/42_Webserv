NAME = webserv
SRC_DIR = src
SRC = main.cpp

TOTAL_SRCS = $(words $(SRC))

OBJ_DIR = obj
OBJ = $(SRC:%.cpp=$(OBJ_DIR)/%.o)
DEP = $(SRC:%.cpp=$(OBJ_DIR)/%.d)

RESET       = \033[0m
PINK        = \033[1;35m
BLUE        = \033[1;36m
GREEN       = \033[1;32m

CXX = c++
INC_DIR = ./includes \
			./includes/config \
			./includes/config/parser \
			./includes/network \
			./includes/utils

ALL_INC = $(addprefix -I, $(INC_DIR))
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 $(ALL_INC)

TEST_DIR        = tests
TEST_OBJ_DIR    = obj/tests
TEST_NAME       = test_runner
TEST_SRC        = $(TEST_DIR)/test.cpp \
                  $(TEST_DIR)/LexerTests.cpp \
				  $(TEST_DIR)/SocketTests.cpp
TEST_DEPS       = src/config/parser/Lexer.cpp \
				  src/network/SocketManager.cpp \
				  src/network/Sockets.cpp
TEST_OBJ        = $(TEST_SRC:$(TEST_DIR)/%.cpp=$(TEST_OBJ_DIR)/%.o) \
                  $(TEST_DEPS:src/%.cpp=$(TEST_OBJ_DIR)/deps/%.o)

all: $(NAME)

-include $(DEP)

$(NAME): $(OBJ)
	@echo "$(BLUE)linking objects...$(RESET)"
	@$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)
	@echo "$(GREEN)$(NAME) has been served $(RESET)🍲"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp  Makefile | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@
	@curr=$$(find $(OBJ_DIR) -type f -name "*.o" | wc -l); \
	percent=$$(( curr * 100 / $(TOTAL_SRCS) )); \
	bar_len=$$(( percent / 2 )); \
	bar_str=""; i=0; \
	while [ $$i -lt $$bar_len ]; do bar_str="$${bar_str}▓"; i=$$((i+1)); done; \
	spaces=""; i=0; rest=$$((50 - bar_len)); \
	while [ $$i -lt $$rest ]; do spaces="$${spaces}░"; i=$$((i+1)); done; \
	printf "\r$(BLUE)compiling: $(PINK)[$$bar_str$$spaces] $(PINK)$$percent%% $(RESET)"; \
	if [ $$curr -eq $(TOTAL_SRCS) ]; then printf "\n"; fi

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(TEST_OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp | $(TEST_OBJ_DIR)
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(TEST_OBJ_DIR)/deps/%.o: src/%.cpp | $(TEST_OBJ_DIR)
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(TEST_OBJ_DIR):
	@mkdir -p $(TEST_OBJ_DIR)

$(TEST_NAME): $(TEST_OBJ)
	@$(CXX) $(CXXFLAGS) $(TEST_OBJ) -o $(TEST_NAME)

test: $(TEST_NAME)
	@echo "$(BLUE)running tests...$(RESET)"
	@./$(TEST_NAME) && echo "$(GREEN)all tests passed$(RESET)" || echo "$(PINK)some tests failed$(RESET)"


clean:
	@rm -rf $(OBJ_DIR)
	@echo "$(BLUE)objects cleaned $(RESET)🧹"

fclean: clean
	@rm -f $(NAME) $(TEST_NAME)
	@echo "$(PINK)$(NAME) has been black holed $(RESET)🕳"

re: fclean all

.PHONY: all clean fclean re test
