# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/09/27 14:52:58 by TheRed            #+#    #+#              #
#    Updated: 2024/10/13 20:04:05 by ycontre          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

BLACK		=	[90m
RED			=	[91m
GREEN		=	[92m
YELLOW		=	[93m
BLUE		=	[94m
MAGENTA		=	[95m
CYAN		=	[96m
WHITE		=	[97m

RESET		=	[0m

LINE_CLR	=	\33[2K\r

NAME        :=	RT

SRCS_DIR	:=	srcs

OBJS_DIR	:=	.objs

ALL_SRCS	:=	RT.cpp	gl.cpp			\
				Window.cpp

				
SRCS		:=	$(ALL_SRCS:%=$(SRCS_DIR)/%)


OBJS		:=	$(addprefix $(OBJS_DIR)/, $(SRCS:%.cpp=%.o))

CC          :=	g++

IFLAGS	    :=	-Ofast -I./includes -L./lib -lglfw3 -lopengl32 -lgdi32 -lcglm

RM          :=	del /f /s /q

MAKEFLAGS   += --no-print-directory

DIR_DUP     =	if not exist "$(@D)" mkdir "$(@D)"

# RULES ********************************************************************** #

all: $(NAME)

$(NAME): $(OBJS) $(HEADERS)
	@$(CC) -o $(NAME) $(OBJS) $(IFLAGS)
	@echo $(WHITE) $(NAME): PROJECT COMPILED !$(RESET) & echo:

$(OBJS_DIR)/%.o: %.cpp
	@$(DIR_DUP)
	@echo $(WHITE) $(NAME): $(WHITE)$<$(RESET) $(GREEN)compiling...$(RESET)
	@$(CC) -c $^ $(IFLAGS) -o $@ 


fclean:
	@echo  $(WHITE)$(NAME):$(RED) cleaned.$(RESET)
	@del /f /s /q $(NAME).exe
	@rmdir /S /Q "$(OBJS_DIR)"

re:
	@$(MAKE) fclean
	@$(MAKE) all

# **************************************************************************** #

.PHONY: all clean fclean dclean re bonus