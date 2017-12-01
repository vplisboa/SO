#include <unistd.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <linux/limits.h>

#define tamanhoBuffer 1024
#define tamanhoToken 64
#define limitadorToken " \t\r\n\a"

int comandoCD(char **args);
int comandoHelp(char **args);
int comandoExit(char **args);
int comandoDir(char **args);
int comandoEnviron(char **args);
int ehBackground = 0;
int (*listaFuncoes[]) (char **) = {&comandoCD,&comandoHelp,&comandoExit,&comandoDir,&comandoEnviron};
char *lerLinha(void);
char **separarLinha(char *linha);
char *caminho;
char *listaComandos[] = {"cd","help","exit","dir","environ"};
extern char **environ;

FILE* arquivo;
pid_t sid;

int main (int argc, char **argv)
{
	char linhaArquivo[256];
	caminho = (char*) malloc(sizeof(char)*tamanhoBuffer);
	if(argc > 1) //verifica se foi passado um arquivo como argumento
	{
		arquivo = fopen(argv[1],"r"); //lê o arquivo de comandos
		if(arquivo != NULL)
		{	
			while (fgets(linhaArquivo,sizeof(linhaArquivo),arquivo)) //lê o arquivo e pega os comandos
			{
				printf("comando executado: %s",linhaArquivo); //printa os comandos executados(que estavam no arquivo passado como parâmetro)
				executar(separarLinha(linhaArquivo)); //executa cada comando da lista
    	}
		}
		fclose(arquivo);
	}

	loopPrincipal(); //executa o loop principal da shell
	return EXIT_SUCCESS;
}

void loopPrincipal(void)
{
	char *linha;
	char **comando;
	int status;
	caminho = getenv("PWD");
	do {
		fflush(stdout);
   	getcwd(caminho,1024);
		printf("<%s/myshell>>  ",caminho);
		linha = lerLinha();
		comando = separarLinha(linha);
		status = executar(comando);
		free(linha);
		free(comando);
	}
	while(status); //enquanto não retornar 0, continua
}

//função que é executada quando é utilizado o comando environ
int comandoEnviron(char **args)
{
	for (char **env = environ; *env; ++env)
        printf("%s\n", *env);
	return 1;
}

//função que é executada quando é utilizado o comando CD
int comandoCD(char **args)
{
  if (args[1] == NULL) //verifica o argumento passado
	{
		fflush(stdout);
    printf("Argumento Inesperado \n");
  }

	else
	{
    if (chdir(args[1]) != 0)
		{
			fflush(stdout);
      printf("Diretório Inexistente\n");
    }
  }
	caminho = getenv("PWD"); //atualiza a variável caminho, que é utilizada no loop principal, que contém o path atual
  return 1;
}

//função que é executada quando é utilizado o comando dir
int comandoDir(char **args)
{
	struct dirent **listaArquivos; //cria struct que tem as descrições dos arquivos
	int totalArquivosDiretorio;
	if (args[1] != NULL)
	{
		totalArquivosDiretorio = scandir(args[1], &listaArquivos, NULL, alphasort); //recupera total de arquivos no diretorio passado
	}
	else
	{
		totalArquivosDiretorio=scandir(".",&listaArquivos,NULL,alphasort); //se nenhum diretório for passado como parâmetro, recupera do diretório atual
	}
	
	if(totalArquivosDiretorio < 0)
	{
		exit(EXIT_FAILURE);
	}
	else
	{
		int temp = 0;
		while (temp < totalArquivosDiretorio)
		{
			if((listaArquivos[temp]->d_name)[0] != '.') //remove os arquivos que começam com "."
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
  int contador;
	int numeroComandos = sizeof(listaComandos) / sizeof(char *);
  printf("Os comandos disponíveis são:\n");

  for (contador = 0; contador < numeroComandos; contador++)
	{
    printf("  %s\n", listaComandos[contador]);
  }
  return 1;
}

int comandoExit(char **args)
{
  return 0;
}

char *lerLinha(void)
{
	int tamanho = tamanhoBuffer;
	int posicaoBuffer = 0;
	char *buffer = malloc(sizeof(char) * tamanho);
	int temp;

	if (!buffer)
	{
		fflush(stdout);
    exit(EXIT_FAILURE);
  }

  while (1) {
    temp = getchar();
    if (temp == EOF)
		{
			printf("\n");
			exit(0);
    }	

    if (temp == '\n')
		{
      buffer[posicaoBuffer] = '\0';
      return buffer;
    }

		else
		{
      buffer[posicaoBuffer] = temp;
    }
    posicaoBuffer++;

    if (posicaoBuffer >= tamanho)
		{
      tamanho += tamanhoBuffer;
      buffer = realloc(buffer, tamanho);
      if (!buffer)
			{
				fflush(stdout);
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
		fflush(stdout);
		exit(EXIT_FAILURE);
	}

	token = strtok(linha,limitadorToken);
	while(token != NULL)
	{
		tokens[posicaoBuffer] = token;

		if(strcmp("&",token) == 0)
		{
			ehBackground = 1;
			tokens[posicaoBuffer] = NULL;
		}
		else
			ehBackground = 0;

		posicaoBuffer++;
		if(posicaoBuffer >= tamanho)
		{
			tamanho += tamanhoToken;
			tokens = realloc(tokens,tamanho *sizeof(char*));
			if(!tokens)
			{
				fflush(stdout);
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
			fflush(stdout);
			printf("Comando inválido\n");
		}
		exit(EXIT_FAILURE);
	}
	else if (filho < 0)
	{
		fflush(stdout);
		printf("Comando inválido\n");
	}
	else
	{
		if(ehBackground == 0)
		{	
			do{
				waitpid(filho,&status,0);		
			} while(!WIFEXITED(status) && !WIFSIGNALED(status));
		}
	}
	return 1;
}

int executar(char **args)
{
	int numeroComandos = sizeof(listaComandos) / sizeof(char *);
	if(args[0] == NULL)
	{
		return 1;
	}

	for (int i = 0;i<numeroComandos;i++)
	{
		if(strcmp(args[0],listaComandos[i]) == 0)
		{
			return (*listaFuncoes[i])(args);
		}		
	}
	return 	executaFork(args);
}
