#include "models.h"
#include <ncurses.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define YSIZE 30
#define XSIZE 100
#define MANADELAY 3 // Delay para carregar a mana (em segundos)

WINDOW *win;
WINDOW *dialog;
WINDOW *title;

pthread_t mana_t;
pthread_t pular_t;

int xPos = 10, // Posição do personagem
    yPos = 15;

int hp = 3, // Status do player
    mana = 0;

int sizeColorsAttr = 1, pairID = 1, mapa = 1, actionId = 0, ColorID = 8, ymax,
    xmax, pular, item_mapa = 0, GAME_OVER = 0, rumptDerrot = 0,
    permission[200][200], // Matriz com as permissões
    map_rgb[200][200],    // Matriz com as cores no mapa
    **rgb,                // Matriz com as infos das cores
    **rgb_pair;           // Matriz com os pares

char map_chars[200][200]; // Matriz com os caracteres (mapa)

void idIdentifier(int id);
void changeMap(int id);
void clearDialog();
void quizz(int id);

// Cria cores com os valores de rgb e com id i
void createColors(int id, int R, int G, int B) { init_color(id, R, G, B); }

// Cria cores com os valores de rgb e com id i
void createPairs(int id, int F, int B) { init_pair(id, F, B); }

// Faz o print do mapa, bordas e instruções
void printMap() {
  // Zera Mapa
  for (int x = 0; x < xmax; x++) {
    for (int y = 0; y < 32; y++) {
      attron(COLOR_PAIR(0));
      mvprintw(y, x, "");
      printw(" ");
      attroff(COLOR_PAIR(0));
    }
  }
  for (int x = 0; x < xmax; x++) {
    for (int y = 38; y < ymax; y++) {
      attron(COLOR_PAIR(0));
      mvprintw(y, x, "");
      printw(" ");
      attroff(COLOR_PAIR(0));
    }
  }
  for (int x = 102; x < xmax; x++) {
    for (int y = 32; y < ymax; y++) {
      attron(COLOR_PAIR(0));
      mvprintw(y, x, "");
      printw(" ");
      attroff(COLOR_PAIR(0));
    }
  }
  refresh();

  // Printa Mapa
  usleep(10);
  for (int x = 0; x <= XSIZE; x++) {
    for (int y = 0; y <= YSIZE; y++) {
      attron(COLOR_PAIR(map_rgb[y][x]));
      mvprintw(y, x, "");
      printw("%c", map_chars[y][x]);
      attroff(COLOR_PAIR(map_rgb[y][x]));
    }
  }
  refresh();

  // MINI MAPA
  if (item_mapa) {
    win = newwin(21, 33, 2, 108);
    refresh();
    wprintw(win, "     ________     ________\n");
    wprintw(win, "    |  RIO   |   | RESTAU.|\n");
    wprintw(win, "    |___  ___|   |___  ___|\n");
    wprintw(win, "        ||           ||\n");
    wprintw(win, "    ||||||           ||||||\n");
    wprintw(win, " ___||___             ___||___\n");
    wprintw(win, "| IGREJA |           |  UNIV. |\n");
    wprintw(win, "|___  ___|           |___  ___|\n");
    wprintw(win, "    ||                   ||\n");
    wprintw(win, "    ||     _________     ||\n");
    wprintw(win, "    |||||||  COVIL  |||||||\n");
    wprintw(win, "          |___   ___|\n");
    wprintw(win, "              | |\n");
    wprintw(win, "           ___| |___\n");
    wprintw(win, "          |  HOUSE  |\n");
    wprintw(win, "          |___   ___|\n");
    wprintw(win, "              | |\n");
    wprintw(win, "         _____| |_____\n");
    wprintw(win, "        ///THE WALL///");
    switch (mapa) {
    case 1:
      mvwprintw(win, 15, 15, "X");
      break;
    case 2:
      mvwprintw(win, 11, 15, "X");
      break;
    case 3:
      mvwprintw(win, 7, 4, "X");
      break;
    case 4:
      mvwprintw(win, 7, 25, "X");
      break;
    case 5:
      mvwprintw(win, 2, 8, "X");
      break;
    case 6:
      mvwprintw(win, 2, 21, "X");
      break;
    }
    wrefresh(win);
  }

  // BORDAS
  for (int i = 0; i < 102; ++i) {
    mvprintw(31, i, "-");
  }
  for (int i = 0; i < 145; ++i) {
    mvprintw(41, i, "-");
  }
  for (int i = 0; i < 41; ++i) {
    mvprintw(i, 102, "|");
  }
  for (int i = 0; i < 102; ++i) {
    mvprintw(38, i, "-");
  }

  // INSTRUÇÔES
  mvprintw(39, 4, "PARA SE MOVER       W");
  mvprintw(40, 4, "USE AS TECLAS:    A S D");

  mvprintw(39, 36, "PARA INTERAGIR");
  mvprintw(40, 36, "APERTE A TECLA:    E");

  mvprintw(39, 65, "PARA PAUSAR E REPRODUZIR");
  mvprintw(40, 65, "A MUSICA APERTE A TECLA:    C");
  refresh();
}

// Faz o print dos status do jogador
void printStatus() {
  init_color(60, 960, 400, 70);  // Laranja
  init_color(50, 910, 70, 100);  // Vermelho
  init_color(70, 420, 400, 420); // Cinza
  init_color(90, 200, 500, 222); // Verde

  init_pair(50, 7, 50);
  init_pair(60, 7, 60);
  init_pair(70, 7, 70);
  init_pair(80, 90, 0);
  init_pair(90, 7, 0);

  attron(COLOR_PAIR(70));
  for (int i = 0; i < 3; ++i) {
    mvprintw(35, 106 + (i * 9), "  ");
    mvprintw(35, 109 + (i * 9), "  ");
    mvprintw(36, 105 + (i * 9), "       ");
    mvprintw(37, 106 + (i * 9), "     ");
    mvprintw(38, 107 + (i * 9), "   ");
    mvprintw(39, 108 + (i * 9), " ");
  }
  attroff(COLOR_PAIR(70));

  attron(COLOR_PAIR(50));
  for (int i = 0; (i < hp) && (i < 3); ++i) {
    mvprintw(35, 106 + (i * 9), "  ");
    mvprintw(35, 109 + (i * 9), "  ");
    mvprintw(36, 105 + (i * 9), "       ");
    mvprintw(37, 106 + (i * 9), "     ");
    mvprintw(38, 107 + (i * 9), "   ");
    mvprintw(39, 108 + (i * 9), " ");
  }
  attroff(COLOR_PAIR(50));

  attron(COLOR_PAIR(90));
  mvprintw(32, 140, "/>");
  mvprintw(33, 139, "/");
  attroff(COLOR_PAIR(90));

  attron(COLOR_PAIR(90));
  mvprintw(33, 136, "___");
  mvprintw(33, 140, "_");
  attroff(COLOR_PAIR(90));

  attron(COLOR_PAIR(70));
  mvprintw(34, 136, "     ");
  mvprintw(35, 134, "         ");
  mvprintw(36, 133, "           ");
  mvprintw(37, 133, "_         _");
  mvprintw(38, 134, "__     __");
  mvprintw(39, 136, "_____");
  attroff(COLOR_PAIR(70));

  attron(COLOR_PAIR(80));
  if (mana >= 1)
    mvprintw(32, 140, "/>");
  if (mana >= 1)
    mvprintw(33, 139, "/");
  attroff(COLOR_PAIR(80));

  attron(COLOR_PAIR(90));
  if (mana >= 1)
    mvprintw(33, 136, "___");
  if (mana >= 1)
    mvprintw(33, 140, "_");
  attroff(COLOR_PAIR(90));

  attron(COLOR_PAIR(60));
  if (mana >= 6)
    mvprintw(34, 136, "     ");
  if (mana >= 5)
    mvprintw(35, 134, "         ");
  if (mana >= 4)
    mvprintw(36, 133, "           ");
  if (mana >= 3)
    mvprintw(37, 133, "_         _");
  if (mana >= 2)
    mvprintw(38, 134, "__     __");
  if (mana >= 1)
    mvprintw(39, 136, "_____");
  attroff(COLOR_PAIR(60));
  refresh();
}

void *manaRecharge() {
  while (1) {
    if (mana < 6) {
      mana++;
      printStatus();
      sleep(MANADELAY);
    }
  }
}

// Leitor de arquivos
void readArqs(int mapaId) {
  char pasta[13], diretorio[32];
  int R, G, B, F, BG;

  switch (mapaId) {
  case 1:
    strcpy(pasta, "mapas/mapa01");
    break;
  case 2:
    strcpy(pasta, "mapas/mapa02");
    break;
  case 3:
    strcpy(pasta, "mapas/mapa03");
    break;
  case 4:
    strcpy(pasta, "mapas/mapa04");
    break;
  case 5:
    strcpy(pasta, "mapas/mapa05");
    break;
  case 6:
    strcpy(pasta, "mapas/mapa06");
    break;
  case 7:
    strcpy(pasta, "mapas/mapa07");
    break;
  case 8:
    strcpy(pasta, "mapas/mapa08");
    break;
  case 9:
    strcpy(pasta, "mapas/mapa09");
    break;
  case 10:
    strcpy(pasta, "mapas/mapa10");
    break;
  case 11:
    strcpy(pasta, "mapas/mapa11");
    break;
  case 12:
    strcpy(pasta, "mapas/mapa12");
    break;
  case 13:
    strcpy(pasta, "mapas/mapa13");
    break;
  case 14:
    strcpy(pasta, "mapas/mapa14");
    break;
  case 15:
    strcpy(pasta, "mapas/mapa15");
    break;
  case 16:
    strcpy(pasta, "mapas/mapa16");
    break;
  case 17:
    strcpy(pasta, "mapas/mapa17");
    break;
  case 18:
    strcpy(pasta, "mapas/mapa18");
    break;
  case 19:
    strcpy(pasta, "mapas/mapa19");
    break;
  case 20:
    strcpy(pasta, "mapas/mapa20");
    break;
  case 21:
    strcpy(pasta, "mapas/mapa21");
    break;
  case 22:
    strcpy(pasta, "mapas/mapa22");
    break;
  case 23:
    strcpy(pasta, "mapas/mapa23");
    break;
  case 24:
    strcpy(pasta, "mapas/mapa24");
    break;
  case 25:
    strcpy(pasta, "mapas/mapa25");
    break;
  case 26:
    strcpy(pasta, "mapas/mapa26");
    break;
  case 27:
    strcpy(pasta, "mapas/mapa27");
    break;
  default:
    printf("\n\n\tDESCULPE, AINDA ESTAMOS TRABALHANDO NISSO :(\n");
  }

  FILE *arq;

  // leitura do arquivo permission.game
  sprintf(diretorio, "%s/permission.game", pasta);

  if ((arq = fopen(diretorio, "rb")) == NULL) {
    printf("ERRO AO ABRIR %s/permission.game\n", diretorio);
  } else {
    for (int y = 0; y <= YSIZE; y++) {
      for (int x = 0; x <= XSIZE; x++) {
        fread(&permission[y][x], sizeof(int), 1, arq);
      }
    }
    fclose(arq);
  }

  // leitura do arquivo map_rgb.game
  sprintf(diretorio, "%s/map_rgb.game", pasta);

  if ((arq = fopen(diretorio, "rb")) == NULL) {
    printf("ERRO AO ABRIR %s/map_rgb.game\n", diretorio);
  } else {
    for (int y = 0; y <= YSIZE; y++) {
      for (int x = 0; x <= XSIZE; x++) {
        fread(&map_rgb[y][x], sizeof(int), 1, arq);
      }
    }
    fclose(arq);
  }

  // leitura do arquivo map_chars.game
  sprintf(diretorio, "%s/map_chars.game", pasta);

  if ((arq = fopen(diretorio, "rb")) == NULL) {
    printf("ERRO AO ABRIR %s/map_chars.game\n", diretorio);
  } else {
    for (int y = 0; y <= YSIZE; y++) {
      for (int x = 0; x <= XSIZE; x++) {
        fread(&map_chars[y][x], sizeof(char), 1, arq);
      }
    }
    fclose(arq);
  }

  // leitura do arquivo rgb.game
  sprintf(diretorio, "%s/rgb.game", pasta);

  if ((arq = fopen(diretorio, "rb")) == NULL) {
    printf("ERRO AO ABRIR %s/rgb.game\n", diretorio);
  } else {
    rgb = (int **)malloc(sizeColorsAttr * sizeof(int *));

    for (int i = 0; i < sizeColorsAttr; i++) {
      rgb[i] = (int *)malloc(3 * sizeof(int));
    }

    fread(&R, sizeof(int), 1, arq);
    fread(&G, sizeof(int), 1, arq);
    fread(&B, sizeof(int), 1, arq);
    createColors(ColorID, R, G, B); // Cria as cores novas

    while (1) {
      sizeColorsAttr++;
      ColorID++;

      rgb = (int **)realloc(rgb, sizeColorsAttr * sizeof(int *));

      rgb[sizeColorsAttr - 1] = (int *)malloc(3 * sizeof(int));

      if (fread(&R, sizeof(int), 1, arq) == 0)
        break;
      if (fread(&G, sizeof(int), 1, arq) == 0)
        break;
      if (fread(&B, sizeof(int), 1, arq) == 0)
        break;
      createColors(ColorID, R, G, B); // Cria as cores novas
    }
    fclose(arq);
  }
  // Reseta o sizeColorAttr para a leitura do proximo arq

  printf("\n");

  // leitura do arquivo rgb_pair.game
  sprintf(diretorio, "%s/rgb_pair.game", pasta);

  if ((arq = fopen(diretorio, "rb")) == NULL) {
    printf("ERRO AO ABRIR %s/rgb_pair.game\n", diretorio);
  } else {
    rgb_pair = (int **)malloc(pairID * sizeof(int *));

    for (int i = 0; i < pairID; i++) {
      rgb_pair[i] = (int *)malloc(2 * sizeof(int));
    }

    fread(&F, sizeof(int), 1, arq);
    fread(&BG, sizeof(int), 1, arq);
    createPairs(pairID - 1, F, BG); // Cria os pares de cores

    while (1) {
      pairID++;

      rgb_pair = (int **)realloc(rgb_pair, pairID * sizeof(int *));

      rgb_pair[pairID - 1] = (int *)malloc(2 * sizeof(int));

      if (fread(&F, sizeof(int), 1, arq) == 0)
        break;
      if (fread(&BG, sizeof(int), 1, arq) == 0)
        break;
      createPairs(pairID - 1, F, BG); // Cria os pares de cores
    }
    fclose(arq);
  }
  ColorID = 8;
  sizeColorsAttr = 1;
  pairID = 1;
}

void readArqsIntro(int frame) {
  char diretorio[32];
  int R, G, B, F, BG;

  FILE *arq;

  // leitura do arquivo map_rgb.game
  sprintf(diretorio, "wallpaper/%d/map_rgb.game", frame);

  if ((arq = fopen(diretorio, "rb")) == NULL) {
    printf("ERRO AO ABRIR wallpaper/%d/map_rgb.game", frame);
  } else {
    for (int y = 0; y <= 45; y++) {
      for (int x = 0; x <= 150; x++) {
        fread(&map_rgb[y][x], sizeof(int), 1, arq);
      }
    }
    fclose(arq);
  }

  // leitura do arquivo rgb.game
  sprintf(diretorio, "wallpaper/%d/rgb.game", frame);

  if ((arq = fopen(diretorio, "rb")) == NULL) {
    printf("ERRO AO ABRIR wallpaper/%d/rgb.game", frame);
  } else {
    rgb = (int **)malloc(sizeColorsAttr * sizeof(int *));

    for (int i = 0; i < sizeColorsAttr; i++) {
      rgb[i] = (int *)malloc(3 * sizeof(int));
    }

    fread(&R, sizeof(int), 1, arq);
    fread(&G, sizeof(int), 1, arq);
    fread(&B, sizeof(int), 1, arq);
    createColors(ColorID, R, G, B); // Cria as cores novas

    while (1) {
      sizeColorsAttr++;
      ColorID++;

      rgb = (int **)realloc(rgb, sizeColorsAttr * sizeof(int *));

      rgb[sizeColorsAttr - 1] = (int *)malloc(3 * sizeof(int));

      if (fread(&R, sizeof(int), 1, arq) == 0)
        break;
      if (fread(&G, sizeof(int), 1, arq) == 0)
        break;
      if (fread(&B, sizeof(int), 1, arq) == 0)
        break;
      createColors(ColorID, R, G, B); // Cria as cores novas
    }
    fclose(arq);
  }
  // Reseta o sizeColorAttr para a leitura do proximo arq

  printf("\n");

  // leitura do arquivo rgb_pair.game
  sprintf(diretorio, "wallpaper/%d/rgb_pair.game", frame);

  if ((arq = fopen(diretorio, "rb")) == NULL) {
    printf("ERRO AO ABRIR wallpaper/%d/rgb_pair.game", frame);
  } else {
    rgb_pair = (int **)malloc(pairID * sizeof(int *));

    for (int i = 0; i < pairID; i++) {
      rgb_pair[i] = (int *)malloc(2 * sizeof(int));
    }

    fread(&F, sizeof(int), 1, arq);
    fread(&BG, sizeof(int), 1, arq);
    createPairs(pairID - 1, F, BG); // Cria os pares de cores

    while (1) {
      pairID++;

      rgb_pair = (int **)realloc(rgb_pair, pairID * sizeof(int *));

      rgb_pair[pairID - 1] = (int *)malloc(2 * sizeof(int));

      if (fread(&F, sizeof(int), 1, arq) == 0)
        break;
      if (fread(&BG, sizeof(int), 1, arq) == 0)
        break;
      createPairs(pairID - 1, F, BG); // Cria os pares de cores
    }
    fclose(arq);
  }
  ColorID = 8;
  sizeColorsAttr = 1;
  pairID = 1;
}

// Faz a movimentação
void walk(int y, int x, int move) {
  attron(COLOR_PAIR(map_rgb[yPos - 2][xPos]));
  mvprintw(yPos - 2, xPos, "");
  printw("%c", map_chars[yPos - 2][xPos]);
  attroff(COLOR_PAIR(map_rgb[yPos - 2][xPos]));

  attron(COLOR_PAIR(map_rgb[yPos - 1][xPos + 1]));
  mvprintw(yPos - 1, xPos + 1, "");
  printw("%c", map_chars[yPos - 1][xPos + 1]);
  attroff(COLOR_PAIR(map_rgb[yPos - 1][xPos + 1]));

  attron(COLOR_PAIR(map_rgb[yPos][xPos]));
  mvprintw(yPos, xPos, "");
  printw("%c", map_chars[yPos][xPos]);
  attroff(COLOR_PAIR(map_rgb[yPos][xPos]));

  attron(COLOR_PAIR(map_rgb[yPos][xPos - 1]));
  mvprintw(yPos, xPos - 1, "");
  printw("%c", map_chars[yPos][xPos - 1]);
  attroff(COLOR_PAIR(map_rgb[yPos][xPos - 1]));

  attron(COLOR_PAIR(map_rgb[yPos][xPos + 1]));
  mvprintw(yPos, xPos + 1, "");
  printw("%c", map_chars[yPos][xPos + 1]);
  attroff(COLOR_PAIR(map_rgb[yPos][xPos + 1]));

  attron(COLOR_PAIR(map_rgb[yPos - 1][xPos + 1]));
  mvprintw(yPos - 1, xPos + 1, "");
  printw("%c", map_chars[yPos - 1][xPos + 1]);
  attroff(COLOR_PAIR(map_rgb[yPos - 1][xPos + 1]));

  attron(COLOR_PAIR(map_rgb[yPos - 1][xPos - 1]));
  mvprintw(yPos - 1, xPos - 1, "");
  printw("%c", map_chars[yPos - 1][xPos - 1]);
  attroff(COLOR_PAIR(map_rgb[yPos - 1][xPos - 1]));

  if (move == 1) {
    if ((xPos % 4) == 0) {
      attron(COLOR_PAIR(map_rgb[y - 2][x]));
      mvprintw(y - 2, x, "O");
      attroff(COLOR_PAIR(map_rgb[y - 2][x]));

      attron(COLOR_PAIR(map_rgb[y - 1][x]));
      mvprintw(y - 1, x, "|");
      attroff(COLOR_PAIR(map_rgb[y - 1][x]));

      attron(COLOR_PAIR(map_rgb[y][x - 1]));
      mvprintw(y, x - 1, "<");
      attroff(COLOR_PAIR(map_rgb[y][x - 1]));

      attron(COLOR_PAIR(map_rgb[y][x + 1]));
      mvprintw(y, x, "/");
      attroff(COLOR_PAIR(map_rgb[y][x + 1]));

      attron(COLOR_PAIR(map_rgb[y - 1][x + 1]));
      mvprintw(y - 1, x + 1, ">");
      attroff(COLOR_PAIR(map_rgb[y - 1][x + 1]));

      attron(COLOR_PAIR(map_rgb[y - 1][x - 1]));
      mvprintw(y - 1, x - 1, "/");
      attroff(COLOR_PAIR(map_rgb[y - 1][x - 1]));
    } else {
      attron(COLOR_PAIR(map_rgb[y - 2][x]));
      mvprintw(y - 2, x, "O");
      attroff(COLOR_PAIR(map_rgb[y - 2][x]));

      attron(COLOR_PAIR(map_rgb[y - 1][x]));
      mvprintw(y - 1, x, "|");
      attroff(COLOR_PAIR(map_rgb[y - 1][x]));

      attron(COLOR_PAIR(map_rgb[y][x]));
      mvprintw(y, x, "<");
      attroff(COLOR_PAIR(map_rgb[y][x]));

      attron(COLOR_PAIR(map_rgb[y][x - 1]));
      mvprintw(y, x - 1, "/");
      attroff(COLOR_PAIR(map_rgb[y][x - 1]));

      attron(COLOR_PAIR(map_rgb[y - 1][x + 1]));
      mvprintw(y - 1, x + 1, ">");
      attroff(COLOR_PAIR(map_rgb[y - 1][x + 1]));

      attron(COLOR_PAIR(map_rgb[y - 1][x - 1]));
      mvprintw(y - 1, x - 1, "/");
      attroff(COLOR_PAIR(map_rgb[y - 1][x - 1]));
    }
  }

  if (move == 2) {
    if ((xPos % 4) == 0) {
      attron(COLOR_PAIR(map_rgb[y - 2][x]));
      mvprintw(y - 2, x, "O");
      attroff(COLOR_PAIR(map_rgb[y - 2][x]));

      attron(COLOR_PAIR(map_rgb[y - 1][x]));
      mvprintw(y - 1, x, "|");
      attroff(COLOR_PAIR(map_rgb[y - 1][x]));

      attron(COLOR_PAIR(map_rgb[y][x]));
      mvprintw(y, x, ">");
      attroff(COLOR_PAIR(map_rgb[y][x]));

      attron(COLOR_PAIR(map_rgb[y][x + 1]));
      mvprintw(y, x + 1, "\\");
      attroff(COLOR_PAIR(map_rgb[y][x + 1]));

      attron(COLOR_PAIR(map_rgb[y - 1][x + 1]));
      mvprintw(y - 1, x + 1, "\\");
      attroff(COLOR_PAIR(map_rgb[y - 1][x + 1]));

      attron(COLOR_PAIR(map_rgb[y - 1][x - 1]));
      mvprintw(y - 1, x - 1, "<");
      attroff(COLOR_PAIR(map_rgb[y - 1][x - 1]));
    } else {
      attron(COLOR_PAIR(map_rgb[y - 2][x]));
      mvprintw(y - 2, x, "O");
      attroff(COLOR_PAIR(map_rgb[y - 2][x]));

      attron(COLOR_PAIR(map_rgb[y - 1][x]));
      mvprintw(y - 1, x, "|");
      attroff(COLOR_PAIR(map_rgb[y - 1][x]));

      attron(COLOR_PAIR(map_rgb[y][x]));
      mvprintw(y, x, "\\");
      attroff(COLOR_PAIR(map_rgb[y][x]));

      attron(COLOR_PAIR(map_rgb[y][x + 1]));
      mvprintw(y, x + 1, ">");
      attroff(COLOR_PAIR(map_rgb[y][x + 1]));

      attron(COLOR_PAIR(map_rgb[y - 1][x + 1]));
      mvprintw(y - 1, x + 1, "\\");
      attroff(COLOR_PAIR(map_rgb[y - 1][x + 1]));

      attron(COLOR_PAIR(map_rgb[y - 1][x - 1]));
      mvprintw(y - 1, x - 1, "<");
      attroff(COLOR_PAIR(map_rgb[y - 1][x - 1]));
    }
  } else

      if (move == 3) {
    if ((yPos % 3) == 0) {
      attron(COLOR_PAIR(map_rgb[y - 2][x]));
      mvprintw(y - 2, x, "O");
      attroff(COLOR_PAIR(map_rgb[y - 2][x]));

      attron(COLOR_PAIR(map_rgb[y - 1][x]));
      mvprintw(y - 1, x, "|");
      attroff(COLOR_PAIR(map_rgb[y - 1][x]));

      attron(COLOR_PAIR(map_rgb[y][x]));
      mvprintw(y, x, "|");
      attroff(COLOR_PAIR(map_rgb[y][x]));

      attron(COLOR_PAIR(map_rgb[y][x - 1]));
      mvprintw(y, x - 1, "(");
      attroff(COLOR_PAIR(map_rgb[y][x - 1]));

      attron(COLOR_PAIR(map_rgb[y - 1][x + 1]));
      mvprintw(y - 1, x + 1, ">");
      attroff(COLOR_PAIR(map_rgb[y - 1][x + 1]));

      attron(COLOR_PAIR(map_rgb[y - 1][x - 1]));
      mvprintw(y - 1, x - 1, "<");
      attroff(COLOR_PAIR(map_rgb[y - 1][x - 1]));
    } else {
      attron(COLOR_PAIR(map_rgb[y - 2][x]));
      mvprintw(y - 2, x, "O");
      attroff(COLOR_PAIR(map_rgb[y - 2][x]));

      attron(COLOR_PAIR(map_rgb[y - 1][x]));
      mvprintw(y - 1, x, "|");
      attroff(COLOR_PAIR(map_rgb[y - 1][x]));

      attron(COLOR_PAIR(map_rgb[y][x - 1]));
      mvprintw(y, x - 1, "/");
      attroff(COLOR_PAIR(map_rgb[y][x - 1]));

      attron(COLOR_PAIR(map_rgb[y][x]));
      mvprintw(y, x, "|");
      attroff(COLOR_PAIR(map_rgb[y][x]));

      attron(COLOR_PAIR(map_rgb[y - 1][x + 1]));
      mvprintw(y - 1, x + 1, ">");
      attroff(COLOR_PAIR(map_rgb[y - 1][x + 1]));

      attron(COLOR_PAIR(map_rgb[y - 1][x - 1]));
      mvprintw(y - 1, x - 1, "<");
      attroff(COLOR_PAIR(map_rgb[y - 1][x - 1]));
    }
  }

  yPos = y;
  xPos = x;
}

void apagaPlayer() {
  attron(COLOR_PAIR(map_rgb[yPos - 2][xPos]));
  mvprintw(yPos - 2, xPos, "");
  printw("%c", map_chars[yPos - 2][xPos]);
  attroff(COLOR_PAIR(map_rgb[yPos - 2][xPos]));

  attron(COLOR_PAIR(map_rgb[yPos - 2][xPos - 1]));
  mvprintw(yPos - 2, xPos, "");
  printw("%c", map_chars[yPos - 2][xPos - 1]);
  attroff(COLOR_PAIR(map_rgb[yPos - 2][xPos - 1]));

  attron(COLOR_PAIR(map_rgb[yPos - 2][xPos + 1]));
  mvprintw(yPos - 2, xPos, "");
  printw("%c", map_chars[yPos - 2][xPos + 1]);
  attroff(COLOR_PAIR(map_rgb[yPos - 2][xPos + 1]));

  attron(COLOR_PAIR(map_rgb[yPos - 1][xPos + 1]));
  mvprintw(yPos - 1, xPos + 1, "");
  printw("%c", map_chars[yPos - 1][xPos + 1]);
  attroff(COLOR_PAIR(map_rgb[yPos - 1][xPos + 1]));

  attron(COLOR_PAIR(map_rgb[yPos][xPos]));
  mvprintw(yPos, xPos, "");
  printw("%c", map_chars[yPos][xPos]);
  attroff(COLOR_PAIR(map_rgb[yPos][xPos]));

  attron(COLOR_PAIR(map_rgb[yPos][xPos - 1]));
  mvprintw(yPos, xPos - 1, "");
  printw("%c", map_chars[yPos][xPos - 1]);
  attroff(COLOR_PAIR(map_rgb[yPos][xPos - 1]));

  attron(COLOR_PAIR(map_rgb[yPos][xPos + 1]));
  mvprintw(yPos, xPos + 1, "");
  printw("%c", map_chars[yPos][xPos + 1]);
  attroff(COLOR_PAIR(map_rgb[yPos][xPos + 1]));

  attron(COLOR_PAIR(map_rgb[yPos - 1][xPos + 1]));
  mvprintw(yPos - 1, xPos + 1, "");
  printw("%c", map_chars[yPos - 1][xPos + 1]);
  attroff(COLOR_PAIR(map_rgb[yPos - 1][xPos + 1]));

  attron(COLOR_PAIR(map_rgb[yPos - 1][xPos - 1]));
  mvprintw(yPos - 1, xPos - 1, "");
  printw("%c", map_chars[yPos - 1][xPos - 1]);
  attroff(COLOR_PAIR(map_rgb[yPos - 1][xPos - 1]));
}

// Ouve o teclado
void keyBoardListener() {
  char c; // recebe a tecla digitada

  while ((c = getch()) != 'q') {

    switch (c) {
    case 'A':
    case 'a':

      if (permission[yPos][xPos - 1] && (permission[yPos][xPos - 2])) {
        walk(yPos, (xPos - 1), 1);
      }

      break;

    case 'D':
    case 'd':

      if ((permission[yPos][xPos + 1]) && (permission[yPos][xPos + 2])) {
        walk(yPos, (xPos + 1), 2);
      }

      break;

    case 'W':
    case 'w':

      if (permission[yPos - 1][xPos]) {
        walk((yPos - 1), xPos, 3);
      }

      break;

    case 'S':
    case 's':

      if (permission[yPos + 1][xPos]) {
        walk((yPos + 1), xPos, 3);
      }

      break;

    case 'E':
    case 'e':
      if ((permission[yPos][xPos] != 1) || (permission[yPos][xPos + 1] != 1) ||
          (permission[yPos][xPos - 1] != 1)) {
        if (permission[yPos][xPos] != 1) {
          idIdentifier(permission[yPos][xPos]);
        } else if (permission[yPos][xPos + 1] != 1) {
          idIdentifier(permission[yPos][xPos + 1]);
        } else if (permission[yPos][xPos - 1] != 1) {
          idIdentifier(permission[yPos][xPos - 1]);
        }
      }
      if (GAME_OVER) {
        if (mapa == 5)
          morte(1);
        else
          morte(0);
      } else if ((mapa == 5) && (rumptDerrot == 1))
        pos();
      break;

    case 'C':
    case 'c':

      turnSong(mapa);
      printMap();
      printStatus();
      walk(yPos, xPos, 3);

      break;
    }
  }
}

// Transfere o id do mapa recebido para uma variável global
void selectMap(int idMap) { mapa = idMap; }

// Limpa a janela de diálogos
void clearDialog() {
  for (int i = 0; i < 6; ++i) {
    for (int j = 0; j < 98; ++j) {
      mvwprintw(dialog, i, j, " ");
    }
  }
  wrefresh(dialog);
}

// Printa caractere a caractere pausadamente
void printPausado(char *frase) {
  char cont_frase[512];
  int num_elem = 0, y = 0;
  sprintf(cont_frase, "%s", frase);
  while (cont_frase[num_elem] != '\0') {
    num_elem++;
  }
  if ((frase[7] > 96) || (frase[8] > 96) || (frase[9] > 96)) {
    for (int x = 0, i = 0; i < num_elem; x++, i++) {
      mvwprintw(dialog, y, x, "%c", frase[i]);
      wrefresh(dialog);
      usleep(7000);
      if (x > 70) {
        x = 0;
        y++;
      }
    }
  } else {
    for (int x = 0, i = 0; i < num_elem; x++, i++) {
      mvwprintw(dialog, y, x, "%c", frase[i]);
      wrefresh(dialog);
      usleep(50000);
      if (x > 70) {
        x = 0;
        y++;
      }
    }
  }
}

void printPausadoIntro(int y, char *frase) {
  char cont_frase[512];
  int num_elem = 0;
  sprintf(cont_frase, "%s", frase);
  while (cont_frase[num_elem] != '\0') {
    num_elem++;
  }

  for (int x = 0, i = 0; i < num_elem; x++, i++) {
    mvprintw(y, x, "%c", frase[i]);
    refresh();
    usleep(7000);
    if (x > 100) {
      x = 0;
      y++;
    }
  }
}

// Printa os diálogos do jogo
void dialogo(int actionId) {
  int nexec;

  dialog = newwin(6, 98, 32, 2);

  switch (mapa) {
  case 1:
    nexec = atoi(casa(actionId, 1, -1));
    for (int i = 0; i < nexec; ++i) {
      printPausado(casa(actionId, 2, i));
      getch();
      clearDialog();
    }
    break;

  case 2:
    nexec = atoi(casa_to_covil(actionId, 1, -1));
    for (int i = 0; i < nexec; ++i) {
      printPausado(casa_to_covil(actionId, 2, i));
      getch();
      clearDialog();
    }
    break;

  case 3:
    nexec = atoi(covil(actionId, 1, -1));
    for (int i = 0; i < nexec; ++i) {
      printPausado(covil(actionId, 2, i));
      getch();
      clearDialog();
    }
    quizz(3);
    break;

  case 4:
    nexec = atoi(muro(actionId, 1, -1));
    for (int i = 0; i < nexec; ++i) {
      printPausado(muro(actionId, 2, i));
      getch();
      clearDialog();
    }
    quizz(6);
    apagaPlayer();
    xPos = 70;
    yPos = 15;
    break;

  case 5:
    nexec = atoi(inferno(actionId, 1, -1));
    for (int i = 0; i < nexec; ++i) {
      printPausado(inferno(actionId, 2, i));
      getch();
      clearDialog();
    }
    quizz(6);
    rumptDerrot = 1;
    break;
  }
}

void quizz(int id) {
  int nexec, i = 0;
  char resposta[2] = {'0', '0'};

  dialog = newwin(6, 98, 32, 2);

  do {
    switch (mapa) {
    case 1:
      nexec = atoi(casa(id, 1, -1));
      mvwprintw(dialog, 0, 0, "%s", casa(id, 2, i));
      i++;
      mvwprintw(dialog, 2, 0, "%s", casa(id, 2, i));
      i++;
      mvwprintw(dialog, 3, 0, "%s", casa(id, 2, i));
      i++;
      mvwprintw(dialog, 4, 0, "%s", casa(id, 2, i));
      i++;

      break;

    case 2:
      nexec = atoi(casa_to_covil(id, 1, -1));
      mvwprintw(dialog, 0, 0, "%s", casa_to_covil(id, 2, i));
      i++;
      mvwprintw(dialog, 2, 0, "%s", casa_to_covil(id, 2, i));
      i++;
      mvwprintw(dialog, 3, 0, "%s", casa_to_covil(id, 2, i));
      i++;
      mvwprintw(dialog, 3, 0, "%s", casa_to_covil(id, 2, i));
      i++;
      break;
    case 3:
      nexec = atoi(covil(id, 1, -1));
      mvwprintw(dialog, 0, 0, "%s", covil(id, 2, i));
      i++;
      mvwprintw(dialog, 2, 0, "%s", covil(id, 2, i));
      i++;
      mvwprintw(dialog, 3, 0, "%s", covil(id, 2, i));
      i++;
      mvwprintw(dialog, 4, 0, "%s", covil(id, 2, i));
      i++;
      break;
    case 4:
      nexec = atoi(muro(id, 1, -1));
      mvwprintw(dialog, 0, 0, "%s", muro(id, 2, i));
      i++;
      mvwprintw(dialog, 2, 0, "%s", muro(id, 2, i));
      i++;
      mvwprintw(dialog, 3, 0, "%s", muro(id, 2, i));
      i++;
      mvwprintw(dialog, 4, 0, "%s", muro(id, 2, i));
      i++;
      break;
    case 5:
      nexec = atoi(inferno(id, 1, -1));
      mvwprintw(dialog, 0, 0, "%s", inferno(id, 2, i));
      i++;
      mvwprintw(dialog, 2, 0, "%s", inferno(id, 2, i));
      i++;
      mvwprintw(dialog, 3, 0, "%s", inferno(id, 2, i));
      i++;
      mvwprintw(dialog, 4, 0, "%s", inferno(id, 2, i));
      i++;
      break;
    }

    mvwprintw(dialog, 5, 0, "*digite o numero da resposta*");
    wrefresh(dialog);

    resposta[1] = getch();

    switch (mapa) {
    case 1:
      if (atoi(resposta) == atoi(casa(id, 2, i))) {
        if (hp < 3)
          printStatus();
      } else {
        if (hp > 0)
          hp--;
        printStatus();
      }
      break;
    case 2:
      if (atoi(resposta) == atoi(casa_to_covil(id, 2, i))) {
        if (hp < 3)
          printStatus();
      } else {
        if (hp > 0)
          hp--;
        printStatus();
      }
      break;
    case 3:
      if (atoi(resposta) == atoi(covil(id, 2, i))) {
        if (hp < 3)
          printStatus();
      } else {
        if (hp > 0)
          hp--;
        printStatus();
      }
      break;
    case 4:
      if (atoi(resposta) == atoi(muro(id, 2, i))) {
        if (hp < 3)
          printStatus();
      } else {
        if (hp > 0)
          hp--;
        printStatus();
      }
      break;
    case 5:
      if (atoi(resposta) == atoi(inferno(id, 2, i))) {
        if (hp < 3)
          printStatus();
      } else {
        if (hp > 0)
          hp--;
        printStatus();
      }
      break;
    }
    i++;
    clearDialog();
    if (hp == 0) {
      GAME_OVER = 1;
      break;
    }
  } while (i < 5 * nexec);

  clearDialog();
}

// Muda o mapa
void changeMap(int id) {
  int idmapa;
  switch (mapa) {
  case 1:
    idmapa = atoi(casa(id, 2, -1));
    break;
  case 2:
    idmapa = atoi(casa_to_covil(id, 2, -1));
    break;
  case 3:
    idmapa = atoi(covil(id, 2, -1));
    break;
  case 4:
    idmapa = atoi(muro(id, 2, -1));
    break;
  case 5:
    idmapa = atoi(inferno(id, 2, -1));
    break;
  }
  switch (idmapa) {
  case 1:
    xPos = 10;
    yPos = 15;
    break;
  case 2:
    xPos = 5;
    yPos = 15;
    break;
  case 3:
    xPos = 5;
    yPos = 17;
    break;
  case 4:
    xPos = 5;
    yPos = 28;
    break;
  case 5:
    xPos = 3;
    yPos = 15;
    break;
  }
  selectMap(idmapa);
  readArqs(idmapa);
  selectSong(idmapa);
  printMap();
  printStatus();
}

void item(int id) {
  int item;
  dialog = newwin(6, 98, 32, 2);
  refresh();

  switch (mapa) {
  case 1:
    item = atoi(casa(id, 2, -1));
    break;
  case 2:
    item = atoi(casa_to_covil(id, 2, -1));
    break;
  case 3:
    item = atoi(covil(id, 2, -1));
    break;
  case 4:
    item = atoi(muro(id, 2, -1));
    break;
  case 5:
    item = atoi(inferno(id, 2, -1));
    break;
  }

  switch (item) {
  case 1:
    item_mapa = 1;
    printMap();
    printStatus();
    mvwprintw(dialog, 0, 0, "VOCE ACHOU UM MAPA E UM BILHETE");
    wrefresh(dialog);
    printMap();
    printStatus();
    getch();
    clearDialog();
    mvwprintw(dialog, 0, 0,
              ""
              "Ola Jari,");
    mvwprintw(dialog, 1, 0,
              "Como tenho dependências que não vou cumprir, não posso sair do "
              "meu covil.,");
    mvwprintw(dialog, 2, 0, "Quando precisar de mim venha até aqui....,");
    wrefresh(dialog);
    getch();
    clearDialog();
    mvwprintw(dialog, 0, 0,
              "Ps: Com saturno retrogrado e a influencia da lua sobre Aries o "
              "seu signo, acredito que passará \npor grandes "
              "provações.\nAbraços,\n\nSr. Oak de Oak.");
    wrefresh(dialog);
    getch();
    clearDialog();
    break;
  }
}

// Comanda a chama de funções
void controller(int actionId, int id) {
  switch (actionId) {
  case -1:
    selectMap(1);
    readArqs(mapa);
    selectSong(1);
    printMap();
    printStatus();
    break;

  case 2:
    dialogo(id);
    break;
  case 3:
    changeMap(id);
    break;
  case 4:
    quizz(id);
    break;
  case 5:
    item(id);
    break;
  default:
    break;
  }
}

// Identifica os ids para passar para o controller
void idIdentifier(int id) {
  switch (mapa) {
  case 1:
    controller(atoi(casa(id, 0, -1)), id);
    break;
  case 2:
    controller(atoi(casa_to_covil(id, 0, -1)), id);
    break;
  case 3:
    controller(atoi(covil(id, 0, -1)), id);
    break;
  case 4:
    controller(atoi(muro(id, 0, -1)), id);
    break;
  case 5:
    controller(atoi(inferno(id, 0, -1)), id);
    break;
  case -2:
    controller(id + 1, -1);
    break;
  }
}

void *pularIntro() {
  char c;

  c = getch();
  pular = 1;
}

void limpaTelaIntro() {
  for (int x = 0; x < xmax; x++) {
    for (int y = 0; y < ymax; y++) {
      attron(COLOR_PAIR(222));
      mvprintw(y, x, "");
      printw(" ");
      attroff(COLOR_PAIR(222));
    }
  }
}

void startGame() {
  init_color(222, 110, 129, 114);
  init_pair(222, 7, 222);

  selectSong(8);
  for (int x = 0; x < xmax; x++) {
    for (int y = 0; y < ymax; y++) {
      attron(COLOR_PAIR(222));
      mvprintw(y, x, "");
      printw(" ");
      attroff(COLOR_PAIR(222));
    }
  }
  refresh();
  usleep(2500000);

  pthread_create(&pular_t, NULL, &pularIntro, NULL);

  while (1) {
    readArqsIntro(1);
    for (int x = 0; x < xmax; x++) {
      for (int y = 0; y < ymax; y++) {
        attron(COLOR_PAIR(222));
        mvprintw(y, x, "");
        printw(" ");
        attroff(COLOR_PAIR(222));
      }
    }
    for (int x = 1; x < 149; x++) {
      for (int y = 1; y < 45; y++) {
        attron(COLOR_PAIR(map_rgb[y][x]));
        mvprintw(y, x, "");
        printw(" ", map_chars[y][x]);
        attroff(COLOR_PAIR(map_rgb[y][x]));
      }
    }
    refresh();

    usleep(1200000);
    if (pular)
      break;

    readArqsIntro(2);
    for (int x = 1; x < 149; x++) {
      for (int y = 1; y < 45; y++) {
        attron(COLOR_PAIR(0));
        mvprintw(y, x, "");
        printw(" ");
        attroff(COLOR_PAIR(0));
      }
    }
    for (int x = 1; x < 149; x++) {
      for (int y = 1; y < 45; y++) {
        attron(COLOR_PAIR(map_rgb[y][x]));
        mvprintw(y, x, "");
        printw(" ", map_chars[y][x]);
        attroff(COLOR_PAIR(map_rgb[y][x]));
      }
    }
    refresh();

    usleep(700000);
    if (pular)
      break;

    readArqsIntro(3);
    for (int x = 1; x < 149; x++) {
      for (int y = 1; y < 45; y++) {
        attron(COLOR_PAIR(0));
        mvprintw(y, x, "");
        printw(" ");
        attroff(COLOR_PAIR(0));
      }
    }
    for (int x = 1; x < 149; x++) {
      for (int y = 1; y < 45; y++) {
        attron(COLOR_PAIR(map_rgb[y][x]));
        mvprintw(y, x, "");
        printw(" ", map_chars[y][x]);
        attroff(COLOR_PAIR(map_rgb[y][x]));
      }
    }
    refresh();

    usleep(700000);
    if (pular)
      break;

    readArqsIntro(4);
    for (int x = 1; x < 149; x++) {
      for (int y = 1; y < 45; y++) {
        attron(COLOR_PAIR(0));
        mvprintw(y, x, "");
        printw(" ");
        attroff(COLOR_PAIR(0));
      }
    }
    for (int x = 1; x < 149; x++) {
      for (int y = 1; y < 45; y++) {
        attron(COLOR_PAIR(map_rgb[y][x]));
        mvprintw(y, x, "");
        printw(" ", map_chars[y][x]);
        attroff(COLOR_PAIR(map_rgb[y][x]));
      }
    }
    refresh();

    usleep(700000);
    if (pular)
      break;

    readArqsIntro(5);
    for (int x = 1; x < 149; x++) {
      for (int y = 1; y < 45; y++) {
        attron(COLOR_PAIR(0));
        mvprintw(y, x, "");
        printw(" ");
        attroff(COLOR_PAIR(0));
      }
    }
    for (int x = 1; x < 149; x++) {
      for (int y = 1; y < 45; y++) {
        attron(COLOR_PAIR(map_rgb[y][x]));
        mvprintw(y, x, "");
        printw(" ", map_chars[y][x]);
        attroff(COLOR_PAIR(map_rgb[y][x]));
      }
    }
    refresh();

    usleep(700000);
    if (pular)
      break;
  }

  system("killall vlc");
  for (int x = 0; x < xmax; x++) {
    for (int y = 0; y < ymax; y++) {
      attron(COLOR_PAIR(222));
      mvprintw(y, x, "");
      printw(" ");
      attroff(COLOR_PAIR(222));
    }
  }

  refresh();
  pular = 0;
  printPausadoIntro(2, "Terra Plana. Ano 129. Dia 1");
  printPausadoIntro(4, "Após 27 anos servindo a RumpT, Jari finalmente "
                       "conseguiria o que almejava....");
  sleep(3);
  limpaTelaIntro();
  printPausadoIntro(2, "Jari: Finalmente eu vou DOMINAR O  MUNDO!!");
  sleep(2);
  limpaTelaIntro();
  printPausadoIntro(
      2, "Jari: Aqui está RumpT, tudo o que me pediu, cedo tudo ao hell.");
  sleep(2);
  limpaTelaIntro();
  printPausadoIntro(2, "RumpT: Eu deveria dizer obrigado....");
  sleep(2);
  limpaTelaIntro();
  printPausadoIntro(2, "Jari:..... Mas por que?");
  sleep(2);
  limpaTelaIntro();
  printPausadoIntro(
      2,
      "RumpT: Pela sua lealdade cega, por me entregar o que lhe pedi. Parabéns "
      "JARI, você é realmente um idiota. Quando eu disse “Inferno para os "
      "infernianos” você se considerou um inferniano? Vocês Outsiders, como "
      "OUSAM se considerar do inferno? Você não é nada JARI.");
  sleep(6);
  limpaTelaIntro();
  printPausadoIntro(2, "*RumpT  desmaia Jari.");
  sleep(2);
  limpaTelaIntro();
  printPausadoIntro(2, "*Jari  acorda jogado em seu quarto.");
  sleep(2);
  limpaTelaIntro();
  printPausadoIntro(2, "Jari: Hamm....... O que aconteceu?");
  sleep(2);
  limpaTelaIntro();
  printPausadoIntro(2, "Jari: RumpT... não irá ficar assim... eu ainda tenho o "
                       "apoio de uma pessoa....");
  sleep(3);
  limpaTelaIntro();
}

void main() {
  initscr();
  noecho();
  curs_set(0);
  start_color();
  getmaxyx(stdscr, ymax, xmax);
  raw();

  startGame();
  selectMap(-2);
  idIdentifier(-2);
  pthread_create(&mana_t, NULL, &manaRecharge, NULL);
  keyBoardListener();
  system("killall vlc");
  endwin();
}
