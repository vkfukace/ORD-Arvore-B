#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// TODO: insere_na_arvore
//       arrumar problema da linha vazia retornar 0 no pega_chave
//       no divide_pagina o ceil() n funciona com debugger

#define NULO -1

#define FIM_ARQUIVO -1

#define M 5 // Ordem da árvore B - Não pode ser menor que 2

#define MAX_DIGITO_CHAVE 3 // Número máximo de digitos de uma chave. \
                           // EX: MAX_DIGITO_CHAVE = 3 então valor máximo da chave é 999

// Defines de retorno na função divide_pagina
#define ERRO -1
#define SEM_PROMOCAO 0
#define PROMOCAO 1

typedef struct
{
    int num_chaves;
    int chaves[M - 1];
    int filhos[M];
} pagina;

// Guarda uma chave de f em chave
// Requer que MAX_DIGITO_CHAVE seja tamanho necessário para a chave
void pega_chave(FILE *f, int *chave)
{
    int i = 0;
    char chave_str[MAX_DIGITO_CHAVE + 1];
    char ch = fgetc(f);

    while (ch != EOF && ch != '\n')
    {
        chave_str[i] = ch;
        i++;
        ch = fgetc(f);
    }
    chave_str[i] = '\0';
    *chave = atoi(chave_str);
}

// Preenche pag com NULO e define o numero de chaves em 0
void esvaziar_pagina(pagina *pag)
{
    int i;

    pag->num_chaves = 0;

    for (i = 0; i < M - 1; i++)
        pag->chaves[i] = NULO;

    for (i = 0; i < M; i++)
        pag->filhos[i] = NULO;
}

// Lê uma página encontrada em f no rrn dado e salva em pag
void ler_pagina(FILE *f, pagina *pag, int rrn)
{
    int i;
    fseek(f, sizeof(int) + sizeof(pagina) * rrn, SEEK_SET); // vai até o rrn dado

    if (feof(f))
    {
        printf("Erro: Fim do arquivo");
        return;
    }

    fread(&pag->num_chaves, sizeof(int), 1, f);

    for (i = 0; i < M - 1; i++)
        fread(&pag->chaves[i], sizeof(int), 1, f);

    for (i = 0; i < M; i++)
        fread(&pag->filhos[i], sizeof(int), 1, f);
}

// Escreve pag em f no rrn dado
// rrn pode ser o rrn da pagina ou FIM_ARQUIVO
// Retorna o rrn em que pag foi inserido
int escrever_pagina(FILE *f, pagina pag, int rrn)
{
    long int rrn_retorno;
    if (rrn != FIM_ARQUIVO)
    {
        fseek(f, sizeof(int) + sizeof(pagina) * rrn, SEEK_SET); // vai até o rrn dado
        rrn_retorno = rrn;
    }
    else
    {
        fseek(f, 0, SEEK_END);
        rrn_retorno = ftell(f);
        rrn_retorno = (rrn_retorno - sizeof(int)) / sizeof(pagina);
    }

    fwrite(&pag, sizeof(pagina), 1, f);

    return rrn_retorno;
}

// Busca a chave em pag
// pos é uma variável de retorno que contém a posição da chave na página se for encontrado
//      se não for encontrado, pos contém a posição em que chave estaria
// Retorna 1 se for encontrado
//         0 se não for encontrado
int busca_na_pagina(int chave, pagina pag, int *pos)
{
    // enquanto a chave vista estiver na página e não for nula
    for (*pos = 0; *pos < M - 1 && pag.chaves[*pos] != NULO; *pos = *pos + 1)
    {
        if (pag.chaves[*pos] == chave)
            return 1;
        if (pag.chaves[*pos] > chave)
            return 0;
    }
    return 0;
}

// Insere chave e o filho direito associado à chave em pag
// Requer que haja espaço em pag para inserir
void insere_na_pagina(pagina *pag, int chave, int filho)
{
    int i, j;
    for (i = M - 2; i > 0; i--)
    {
        if (pag->chaves[i - 1] != NULO && chave > pag->chaves[i - 1])
            break;
        else
            pag->chaves[i] = pag->chaves[i - 1];
    }
    pag->chaves[i] = chave;

    for (j = M - 1; j > i + 1; j--)
    {
        pag->filhos[j] = pag->filhos[j - 1];
    }
    pag->filhos[j] = filho;

    pag->num_chaves = pag->num_chaves + 1;
}

// Cria a pagina em overflow
// Requer que pag.chaves esteja cheia
// Assume que chaves_over tem tamanho M
//            filhos_over tem tamanho M+1
void cria_overflow(pagina pag, int chave, int filho, int chaves_over[], int filhos_over[])
{
    int i = 0, j = 0, aux, flag = 0;

    // Cria o chaves_over
    while (i < M)
    {
        // Se tem elementos em pag.chaves e
        // ou a chave atual é menor ou a chave que causa overflow já foi inserida
        if (j < M - 1 && (chave > pag.chaves[j] || flag))
        {
            chaves_over[i] = pag.chaves[j];
            i++;
            j++;
        }
        else
        {
            chaves_over[i] = chave;
            i++;
            aux = i; // aux recebe a posição do filho
            flag = 1;
        }
    }

    i = 0;
    j = 0;
    // Cria o filhos_over
    while (i < M + 1)
    {
        if (i != aux)
        {
            filhos_over[i] = pag.filhos[j];
            i++;
            j++;
        }
        else
        {
            filhos_over[i] = filho;
            i++;
        }
    }
}

// Cria uma nova raiz com as informações dadas
void cria_nova_raiz(pagina *nova_raiz, int chave, int filho_e, int filho_d)
{
    esvaziar_pagina(nova_raiz);
    nova_raiz->chaves[0] = chave;
    nova_raiz->filhos[0] = filho_e;
    nova_raiz->filhos[1] = filho_d;
    nova_raiz->num_chaves = 1;
}

// Copia o subvetor v1[inicio ... fim] em v2 e preenche o restante com NULO
void copia_vetor(int v1[], int inicio, int fim, int v2[], int tam_v2)
{
    int i, j = 0;
    for (i = inicio; i < fim; i++)
    {
        v2[j] = v1[i];
        j++;
    }

    while (j < tam_v2)
    {
        v2[j] = NULO;
        j++;
    }
}

// Divide a pagina em overflow em 2
// A metade esquerda fica em pag, e a direita em novapag
// chave_pro retorna a chave que vai ser promovida
void divide_pagina(int chave, int filho, int *chave_pro, pagina *pag, pagina *novapag)
{
    int meio = M / 2;
    double M_double = M; // Necessário para ceil()

    int chaves_over[M], filhos_over[M + 1];
    cria_overflow(*pag, chave, filho, chaves_over, filhos_over);

    *chave_pro = chaves_over[meio];

    copia_vetor(chaves_over, 0, meio, pag->chaves, M - 1);
    copia_vetor(chaves_over, meio + 1, M, novapag->chaves, M - 1);
    copia_vetor(filhos_over, 0, meio + 1, pag->filhos, M);
    copia_vetor(filhos_over, meio + 1, M + 1, novapag->filhos, M);

    pag->num_chaves = meio;
    // novapag->num_chaves = (int)(ceil(M_double / 2) - 1);
    novapag->num_chaves = meio; // debugger n funciona com ceil()
}

// Insere chave na árvore B no arquivo btree
// Se a página em que a chave for inserida já conter M chaves, será necessário divisão e promoção
//
// raiz, chave_pro e filho_d_pro são variáveis de retorno
// raiz contém a raiz da árvore
// Se a inserção da chave resultar em divisão e promoção, CHAVE_PRO conterá a chave promovida
// Quando houver uma CHAVE_PRO, FILHO_D_PRO conterá o ponteiro para o seu filho direito
//
// Retorna ERRO         se houver promoção na árvore
//         SEM_PROMOCAO se não houver promoção na árvore
//         PROMOCAO     se a chave a ser inserida já se encontra na árvore
int insere_na_arvore(FILE *btree, int rrn_atual, int chave, int *raiz, int *filho_d_pro, int *chave_pro)
{
    // https://classroom.google.com/u/1/c/MTQxMzczNDcxNDI4/m/MjEyNDMxODMwMzUw/details       pag 13

    //     Variáveis locais importantes da função insere:
    // – PAG: página que está sendo examinada
    // – NOVAPAG: nova página que é criada caso ocorra uma divisão
    // – POS: posição da chave em PAG, se ela estiver lá; caso contrário, a posição em
    // que deve ser inserida (ou a posição do ponteiro para a próxima página)
    // – RRN_PRO: recebe o valor do RRN da página promovida para o nível corrente
    // (via FILHO_D_PRO)
    //      • Se uma divisão ocorre no nível imediatamente inferior, RRN_PRO contém o RRN da
    //      nova página criada durante a divisão. RRN_PRO é o filho direito que deve ser
    //      inserido junto com CHV_PRO em PAG
    // – CHV_PRO: recebe o valor da chave promovida para o nível corrente
    // (via CHAVE_PRO)

    // TODO: como cria nova pagina aqui???

    pagina pag, novapag;
    int pos = NULO, resultado, retorno;

    if (rrn_atual == NULO) // se não está mais em um nó da arvore
    {
        *chave_pro = chave;
        *filho_d_pro = NULO;
        // A primeira tentativa de inserção vai ser tratada como uma promoção
        return PROMOCAO;
    }
    else // se está em um nó da árvore
    {
        ler_pagina(btree, &pag, rrn_atual);
        resultado = busca_na_pagina(chave, pag, &pos); // pos guarda qual filho a chave deveria estar
    }

    if (resultado == 1) // Se a chave foi encontrada, manda mensagem de erro
    {
        printf("Chave %d foi repetida\n", chave);
        return ERRO;
    }

    // Insere recursivamente no filho pos e guarda qual o resultado
    retorno = insere_na_arvore(btree, pag.filhos[pos], chave, raiz, filho_d_pro, chave_pro);

    // Se houver inserção sem promoção ou houver erro não há nada mais a se fazer
    if (retorno == SEM_PROMOCAO || retorno == ERRO)
        return retorno;
    else // Se houver promoção
    {
        int chv_pro = *chave_pro;
        int rrn_pro = *filho_d_pro;

        if (pag.num_chaves < M - 1) // Se houver espaço em pag para inserir chave
        {
            insere_na_pagina(&pag, chv_pro, rrn_pro);
            escrever_pagina(btree, pag, rrn_atual);
            return SEM_PROMOCAO;
        }
        else // Se não houver espaço
        {
            divide_pagina(chv_pro, rrn_pro, chave_pro, &pag, &novapag);
            escrever_pagina(btree, pag, rrn_atual);
            // filho_d_pro recebe o rrn da pagina escrita no final do arquivo
            *filho_d_pro = escrever_pagina(btree, novapag, FIM_ARQUIVO);

            if (rrn_atual == *raiz)
            {
                // tratamento especial para raiz
                pagina nova_raiz;
                cria_nova_raiz(&nova_raiz, *chave_pro, rrn_atual, *filho_d_pro);
                *raiz = escrever_pagina(btree, nova_raiz, FIM_ARQUIVO);
            }
            return PROMOCAO;
        }
    }
}

int main(int argc, char **argv)
{
    if (argc < 2) // Menos argumentos que o necessário
    {
        fprintf(stderr, "Argumentos Incorretos!\n");
        fprintf(stderr, "Modo de uso:\n");
        fprintf(stderr, "$ %s (-c nome_arquivo_chaves | -p)\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "-c") == 0) // Modo de Criação de Árvore
    {
        if (argc < 3) // Menos argumentos que o necessário
        {
            fprintf(stderr, "Argumentos Incorretos!\n");
            fprintf(stderr, "Modo de uso:\n");
            fprintf(stderr, "$ %s (-c nome_arquivo_chaves | -p)\n", argv[0]);
            exit(EXIT_FAILURE);
        }

        FILE *arq_chave;
        arq_chave = fopen(argv[2], "r");
        FILE *btree;
        btree = fopen("btree.dat", "w+b");

        printf("Modo de importacao ativado ... nome do arquivo de chaves = %s\n", argv[2]);

        int raiz = 0, filho_d_pro, chave_pro;
        int chave;
        pagina pag;

        // Escreve a primeira página vazia
        fwrite(&raiz, sizeof(int), 1, btree);
        esvaziar_pagina(&pag);
        escrever_pagina(btree, pag, 0);

        // =================================================================
        // testes
        insere_na_pagina(&pag, 1, 2);
        insere_na_pagina(&pag, 10, 4);
        insere_na_pagina(&pag, 5, 3);
        insere_na_pagina(&pag, 7, 5);

        pagina novapag;
        int chavepro;
        divide_pagina(15, 6, &chavepro, &pag, &novapag);

        // // teste=======================================
        // int i;
        // for (i = 0; i < M - 1; i++)
        // {
        //     printf("%d /", pag.chaves[i]);
        // }
        // printf("\n");

        // for (i = 0; i < M; i++)
        // {
        //     printf("%d /", pag.filhos[i]);
        // }
        // printf("(%d)\n\nasdfasdfaf\n\n", pag.num_chaves);

        // for (i = 0; i < M - 1; i++)
        // {
        //     printf("%d /", novapag.chaves[i]);
        // }
        // printf("\n");

        // for (i = 0; i < M; i++)
        // {
        //     printf("%d /", novapag.filhos[i]);
        // }
        // printf("(%d)\n", novapag.num_chaves);
        // //=============================================

        // busca_na_pagina(6, pag, &chave);
        // busca_na_pagina(21, pag, &chave);
        // busca_na_pagina(1, pag, &chave);
        // busca_na_pagina(0, pag, &chave);
        // busca_na_pagina(11, pag, &chave);

        // // teste ler_pagina
        // escrever_pagina(btree, pag, 1); // teste
        // pag.num_chaves = 45;
        // ler_pagina(btree, &pag, 0);

        // escrever_pagina(btree, pag, FIM_ARQUIVO);

        // // ===================================================================

        while (!feof(arq_chave))
        {
            pega_chave(arq_chave, &chave);
            insere_na_arvore(btree, raiz, chave, &raiz, &filho_d_pro, &chave_pro);
            fseek(btree, 0, SEEK_SET);
            fwrite(&raiz, sizeof(int), 1, btree);
        }

        fclose(btree);
        fclose(arq_chave);
    }
    else if (strcmp(argv[1], "-e") == 0) // Modo de Impressão de Árvore
    {
        FILE *btree;
        btree = fopen("btree.dat", "rb");

        if (btree == NULL)
        {
            fprintf(stderr, "Erro: Arquivo \"btree.dat\" nao existe\n");
            exit(EXIT_FAILURE);
        }

        printf("Modo de Impressão da Árvore B ativado\n\n");

        pagina pag;
        char op, operando[150];

        // while (!feof(operacoes))
        // {
        //     op = fgetc(operacoes);
        //     fseek(operacoes, 1, SEEK_CUR);
        //     pega_linha(operacoes, operando, 150);

        //     switch (op)
        //     {
        //     case 'b':
        //         printf("Busca pelo registro de chave \"%s\"\n", operando);
        //         if (busca(btree, operando)) // Se achou
        //         {
        //             fread(&tam_str, sizeof(short), 1, btree);
        //             str = malloc(sizeof(char) * tam_str);
        //             pega_linha(btree, str, tam_str);
        //             //arrumarsaida(str);
        //             printf("%s (%ld bytes)\n", str, strlen(str));
        //             free(str);
        //         }
        //         else
        //         {
        //             printf("Erro: registro nao encontrado!\n");
        //         }
        //         break;

        //     case 'r':
        //         printf("Remocao do registro de chave \"%s\"\n", operando);
        //         if (remover(btree, operando)) // Se achou
        //         {
        //             fread(&tam_str, sizeof(short), 1, btree);
        //             printf("Registro removido! (%d bytes)\n", tam_str);
        //             fseek(btree, -(sizeof(short)), SEEK_CUR);
        //             offset = ftell(btree);
        //             printf("Posicao: offset = %d bytes (0x%X)\n", offset, offset);
        //         }
        //         else
        //         {
        //             printf("Erro: registro nao encontrado!\n");
        //         }
        //         break;

        //     case 'i':;
        //         int resultado;
        //         char chave[4]; // Tinha que ter 1 a mais de tamanho por erros desconhecidos
        //         pega_chave(operando, chave, 4);
        //         printf("Insercao do registro de chave \"%s\" (%ld bytes)\n", chave, strlen(operando));

        //         short espaco, tam_sobra, tam_reg;
        //         int offset_sobra;
        //         resultado = insere(btree, operando, &espaco, &offset, &tam_reg, &offset_sobra, &tam_sobra);

        //         switch (resultado)
        //         {
        //         case -1:
        //             printf("Local: fim do arquivo\n");
        //             break;
        //         case 0:
        //             printf("Local: offset = %d bytes (0x%X)\n", offset, offset);
        //             printf("Tamanho do espaco: %d bytes\n", espaco);

        //             break;
        //         case 1:
        //             printf("Local: offset = %d bytes (0x%X)\n", offset, offset);
        //             printf("Tamanho do espaco: %d bytes\n", espaco);
        //             printf("Tamanho da sobra: %d bytes\n", tam_sobra);
        //             printf("Offset da sobra: %d bytes (0x%X)\n", offset_sobra, offset_sobra);
        //             break;
        //         }
        //         break;
        //     }
        //     printf("\n");
        // }
        fclose(btree);
    }
    else
    {
        fprintf(stderr, "Opcao \"%s\" nao suportada!\n", argv[1]);
    }

    return 0;
}