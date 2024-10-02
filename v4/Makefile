NAME = ircserv
BONUS_NAME = ircserv_bonus
CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98 #-DBONUS

SRCS = 	src/Client.cpp \
		src/Server.cpp \
		src/Server_connection.cpp \
		src/Server_invite.cpp \
		src/Server_join.cpp \
		src/Server_kick.cpp \
		src/Server_mode.cpp \
		src/Server_nick.cpp \
		src/Server_privmsg.cpp \
		src/Server_topic.cpp \
		src/Server_user.cpp \
		src/Server_utils.cpp \
		src/Channel.cpp \
		src/main.cpp

BONUS_SRCS = src/Server_file_transfer.cpp src/Bot.cpp 

OBJDIR = obj

OBJS = $(SRCS:%.cpp=$(OBJDIR)/%.o)
BONUS_OBJS = $(OBJS) $(BONUS_SRCS:%.cpp=$(OBJDIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
		$(CC) $(CFLAGS) $^ -o $@

$(BONUS_NAME): $(BONUS_OBJS)
		$(CC) $(CFLAGS) $^ -o $@

$(OBJDIR)/%.o: %.cpp
		@mkdir -p $(dir $@)  
		$(CC) $(CFLAGS) -c $< -o $@

bonus: $(BONUS_NAME)

clean:
		rm -rf $(OBJDIR)

fclean: clean
		rm -f $(NAME) $(BONUS_NAME)

re: fclean all

.PHONY: all clean fclean re bonus
