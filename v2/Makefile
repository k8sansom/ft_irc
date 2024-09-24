NAME = ircserv
CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98
SRCS = 	Client.cpp \
		Server.cpp \
		Server_connection.cpp \
		Server_join.cpp \
		Server_opcmds.cpp \
		Server_nick.cpp \
		Server_privmsg.cpp \
		Server_user.cpp \
		Channel.cpp \
		main.cpp
OBJDIR = obj
OBJS = $(SRCS:%.cpp=$(OBJDIR)/%.o)

all: $(NAME)
$(NAME): $(OBJS)
		$(CC) $(CFLAGS) $^ -o $@
$(OBJDIR)/%.o: %.cpp
		@mkdir -p $(dir $@)  # Ensure the object directory exists
		$(CC) $(CFLAGS) -c $< -o $@

clean:
		rm -rf $(OBJDIR)
fclean: clean
		rm -f $(NAME)
re: fclean all

.PHONY: all clean fclean re
