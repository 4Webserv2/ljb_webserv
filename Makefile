NAME = webserv

CC = c++

C_FLAGS = -Wall -Wextra -Werror -std=c++98

SRC_DIR = src/

SRCS := $(shell find $(SRC_DIR) -type f -name "*.cpp")

RED =		\033[0;35m
BLUE =		\033[0;34m
RESET =		\033[0m

OBJS =		${SRCS:.cpp=.o}

.cpp.o:
			$(CC) $(C_FLAGS) -c $< -o $@


all:		$(NAME)
			@echo "$(RED)$(NAME) is ready!$(RESET)"

$(NAME):	$(OBJS)
			ar -rcs webserv.a $(OBJS)
			@$(CC) $(C_FLAGS) main.cpp webserv.a -o $(NAME)

clean:
			@rm -rf uploads/
			@rm -f $(OBJS)
			@echo "$(BLUE) 📤 Objects deleted$(RESET)"

fclean:
			@rm -f $(NAME)
			@rm -f $(OBJS)
			@rm -f webserv.a
			@rm -f app.log
			@echo "$(BLUE) 🧼 All cleaned$(RESET)"

re:			fclean all

run:		re
			./$(NAME)

val:		re
			valgrind  --leak-check=full --show-leak-kinds=all ./$(NAME) config/default.conf
