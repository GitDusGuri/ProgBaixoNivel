#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// **********************************************************************
// PUCRS/EP
// Leitura de arquivo PGM
//
// Marcelo Cohen
//
// marcelo.cohen@pucrs.br
// **********************************************************************

typedef struct {
    int largura, altura;
    unsigned char* imagemOrig;    // vetor com o conteúdo do arquivo
    int tamOrig;                  // quantidade de bytes lidos do arquivo
    unsigned char* imagemCompact; // vetor com a imagem compactada (a ser preenchido)
    int tamCompact;               // quantidade de bytes na imagem compactada (a ser preenchido)
} Imagem;

void carregaPGM(const char *nome, Imagem* img);
void dump(Imagem* img);
void comprimePGM(Imagem* img);

// **********************************************************************
void carregaPGM(const char *nome, Imagem* img)
{
    FILE *arq;
    arq = fopen (nome,"rb");
    if (arq == NULL)
    {
        printf("Problemas na abertura do arquivo %s...\n", nome);
        exit(1);
    }
    printf("Arquivo %s aberto!\n", nome);

    fseek(arq, 0L, SEEK_END);
    int sz = ftell(arq);
    rewind(arq);

    printf("Tamanho em bytes: %d\n", sz);
    char c;
    unsigned char* ptr = malloc(sz);
    fread(ptr, sz, 1, arq);
    rewind(arq);
    char header[15];
    for(int i = 0; i < 14; i++) {
        c = getc(arq);
        header[i] = c;
    }

    int centenalargura = 100 * (header[3] - '0');
    int dezenalargura = 10 * (header[4] - '0');
    int unidadelargura = header[5] - '0';
    int largura = centenalargura + dezenalargura + unidadelargura;

    int centenaaltura = 100 * (header[7] - '0');
    int dezenaaltura = 10 * (header[8] - '0');
    int unidadealtura = header[9] - '0';
    int altura = centenaaltura + dezenaaltura + unidadealtura;

    fclose(arq);

    // Reservando o dobro de espaco da imagem original
    // para a compactada (caso ela nao compacte bem)
    img->imagemCompact = malloc(sz*2);
    img->imagemOrig = ptr;
	img->tamOrig = sz;
    img->altura = altura;
    img->largura = largura;
}

// Teste: exibe primeiros 256 bytes do vetor original
// (primeiros bytes sao o header em formato texto)
void dump(Imagem* img)
{
	for(int i=0; i<256; i+=16)
	{
	    printf("%04d: ",i);
	    for(int j=0; j<16; j++)
            printf("%02X ", img->imagemOrig[i+j]);
        for(int j=0; j<16; j++)
            printf("%c ", isprint(img->imagemOrig[i+j]) ? img->imagemOrig[i+j] : '.');
        printf("\n");
	}
	printf("\n");
}

void comprimePGM(Imagem* img) {
    int counter;
    int compControl = 0;
    // while (img->imagemOrig[j] !='\0')
    // {
    //     printf("%02X", img->imagemOrig[j]);
    //     j++;
    // }
    
    for (int i = 15; i < img->tamOrig; i++) {
        counter = 1;
        while(img->imagemOrig[i] == img->imagemOrig[i+1]) {
            counter++;
            i++;
        }
        if (counter > 9 && counter < 100) {
            char unidadeCounter = (counter % 10) + '0';
            char dezenaCounter = ((counter - (counter % 10)) / 10) + '0';
            img->imagemCompact[compControl] = dezenaCounter;
            compControl++;
            img->imagemCompact[compControl] = unidadeCounter;
            compControl++;
            img->imagemCompact[compControl] = img->imagemOrig[i];    
        } else if (counter > 99 && counter < 1000) {
            char unidadeCounter = (counter % 10) + '0';
            char dezenaCounter = ((counter - (counter % 10)) / 10) + '0';
            char centenaCounter = ((counter - (counter % 100)) / 100) + '0';
            img->imagemCompact[compControl] = centenaCounter;
            compControl++;
            img->imagemCompact[compControl] = dezenaCounter;
            compControl++;
            img->imagemCompact[compControl] = unidadeCounter;
            compControl++;
            img->imagemCompact[compControl] = img->imagemOrig[i];
        } else {
            img->imagemCompact[compControl] = counter + '0';
            compControl++;
            img->imagemCompact[compControl] = img->imagemOrig[i];
        }
    }
    printf("%d\n",compControl);
    printf("\n");
    
    for (int i = 0; i < compControl; i++) {
        printf("%02X",img->imagemCompact[i]);
    }
}

// **********************************************************************
int main()
{
    char nomeArquivo[] = "logofacin.pgm";
    int largura, altura;

	Imagem imagem;
    carregaPGM(nomeArquivo, &imagem);

	// dump(&imagem);
    comprimePGM(&imagem);

	// Libera memoria de ambas as imagens
	free(imagem.imagemOrig);
    free(imagem.imagemCompact);
    return 0;
}
