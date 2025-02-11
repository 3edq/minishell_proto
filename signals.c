#include "minishell.h"

void	sigint_handler(int signo)
{
    (void)signo;
	printf("\nminishell>　");
}

void	sigquit_handler(int signo)
{
    (void)signo;
}

void	handle_signals(void)
{
    signal(SIGINT, sigint_handler);
    signal(SIGQUIT, sigquit_handler);
}
