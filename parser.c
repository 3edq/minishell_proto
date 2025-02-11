#include "minishell.h"

char	**tokenize_input(char *input)
{
    char	**tokens;
	char	*token;
	int		i;

	tokens = malloc(sizeof(char *) * BUFFER_SIZE);
	if (!tokens)
		return (NULL);
	i = 0;
	token = strtok(input, " ");
	while (token)
	{
		tokens[i] = strdup(token);
		i++;
		token = strtok(NULL, " ");
	}
	tokens[i] = NULL;
	return (tokens);
}
