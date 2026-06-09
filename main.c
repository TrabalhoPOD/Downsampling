#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "downsampling.h"

/*
============================================================================
PARA O RELATÓRIO: FUNÇÃO MAIN E INTEGRAÇÃO

Ideia Principal:
    A função main() é o ponto de entrada e orquestrador do programa.
    Ela é responsável por receber os parâmetros do usuário via linha de
    comando, validar as entradas, chamar as funções de downsampling e
    upsampling na ordem correta, e exibir as métricas de resultado.

Argumentos (argc/argv):
    O programa aceita os seguintes argumentos obrigatórios:
        argv[1] — caminho do arquivo de entrada (.pgm ou .ppm)
        argv[2] — fator de redução (número inteiro positivo, ex: 2, 4, 8)
        argv[3] — caminho do arquivo de saída compactado
    E um argumento opcional:
        argv[4] — método de upsampling: "nn" (Nearest-Neighbor) ou "bilinear"
                  Padrão: "nn"

Etapas de Execução:
    1. Validar o número de argumentos e o fator de redução.
    2. Carregar a imagem original com ler_imagem().
    3. Realizar o downsampling com downsample().
    4. Salvar a imagem compactada com escrever_imagem().
    5. Calcular e exibir a taxa de compactação.
    6. Realizar o upsampling com o método escolhido.
    7. Salvar a imagem reconstituída.
    8. Calcular e exibir as métricas de qualidade (MSE e PSNR).
    9. Liberar toda a memória alocada.
============================================================================
*/

/* Constrói o nome do arquivo reconstituído inserindo
   "_reconstituido" antes da extensão. */
static void nome_reconstituido(const char *caminho_compactado, char *saida, int max)
{
    const char *ponto = strrchr(caminho_compactado, '.');
    if (ponto)
    {
        int base = (int)(ponto - caminho_compactado);
        snprintf(saida, max, "%.*s_reconstituido%s", base, caminho_compactado, ponto);
    }
    else
    {
        snprintf(saida, max, "%s_reconstituido", caminho_compactado);
    }
}

static void imprimir_uso(const char *prog)
{
    printf("Uso: %s <entrada> <fator> <saida_compactada> [metodo_upsampling]\n\n", prog);
    printf("  <entrada>             Arquivo de imagem .pgm (P2) ou .ppm (P3)\n");
    printf("  <fator>               Fator inteiro de reducao (ex: 2, 4, 8)\n");
    printf("  <saida_compactada>    Nome do arquivo compactado a ser gerado\n");
    printf("  [metodo_upsampling]   \"nn\" (nearest-neighbor, padrao) ou \"bilinear\"\n\n");
    printf("Exemplo:\n");
    printf("  %s imagem.ppm 4 compactada.ppm bilinear\n", prog);
}

int main(int argc, char *argv[])
{
    /* ---- 1. Validação dos argumentos ---- */
    if (argc < 4)
    {
        imprimir_uso(argv[0]);
        return 1;
    }

    const char *caminho_entrada    = argv[1];
    int         fator              = atoi(argv[2]);
    const char *caminho_compactado = argv[3];
    const char *metodo             = (argc >= 5) ? argv[4] : "nn";

    if (fator <= 0)
    {
        printf("Erro: o fator de reducao deve ser um inteiro positivo (recebido: %s).\n", argv[2]);
        return 1;
    }

    if (strcmp(metodo, "nn") != 0 && strcmp(metodo, "bilinear") != 0)
    {
        printf("Erro: metodo invalido '%s'. Use \"nn\" ou \"bilinear\".\n", metodo);
        return 1;
    }

    printf("=== Downsampling ===\n");
    printf("Entrada:    %s\n", caminho_entrada);
    printf("Fator:      %d\n", fator);
    printf("Compactado: %s\n", caminho_compactado);
    printf("Upsampling: %s\n\n", metodo);

    /* ---- 2. Leitura da imagem original ---- */
    Image *original = ler_imagem(caminho_entrada);
    if (!original)
        return 1;

    printf("Imagem original: %dx%d  canais=%d\n",
           original->largura, original->altura, original->canais);

    /* ---- 3. Downsampling  ---- */
    Image *reduzida = downsample(original, fator);
    if (!reduzida)
    {
        printf("Erro: falha no downsampling.\n");
        liberar_imagem(original);
        return 1;
    }

    printf("Imagem reduzida: %dx%d\n", reduzida->largura, reduzida->altura);

    /* ---- 4. Salvar imagem compactada ---- */
    escrever_imagem(caminho_compactado, reduzida);
    printf("Arquivo compactado salvo em: %s\n", caminho_compactado);

    /* ---- 5. Taxa de compactação ---- */
    calcular_taxa(caminho_entrada, caminho_compactado);

    /* ---- 6. Upsampling / reconstituição ---- */
    Image *reconstituida = NULL;

    if (strcmp(metodo, "bilinear") == 0)
        reconstituida = upsampling_bilinear(reduzida, original->largura, original->altura);
    else
        reconstituida = upsampling_nearest_neighbor(reduzida, original->largura, original->altura);

    if (!reconstituida)
    {
        printf("Erro: falha no upsampling.\n");
        liberar_imagem(reduzida);
        liberar_imagem(original);
        return 1;
    }

    /* ---- 7. Salvar imagem reconstituída ---- */
    char caminho_reconstituido[512];
    nome_reconstituido(caminho_compactado, caminho_reconstituido, sizeof(caminho_reconstituido));
    escrever_imagem(caminho_reconstituido, reconstituida);
    printf("Imagem reconstituida salva em: %s\n\n", caminho_reconstituido);

    /* ---- 8. Métricas de qualidade ---- */
    exibir_qualidade(original, reconstituida);

    /* ---- 9. Liberar memória ---- */
    liberar_imagem(original);
    liberar_imagem(reduzida);
    liberar_imagem(reconstituida);

    return 0;
}
