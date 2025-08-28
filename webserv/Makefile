# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/08/28 14:08:13 by rhanitra          #+#    #+#              #
#    Updated: 2025/08/28 14:13:17 by rhanitra         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = webserver

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g

SRC_DIR = ./srcs
UTILS_DIR = ./utils
OBJ_DIR = ./obj
INC_DIR = ./include

SRCS = main.cpp \
       $(SRC_DIR)/configParser.cpp \
       $(UTILS_DIR)/libft.cpp

OBJS = $(SRCS:.cpp=.o)
OBJS := $(addprefix $(OBJ_DIR)/,$(notdir $(OBJS)))

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

$(OBJ_DIR)/%.o: $(UTILS_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
