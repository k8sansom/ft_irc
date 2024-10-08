NAME = ircserv
BONUS_NAME = ircserv_bonus
CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98
BONUS_CFLAGS = $(CFLAGS) -DBONUS

SRCS = 	src/Client.cpp \
		src/Server.cpp \
		src/Server_connection.cpp \
		src/Server_pass_who_part.cpp \
		src/Server_invite.cpp \
		src/Server_join.cpp \
		src/Server_kick.cpp \
		src/Server_mode.cpp \
		src/Server_nick.cpp \
		src/Server_privmsg.cpp \
		src/Server_topic.cpp \
		src/Server_user.cpp \
		src/Server_utils.cpp \
		src/Server_msgHandle.cpp \
		src/Channel.cpp \
		src/main.cpp

BONUS_SRCS = src/Bot.cpp

OBJDIR = obj
BONUS_OBJDIR = bonus_obj

OBJS = $(SRCS:%.cpp=$(OBJDIR)/%.o)
BONUS_OBJS = $(SRCS:%.cpp=$(BONUS_OBJDIR)/%.o) $(BONUS_SRCS:%.cpp=$(BONUS_OBJDIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
		$(CC) $(CFLAGS) $^ -o $@

$(BONUS_NAME): $(BONUS_OBJS)
		$(CC) $(BONUS_CFLAGS) $^ -o $@

$(OBJDIR)/%.o: %.cpp
		@mkdir -p $(dir $@)
		$(CC) $(CFLAGS) -c $< -o $@

$(BONUS_OBJDIR)/%.o: %.cpp
		@mkdir -p $(dir $@)
		$(CC) $(BONUS_CFLAGS) -c $< -o $@

bonus: $(BONUS_NAME)

clean:
		rm -rf $(OBJDIR) $(BONUS_OBJDIR)

fclean: clean
		rm -f $(NAME) $(BONUS_NAME)

re: fclean all

.PHONY: all clean fclean re bonus
