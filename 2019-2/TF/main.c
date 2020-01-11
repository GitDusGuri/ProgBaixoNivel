#include <stdio.h>
#include <stdlib.h>
#include <string.h>        // Para usar strings

#ifdef WIN32
#include <windows.h>    // Apenas para Windows
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>     // Funções da OpenGL
#include <GL/glu.h>    // Funções da GLU
#include <GL/glut.h>   // Funções da FreeGLUT
#endif

// SOIL é a biblioteca para leitura das imagens
#include "SOIL.h"

// Um pixel RGB (24 bits)
typedef struct {
    unsigned char r, g, b;
} RGB;

// Uma imagem RGB
typedef struct {
    int width, height;
    RGB* img;
} Img;

// Protótipos
void load(char* name, Img* pic);
void uploadTexture();

// Funções da interface gráfica e OpenGL
void init();
void draw();
void keyboard(unsigned char key, int x, int y);

// Funções adicionadas
void copyImage();
void transformImage();
void energyCalculation(int* em);
void costCalculation(int* em);

// Largura e altura da janela
int width, height;

// Identificadores de textura
GLuint tex[3];

// As 3 imagens
Img pic[3];

// Imagem selecionada (0,1,2)
int sel;

// Numero de colunas a serem removidas
int columns;

// Carrega uma imagem para a struct Img
void load(char* name, Img* pic)
{
    int chan;
    pic->img = (RGB*) SOIL_load_image(name, &pic->width, &pic->height, &chan, SOIL_LOAD_RGB);
    if(!pic->img)
    {
        printf( "SOIL loading error: '%s'\n", SOIL_last_result() );
        exit(1);
    }
    printf("Load: %d x %d x %d\n", pic->width, pic->height, chan);
}

int main(int argc, char** argv)
{
    if(argc < 2) {
        printf("seamcarving [origem] [mascara]\n");
        printf("Origem é a imagem original, mascara é a máscara desejada\n");
        exit(1);
    }
    glutInit(&argc,argv);

    // Define do modo de operacao da GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    // pic[0] -> imagem original
    // pic[1] -> máscara desejada
    // pic[2] -> resultado do algoritmo

    // Carrega as duas imagens
    load(argv[1], &pic[0]);
    load(argv[2], &pic[1]);

    if(pic[0].width != pic[1].width || pic[0].height != pic[1].height) {
        printf("Imagem e máscara com dimensões diferentes!\n");
        exit(1);
    }

    // A largura e altura da janela são calculadas de acordo com a maior
    // dimensão de cada imagem
    width = pic[0].width;
    height = pic[0].height;

    // A largura e altura da imagem de saída são iguais às da imagem original (1)
    pic[2].width  = pic[1].width;
    pic[2].height = pic[1].height;

    // Especifica o tamanho inicial em pixels da janela GLUT
    glutInitWindowSize(width, height);

    // Cria a janela passando como argumento o titulo da mesma
    glutCreateWindow("Seam Carving");

    // Registra a funcao callback de redesenho da janela de visualizacao
    glutDisplayFunc(draw);

    // Registra a funcao callback para tratamento das teclas ASCII
    glutKeyboardFunc (keyboard);

    // Cria texturas em memória a partir dos pixels das imagens
    tex[0] = SOIL_create_OGL_texture((unsigned char*) pic[0].img, pic[0].width, pic[0].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
    tex[1] = SOIL_create_OGL_texture((unsigned char*) pic[1].img, pic[1].width, pic[1].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

    // Exibe as dimensões na tela, para conferência
    printf("Origem  : %s %d x %d\n", argv[1], pic[0].width, pic[0].height);
    printf("Destino : %s %d x %d\n", argv[2], pic[1].width, pic[0].height);
    sel = 0; // pic1

    // Define a janela de visualizacao 2D
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0.0,width,height,0.0);
    glMatrixMode(GL_MODELVIEW);

    // Aloca memória para a imagem de saída
    pic[2].img = malloc(pic[1].width * pic[1].height * 3); // W x H x 3 bytes (RGB)
    // Pinta a imagem resultante de preto!
    memset(pic[2].img, 0, width*height*3);

    // Cria textura para a imagem de saída
    tex[2] = SOIL_create_OGL_texture((unsigned char*) pic[2].img, pic[2].width, pic[2].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

    // printf("Quantas colunas deseja remover?\n");
    // scanf("%d", &columns);

    // Entra no loop de eventos, não retorna
    glutMainLoop();
}


// Gerencia eventos de teclado
void keyboard(unsigned char key, int x, int y)
{
    if(key==27) {
      // ESC: libera memória e finaliza
      free(pic[0].img);
      free(pic[1].img);
      free(pic[2].img);
      exit(1);
    }
    if(key >= '1' && key <= '3')
        // 1-3: seleciona a imagem correspondente (origem, máscara e resultado)
        sel = key - '1';
    if(key == 's') {
        // Aplica o algoritmo e gera a saida em pic[2].img...
        
        // Copia a imagem original
        copyImage();
        transformImage();
        // uploadTexture();
    }
    glutPostRedisplay();
}

// Faz upload da imagem para a textura,
// de forma a exibi-la na tela
void uploadTexture()
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
        pic[2].width, pic[2].height, 0,
        GL_RGB, GL_UNSIGNED_BYTE, pic[2].img);
    glDisable(GL_TEXTURE_2D);
}

// Callback de redesenho da tela
void draw()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // Preto
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    // Para outras cores, veja exemplos em /etc/X11/rgb.txt

    glColor3ub(255, 255, 255);  // branco

    // Ativa a textura corresponde à imagem desejada
    glBindTexture(GL_TEXTURE_2D, tex[sel]);
    // E desenha um retângulo que ocupa toda a tela
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);

    glTexCoord2f(0,0);
    glVertex2f(0,0);

    glTexCoord2f(1,0);
    glVertex2f(pic[sel].width,0);

    glTexCoord2f(1,1);
    glVertex2f(pic[sel].width, pic[sel].height);

    glTexCoord2f(0,1);
    glVertex2f(0,pic[sel].height);

    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Exibe a imagem
    glutSwapBuffers();
}

void copyImage() {
    for (int i = 0; i < pic[0].width * pic[0].height; i++) {
        pic[2].img[i] = pic[0].img[i];
    }
}

void transformImage(){
    int energyMatrix[pic[0].height][pic[0].width];
    int path[pic[0].height];
    int* em = &energyMatrix[0][0];
    // Variáveis para encontrar o melhor caminho
    int lowest = 2147483646, lowestIndex = 0;
    int loopControl;
    for (int k = 0; k < 400; k++) {
        energyCalculation(em);
        costCalculation(em);
        // Encontra o elemento de menor energia na
        // última linha do 
        for (int i = 0; i < pic[2].width; i++) {
            if (energyMatrix[pic[2].height-1][i] < lowest) {
                lowest = energyMatrix[pic[2].height-1][i];
                lowestIndex = i;
            }
        }

        for (int i = pic[2].height-1; i >= 0; i--) {
            // Reseta a variável para comparações
            lowest = 2147483646;
            loopControl = lowestIndex;
            // Inicia o loop um píxel anterior e acaba ele um pixel após
            // o pixel de menor energia da linha anterior
            for (int j = loopControl-1; j <= loopControl+1; j++) {
                if (energyMatrix[i][j] < lowest) {
                    lowest = energyMatrix[i][j];
                    lowestIndex = j;
                }
            }
            path[i] = lowestIndex;
        }

        // Pinta o melhor caminho na imagem destinho
        for (int i = 0; i < pic[2].height; i++) {
            pic[2].img[i * pic[2].width + path[i]].r = 255;
            pic[2].img[i * pic[2].width + path[i]].g = 0;
            pic[2].img[i * pic[2].width + path[i]].b = 0;
        }

        for (int i = 0; i < pic[2].height; i++) {
            for (int j = path[i]; j < pic[2].width - 1; j++) {
                pic[2].img[(i * pic[2].width) + j] = pic[2].img[(i * pic[2].width) + j + 1];
                pic[1].img[(i * pic[1].width) + j] = pic[1].img[(i * pic[1].width) + j + 1];
            }
        }
    }
    uploadTexture();
}

void energyCalculation(int* em) {
    int picSize = pic[2].height * pic[2].width;
    int deltaX, deltaY, deltaT, redX, greenX, blueX, redY, greenY, blueY;
    int left, right, over, under;
    for (int i = 0; i < picSize ; i++){
        // Caso o pixel seja o primeiro da coluna
        // compara a energia em x com o segundo e o último
        // da mesma coluna
        if (i % pic[2].width == 0) {
            left = ((i / pic[2].width) * pic[2].width) + (pic[2].width-1);
            right = i + 1;
        }
        // Caso o pixel seja o último da coluna
        // compara a energia em x com o penúltimo e o primeiro
        // da mesma coluna
        else if (i % pic[2].width == pic[2].width - 1) {
            left = i - 1;
            right = i - (pic[2].width - 1);
        }
        // Caso seja um píxel em qualquer espaço do meio
        // da coluna, compara com os que estão ao lado
        else {
            left = i - 1;
            right = i + 1;
        }
        redX = pic[2].img[right].r - pic[2].img[left].r;
        greenX = pic[2].img[right].g - pic[2].img[left].g;
        blueX = pic[2].img[right].b - pic[2].img[left].b;
        deltaX = (redY * redY) + (greenY * greenY) + (blueY * blueY);

        // Caso seja um pixel da primeira linha
        // compara a energia em y com o abaixo e o
        // pixel na ultima linha da mesma coluna
        if (i < pic[2].width) {
            over = (pic[2].width * (pic[2].height-1)) + i;
            under = i + pic[2].width;
        }
        // Caso seja um pixel da última linha
        // compara a energia em y com o acima e o
        // pixel na primeira linha da mesma coluna
        else if (i >= (pic[2].width * (pic[2].height-1))) {
            under = i - (pic[2].width * (pic[2].height-1));
            over = i - pic[2].width;
        }
        // Caso seja um píxel em qualquer espaço do meio
        // da linha, compara com os que estão aciam e abaixo
        else {
            over = i - pic[2].width;
            under = i + pic[2].width;
        }
        redY = pic[2].img[under].r - pic[2].img[over].r;
        greenY = pic[2].img[under].g - pic[2].img[over].g;
        blueY = pic[2].img[under].b - pic[2].img[over].b;
        deltaY = (redX * redX) + (greenX * greenX) + (blueX * blueX);
        deltaT = deltaX + deltaY;

        // Checa a máscara para adicionar ou remover
        // energia do pixel atual
        if (pic[1].img[i].r > 230 && pic[1].img[i].b < 200) {
            deltaT -= 1000000;
        } else if (pic[1].img[i].g > 230 && pic[1].img[i].b < 200) {
            deltaT += 1000000;
        }

        // Guarda o resultado na matriz de energia
        *em = deltaT;
        em++;
    }
}

void costCalculation(int* em) {
    int picSize = pic[2].height * pic[2].width;
    for (int i = 0; i < picSize; i++){
        if (i % pic[2].width == 0) {
            if (*em < *(em+1)) {
                *(em + pic[2].width) += *em;
            } else {
                *(em + pic[2].width) += *(em+1);
            }
        } else if (i % pic[2].width == pic[2].width - 1) {
            if (*em < *(em-1)) {
                *(em + pic[2].width) += *em;
            } else {
                *(em + pic[2].width) += *(em-1);
            }
        } else {
            if (*em < *(em+1)) {
                if (*em < *(em-1)) {
                    *(em + pic[2].width) += *em;
                } else {
                    *(em + pic[2].width) += *(em-1);
                }
            } else {
                if (*(em-1) < *(em+1)) {
                    *(em + pic[2].width) += *(em-1);
                } else {
                    *(em + pic[2].width) += *(em+1);
                }
            }
        }
        // Quebra o laço na última linha
        if (i > 0 && i % (pic[2].width * (pic[2].height-1)) == 0) {
            break;
        }
        em++;
    }
}
