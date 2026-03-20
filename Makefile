NAME		:= webserv
CC			:= c++ -std=c++17
CXXFLAGS	:= -Wall -Wextra -Werror
CXXINC		:= -I./include
CXXEXTRA	:=
D_FLAGS		:= -MMD -MP
SRC_DIR		:= src/
BIN			:= bin/
BIN_DIRS	:= $(BIN) $(BIN)exception/ $(BIN)config/ $(BIN)io/ $(BIN)http/
MAIN		:= $(BIN)main.o
SRCS		:= Logger.cpp helper.cpp signal.cpp Server.cpp EpollManager.cpp string.cpp Connection.cpp
SRCS_CONFIG	:= $(addprefix config/, Lexer.cpp Parser.cpp Validator.cpp load_configs.cpp)
SRCS_EXCEPT	:= $(addprefix exception/, FileDescriptorException.cpp ServerException.cpp EPollManagerException.cpp)
SRCS_IO		:= $(addprefix io/, SharedFD.cpp Socket.cpp)
SRCS_HTTP	:= $(addprefix http/, HTTPMessage.cpp HTTPParser.cpp HTTPUtils.cpp HTTPRequest.cpp HTTPResponse.cpp\
				HTTPValidator.cpp HTTPStatus.cpp HTTPResponse.cpp SessionManager.cpp HTTPCookie.cpp)
SRCS		:= $(SRCS) $(SRCS_CONFIG) $(SRCS_EXCEPT) $(SRCS_IO) $(SRCS_HTTP)

OBJS		:= $(SRCS:%.cpp=$(BIN)%.o)
DEPS		:= $(BIN)main.d $(SRCS:%.cpp=$(BIN)%.d)

# UNIT TEST VARIABLES
TEST_DIR	:= tests/
TEST_NAME	:= $(BIN)$(TEST_DIR)run_tests
TEST_SRCS	:= catch_amalgamated.cpp test_sharedfd.cpp test_socket.cpp Client.cpp test_server.cpp test_epollmanager.cpp test_logger.cpp \
				test_lexer.cpp test_parser.cpp test_http_paths.cpp test_http_parser.cpp test_http_validator.cpp \
				test_integrate_parser_validator.cpp test_http_response.cpp test_validator.cpp test_http_session_manager.cpp
TEST_OBJS	:= $(TEST_SRCS:%.cpp=$(BIN)$(TEST_DIR)%.o)
TEST_DIRS	:= $(BIN)$(TEST_DIR)

all: $(NAME)

debug: CXXEXTRA += -g3 -DDEBUG
debug: $(NAME)

sanitize: CXXEXTRA += -fsanitize=address,undefined,leak
sanitize: debug

SRC_PING	:= tests/ping.cpp tests/Client.cpp src/Logger.cpp src/string.cpp src/helper.cpp\
				src/exception/FileDescriptorException.cpp\
				$(addprefix src/, $(SRCS_IO))
ping:
	$(CC) $(CXXFLAGS) $(CXXINC) $(SRC_PING) -o ping

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
	$(RM) $(NAME) $(TEST_NAME) ping
	$(RM) -r $(BIN)

re: fclean all

.PHONY: all clean fclean re test debug sanitize ping
.SECONDARY: $(BIN_DIRS) $(TEST_DIRS)
