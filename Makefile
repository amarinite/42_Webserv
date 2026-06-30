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
			./includes/utils

ALL_INC = $(addprefix -I, $(INC_DIR))
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 $(ALL_INC)

all: $(NAME)

-include $(DEP)

$(NAME): $(OBJ)
	@echo "$(BLUE)linking objects...$(RESET)"
	@$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)
	@echo "$(GREEN)$(NAME) has been cooked $(RESET)🍲"

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

clean:
	@rm -rf $(OBJ_DIR)
	@echo "$(BLUE)objects cleaned $(RESET)🧹"

fclean: clean
	@rm -f $(NAME)
	@echo "$(PINK)$(NAME) has been black holed $(RESET)🕳"

re: fclean all

.PHONY: all clean fclean re