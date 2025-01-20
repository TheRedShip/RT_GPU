ifeq ($(OS),Windows_NT)
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
	RM          :=	del /S /Q
	DIR_DUP     =	if not exist "$(@D)" mkdir "$(@D)"
	CC          :=	g++ -O3
	IFLAGS	    :=	-I./includes -I./includes/RT -I./includes/imgui
	LDFLAGS     :=  -L./lib -lglfw3 -lopengl32 -lgdi32 -lcglm
else
	BLACK		=	\033[30;49;3m
	RED			=	\033[31;49;3m
	GREEN		=	\033[32;49;3m
	YELLOW		=	\033[33;49;3m
	BLUE		=	\033[34;49;3m
	MAGENTA		=	\033[35;49;3m
	CYAN		=	\033[36;49;3m
	WHITE		=	\033[37;49;3m
	RESET		=	\033[0m
	LINE_CLR	=	\33[2K\r
	RM          :=	rm -rf
	DIR_DUP     =	mkdir -p $(@D)
	CC          :=	clang++
	CFLAGS      :=	-Wall -Wextra -Werror -g -O3
	IFLAGS	    :=	-I./includes -I./includes/RT -I./includes/imgui -I/usr/include
	LDFLAGS		:=  -L/usr/lib/x86_64-linux-gnu -lglfw -lGL -lGLU -lX11 -lpthread -ldl -lstdc++
	FILE		=	$(shell ls -lR srcs/ | grep -F .c | wc -l)
	CMP			=	1
endif

NAME        :=	RT
SRCS_DIR	:=	srcs
OBJS_DIR	:=	.objs

IMGUI_SRCS := imgui/imgui.cpp         \
              imgui/imgui_draw.cpp    \
              imgui/imgui_widgets.cpp \
              imgui/imgui_tables.cpp  \
              imgui/imgui_impl_glfw.cpp \
              imgui/imgui_impl_opengl3.cpp

ALL_SRCS	:=	$(IMGUI_SRCS)			\
				RT.cpp	gl.cpp			\
				class/Window.cpp		\
				class/Shader.cpp		\
				class/Camera.cpp		\
				class/Scene.cpp			\
				class/SceneParser.cpp	\
				class/ObjParser.cpp		\
				class/BVH.cpp			\

SRCS		:=	$(ALL_SRCS:%=$(SRCS_DIR)/%)
OBJS		:=	$(addprefix $(OBJS_DIR)/, $(SRCS:%.cpp=%.o))
HEADERS		:=	includes/RT.hpp
MAKEFLAGS   += --no-print-directory

ifeq ($(OS),Windows_NT)
all: windows
else
all: linux
endif

windows: $(OBJS) $(HEADERS)
	@$(CC) $(OBJS) $(IFLAGS) $(LDFLAGS) -o $(NAME)
	@echo $(WHITE) $(NAME): PROJECT COMPILED !$(RESET)

linux: $(OBJS) $(HEADERS)
	@$(CC) $(OBJS) $(IFLAGS) $(CFLAGS) $(LDFLAGS) -o $(NAME)
	@printf "$(LINE_CLR)$(WHITE) $(NAME): PROJECT COMPILED !$(RESET)\n\n"

$(OBJS_DIR)/%.o: %.cpp
	@$(DIR_DUP)
ifeq ($(OS),Windows_NT)
	@echo $(WHITE) $(NAME): $(WHITE)$<$(RESET) $(GREEN)compiling...$(RESET)
	@$(CC) $(IFLAGS) -c $^ -o $@
else
	@if [ $(CMP) -eq '1' ]; then printf "\n"; fi;
	@printf "$(LINE_CLR)$(WHITE) $(NAME): $(CMP)/$(FILE) $(WHITE)$<$(RESET) $(GREEN)compiling...$(RESET)"
	@$(CC) $(CFLAGS) $(IFLAGS) -c $^ -o $@
	@$(eval CMP=$(shell echo $$(($(CMP)+1))))
	@if [ $(CMP) -gt $(FILE) ]; then \
		printf "$(LINE_CLR)$(WHITE) $(NAME): $$(($(CMP)-1))/$(FILE)\n$(LINE_CLR)$(GREEN) Compilation done !$(RESET)\n"; \
	fi
endif

ifeq ($(OS),Windows_NT)
clean:
else
clean:
	@$(RM) $(OBJS)
endif

fclean: clean
ifeq ($(OS),Windows_NT)
	@echo $(WHITE)$(NAME):$(RED) cleaned.$(RESET)
	@$(RM) $(NAME).exe
	@rmdir /S /Q "$(OBJS_DIR)"
else
	@printf "$(WHITE)$(NAME):$(RED) cleaned.$(RESET)\n"
	@$(RM) $(NAME)
	@$(RM) $(OBJS_DIR)
endif

ifeq ($(OS),Windows_NT)
re: fclean windows
else
re: fclean linux
endif

.PHONY: all clean fclean re windows linux
