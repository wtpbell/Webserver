NAME		:= webserv
CXX			:= c++ -std=c++17
CXXFLAGS	:= -Wall -Wextra -Werror
CXXINC		:= -I./include
CXXEXTRA	:=
D_FLAGS		:= -MMD -MP
SRC_DIR		:= src/
BIN			:= bin/
BIN_DIRS	:= $(BIN) $(BIN)exception/ $(BIN)config/ $(BIN)io/ $(BIN)http/ $(BIN)router/ $(BIN)cgi/
MAIN		:= $(BIN)main.o

SRCS		:= Logger.cpp helper.cpp signal.cpp Server.cpp EpollManager.cpp string.cpp Connection.cpp
SRCS_CONFIG	:= $(addprefix config/, Lexer.cpp Parser.cpp Validator.cpp ValidatorIpPort.cpp Builder.cpp ServerRegistry.cpp loadConfigs.cpp)
SRCS_EXCEPT	:= $(addprefix exception/, FileDescriptorException.cpp)
SRCS_IO		:= $(addprefix io/, SharedFD.cpp Socket.cpp TimerFD.cpp)
SRCS_HTTP	:= $(addprefix http/, HTTPMessage.cpp HTTPParser.cpp HTTPUtils.cpp HTTPRequest.cpp HTTPResponse.cpp\
				HTTPValidator.cpp HTTPStatus.cpp ResponseFactory.cpp SessionManager.cpp HTTPCookie.cpp)
SRCS_ROUTER	:= $(addprefix router/, Router.cpp RequestHandler.cpp)
SRCS_CGI	:= $(addprefix cgi/, CGI.cpp CGIParser.cpp CGIProcess.cpp CGIRequest.cpp CGIResponse.cpp)
SRCS		:= $(SRCS) $(SRCS_CONFIG) $(SRCS_EXCEPT) $(SRCS_IO) $(SRCS_HTTP) $(SRCS_ROUTER) $(SRCS_CGI)

OBJS		:= $(SRCS:%.cpp=$(BIN)%.o)
DEPS		:= $(BIN)main.d $(SRCS:%.cpp=$(BIN)%.d)

# UNIT TEST VARIABLES
TEST_DIR		:= tests/
TEST_DIRS		:= $(BIN)$(TEST_DIR) $(BIN)$(TEST_DIR)cgi/
TEST_NAME		:= $(BIN)$(TEST_DIR)run_tests
TEST_SRCS		:= catch_amalgamated.cpp test_sharedfd.cpp test_socket.cpp Client.cpp test_epollmanager.cpp test_logger.cpp\
					test_configs_lexer.cpp test_configs_parser.cpp test_configs_validator.cpp test_configs_builder.cpp test_configs_server_registry.cpp test_http_paths.cpp test_http_parser.cpp test_http_validator.cpp\
					test_integrate_parser_validator.cpp test_http_response.cpp test_TimerFD.cpp test_http_session_manager.cpp test_body_sink.cpp test_router.cpp test_http_delete_request.cpp \
					test_http_get_request.cpp test_http_post_request.cpp test_connection.cpp test_http_wire_utils.cpp
TEST_SRCS_CGI	:= $(addprefix cgi/, test_CGI.cpp test_CGIRequest.cpp test_CGIResponse.cpp test_CGIParser.cpp)
TEST_SRCS		:= $(TEST_SRCS) $(TEST_SRCS_CGI)
TEST_OBJS		:= $(TEST_SRCS:%.cpp=$(BIN)$(TEST_DIR)%.o)

all: $(NAME)

debug: CXXEXTRA += -g3 -DDEBUG
debug: $(NAME)

sanitize: CXXEXTRA += -fsanitize=address,undefined,leak
sanitize: debug

$(NAME): $(MAIN) $(OBJS)
	$(CXX) $(CXXEXTRA) $^ -o $@

$(TEST_NAME): $(OBJS) $(TEST_OBJS)
	$(CXX) $(CXXEXTRA) $^ -o $@

-include $(DEPS)

%/:
	mkdir -p $@

$(BIN)%.o: $(SRC_DIR)%.cpp | $(BIN_DIRS)
	$(CXX) $(CXXFLAGS) $(CXXEXTRA) $(CXXINC) $(D_FLAGS) -o $@ -c $<

$(BIN)$(TEST_DIR)%.o: $(TEST_DIR)%.cpp | $(TEST_DIRS)
	$(CXX) $(CXXFLAGS) $(CXXEXTRA) $(CXXINC) $(D_FLAGS) -o $@ -c $<

test: CXXEXTRA	+= -DUNIT_TEST -g3
test: CXXINC	+= -I./tests
test: $(TEST_NAME)
	./$<

# Integration Test CGI
TEST_CGI_SRCS	:= cgi/test_integration_cgi.cpp cgi/wrap.cpp
TEST_CGI_OBJS	:= $(TEST_CGI_SRCS:%.cpp=$(BIN)$(TEST_DIR)%.o)
TEST_CGI_NAME	:= $(BIN)$(TEST_DIR)cgi_test

FORK_COUNT	?= 10
PAIR_COUNT	?= 10
DUP_COUNT	?= 10
$(TEST_CGI_NAME): $(OBJS) $(TEST_CGI_OBJS)
	$(CXX) $(CXXEXTRA) -Wl,--wrap=fork,--wrap=socketpair,--wrap=dup2 $^ -o $@

cgi_test: CXXEXTRA	+= -DUNIT_TEST -g3 -D_GLIBCXX_DEBUG -DFORK_COUNT=$(FORK_COUNT) -DPAIR_COUNT=$(PAIR_COUNT) -DDUP_COUNT=$(DUP_COUNT)
cgi_test: $(TEST_CGI_NAME)

cgi_clean:
	$(RM) $(TEST_CGI_OBJS) $(TEST_CGI_NAME)

clean:
	$(RM) $(MAIN) $(OBJS) $(DEPS) $(TEST_OBJS) $(TEST_CGI_OBJS)

fclean: clean
	$(RM) $(NAME) $(TEST_NAME) $(TEST_CGI_NAME)
	$(RM) -r $(BIN)

re: fclean all

.PHONY: all clean fclean re test debug sanitize cgi_test cgi_clean
.SECONDARY: $(BIN_DIRS) $(TEST_DIRS)
