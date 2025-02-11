#include "minishell.h"

int	handle_redirections(char **args)
{
    int	i;
    int	fd;

    i = 0;
    while (args[i])
    {
        if (strcmp(args[i], ">") == 0)
        {
            fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd == -1)
            {
                perror("minishell");
                return (-1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[i] = NULL;
        }
        else if (strcmp(args[i], ">>") == 0)
        {
            fd = open(args[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (fd == -1)
            {
                perror("minishell");
                return (-1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[i] = NULL;
        }
        else if (strcmp(args[i], "<") == 0)
        {
            fd = open(args[i + 1], O_RDONLY);
            if (fd == -1)
            {
                perror("minishell");
                return (-1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
            args[i] = NULL;
        }
        i++;
    }
    return (0);
}

// int	handle_heredoc(char *delimiter)
// {
//     int		fd[2];
//     char	*line;

//     if (pipe(fd) == -1)
//     {
//         perror("minishell");
//         return (-1);
//     }
//     while (1)
//     {
//         line = readline("> ");
//         if (!line || strcmp(line, delimiter) == 0)
//             break;
//         write(fd[1], line, strlen(line));
//         write(fd[1], "\n", 1);
//         free(line);
//     }
//     close(fd[1]);
//     dup2(fd[0], STDIN_FILENO);
//     close(fd[0]);
//     return (0);
// }
