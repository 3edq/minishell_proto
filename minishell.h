#ifndef MINISHELL_H
# define MINISHELL_H

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <fcntl.h>
# include <readline/readline.h>
# include <readline/history.h>
# include <string.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <glob.h>

# define BUFFER_SIZE 1024

char	**tokenize_input(char *input);
void	execute_command(char **args, char **env);
int		is_builtin(char *cmd);
void	execute_builtin(char **args, char ***envp);
void	builtin_echo(char **args);
void	builtin_pwd(void);
void	builtin_cd(char **args);
void	builtin_exit(void);
void	builtin_env(char **envp);
void	builtin_export(char ***envp, char *arg);
void	builtin_unset(char ***envp, char *key);
int		handle_redirections(char **args);
// int		handle_heredoc(char *delimiter);
void	handle_signals(void);
void	execute_pipeline(char ***commands, char **env);
char	***split_pipes(char **args);

#endif
