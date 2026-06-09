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

//metricas

int tamanho_arquivo(const char *caminho)
{
    FILE *f = fopen(caminho, "rb");
    if (!f)
    {
        printf("Erro ao abrir arquivo \n");
        return -1;
    }
    fseek(f, 0, SEEK_END);
    int tamanho = ftell(f);
    fclose(f);
    return tamanho;
}

void calcular_taxa(const char *caminho_original, const char *caminho_compactado)
{
    int original   = tamanho_arquivo(caminho_original);
    int compactado = tamanho_arquivo(caminho_compactado);

    if (original < 0 || compactado < 0)
    {
        return;
    }

    float taxa = (1.0 - (float)compactado / (float)original) * 100.0;

    printf("\n=== Metricas===\n");
    printf("Tamanho original:    %d bytes\n", original);
    printf("Tamanho compactado:  %d bytes\n", compactado);
    printf("Taxa de compactacao: %.2f%%\n", taxa);

    if (taxa < 0)
    {
        printf("Arquivo reduzoiu %.2f%%\n", -taxa);
    }
    else
    {
        printf("Arquivo aumentou %.2f%%\n", taxa);
    }

    printf("===============================\n\n");
}

float calcular_mse(Image *original, Image *reconstituida)
{
    float soma = 0.0;
    int total = original->largura * original->altura * original->canais;

    for (int i = 0; i < original->altura; i++)
    {
        int cols = original->largura * original->canais;
        for (int j = 0; j < cols; j++)
        {
            float diff = (float)original->pixels[i][j] - (float)reconstituida->pixels[i][j];
            soma += diff * diff;
        }
    }

    return soma / total;
}

float calcular_psnr(float mse)
{
    if (mse == 0.0)
    {
        return -1.0;
    }
    return 10.0 * log10((255.0 * 255.0) / mse);
}

void exibir_qualidade(Image *original, Image *reconstituida)
{
    float mse  = calcular_mse(original, reconstituida);
    float psnr = calcular_psnr(mse);

    printf("=== Qualidade da Reconstituicao ===\n");
    printf("MSE:  %.4f\n", mse);

    if (psnr < 0)
    {
        printf("PSNR: imagens identicas\n");
    }
    else
    {
        printf("PSNR: %.2f dB\n", psnr);
    }

    printf("Interpretacao: ");
    if (psnr < 0)
    {
        printf("Imagens identicas.\n");
    }
    else if (psnr >= 40)
    {
        printf("Excelente.\n");
    }
    else if (psnr >= 30)
    {
        printf("Boa qualidade.\n");
    }
    else if (psnr >= 20)
    {
        printf("Qualidade aceitavel.\n");
    }
    else
    {
        printf("Baixa qualidade.\n");
    }

    printf("===================================\n\n");
}


// ============================================================
// DOWNSAMPLING 
// ============================================================
/*
============================================================================
PARA O RELATÓRIO: 

Funcionamento Detalhado:
    Inicialmente é criada uma nova imagem com dimensões reduzidas. A largura
    e a altura são calculadas utilizando divisão inteira arredondada para
    cima, para que todos os pixels da imagem original sejam
    considerados (bordas sao desconsideradas sem isso).

    Em seguida, a função percorre cada posição da imagem reduzida. Para cada
    pixel da nova imagem, é identificado o bloco correspondente na imagem
    original. Todos os pixels desse bloco são somados separadamente para
    cada canal de cor (tons de cinza ou RGB).

    Durante a soma, é realizada uma verificação de limites para garantir que
    apenas pixels válidos da imagem original sejam considerados. Essa etapa
    é necessária para tratar corretamente blocos localizados nas bordas da
    imagem, que podem possuir menos pixels do que o tamanho completo do
    bloco fator x fator.

    Após a soma, é calculada a média dos valores encontrados dividindo-se a
    soma pela quantidade real de pixels processados. O resultado é armazenado
    no pixel correspondente da imagem reduzida.
    
============================================================================
*/
Image *downsample(Image *original, int fator)
{
    if (original == NULL || fator <= 1)
        return NULL;

    Image *reduzida = malloc(sizeof(Image));

    reduzida->largura = (original->largura + fator - 1) / fator; //altura e largura sao arredondadas para cima
    reduzida->altura  = (original->altura + fator - 1) / fator;  //para garantir que todos os pixels da imagem 
    reduzida->max_val = original->max_val;                       //original sao aproveitados   
    reduzida->canais = original->canais;

    reduzida->pixels = alocar_pixels(
        reduzida->largura,
        reduzida->altura,
        reduzida->canais
    );

    //os tres primeiros for percorrem todos os canais dentro dos pixels da imagem reduzida
    for (int y = 0; y < reduzida->altura; y++)
    {
        for (int x = 0; x < reduzida->largura; x++)
        {
            for (int c = 0; c < reduzida->canais; c++)
            {   
                int soma = 0;
                int quantidade = 0;

                //percorrendo o bloco fator x fator
                for (int dy = 0; dy < fator; dy++)
                {
                    for (int dx = 0; dx < fator; dx++)
                    {   
                        // mapeamento na dos pixels equivalentes na imagem original
                        int origem_y = y * fator + dy;
                        int origem_x = x * fator + dx;

                        // verificando se o pixel existe na imagem original
                        if (origem_y < original->altura && origem_x < original->largura)
                        {
                            soma += original->pixels[origem_y][origem_x * original->canais + c];
                            quantidade++;
                        }
                    }
                }
                reduzida->pixels[y][x * reduzida->canais + c] = soma / quantidade;
            }
        }
    }
    return reduzida;
}


// ============================================================
// UPSAMPLING — NEAREST-NEIGHBOR
// ============================================================
/*
============================================================================
PARA O RELATÓRIO: NEAREST-NEIGHBOR (Vizinho Mais Próximo)

Funcionamento Detalhado:
    O algoritmo percorre cada pixel (i, j) da imagem AMPLIADA e mapeia sua
    posição de volta para a imagem REDUZIDA usando divisão inteira.

    A fórmula do mapeamento é:
        src_i = (i * img_reduzida->altura) / altura_original
        src_j = (j * img_reduzida->largura) / largura_original

    Usando multiplicação antes da divisão para manter a proporcionalidade
    mesmo quando o fator não é exato (ex: de 100px para 256px).

    O resultado é que um bloco de pixels na imagem ampliada recebe exatamente
    o mesmo valor, criando o característico efeito "pixelizado" ou de blocos
    quadrados visíveis — especialmente em fatores grandes como 8x ou 16x.

Exemplo com fator 2 e imagem 4x4 → 8x8:
    Pixel (0,0) na saída → src (0,0)
    Pixel (1,0) na saída → src (0,0)  [mesmo bloco]
    Pixel (2,0) na saída → src (1,0)  [próximo bloco]
============================================================================
*/
Image *upsampling_nearest_neighbor(Image *img_reduzida, int largura_original, int altura_original)
{
    if (!img_reduzida || largura_original <= 0 || altura_original <= 0)
        return NULL;

    Image *saida = (Image *)malloc(sizeof(Image));
    if (!saida)
        return NULL;

    saida->largura = largura_original;
    saida->altura = altura_original;
    saida->max_val = img_reduzida->max_val;
    saida->canais = img_reduzida->canais;
    saida->pixels = alocar_pixels(largura_original, altura_original, img_reduzida->canais);

    if (!saida->pixels)
    {
        free(saida);
        return NULL;
    }

    for (int i = 0; i < altura_original; i++)
    {
        for (int j = 0; j < largura_original; j++)
        {
            // Mapeia pixel da saída para o vizinho mais próximo na imagem reduzida
            int src_i = (i * img_reduzida->altura) / altura_original;
            int src_j = (j * img_reduzida->largura) / largura_original;

            for (int c = 0; c < img_reduzida->canais; c++)
            {
                saida->pixels[i][j * img_reduzida->canais + c] =
                    img_reduzida->pixels[src_i][src_j * img_reduzida->canais + c];
            }
        }
    }

    return saida;
}

// ============================================================
// UPSAMPLING — BILINEAR
// ============================================================
/*
============================================================================
PARA O RELATÓRIO: INTERPOLAÇÃO BILINEAR

Funcionamento Detalhado:
    A interpolação bilinear é uma extensão da interpolação linear para 2
    dimensões. Para cada pixel (i, j) da imagem ampliada:

    1. Calcula a posição FRACIONÁRIA correspondente na imagem reduzida:
           pos_y = i * (altura_reduzida - 1) / (altura_original - 1)
           pos_x = j * (largura_reduzida - 1) / (largura_original - 1)

    2. Determina os 4 pixels vizinhos na imagem reduzida:
           (y0, x0) = floor(pos_y), floor(pos_x)  → vizinho superior-esquerdo
           (y0, x1)                                → vizinho superior-direito
           (y1, x0)                                → vizinho inferior-esquerdo
           (y1, x1)                                → vizinho inferior-direito

    3. Calcula os pesos fracionários:
           dy = pos_y - y0  (quanto estamos "abaixo" do vizinho superior)
           dx = pos_x - x0  (quanto estamos "à direita" do vizinho esquerdo)

    4. Interpola usando os 4 vizinhos com a fórmula bilinear:
           valor = (1-dy) * [(1-dx)*P(y0,x0) + dx*P(y0,x1)]
                 +    dy  * [(1-dx)*P(y1,x0) + dx*P(y1,x1)]

    Geometricamente, é como se calculássemos primeiro a interpolação
    linear horizontal entre os dois pares de pixels, e depois a
    interpolação vertical entre os dois resultados.

Tratamento de Borda:
    O índice x1 e y1 são limitados (clamped) à dimensão máxima da imagem
    reduzida menos 1, garantindo que acessos nas bordas direita e inferior
    da imagem ampliada não causem acesso fora dos limites do array.
============================================================================
*/
Image *upsampling_bilinear(Image *img_reduzida, int largura_original, int altura_original)
{
    if (!img_reduzida || largura_original <= 0 || altura_original <= 0)
        return NULL;

    Image *saida = (Image *)malloc(sizeof(Image));
    if (!saida)
        return NULL;

    saida->largura = largura_original;
    saida->altura = altura_original;
    saida->max_val = img_reduzida->max_val;
    saida->canais = img_reduzida->canais;
    saida->pixels = alocar_pixels(largura_original, altura_original, img_reduzida->canais);

    if (!saida->pixels)
    {
        free(saida);
        return NULL;
    }

    int h_red = img_reduzida->altura;
    int w_red = img_reduzida->largura;

    for (int i = 0; i < altura_original; i++)
    {
        for (int j = 0; j < largura_original; j++)
        {
            // Posição fracionária na imagem reduzida
            double pos_y = (double)i * (h_red - 1) / (altura_original - 1);
            double pos_x = (double)j * (w_red - 1) / (largura_original - 1);

            // Coordenadas dos 4 vizinhos (com clamp para as bordas)
            int y0 = (int)pos_y;
            int x0 = (int)pos_x;
            int y1 = (y0 + 1 < h_red) ? y0 + 1 : y0;
            int x1 = (x0 + 1 < w_red) ? x0 + 1 : x0;

            // Pesos fracionários (distância até o vizinho superior-esquerdo)
            double dy = pos_y - y0;
            double dx = pos_x - x0;

            for (int c = 0; c < img_reduzida->canais; c++)
            {
                // Valores dos 4 vizinhos
                double p00 = img_reduzida->pixels[y0][x0 * img_reduzida->canais + c];
                double p01 = img_reduzida->pixels[y0][x1 * img_reduzida->canais + c];
                double p10 = img_reduzida->pixels[y1][x0 * img_reduzida->canais + c];
                double p11 = img_reduzida->pixels[y1][x1 * img_reduzida->canais + c];

                // Fórmula bilinear: interpolação horizontal + vertical
                double valor = (1.0 - dy) * ((1.0 - dx) * p00 + dx * p01)
                             +        dy  * ((1.0 - dx) * p10 + dx * p11);

                saida->pixels[i][j * img_reduzida->canais + c] = (unsigned char)(valor + 0.5);
            }
        }
    }

    return saida;
}