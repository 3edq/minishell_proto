#include "minishell.h"

char	*find_executable_path(char *command, char **env)
{
	char	*path_env;
	char	**paths;
	char	*full_path;
	int		i;

	if (strchr(command, '/') != NULL)
	{
		if (access(command, X_OK) == 0)
			return (strdup(command));
		return (NULL);
	}
	path_env = NULL; // ← NULL で初期化（★重要）
	while (*env)
	{
		if (strncmp(*env, "PATH=", 5) == 0)
		{
			path_env = *env + 5;
			break ;
		}
		env++;
	}
	if (!path_env)
		return (NULL);
	paths = malloc(1024 * sizeof(char *));
	if (!paths)
		return (NULL);
	paths[0] = strtok(path_env, ":");
	i = 1;
	while ((paths[i] = strtok(NULL, ":")) != NULL)
		i++;
	i = 0;
	while (paths[i])
	{
		full_path = malloc(strlen(paths[i]) + strlen(command) + 2);
		if (!full_path)
			break ;
		sprintf(full_path, "%s/%s", paths[i], command);
		if (access(full_path, X_OK) == 0)
		{
			free(paths);
			return (full_path);
		}
		free(full_path);
		i++;
	}
	free(paths);
	return (NULL);
}

void	execute_command(char **args, char **env)
{
	pid_t	pid;
	int		status;
	char	***commands;
	char	*full_path;

	if (!args[0])
		return ;
	if (is_builtin(args[0]))
	{
		execute_builtin(args, &env);
		return ;
	}
	commands = split_pipes(args);
	if (commands[1])
		execute_pipeline(commands, env);
	else
	{
		pid = fork();
		if (pid == 0)
		{
			handle_redirections(args);
			full_path = find_executable_path(args[0], env);
			if (!full_path)
			{
				fprintf(stderr, "minishell: command not found: %s\n", args[0]);
				exit(127);
			}
			if (execve(full_path, args, env) == -1)
			{
				perror("minishell");
				free(full_path);
				exit(EXIT_FAILURE);
			}
		}
		else if (pid < 0)
			perror("minishell");
		else
			waitpid(pid, &status, 0);
	}
	free(commands);
}
