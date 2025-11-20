NAME	:= webserv
CC		:= c++ -std=c++17
CXXFLAGS:= -Wall -Wextra -Werror
CXXINC	:= -I./include
CXXEXTRA:=
D_FLAGS	:= -MMD -MP
SRC_DIR	:= src/
BIN		:= bin/
SRCS	:= main.cpp Logger.cpp
OBJS	:= $(SRCS:%.cpp=$(BIN)%.o)
DEPS	:= $(SRCS:%.cpp=$(BIN)%.d)
SRCS	:= $(addprefix $(SRC_DIR), $(SRCS))

all: $(NAME)

debug: CXXEXTRA += -g3 -DDEBUG
debug: $(NAME)

$(NAME): $(BIN) $(OBJS)
	$(CC) $(CXXEXTRA) $(OBJS) -o $(NAME)

-include $(DEPS)

%/:
	mkdir $@

$(BIN)%.o: $(SRC_DIR)%.cpp
	$(CC) $(CXXFLAGS) $(CXXEXTRA) $(CXXINC) $(D_FLAGS) -o $@ -c $<

clean:
	$(RM) $(OBJS) $(DEPS)
	$(RM) -r bin

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
