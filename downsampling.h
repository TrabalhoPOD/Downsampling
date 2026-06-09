#ifndef DOWNSAMPLING_H
#define DOWNSAMPLING_H

#include <stdio.h>

// Estrutura para armazenar a imagem na memória
/*
============================================================================
PARA O RELATÓRIO: ESTRUTURA DE DADOS (struct Image)

Ideia Principal:
A estrutura Image foi concebida para servir como o contêiner central de dados
do projeto. Ela encapsula todas as propriedades essenciais de uma imagem
digital, permitindo que as funções de processamento manipulem os dados de
forma coesa e organizada.

Estruturas de Dados Utilizadas:
A struct contém quatro variáveis inteiras (largura, altura, max_val e
canais) que definem os metadados da imagem, e um ponteiro de ponteiro
(unsigned char **) para gerir a matriz de pixels. O uso de ponteiro para
ponteiro é fundamental para permitir a alocação dinâmica de matrizes em
tempo de execução, adaptando o programa a qualquer dimensão de imagem de
entrada sem desperdício de memória.

Vantagens:
Esta abstração garante que as funções de leitura, compressão e escrita
interajam com um objeto único. Isso reduz drasticamente a complexidade da
passagem de parâmetros em C, diminui a probabilidade de erros de gerenciamento
de memória e torna o código significativamente mais legível e modular.
============================================================================
*/
typedef struct
{
    int largura;
    int altura;
    int max_val;            // Valor de intensidade
    int canais;             // 1 para PGM (tons de cinza), 3 para PPM (RGB colorido)
    unsigned char **pixels; // Matriz 2D alocada dinamicamente
} Image;

unsigned char **alocar_pixels(int largura, int altura, int canais);
void liberar_imagem(Image *img);
Image *ler_imagem(const char *filename);
void escrever_imagem(const char *filename, Image *img);

/*
============================================================================
PARA O RELATÓRIO: ALGORITMOS DE RECONSTITUIÇÃO (Upsampling / Interpolação)

A reconstituição tenta "adivinhar" os pixels perdidos durante o downsampling.
Como a informação foi descartada, há sempre uma perda residual — o que
caracteriza a compactação com perda (lossy compression).

1. NEAREST-NEIGHBOR (Vizinho Mais Próximo):
   - Estratégia: Cada pixel da imagem ampliada copia o valor do pixel mais
     próximo na imagem reduzida (mapeamento direto via divisão inteira).
   - Complexidade: O(n) onde n = pixels da imagem de saída.
   - Artefatos visuais: Blocagem (efeito de "pixelização"), bordas serrilhadas.
   - Vantagem: Muito rápido, preserva valores exatos dos pixels originais.

2. BILINEAR (Interpolação Bilinear):
   - Estratégia: Cada pixel da imagem ampliada é calculado como uma média
     ponderada de seus 4 vizinhos mais próximos na imagem reduzida, com
     pesos proporcionais à distância fracionária até cada vizinho.
   - Complexidade: O(n) onde n = pixels da imagem de saída (constante de 4
     acessos por pixel).
   - Artefatos visuais: Borramento (blur) nas bordas e detalhes finos.
   - Vantagem: Transições suaves entre regiões, resultado visualmente superior
     ao nearest-neighbor na maioria dos casos.
============================================================================
*/

// ============================================================
// FUNÇÕES DE UPSAMPLING (Reconstituição)
// ============================================================

// Amplia a imagem replicando o pixel mais próximo (sem interpolação)
Image *upsampling_nearest_neighbor(Image *img_reduzida, int largura_original, int altura_original);

// Amplia a imagem com interpolação bilinear (média ponderada de 4 vizinhos)
Image *upsampling_bilinear(Image *img_reduzida, int largura_original, int altura_original);

// ============================================================
// FUNÇÕES DE MÉTRICAS E QUALIDADE
// ============================================================
int tamanho_arquivo(const char *caminho);
void calcular_taxa(const char *caminho_original, const char *caminho_compactado);
float calcular_mse(Image *original, Image *reconstituida);
float calcular_psnr(float mse);
void exibir_qualidade(Image *original, Image *reconstituida);

#endif