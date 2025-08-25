# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/03/15 14:05:43 by rhanitra          #+#    #+#              #
#    Updated: 2025/07/06 17:28:14 by rhanitra         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #


MINI_CLIENT = client.c

DIR_MINI_CLIENT = ./
SRC_MINI_CLIENT= $(addprefix $(DIR_MINI_CLIENT),$(MINI_CLIENT))

#SERVER
MINI_SERVER = servweb.cpp

DIR_MINI_SERVER = ./
SRC_MINI_SERVER= $(addprefix $(DIR_MINI_SERVER),$(MINI_SERVER))

SRC_CLIENT = $(SRC_MINI_CLIENT)
SRC_SERVER = $(SRC_MINI_SERVER)
OBJ_CLIENT = $(SRC_CLIENT:.c=.o)
OBJ_SERVER = $(SRC_SERVER:.c=.o)

#****************TARGET**********************************************
CLIENT = client
SERVER = server


FLAGS = -Wall -Wextra -Werror -std=c++98
CC = c++
RM = rm -rf

all : $(CLIENT) $(SERVER) 

%.o : %.cpp
	$(CC) $(FLAGS) -o $@ -c $<

$(CLIENT) : $(OBJ_CLIENT)
	$(CC)  $(CFLAGS) -o $@ $^ 

$(SERVER) : $(OBJ_SERVER)
	$(CC)  $(CFLAGS) -o $@ $^

clean :
	$(RM) $(OBJ_CLIENT) $(OBJ_SERVER)
	
fclean : clean
	$(RM) $(CLIENT) $(SERVER)

re : fclean all

.PHONY: all clean fclean re