#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "downsampling.h"

// Função para alocar a memória da imagem dinamicamente
/*
============================================================================
PARA O RELATÓRIO: GERENCIAMENTO DE MEMÓRIA (alocar_pixels)
    Estruturas de Dados: Utiliza ponteiro para ponteiro (unsigned char **)
para criar uma matriz bidimensional dinâmica.
    Ideia Principal: Alocar a quantidade exata de memória necessária
com base na largura, altura e quantidade de canais de cor da imagem.
    Vantagens e Segurança: A implementação possui um mecanismo interno de
"rollback". Caso ocorra falha na alocação, o código libera todas as linhas
alocadas anteriormente antes de retornar NULL, prevenindo Memory Leaks.
============================================================================
*/
unsigned char **alocar_pixels(int largura, int altura, int canais)
{
    unsigned char **pixels = (unsigned char **)malloc(altura * sizeof(unsigned char *));
    if (!pixels)
        return NULL;

    for (int i = 0; i < altura; i++)
    {
        pixels[i] = (unsigned char *)malloc(largura * canais * sizeof(unsigned char));
        if (!pixels[i])
        {
            for (int j = 0; j < i; j++)
                free(pixels[j]);
            free(pixels);
            return NULL;
        }
    }
    return pixels;
}

// Função para liberar a memória alocada para a imagem
/*
============================================================================
PARA O RELATÓRIO: GERENCIAMENTO DE MEMÓRIA (liberar_imagem)
    Ideia Principal: Desalocar com segurança a estrutura da imagem. Em C,
memórias alocadas dinamicamente não são limpas automaticamente por um
Garbage Collector, já que não existente na linguagem.
    Etapas de Execução: Primeiro, itera-se sobre as linhas da matriz liberando
os vetores de pixels. Depois, libera-se o vetor de ponteiros (as colunas)
e, por fim, a struct `Image` em si.
============================================================================
*/
void liberar_imagem(Image *img)
{
    if (img)
    {
        if (img->pixels)
        {
            for (int i = 0; i < img->altura; i++)
            {
                free(img->pixels[i]);
            }
            free(img->pixels);
        }
        free(img);
    }
}

// LEITURA DE ARQUIVO PGM/PPM
/*
============================================================================
PARA O RELATÓRIO: FUNÇÃO DE LEITURA (ler_imagem)
    Ideia Principal: Interpretar e carregar um arquivo de imagem formatado em
ASCII para a estrutura de dados `Image` na memória RAM.
    Tomadas de Decisão e Estrutura: Optou-se por criar uma lógica dinâmica capaz
de identificar o "tipo" no cabeçalho. Se for "P2", o programa
configura a struct para 1 canal (escala de cinza). Se for "P3", ajusta
para 3 canais (RGB). Isso garante alta flexibilidade ao algoritmo.
    Etapas de Execução:
1. Abertura do arquivo via `fopen`.
2. Validação do tipo (P2 ou P3).
3. Alocação da matriz na memória usando as dimensões obtidas.
4. Leitura dos pixels através de laços aninhados usando `fscanf`.
============================================================================
*/
Image *ler_imagem(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        printf("Erro: Nao foi possivel abrir o arquivo de entrada '%s'.\n", filename);
        return NULL;
    }

    char formato[3];
    if (fscanf(file, "%2s", formato) != 1)
    {
        printf("Erro ao ler o formato da imagem.\n");
        fclose(file);
        return NULL;
    }

    int canais = 0;
    if (strcmp(formato, "P2") == 0)
    {
        canais = 1; // PGM Escala de Cinza
    }
    else if (strcmp(formato, "P3") == 0)
    {
        canais = 3; // PPM RGB Colorido
    }
    else
    {
        printf("Erro: Formato '%s' nao suportado. Use P2 ou P3.\n", formato);
        fclose(file);
        return NULL;
    }

    Image *img = (Image *)malloc(sizeof(Image));
    if (!img)
    {
        fclose(file);
        return NULL;
    }
    img->canais = canais;

    fscanf(file, "%d", &img->largura);

    fscanf(file, "%d", &img->altura);

    fscanf(file, "%d", &img->max_val);

    img->pixels = alocar_pixels(img->largura, img->altura, img->canais);
    if (!img->pixels)
    {
        printf("Erro: Falha na alocacao de memoria para os pixels.\n");
        free(img);
        fclose(file);
        return NULL;
    }

    for (int i = 0; i < img->altura; i++)
    {
        int colunas_reais = img->largura * img->canais;
        for (int j = 0; j < colunas_reais; j++)
        {
            int valor_pixel;
            if (fscanf(file, "%d", &valor_pixel) != 1)
            {
                printf("Erro: Pixel corrompido na linha %d.\n", i);
                liberar_imagem(img);
                fclose(file);
                return NULL;
            }
            img->pixels[i][j] = (unsigned char)valor_pixel;
        }
    }

    fclose(file);
    return img;
}

// ESCRITA DO ARQUIVO DE SAÍDA
/*
============================================================================
PARA O RELATÓRIO: FUNÇÃO DE ESCRITA (escrever_imagem)
    Ideia Principal: Serializar os dados da matriz na memória de volta para o
armazenamento secundário (disco), reconstruindo um arquivo .pgm ou .ppm válido.
    Etapas de Execução: A função grava o "tipo" correto baseado na
quantidade de canais da struct. Em seguida, grava as dimensões e o
valor máximo de cor. Por fim, percorre a matriz alocada dinamicamente,
escrevendo pixel a pixel espaçados no arquivo com `fprintf`.
============================================================================
*/
void escrever_imagem(const char *filename, Image *img)
{
    if (!img || !img->pixels)
        return;

    FILE *file = fopen(filename, "w");
    if (!file)
    {
        printf("Erro: Nao foi possivel criar o arquivo de saida '%s'.\n", filename);
        return;
    }

    if (img->canais == 1)
    {
        fprintf(file, "P2\n");
    }
    else
    {
        fprintf(file, "P3\n");
    }

    fprintf(file, "%d %d\n", img->largura, img->altura);
    fprintf(file, "%d\n", img->max_val);

    for (int i = 0; i < img->altura; i++)
    {
        int colunas_reais = img->largura * img->canais;
        for (int j = 0; j < colunas_reais; j++)
        {
            fprintf(file, "%d ", img->pixels[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

int tamanho_arquivo(const char *caminho){
    FILE *f = fopen(caminho, "rb");
    if (!f){
        printf("Erro ao abrir arquivo \n");
        return -1;
    }
    fseek(f, 0, SEEK_END);
    int tamanho = ftell(f);
    fclose(f);
    return tamanho;
}

void calcular_taxa(const char *caminho_original, const char *caminho_compactado) {
    int original   = tamanho_arquivo(caminho_original);
    int compactado = tamanho_arquivo(caminho_compactado);

    if (original < 0 || compactado < 0) return;

    float taxa = (1.0 - (float)original / (float)compactado) * 100.0;

    printf("\n=== Metricas===\n");
    printf("Tamanho original:    %d bytes\n", original);
    printf("Tamanho compactado:  %d bytes\n", compactado);
    printf("Taxa de compactacao: %.2f%%\n", taxa);

    if (taxa < 0)
        printf("Arquivo reduzoiu %.2f%%\n", -taxa);
    else
        printf("Arquivo aumentou %.2f%%\n", taxa);

    printf("===============================\n\n");
}

float calcular_mse(Image *original, Image *reconstituida) {
    float soma = 0.0;
    int total = original->largura * original->altura * original->canais;

    for (int i = 0; i < original->altura; i++) {
        int cols = original->largura * original->canais;
        for (int j = 0; j < cols; j++) {
            float diff = (float)original->pixels[i][j] - (float)reconstituida->pixels[i][j];
            soma += diff * diff;
        }
    }

    return soma / total;
}

float calcular_psnr(float mse) {
    if (mse == 0.0)
        return -1.0;
    return 10.0 * log10((255.0 * 255.0) / mse);
}

void exibir_qualidade(Image *original, Image *reconstituida) {
    float mse  = calcular_mse(original, reconstituida);
    float psnr = calcular_psnr(mse);

    printf("=== Qualidade da Reconstituicao ===\n");
    printf("MSE:  %.4f\n", mse);

    if (psnr < 0)
        printf("PSNR: imagens identicas\n");
    else
        printf("PSNR: %.2f dB\n", psnr);

    printf("Interpretacao: ");
    if      (psnr < 0)   printf("Imagens identicas.\n");
    else if (psnr >= 40) printf("Excelente.\n");
    else if (psnr >= 30) printf("Boa qualidade.\n");
    else if (psnr >= 20) printf("Qualidade aceitavel.\n");
    else                 printf("Baixa qualidade.\n");

    printf("===================================\n\n");
}