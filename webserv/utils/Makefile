# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/03/22 08:39:35 by rhanitra          #+#    #+#              #
#    Updated: 2025/09/08 20:01:41 by rhanitra         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

UTILS = ftToInt.cpp ftPutFileContent.cpp ftSplitStr.cpp

CXXFLAGS = -Wall -Wextra -Werror -std=c++98

CXX = c++

SRC = $(UTILS)

OBJ = $(SRC:.cpp=.o)

RM = rm -rf

AR = ar rcs

NAME = libutils.a


all : $(NAME)

$(NAME) : $(OBJ)
	$(AR) $(NAME) $^

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $< 

clean :
	$(RM) $(OBJ)
	
fclean : clean
	$(RM) $(NAME)

re : fclean all

.PHONY: all clean fclean re bonus
