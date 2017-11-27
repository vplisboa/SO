#include <unistd.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

#define tamanhoBuffer 1024
#define tamanhoToken 64
#define limitadorToken " \t\r\n\a"

int comandoCD(char **args);
int comandoHelp(char **args);
int comandoExit(char **args);
int comandoDir(char **args);
int comandoEnviron(char **args);

extern char **environ;

char *lerLinha(void);
char **separarLinha(char *linha);

char *listaComandos[] = {"cd","help","exit","dir","environ"};
int (*listaFuncoes[]) (char **) = {&comandoCD,&comandoHelp,&comandoExit,&comandoDir,&comandoEnviron};

int main (int argc, char **argv)
{
	loopPrincipal();
	return EXIT_SUCCESS;
}

int numeroComandos()
{
	return sizeof(listaComandos) / sizeof(char *);
}

int comandoEnviron(char **args)
{
	for (char **env = environ; *env; ++env)
        printf("%s\n", *env);
	return 1;
}

int comandoCD(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "Argumento Inesperado \n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("Diretório Inexistente");
    }
  }
  return 1;
}

int comandoDir(char **args)
{
	struct dirent **listaArquivos;
	int n;
	if (args[1] != NULL)
	{
		n = scandir(args[1], &listaArquivos, NULL, alphasort);
	}
	else
	{
		n=scandir(".",&listaArquivos,NULL,alphasort);
	}
	
	if(n < 0)
	{
		perror("scandir");
		exit(EXIT_FAILURE);
	}
	else
	{
		int temp = 0;
		while (temp < n)
		{
			if((listaArquivos[temp]->d_name)[0] != '.')
			{
				printf("%s  ",listaArquivos[temp]->d_name);
			}
			free(listaArquivos[temp]);
			temp++;
		}
		printf("\n");		
		free(listaArquivos);
	}
	return 1;
}

int comandoHelp(char **args)
{
  int i;
  printf("Os comandos disponíveis são:\n");

  for (i = 0; i < numeroComandos(); i++) {
    printf("  %s\n", listaComandos[i]);
  }
  return 1;
}

int comandoExit(char **args)
{
  return 0;
}

void loopPrincipal(void)
{
	char *linha;
	char **comando;
	int status;
	do {
		fflush(stdout);
		printf("Pandora>>  ");
		linha = lerLinha();
		comando = separarLinha(linha);
		status = executar(comando);
		free(linha);
		free(comando);
	} while(status);
}

char *lerLinha(void)
{
	int tamanho = tamanhoBuffer;
	int posicaoBuffer = 0;
	char *buffer = malloc(sizeof(char) * tamanho);
	int temp;

	if (!buffer) {
    fprintf(stderr, "Erro de alocação\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    temp = getchar();

    if (temp == EOF) {
	printf("\n");
	exit(0);
    }	
    if (temp == '\n') {
      buffer[posicaoBuffer] = '\0';
      return buffer;
    } else {
      buffer[posicaoBuffer] = temp;
    }
    posicaoBuffer++;

    if (posicaoBuffer >= tamanho) {
      tamanho += tamanhoBuffer;
      buffer = realloc(buffer, tamanho);
      if (!buffer) {
        fprintf(stderr, "Erro de alocação\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

char **separarLinha(char *linha)
{
	int tamanho = tamanhoToken;
	int posicaoBuffer = 0;
	char **tokens = malloc(tamanho * sizeof(char*));
	char *token;

	if(!tokens)
	{
		fprintf(stderr,"Erro na alocação.\n");
		exit(EXIT_FAILURE);
	}
	token = strtok(linha,limitadorToken);
	while(token != NULL)
	{
		tokens[posicaoBuffer] = token;
		posicaoBuffer++;
		if(posicaoBuffer >= tamanho)
		{
			tamanho += tamanhoToken;
			tokens = realloc(tokens,tamanho *sizeof(char*));
			if(!tokens)
			{
				fprintf(stderr,"Erro na alocação.\n");
				exit(EXIT_FAILURE);
			}
		}
		token = strtok(NULL,limitadorToken);
	}
	tokens[posicaoBuffer] = NULL;
	return tokens;
}

int executaFork(char **args)
{
	pid_t filho;
	int status;

	filho = fork();
	if (filho == 0)
	{
		if(execvp(args[0],args) == -1)
		{
			perror("Erro no filho");
		}
		exit(EXIT_FAILURE);
	}
	else if (filho < 0)
	{
		perror("Erro no filho");
	}
	else
	{
		do{
			waitpid(filho,&status,WUNTRACED);		
		} while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return 1;
}

int executar(char **args)
{
	if(args[0] == NULL)
	{
		return 1;
	}

	for (int i = 0;i<numeroComandos();i++)
	{
		if(strcmp(args[0],listaComandos[i]) == 0)
		{
			return (*listaFuncoes[i])(args);
		}		
	}
	return 	executaFork(args);
}
