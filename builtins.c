#include "minishell.h"

void	builtin_echo(char **args)
{
    int	i;
	int	newline;

	newline = 1;
	i = 1;
	if (args[i] && strcmp(args[i], "-n") == 0)
	{
		newline = 0;
		i++;
	}
	while (args[i])
	{
		write(1, args[i], strlen(args[i]));
		if (args[i + 1])
			write(1, " ", 1);
		i++;
	}
	if (newline)
		write(1, "\n", 1);
}

void	builtin_pwd(void)
{
	char	cwd[BUFFER_SIZE];

	if (getcwd(cwd, sizeof(cwd)))
		printf("%s\n", cwd);
	else
		perror("minishell");
}

void	builtin_cd(char **args)
{
	if (!args[1])
	{
		fprintf(stderr, "minishell: cd: missing argument\n"); //本家の挙動と同じかはわからないけど、エラー表示
		return;
	}
	if (chdir(args[1]) != 0)
		perror("minnishell");
}

void	builtin_exit(void)
{
	char	*exit_status_str;
	int		exit_status;

    exit_status_str = strtok(NULL, " ");
    if (!exit_status_str)
        exit(0);
    exit_status = atoi(exit_status_str);
    if (exit_status == 0 && exit_status_str[0] != '0')
    {
        fprintf(stderr, "minishell: exit: %s: numeric argument required\n", exit_status_str);
        exit(255);
    }
    exit(exit_status);
}

void	builtin_env(char **envp) //ループしてすべての環境変数を表示
{
	int	i;

	i = 0;
	while (envp[i])
	{
		printf("%s\n", envp[i]);
		i++;
	}
}

void	builtin_export(char ***envp, char *arg)
{
	int		i;
	char	**new_env;
	char	*key;
	size_t	len;

	if (!arg || !strchr(arg, '=')) //以降、KEY=VALUE の形式でない場合はエラー
	{
		fprintf(stderr, "minishell: export: invalid argument\n");
        return;
	}
	key = strtok(arg, "=");
	len = strlen(key);
	i = 0;
	while ((*envp)[i])
	{
		if (strncmp((*envp)[i], key, len) == 0 && (*envp)[i][len] == '=')
		{
			free((*envp)[i]);
			(*envp)[i] = strdup(arg);
			return;
		}
		i++;
	}
	new_env = malloc(sizeof(char *) * (i + 2));
	if (!new_env)
		return;
	i = 0;
	while ((*envp)[i])
	{
		new_env[i] = (*envp)[i];
		i++;
	}
	new_env[i] = strdup(arg);
	new_env[i + 1] = NULL;
	free(*envp);
	*envp = new_env;
}

void	builtin_unset(char ***envp, char *key)
{
	int		i;
	int		j;
	size_t	len;
	char	**new_env;

	len = strlen(key);
	i = 0;
	while ((*envp)[i])
	{
		if (strncmp((*envp)[i], key, len) == 0 && (*envp)[i][len] == '=')
            break;
		i++;
	}
	if (!(*envp)[i])
        return;
	free((*envp)[i]);
	new_env = malloc(sizeof(char *) * i);
	if (!new_env)
		return;
	j = 0;
	while ((*envp)[j])
	{
		if (j != i)
			new_env[j - (j > i)] = (*envp)[j]; //不正なアクセスを防ぐための小技
		j++;
	}
	new_env[j - 1] = NULL;
	free(*envp);
	*envp = new_env;
}

int	is_builtin(char *cmd)
{
    if (strcmp(cmd, "echo") == 0 || strcmp(cmd, "pwd") == 0 ||
        strcmp(cmd, "cd") == 0 || strcmp(cmd, "exit") == 0)
        return (1);
    return (0);
}

void	execute_builtin(char **args, char ***envp)
{
	if (strcmp(args[0], "echo") == 0)
		builtin_echo(args);
	else if (strcmp(args[0], "pwd") == 0)
        builtin_pwd();
	else if (strcmp(args[0], "cd") == 0)
        builtin_cd(args);
	else if (strcmp(args[0], "exit") == 0)
        builtin_exit();
	else if (strcmp(args[0], "env") == 0)
        builtin_env(*envp);
	else if (strcmp(args[0], "export") == 0)
        builtin_export(envp, args[1]);
	else if (strcmp(args[0], "unset") == 0)
        builtin_unset(envp, args[1]);
}
