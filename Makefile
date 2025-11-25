NAME	:= webserv
CC		:= c++ -std=c++17
CXXFLAGS:= -Wall -Wextra -Werror
CXXINC	:= -I./include
CXXEXTRA:=
D_FLAGS	:= -MMD -MP
SRC_DIR	:= src/
BIN		:= bin/
SRCS	:= main.cpp EpollManager.cpp Logger.cpp signal.cpp
OBJS	:= $(SRCS:%.cpp=$(BIN)%.o)
DEPS	:= $(SRCS:%.cpp=$(BIN)%.d)
SRCS	:= $(addprefix $(SRC_DIR), $(SRCS))

# UNIT TEST VARIABLES
TEST_NAME	:= run_tests
TEST_DIR	:= tests/
TEST_SRCS	:= catch_amalgamated.cpp test_epollmanager.cpp
TEST_OBJS	:= $(TEST_SRCS:%.cpp=$(BIN)%.o)
TEST_OBJS	+= $(filter-out $(BIN)main.o, $(OBJS))
TEST_SRCS	:= $(addprefix $(TEST_DIR), $(TEST_SRCS))

all: $(NAME)

debug: CXXEXTRA += -g3 -DDEBUG
debug: $(NAME)

sanitize: CXXEXTRA += -fsanitize=address,undefined,leak
sanitize: debug

$(NAME): $(BIN) $(OBJS)
	$(CC) $(CXXEXTRA) $(OBJS) -o $(NAME)

-include $(DEPS)

%/:
	mkdir $@

$(BIN)%.o: $(SRC_DIR)%.cpp
	$(CC) $(CXXFLAGS) $(CXXEXTRA) $(CXXINC) $(D_FLAGS) -o $@ -c $<

$(BIN)%.o: $(TEST_DIR)%.cpp
	$(CC) $(CXXFLAGS) $(CXXEXTRA) $(CXXINC) $(D_FLAGS) -o $@ -c $<

$(TEST_NAME): $(BIN) $(TEST_OBJS)
	$(CC) $(TEST_OBJS) -o $(BIN)$(NAME)

test: CXXEXTRA	+= -DUNIT_TEST
test: CXXINC	+= -Itests
test: $(TEST_NAME)
	./$(BIN)$(NAME)

clean:
	$(RM) $(OBJS) $(DEPS)
	$(RM) -r bin

fclean: clean
	$(RM) $(NAME)
	$(RM) $(BIN)$(NAME)

re: fclean all

.PHONY: all clean fclean re test debug sanitize
