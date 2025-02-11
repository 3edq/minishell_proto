#include "minishell.h"

// void signal_handler(int sig)
// {
//    if (sig == SIGINT)
//        printf("\nminishell> ");
// }

int	main(int argc, char **argv, char **env)
{
	char	*input;
	char	**args;

	(void)argc;
	(void)argv;
	handle_signals();
	while(1)
	{
		input = readline("minishell> ");
		if (!input)
			break;
		if (*input)
		{
			add_history(input);
			args = tokenize_input(input);
			if (args && args[0])
				execute_command(args, env);
			free(args);
		}
		free(input);
	}
	return (0);
}
