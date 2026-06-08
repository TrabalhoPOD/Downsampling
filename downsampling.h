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

#endif