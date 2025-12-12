NAME	:= webserv
CC		:= c++ -std=c++17
CXXFLAGS:= -Wall -Wextra -Werror
CXXINC	:= -I./include
CXXEXTRA:=
D_FLAGS	:= -MMD -MP
SRC_DIR	:= src/
BIN		:= bin/
BIN_DIRS:= $(BIN) $(BIN)exception/ $(BIN)config/
MAIN	:= $(BIN)main.o
SRCS	:= EpollManager.cpp Logger.cpp SharedFD.cpp helper.cpp exception/FileDescriptorException.cpp signal.cpp config/Lexer.cpp config/Parser.cpp config/load_configs.cpp
OBJS	:= $(SRCS:%.cpp=$(BIN)%.o)
DEPS	:= $(BIN)main.d $(SRCS:%.cpp=$(BIN)%.d)

# UNIT TEST VARIABLES
TEST_DIR	:= tests/
TEST_NAME	:= $(BIN)$(TEST_DIR)run_tests
TEST_SRCS	:= catch_amalgamated.cpp test_sharedfd.cpp test_epollmanager.cpp test_logger.cpp test_lexer.cpp test_parser.cpp
TEST_OBJS	:= $(TEST_SRCS:%.cpp=$(BIN)$(TEST_DIR)%.o)
TEST_DIRS	:= $(BIN)$(TEST_DIR)

all: $(NAME)

debug: CXXEXTRA += -g3 -DDEBUG
debug: $(NAME)

sanitize: CXXEXTRA += -fsanitize=address,undefined,leak
sanitize: debug

$(NAME): $(MAIN) $(OBJS)
	$(CC) $(CXXEXTRA) $^ -o $@

$(TEST_NAME): $(OBJS) $(TEST_OBJS)
	$(CC) $(CXXEXTRA) $^ -o $@

-include $(DEPS)

%/:
	mkdir -p $@

$(BIN)%.o: $(SRC_DIR)%.cpp | $(BIN_DIRS)
	$(CC) $(CXXFLAGS) $(CXXEXTRA) $(CXXINC) $(D_FLAGS) -o $@ -c $<

$(BIN)$(TEST_DIR)%.o: $(TEST_DIR)%.cpp | $(TEST_DIRS)
	$(CC) $(CXXFLAGS) $(CXXEXTRA) $(CXXINC) $(D_FLAGS) -o $@ -c $<

test: CXXEXTRA	+= -DUNIT_TEST
test: CXXINC	+= -I./tests
test: $(TEST_NAME)
	./$<

clean:
	$(RM) $(MAIN) $(OBJS) $(DEPS) $(TEST_OBJS)

fclean: clean
	$(RM) $(NAME) $(TEST_NAME)
	$(RM) -r $(BIN)

re: fclean all

.PHONY: all clean fclean re test debug sanitize
.SECONDARY: $(BIN_DIRS) $(TEST_DIRS)
