#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

void	exit_fatal()
{
	write(2, "error: fatal\n", 13);
	exit(1);
}

void	ft_perror(char *error, char *str)
{
	while (error && *error)
		write(2, error++, 1);
	while (str && *str)
		write(2, str++, 1);
	write(2, "\n", 1);
}

void	ft_execve(char **str, char **env, int i)
{
	str[i] = NULL;
	if (execve(str[0], str, env) == -1)
	{
		ft_perror("error: cannot execute ", str[0]);
		exit(1);
	}
}

int	main(int ac, char **av, char **env)
{
	int	i, j, prev_fd, pid, child_count;
	int	pipe_fd[2];

	i = 1;
	if ((prev_fd = dup(0)) == -1)
		exit_fatal();
	child_count = 0;
	while (i < ac)
	{
		j = 0;
		while (av[i + j] && strcmp(av[i + j], ";") != 0 && strcmp(av[i + j], "|") != 0)
			j++;
		if (strcmp(av[i], "cd") == 0)
		{
			if (j != 2)
				ft_perror("error: cd: bad arguments", NULL);
			else if (chdir(av[i + 1]) != 0)
				ft_perror("error: cd: cannot change directory to ", av[i + 1]);
		}
		else if (av[i + j] && strcmp(av[i + j], "|") == 0)
		{
			if (pipe(pipe_fd) == -1)
				exit_fatal();
			if ((pid = fork()) == -1)
				exit_fatal();
			if (pid == 0)
			{
				if (dup2(pipe_fd[1], 1) == -1)
					exit_fatal();
				close(pipe_fd[0]);
				close(pipe_fd[1]);
				if (dup2(prev_fd, 0) == -1)
					exit_fatal();
				close(prev_fd);
				ft_execve(&av[i], env, j);
			}
			close(prev_fd);
			close(pipe_fd[1]);
			prev_fd = pipe_fd[0];
			child_count++;
		}
		else if (j != 0 && (!av[i + j] || strcmp(av[i + j], ";") == 0))
		{
			if ((pid = fork()) == -1)
				exit_fatal();
			if (pid == 0)
			{
				if (dup2(prev_fd, 0) == -1)
					exit_fatal();
				close(prev_fd);
				ft_execve(&av[i], env, j);
			}
			close(prev_fd);
			child_count++;
			while (child_count)
			{
				waitpid(-1, NULL, 0);
				child_count--;
			}
			if ((prev_fd = dup(0)) == -1)
				exit_fatal();
			child_count = 0;
		}
		i = i + j + 1;
	}
	close(prev_fd);
	return (0);
}
