#include "minishell.h"

// シグナルハンドラ
void	signal_handler(int sig)
{
	if (sig == SIGINT)
		printf("\nminishell> ");
}

// コマンドのパース
void	parse_command(char *command, char **args)
{
	char	*token;
	int		i;

	i = 0;
	token = strtok(command, " \t\n");
	while (token != NULL)
	{
		args[i] = token;
		token = strtok(NULL, " \t\n");
		i++;
	}
	args[i] = NULL;
}

// 環境変数を展開する関数
char	*expand_env_var(char *str)
{
	char	*expanded_str;

	expanded_str = malloc(strlen(str) + 1);
	char *env_var = getenv(str + 1); // `$`を飛ばして環境変数を探す
	if (env_var)
	{
		strcpy(expanded_str, env_var);
	}
	else
	{
		expanded_str = strdup(str); // 見つからない場合はそのまま
	}
	return (expanded_str);
}
// コマンド実行（execvpの代わりに）
void	execute(char *cmd, char **envp)
{
	char	*args[BUFFER_SIZE];
	pid_t	pid;
	int		status;

	parse_command(cmd, args);
	pid = fork();
	if (pid == 0)
	{
		execve(args[0], args, envp); // execvpの代わりにexecveを使用
		perror("execve failed");
		exit(1);
	}
	else if (pid > 0)
	{
		waitpid(pid, &status, 0);
	}
	else
	{
		perror("fork failed");
	}
}

// ビルトインコマンドの実行
void	execute_cd(char *path)
{
	if (chdir(path) != 0)
	{
		perror("cd");
	}
}

void	execute_pwd(void)
{
	char	cwd[1024];

	if (getcwd(cwd, sizeof(cwd)) != NULL)
	{
		printf("%s\n", cwd);
	}
	else
	{
		perror("pwd");
	}
}

void	execute_echo(char **args)
{
	int	i;
	int	no_newline;

	i = 1;
	no_newline = 0;
	if (args[1] && strcmp(args[1], "-n") == 0)
	{
		no_newline = 1;
		i++;
	}
	while (args[i])
	{
		printf("%s", args[i]);
		if (args[i + 1])
		{
			printf(" ");
		}
		i++;
	}
	if (!no_newline)
	{
		printf("\n");
	}
}

void	execute_export(char **args)
{
	char		*eq_pos;
	extern char	**environ;

	if (args[1])
	{
		eq_pos = strchr(args[1], '=');
		if (eq_pos)
		{
			*eq_pos = '\0';
			setenv(args[1], eq_pos + 1, 1);
		}
		else
		{
			printf("export: invalid syntax\n");
		}
	}
	else
	{
		for (int i = 0; environ[i]; i++)
		{
			printf("%s\n", environ[i]);
		}
	}
}

void	execute_unset(char *var)
{
	unsetenv(var);
}

void	execute_env(void)
{
	extern char	**environ;

	for (int i = 0; environ[i]; i++)
	{
		printf("%s\n", environ[i]);
	}
}

void	execute_exit(char **args)
{
	int	status;

	status = 0;
	if (args[1])
	{
		status = atoi(args[1]);
	}
	exit(status);
}

// ビルトインコマンドかどうかを確認
int	is_builtin_command(char *command)
{
	return (strcmp(command, "cd") == 0 || strcmp(command, "echo") == 0
		|| strcmp(command, "pwd") == 0 || strcmp(command, "export") == 0
		|| strcmp(command, "unset") == 0 || strcmp(command, "env") == 0
		|| strcmp(command, "exit") == 0);
}

// ビルトインコマンドの実行
void	execute_builtin_command(char **args)
{
	if (strcmp(args[0], "cd") == 0)
	{
		execute_cd(args[1]);
	}
	else if (strcmp(args[0], "echo") == 0)
	{
		execute_echo(args);
	}
	else if (strcmp(args[0], "pwd") == 0)
	{
		execute_pwd();
	}
	else if (strcmp(args[0], "export") == 0)
	{
		execute_export(args);
	}
	else if (strcmp(args[0], "unset") == 0)
	{
		execute_unset(args[1]);
	}
	else if (strcmp(args[0], "env") == 0)
	{
		execute_env();
	}
	else if (strcmp(args[0], "exit") == 0)
	{
		execute_exit(args);
	}
}

// パイプ処理
void	execute_pipe(char **cmd1, char **cmd2, char **envp)
{
	int		fd[2];
	pid_t	pid1;
	pid_t	pid2;

	if (pipe(fd) == -1)
	{
		perror("pipe");
		exit(1);
	}
	pid1 = fork();
	if (pid1 == -1)
	{
		perror("fork");
		exit(1);
	}
	if (pid1 == 0)
	{
		// 子プロセス1
		dup2(fd[1], STDOUT_FILENO);
		close(fd[0]);
		execve(cmd1[0], cmd1, envp); // execvpの代わりにexecveを使用
		exit(0);
	}
	pid2 = fork();
	if (pid2 == -1)
	{
		perror("fork");
		exit(1);
	}
	if (pid2 == 0)
	{
		// 子プロセス2
		dup2(fd[0], STDIN_FILENO);
		close(fd[1]);
		execve(cmd2[0], cmd2, envp); // execvpの代わりにexecveを使用
		exit(0);
	}
	close(fd[0]);
	close(fd[1]);
	wait(NULL);
	wait(NULL);
}

// 入力リダイレクト
void	redirect_input(char *filename)
{
	int	fd;

	fd = open(filename, O_RDONLY);
	if (fd == -1)
	{
		perror("open");
		exit(1);
	}
	dup2(fd, STDIN_FILENO);
	close(fd);
}

// 出力リダイレクト
void	redirect_output(char *filename, int append)
{
	int	fd;

	if (append)
		fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
	else
		fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd == -1)
	{
		perror("open");
		exit(1);
	}
	dup2(fd, STDOUT_FILENO);
	close(fd);
}

// `ft_split` の代わりに使う関数
char	**split_string(char *str, char delimiter)
{
	int		count;
	char	**result;
	char	*token;

	count = 0;
	// 空の文字列は無視
	if (str == NULL || str[0] == '\0')
	{
		return (NULL);
	}
	// トークンをカウント
	for (int i = 0; str[i] != '\0'; i++)
	{
		if (str[i] == delimiter)
			count++;
	}
	count++;
	result = malloc((count + 1) * sizeof(char *));
	if (!result)
	{
		return (NULL);
	}
	// トークン化
	token = strtok(str, &delimiter);
	for (int i = 0; i < count; i++)
	{
		if (token)
		{
			result[i] = strdup(token);
			token = strtok(NULL, &delimiter);
		}
		else
		{
			result[i] = NULL;
		}
	}
	result[count] = NULL;
	return (result);
}

// パスを検索
char	*find_path(char *cmd, char **envp)
{
	char	**paths;
	char	*path;
	int		i;

	if (cmd[0] == '/' || (cmd[0] == '.' && (cmd[1] == '/' || cmd[1] == '.')))
		return (strdup(cmd)); // 絶対パスまたは相対パスの場合
	while (*envp && strncmp(*envp, "PATH", 4) != 0)
		envp++;
	if (!*envp)
		return (NULL);
	paths = split_string(*envp + 5, ':'); // ":"で区切る
	if (!paths)
		return (NULL);
	for (i = 0; paths[i]; i++)
	{
		// build_path の代わりに
		path = malloc(strlen(paths[i]) + strlen(cmd) + 2);
		strcpy(path, paths[i]);
		strcat(path, "/");
		strcat(path, cmd);
		if (access(path, F_OK) == 0)
			return (path);
	}
	return (NULL);
}

int	main(int argc, char **argv, char **envp)
{
	char	*input;
	char	*cmd1[BUFFER_SIZE];
	char	*cmd2[BUFFER_SIZE];
	int		append;
	char	*filename;
	char	*cmd;
	char	*filename;
	char	*cmd;
	char	*cmd1_str;
	char	*cmd2_str;

	append = 0;
	(void)argc;
	(void)argv;
	signal(SIGINT, signal_handler); // SIGINT シグナルハンドラ
	while (1)
	{
		input = readline("minishell> ");
		if (!input)
			break ;
		add_history(input); // ヒストリに追加
		if (strstr(input, ">"))
		{ // 出力リダイレクト
			filename = strtok(input, ">");
			cmd = strtok(NULL, ">");
			redirect_output(filename, append);
			parse_command(cmd, cmd1);
			execute(cmd, envp);
		}
		else if (strstr(input, "<"))
		{ // 入力リダイレクト
			filename = strtok(input, "<");
			cmd = strtok(NULL, "<");
			redirect_input(filename);
			parse_command(cmd, cmd1);
			execute(cmd, envp);
		}
		else if (strstr(input, "|"))
		{ // パイプ
			cmd1_str = strtok(input, "|");
			cmd2_str = strtok(NULL, "|");
			parse_command(cmd1_str, cmd1);
			parse_command(cmd2_str, cmd2);
			execute_pipe(cmd1, cmd2, envp);
		}
		else
		{
			parse_command(input, cmd1);
			if (is_builtin_command(cmd1[0]))
			{
				execute_builtin_command(cmd1);
			}
			else
			{
				execute(input, envp);
			}
		}
		free(input);
	}
	return (0);
}
