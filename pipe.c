#include "minishell.h"

void	execute_pipeline(char ***commands, char **env)
{
	int		i;
	int		pipefd[2];
	pid_t	pid;
	int		fd_in;

	fd_in = 0;
	i = 0;
	while (commands[i])
	{
		if (commands[i + 1])
			if (pipe(pipefd) == -1)
			{
				perror("minishell");
				exit(EXIT_FAILURE);
			}
		pid = fork();
		if (pid == 0)
		{
			dup2(fd_in, STDIN_FILENO);
			if (commands[i + 1])
				dup2(pipefd[1], STDOUT_FILENO);
			close(pipefd[0]);
			execute_command(commands[i], env);
			exit(EXIT_FAILURE);
		}
		else if (pid < 0)
		{
			perror("minishell");
			exit(EXIT_FAILURE);
		}
		close(pipefd[1]);
		fd_in = pipefd[0];
		i++;
	}
	while (wait(NULL) > 0)
		;
}

char	***split_pipes(char **args)
{
	char	***commands;
	int		i;
	int		j;
	int		cmd_count;

	cmd_count = 1;
	i = 0;
	while (args[i])
	{
		if (strcmp(args[i], "|") == 0)
			cmd_count++;
		i++;
	}
	commands = malloc(sizeof(char **) * (cmd_count + 1));
	if (!commands)
		return (NULL);
	i = 0;
	j = 0;
	commands[j] = &args[i];
	while (args[i])
	{
		if (strcmp(args[i], "|") == 0)
		{
			args[i] = NULL;
			j++;
			commands[j] = &args[i + 1];
		}
		i++;
	}
	commands[j + 1] = NULL;
	return (commands);
}
