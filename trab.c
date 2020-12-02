#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define M 5
#define NULO -1
#define MAX_DIGITO_CHAVE 3

typedef struct
{
    int chaves[M - 1];
    int filhos[M];
    int num_chaves;
} pagina;

// ###################################################################################################################
// ###################################################################################################################

// Arruma o registro para saída retirando algum possivel lixo
// Assume que o registro tenha 7 '|'
void arrumarsaida(char *str)
{
    int controle = 0;
    int tam = strlen(str);
    int i = 0;

    while (i < tam)
    {
        if (str[i] == '|')
        {
            controle++;
        }
        if (controle == 7)
        {
            str[i + 1] = '\0';
        }
        i++;
    }
}

// Função de teste
// Printa a LED inteira
void print_led(FILE *dados)
{
    int offset;
    short tam;
    fseek(dados, 0, SEEK_SET);
    fread(&offset, sizeof(int), 1, dados);
    printf("LED = | ");
    while (offset != -1)
    {
        printf("%d(0x%X)", offset, offset);
        fseek(dados, offset, SEEK_SET);
        fread(&tam, sizeof(short), 1, dados);
        fseek(dados, 1, SEEK_CUR);
        printf("-%hd> ", tam);
        fread(&offset, sizeof(int), 1, dados);
    }
    printf("-1 |\n");
}

// Retorna o tamanho do registro
// Conta até chegar ao fim da linha
int tamanhoreg(FILE *f)
{
    int i = 0;
    char ch = fgetc(f);
    while (!feof(f) && ch != '\n')
    {
        if (ch != '\r')
            i++;
        ch = fgetc(f);
    }
    return i;
}

// Guarda um registo de f em str
// O registro deve ter numero de campos igual a campo_restante
// Requer que str já tenha o tamanho necessário para o registro
void pegareg(FILE *f, char *str)
{
    int i = 0;
    int campo_restante = 7;
    char ch = fgetc(f);

    while (campo_restante > 0)
    {
        if (ch == '|')
        {
            campo_restante--;
        }
        str[i] = ch;
        i++;
        ch = fgetc(f);
    }

    if (ch == '\r') // causava erros na formatação
    {
        ch = fgetc(f);
    }
}

// Guarda dados de f em str até a quebra de linha
// Não inclui '\n', e coloca um '\0' no final
void pega_linha(FILE *f, char *str, int tam)
{
    int i;
    char c;

    c = fgetc(f);
    for (i = 0; i < tam; i++)
    {
        if (feof(f) || c == '\n')
        {
            str[i] = '\0';
        }
        else if (c == '\r') // se não tiver isso aqui o strlen sai com 1 a mais
        {
            str[i] = '\0';
            c = fgetc(f);
        }
        else
        {
            str[i] = c;
            c = fgetc(f);
        }
    }
}

// Guarda a chave de str em ch
// Requer que str tenha formato de registro e chave tenha tamanho adequado
void pega_chave(char *str, char *ch, int tam_ch)
{
    int i = 0;

    while (str[i] != '|')
    {
        ch[i] = str[i];
        i++;
    }
    while (i < tam_ch)
    {
        ch[i] = '\0';
        i++;
    }
}

// Guarda uma chave de f em str
// Retorna o tamanho de str+1
// Requer que o ponteiro de arquivo esteja posicionado corretamente
int pega_chave_arq(FILE *f, char *str)
{
    char c;
    int i = 0;

    c = fgetc(f);
    while (c != '|' && c != '*')
    {
        str[i] = c;
        i++;
        c = fgetc(f);
    }
    return i + 1;
}

// Preenche uma string com '\0'
void limpa_string(char *str, int tam)
{
    int i;
    for (i = 0; i < tam; i++)
    {
        str[i] = '\0';
    }
}

// Busca o registro com a chave dada no arquivo de dados
// Se achar retorna 1 e move o ponteiro de dados para o começo do registro
// Caso contrário, retorna 0
int busca(FILE *dados, char *chave)
{
    short int offset = 0;
    int tam_chave = strlen(chave);
    char aux_chave[3]; // malloc e free não estavam funcionando corretamente
    int tam_aux_chave;

    fseek(dados, sizeof(int), SEEK_SET);
    char c = fgetc(dados);

    while (c != EOF) // enquanto estiver no arquivo
    {
        fseek(dados, -sizeof(char), SEEK_CUR);
        fread(&offset, sizeof(short int), 1, dados);
        tam_aux_chave = pega_chave_arq(dados, aux_chave);
        if (strcmp(chave, aux_chave) == 0)
        {
            // retorna para o começo do registro
            fseek(dados, -(tam_chave + sizeof(char) + sizeof(short)), SEEK_CUR);
            return 1;
        }
        fseek(dados, offset - tam_aux_chave, SEEK_CUR);
        limpa_string(aux_chave, 3);
        c = fgetc(dados);
    }
    return 0;
}

// Insere offset no topo da LED, movendo o antigo topo para a segunda posição
// Volta o ponteiro de arquivo para o começo do registro após a operação
int insere_topo_led(FILE *dados, int offset)
{
    int topo_antigo;
    char ast = '*';
    char pipe = '|';

    // Insere a posição do elemento a ser removido no cabeçalho
    fseek(dados, 0, SEEK_SET);
    fread(&topo_antigo, sizeof(int), 1, dados);
    fseek(dados, 0, SEEK_SET);
    fwrite(&offset, sizeof(int), 1, dados);

    // Remove logicamente o registro colocando o antigo cabeçalho no lugar
    fseek(dados, offset + sizeof(short), SEEK_SET);
    fwrite(&ast, sizeof(char), 1, dados);
    fwrite(&topo_antigo, sizeof(int), 1, dados);
    fwrite(&pipe, sizeof(char), 1, dados);
    fseek(dados, offset, SEEK_SET);
}

// Remove o registro com a chave dada
// Se o registro não for encontrado retorna 0,
// Se for encontrado, remove o registro logicamente,
// insere o registro no topo da LED e retorna 1.
int remover(FILE *dados, char *chave)
{
    int offset;

    fseek(dados, 0, SEEK_SET);

    if (busca(dados, chave))
    {
        offset = ftell(dados);
        insere_topo_led(dados, offset);
        fseek(dados, offset, SEEK_SET);
        return 1;
    }
    else
        return 0;
}

// Busca linearmente na LED por um espaço maior ou igual a tam
// Retorna o offset do primeiro espaço compatível
// Se não houver espaço compatível, retorna -1
int busca_led(FILE *dados, short tam)
{
    int offset;
    short tam_busca;
    int achou = 0;
    fseek(dados, 0, SEEK_SET);
    fread(&offset, sizeof(int), 1, dados);

    while (!achou && offset != -1)
    {
        fseek(dados, offset, SEEK_SET);
        fread(&tam_busca, sizeof(short), 1, dados);
        if (tam_busca >= tam)
        {
            achou = 1;
        }
        else
        {
            fseek(dados, sizeof(char), SEEK_CUR);
            fread(&offset, sizeof(int), 1, dados);
        }
    }
    return offset;
}

// Remove chave da LED, ligando o item anterior ao posterior da lista
// Chave deve existir na lista
void remove_led(FILE *dados, int chave)
{
    int offset_antes = 0, prox;
    fseek(dados, 0, SEEK_SET);
    fread(&prox, sizeof(int), 1, dados);

    while (prox != chave) // Vai até o anterior
    {
        prox += sizeof(short) + sizeof(char);
        fseek(dados, prox, SEEK_SET);
        fread(&prox, sizeof(int), 1, dados);
    }
    fseek(dados, -sizeof(int), SEEK_CUR);
    offset_antes = ftell(dados); // Grava o offset do anterior

    //printf("*** ant:%d ", offset_antes);

    // Avança até o posterior
    prox += sizeof(short) + sizeof(char);
    fseek(dados, prox, SEEK_SET);
    //printf("agr:%d ", prox);
    fread(&prox, sizeof(int), 1, dados);
    fseek(dados, prox, SEEK_SET);
    //printf("dps:%d ***\n", prox);

    fseek(dados, offset_antes, SEEK_SET);
    fwrite(&prox, sizeof(int), 1, dados);
}

// Insere reg na primeira posição compatível na LED
// Se não hover espaço, insere no final do arquivo
// Se houver, insere, atualiza a LED, e se a sobra de espaço for maior que 50,
// reinsere a sobra na led.
//
// espaco recebe o tamanho do espaço antes da operação de inserção.
// offset_reg recebe o offset do local de inserção de reg.
// tam_reg recebe o tamanho do registro inserido.
// offset_sobra recebe o offset da sobra que foi reinserida.
// tam_sobra recebe o tamanho da sobra que foi reinserida.
//
// Retorna -1 se a inserção for no final do arquivo,
//          0 se não houver reinserção na LED,
//          1 se houver reinserção na LED.
int insere(FILE *dados, char *reg, short *espaco, int *offset_reg, short *tam_reg, int *offset_sobra, short *tam_sobra)
{
    *tam_reg = strlen(reg);
    int prim_espaco_disp = busca_led(dados, *tam_reg);

    if (prim_espaco_disp == -1)
    {
        // Escreve no final do arquivo
        fseek(dados, 0, SEEK_END);
        *offset_reg = ftell(dados);
        fwrite(tam_reg, sizeof(short int), 1, dados);
        fwrite(reg, sizeof(char), *tam_reg, dados);
        return -1;
    }
    else
    {
        remove_led(dados, prim_espaco_disp);
        fseek(dados, prim_espaco_disp, SEEK_SET);
        *offset_reg = prim_espaco_disp;
        fread(espaco, sizeof(short), 1, dados);
        *tam_sobra = *espaco - *tam_reg;
        if (*tam_sobra < 50) // Se não houver reinserção na LED
        {
            // Escreve reg e escreve um vetor vazio do tamanho da sobra
            char preenche[*tam_sobra];
            limpa_string(preenche, *tam_sobra);
            fwrite(reg, sizeof(char), *tam_reg, dados);
            fwrite(preenche, sizeof(char), *tam_sobra, dados);
            return 0;
        }
        else // Se houver reinserção na LED
        {
            // Insere reg
            fseek(dados, -sizeof(short), SEEK_CUR);
            fwrite(tam_reg, sizeof(short), 1, dados);
            fwrite(reg, sizeof(char), *tam_reg, dados);

            // Cria uma nova partição e insere na LED
            *tam_sobra -= sizeof(short);
            *offset_sobra = ftell(dados);
            fwrite(tam_sobra, sizeof(short), 1, dados);
            insere_topo_led(dados, *offset_sobra);
            return 1;
        }
    }
}

// ###################################################################################################################
// ###################################################################################################################

// Guarda uma chave de f em chave
// Requer que MAX_DIGITO_CHAVE seja tamanho necessário para a chave
void pegachave(FILE *f, int *chave)
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

// Insere chave na árvore B em btree
// Se a página em que a chave for inserida já conter M chaves, será necessário divisão e promoção
//
// chave_pro e filho_d_pro são variáveis de retorno
// Se a inserção da chave resultar em divisão e promoção, CHAVE_PRO conterá a chave promovida
// Quando houver uma CHAVE_PRO, FILHO_D_PRO conterá o ponteiro para o seu filho direito
//
// Retorna 1 se houver promoção na árvore
//         0 se não houver promoção na árvore
//         -1 se a chave a ser inserida já se encontra na árvore
int insere(FILE *btree, int rrn_atual, int chave, int filho_d_pro, int chave_pro) // talvez tem q ter raiz nos parametros
{
    // https://classroom.google.com/u/1/c/MTQxMzczNDcxNDI4/m/MjEyNDMxODMwMzUw/details       pag 13
    //
    //     Variáveis locais importantes da função insere:
    // – PAG: página que está sendo examinada
    // – NOVAPAG: nova página que é criada caso ocorra uma divisão
    // – POS: posição da chave em PAG, se ela estiver lá; caso contrário, a posição em
    // que deve ser inserida (ou a posição do ponteiro para a próxima página)
    // – RRN_PRO: recebe o valor do RRN da página promovida para o nível corrente
    // (via FILHO_D_PRO)
    // • Se uma divisão ocorre no nível imediatamente inferior, RRN_PRO contém o RRN da
    // nova página criada durante a divisão. RRN_PRO é o filho direito que deve ser
    // inserido junto com CHV_PRO em PAG

    // – CHV_PRO: recebe o valor da chave promovida para o nível corrente
    // (via CHAVE_PRO)
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
        btree = fopen("btree.dat", "wb");

        printf("Modo de importacao ativado ... nome do arquivo de chaves = %s\n", argv[2]);

        int raiz = -1;
        int chave;

        fwrite(&raiz, sizeof(int), 1, btree);
        while (!feof(arq_chave))
        {
            pegachave(arq_chave, &chave);
            printf("%d\n", chave);
            // fwrite(&tam, sizeof(short int), 1, btree);
            // fwrite(str, sizeof(char), tam, btree);
            // free(str);
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