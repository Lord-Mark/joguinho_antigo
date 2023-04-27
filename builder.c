#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>

//__________________________//\
   //*******/##########/*******//  \
  //*******/TODO LIST /*******//    \
 //*******/##########/*******//______\ 
/*|                          |   __  |
/*x      Menu de cores       x  |  | |
/*|__________________________|__| ²|_|
/*+##########################vxxxXXXXX+
            __#X#__|
/**
* Tamanho da tela
*/
#define SIZEX 100
#define SIZEY 30

/**
 * Algumas cores predefinidas
 */
#define VERDAO 8
#define VERDINHO 9

char mapCharacter[SIZEY + 1][SIZEX + 1], charKeeped = ' ';

int keepChangingChar = 0, charMode = 0;

int xPos, yPos,                          // Posição do personagem
    mapPermission[SIZEY + 1][SIZEX + 1], // Matriz das permissões
    mapColors[SIZEY + 1][SIZEX + 1],     // Matriz das cores
    **colorsAtributesSave,               //
    **pairColorSave, sizeColorsAttr = 0;

int colorSwitch = 0,
    colorChange = 0, // A cor que será colocada no rastro do personagem
    permission = 1;  // quando == 1 o rastro do personagem irá marcar como
                     // permitido quando == 0 será sem permissão

int makeColorChange = 0, // deixa o personagem neutro em relação às cores ao
                         // caminhar quando == 0
    makePermissionChange = 0, // deixa o personagem neutro em relação às
                              // permissões ao caminhar quando == 0
    makeInteractionChange = 0;

int colorNumberID = 8; // começa em 8 porque já existem constantes de cor
                       // definidas na biblioteca ncurses

int pairID = 0;

int interactionId;

void setPair();           // Seta um par de cor
void setColorAtributes(); // Seta os RGB de uma cor

//*******************************************//###################//*******************************************//
//*******************************************// Alocação Dinâmica
////*******************************************//
//*******************************************//###################//*******************************************//

void colorAlloc(int id, int R, int G, int B) {

  if (id == -1) {
    /* Armazena os atributos passados*/
    colorsAtributesSave[sizeColorsAttr - 1][0] = R;
    colorsAtributesSave[sizeColorsAttr - 1][1] = G;
    colorsAtributesSave[sizeColorsAttr - 1][2] = B;

    /*Aloca o próximo espaço*/
    colorsAtributesSave = (int **)realloc(colorsAtributesSave,
                                          (sizeColorsAttr + 1) * sizeof(int *));

    /* Aloca os 3 espaços na nova posição*/
    colorsAtributesSave[sizeColorsAttr] = (int *)malloc(3 * sizeof(int));

    mvprintw(sizeColorsAttr + 1, 5, "");
    printw("%d, %d, %d, %d", colorsAtributesSave[sizeColorsAttr - 1][0],
           colorsAtributesSave[sizeColorsAttr - 1][1],
           colorsAtributesSave[sizeColorsAttr - 1][2], colorNumberID);

    sizeColorsAttr++;
  } else {
    /* Armazena os atributos passados*/
    colorsAtributesSave[id - 8][0] = R;
    colorsAtributesSave[id - 8][1] = G;
    colorsAtributesSave[id - 8][2] = B;

    mvprintw(sizeColorsAttr + 1, 5, "");
    printw("%d, %d, %d, %d", colorsAtributesSave[sizeColorsAttr - 1][0],
           colorsAtributesSave[sizeColorsAttr - 1][1],
           colorsAtributesSave[sizeColorsAttr - 1][2], id);
  }
}

void pairColorAlloc(int id1, int id2) {

  /* Armazena os atributos passados*/
  pairColorSave[pairID - 1][0] = id1;
  pairColorSave[pairID - 1][1] = id2;

  /*Aloca o próximo espaço*/
  pairColorSave = (int **)realloc(pairColorSave, (pairID + 1) * sizeof(int *));
  /* Aloca os 2 espaços na nova posição*/
  pairColorSave[pairID] = (int *)malloc(2 * sizeof(int));

  // mvprintw(pairID+1, 35, "");

  /*Por algum motivo mirabolantet esse print causa falha de segmentação*/
  // printw("%d, %d, %d", pairColorSave[pairID-1][0],
  // colorsAtributesSave[pairID-1][1], pairID-1);

  // pairID não é incrementado aqui, pois também é usado em 'setPair'
}

//*******************************************//####################//*******************************************//
//*******************************************// Operações Iniciais
////*******************************************//
//*******************************************//####################//*******************************************//

/*
 * Só cria os pares de cores, aqui você pode criar manualmente seus pares se
 * preferir
 */
void colorsList() {

  setColorAtributes(-1, 200, 500, 222); // VERDAO 8
  setColorAtributes(-1, 200, 700, 222); // VERDINHO 9
  setColorAtributes(-1, 55, 22, 30);    // Preto 10

  setPair(COLOR_WHITE, COLOR_BLACK); // Cor do fundo, deixe este par como o
                                     // primeiro, para que ele fique na pos 0
  setPair(COLOR_WHITE, 8);           // VERDAO
  setPair(COLOR_WHITE, 9);           // VERDINHO
  setPair(COLOR_WHITE, 10);          // Preto
}

/*
 * Delimita as laterais da matriz, só pro construtor saber onde a matriz está
 * Também já define as bordas com permissão zero
 */
void startMap() {
  for (int x = 0; x <= SIZEX; x++) {
    for (int y = 0; y <= SIZEY; y++) {
      mapColors[y][x] = 0; // Seta o mapa com o padrão branco com fundo preto
      mapPermission[y][x] = 1;  // Todo o mapa tem permissão 1
      mapCharacter[y][x] = ' '; // Não há caracteres no mapa ainda
    }
  }
}

void showMap() {

  /*
   * Delimita as bordas da matriz)
   */
  int x, y, i;

  if (charMode) { // Se estiver em charMode, então printa os char ao invés das
                  // permissions
    for (x = 0; x < SIZEX; x++) {
      for (int y = 0; y < SIZEY; y++) {
        attron(COLOR_PAIR(mapColors[y][x]));
        mvprintw(y, x, "");
        printw("%c", mapCharacter[y][x]);
        attroff(COLOR_PAIR(mapColors[y][x]));
      }
    }

  } else {
    for (x = 0; x < SIZEX; x++) {
      for (int y = 0; y < SIZEY; y++) {

        if ((mapPermission[y][x]) == 1) { // Se for 1 o id da permission não é
                                          // mostrado (porque é o default)
          attron(COLOR_PAIR(mapColors[y][x]));
          mvprintw(y, x, " ");
          attroff(COLOR_PAIR(mapColors[y][x]));

        } else { // Se não for 1, então o id é printado

          attron(COLOR_PAIR(mapColors[y][x]));
          mvprintw(y, x, "");
          printw("%d", mapPermission[y][x]);
          attroff(COLOR_PAIR(mapColors[y][x]));
        }
      }
    }
  }

  // Só delimita as bordas (Já que os players não podem sair do mapa)

  /********* BORDA EIXO X **********/
  for (int x = 0; x <= SIZEX; x++) {
    mapPermission[0][x] = 0;
    mapPermission[SIZEY][x] = 0;

    // Sim, podem haver cores nas bordas X
    attron(COLOR_PAIR(mapColors[0][x]));
    mvprintw(0, x, "X");
    attroff(COLOR_PAIR(mapColors[0][x]));

    attron(COLOR_PAIR(mapColors[SIZEY][x]));
    mvprintw(SIZEY, x, "X");
    attroff(COLOR_PAIR(mapColors[SIZEY][x]));
  }

  /********* BORDA EIXO X **********/
  for (int y = 0; y <= SIZEY; y++) {
    mapPermission[y][0] = 0;
    mapPermission[y][SIZEX] = 0;

    // Olha só que doidera, podem haver cores nas bordas Y também! XD
    attron(COLOR_PAIR(mapColors[y][0]));
    mvprintw(y, 0, "X");
    attroff(COLOR_PAIR(mapColors[y][0]));

    attron(COLOR_PAIR(mapColors[y][SIZEY]));
    mvprintw(y, SIZEX, "X");
    attroff(COLOR_PAIR(mapColors[y][SIZEY]));
  }
  // mvprintw(SIZEY, SIZEX, "X"); // só pra fechar um canto que fica aberto

  // mvprintw(20, 40, "");
  // printw("Existem %d cores criadas", colorNumberID-1);
}

/*
 * Faz a movimentação do construtor
 */
void walk(int y, int x) {

  if (makeColorChange) { // Habilita e desabilita a alteração de cor
    mapColors[yPos][xPos] =
        colorChange; // se estiver habilitado, então altera a cor
  }

  /*
   * Esse if é sobre o "rastro" do construtor, a antiga posição (antes de andar)
   */
  if (charMode) {
    if (keepChangingChar) {
      mapCharacter[yPos][xPos] = charKeeped;
      attron(COLOR_PAIR(mapColors[yPos][xPos]));
      mvprintw(yPos, xPos, "");
      printw("%c", charKeeped);
      attroff(COLOR_PAIR(mapColors[yPos][xPos]));
    } else {
      attron(COLOR_PAIR(mapColors[yPos][xPos]));
      mvprintw(yPos, xPos, "");
      printw("%c", mapCharacter[yPos][xPos]);
      attroff(COLOR_PAIR(mapColors[yPos][xPos]));
    }

  } else {
    if (makePermissionChange) { // checa se a alteração de permissões está
                                // habilitada

      // Se a alteração de permissão está habilitada, então vai substituir a
      // permissão por 0 ou 1

      mapPermission[yPos][xPos] = permission;

      if (permission == 1) { // Altera o rastro para " "

        attron(COLOR_PAIR(mapColors[yPos][xPos]));
        mvprintw(yPos, xPos, " ");
        attroff(COLOR_PAIR(mapColors[yPos][xPos]));

      } else { // Altera o rastro para o id da interação

        attron(COLOR_PAIR(mapColors[yPos][xPos]));
        mvprintw(yPos, xPos, "");
        printw("%d", permission);
        attroff(COLOR_PAIR(mapColors[yPos][xPos]));
      }

    } else {

      attron(COLOR_PAIR(mapColors[yPos][xPos]));

      if (mapPermission[yPos][xPos] == 1) {
        mvprintw(yPos, xPos, " ");
      } else {
        mvprintw(yPos, xPos, "");
        printw("%d", mapPermission[yPos][xPos]);
      }
      attroff(COLOR_PAIR(mapColors[yPos][xPos]));
    }
  }

  // Aqui só printa o construtor na nova posição
  attron(COLOR_PAIR(mapColors[y][x]));
  mvprintw(y, x, "#");
  attroff(COLOR_PAIR(mapColors[y][x]));

  yPos = y;
  xPos = x;
}

void clearWindow() {
  for (int x = 0; x <= SIZEX; x++) {
    for (int y = 0; y <= SIZEY; y++) {
      mvprintw(y, x, " ");
    }
  }
}
//*******************************************//#######//*******************************************//
//*******************************************// Cores
////*******************************************//
//*******************************************//#######//*******************************************//

/**
 * Permite editar os valores de uma cor existente (acredito ser ineficiente
 * demais, mas vou deixar aqui pra caso seja útil futuramente)
 */
void changeColor() {
  int R = 0, G = 0, B = 0, id = 0;

  echo();

  endwin();

  system("clear");

  printf("Existem %d cores criadas\n\n", colorNumberID - 1);

  do {
    fflush(stdin);
    __fpurge(stdin);
    printf("O id da cor deve ser maior que 8 e menor ou igual a %d\n",
           colorNumberID - 1);
    printf("Digite o ID da cor a ser alterada: ");
    scanf("%d", &id);
  } while (id > colorNumberID || id < 8);

  do {
    fflush(stdin);
    __fpurge(stdin);
    printf("Digite o valor de RED: ");
    scanf("%d", &R);
  } while (R > 999 || R < 0);

  do {
    fflush(stdin);
    __fpurge(stdin);
    printf("Digite o valor de GREEN: ");
    scanf("%d", &G);
  } while (G > 999 || G < 0);

  do {
    fflush(stdin);
    __fpurge(stdin);
    printf("Digite o valor de BLUE: ");
    scanf("%d", &B);
  } while (B > 999 || B < 0);

  initscr();
  showMap();
  noecho();

  setColorAtributes(id, R, G, B);
}

/**
 * Ajuste fino da cor
 */
void colorAjust() {
  int R = 0, G = 0, B = 0, id = 0;

  echo();

  endwin();

  system("clear");

  printf("Existem %d cores criadas\n\n", colorNumberID - 1);

  do {
    fflush(stdin);
    __fpurge(stdin);
    printf("O id da cor deve ser maior que 8 e menor ou igual a %d\n",
           colorNumberID - 1);
    printf("Digite o ID da cor a ser alterada: ");
    scanf("%d", &id);
  } while (id > colorNumberID || id < 8);

  initscr();
  showMap();
  noecho();

  R = colorsAtributesSave[id - 8][0];
  G = colorsAtributesSave[id - 8][1];
  B = colorsAtributesSave[id - 8][2];

  char c;
  int changeAmount = 10;

  mvprintw(32, 0,
           "                                                                   "
           "                           ");
  mvprintw(32, 0,
           "Digite 1 e 4 para subir e descer o valor de R, 2 e 5 para G e 3 e "
           "6 para B, zero para terminar");
  do {
    mvprintw(33, 0,
             "                                                                 "
             "         ");
    mvprintw(33, 0, "");
    printw("| R = %d, G = %d, B = %d |  Indice de variacaoo: %d", R, G, B,
           changeAmount);

    c = getch();

    if (c == '4') {
      if (R + changeAmount <= 999) {
        R += changeAmount;
      } else {
        R = 999;
      }
    }

    if (c == '1') {
      if (R - changeAmount >= 0) {
        R -= changeAmount;
      } else {
        R = 0;
      }
    }

    if (c == '5') {
      if (G + changeAmount <= 999) {
        G += changeAmount;
      } else {
        G = 999;
      }
    }

    if (c == '2') {
      if (G - changeAmount >= 0) {
        G -= changeAmount;
      } else {
        G = 0;
      }
    }

    if (c == '6') {
      if (B + changeAmount <= 999) {
        B += changeAmount;
      } else {
        B = 999;
      }
    }

    if (c == '3') {
      if (B - changeAmount >= 0) {
        B -= changeAmount;
      } else {
        B = 0;
      }
    }

    if (c == '.') { // seta '>'
      if (changeAmount < 50) {
        changeAmount++;
      }
    }

    if (c == ',') { // seta '<'
      if (changeAmount > 1) {
        changeAmount--;
      }
    }

    setColorAtributes(id, R, G, B);
  } while (c != '0');
}

void inputColor() { // Permite alterar os valores de cor durante a execução
  endwin();

  int R = 0, G = 0, B = 0;

  echo();

  system("clear");
  do {
    fflush(stdin);
    __fpurge(stdin);
    printf("Digite o valor de RED: ");
    scanf("%d", &R);
  } while (R > 999 || R < 0);

  do {
    fflush(stdin);
    __fpurge(stdin);
    printf("Digite o valor de GREEN: ");
    scanf("%d", &G);
  } while (G > 999 || G < 0);

  do {
    fflush(stdin);
    __fpurge(stdin);
    printf("Digite o valor de BLUE: ");
    scanf("%d", &B);
  } while (B > 999 || B < 0);

  initscr();
  showMap();
  noecho();

  setColorAtributes(-1, R, G, B);
  // colorAlloc(-1, R, G, B); //salva as cores na matriz (a função
  // setColorAtributes já está fazendo isso)
}

/*
 * Seta automaticamente as cores (assim é possível criar uma palheta de cores em
 * uma matriz)
 */
void setColorAtributes(int id, int R, int G, int B) {

  /**
   * Se id == -1 então é para adicionar a cor com um novo id na posição seguinte
   * Se id for um número, então os atributos daquele id (cor) será alterado
   */
  if (id == -1) {

    init_color(colorNumberID, R, G, B);
    colorAlloc(id, R, G, B);

    colorNumberID++;

  } else {

    init_color(id, R, G, B);
    colorAlloc(id, R, G, B);
  }
}

/**####################################################**/
/**################## Pares de Cores ##################**/

void inputPair() {

  int colorID1, colorID2;

  echo();

  endwin();

  system("clear");

  printf("Existem %d cores criadas\n\n", colorNumberID - 1);
  do {
    fflush(stdin);
    __fpurge(stdin);
    printf("Digite o valor do ID da cor do caractere: ");
    scanf("%d", &colorID1);
  } while (colorID1 > colorNumberID || colorID1 < 0);

  do {
    fflush(stdin);
    __fpurge(stdin);
    printf("Digite o valor do ID da cor de fundo: ");
    scanf("%d", &colorID2);
  } while (colorID2 > colorNumberID || colorID2 < 0);

  initscr();
  showMap();
  setPair(colorID1, colorID2);

  noecho();
}

void setPairColor(int pair) { // Habilita e desabilita a cor e seleciona qual
                              // cor será printada
  if (pair == colorChange) {
    if (makeColorChange) {
      makeColorChange = 0;
    } else {
      makeColorChange = 1;
      colorChange = pair;
    }
  } else {
    colorChange = pair;
    makeColorChange = 1;
  }
}

/**
 * Cria um par de cores
 */
void setPair(int id1, int id2) {
  init_pair((pairID - 1), id1, id2);

  pairColorAlloc(id1, id2);

  pairID++;
}

void dinamicPairSelection() { // Permite selecionar "dinamicamente" qualquer
                              // conjunto de cor definido

  // char c[2], *ptr;
  int selection;

  echo();
  endwin();

  system("clear");
  printf("Existem %d pares criados(começando do zero)\n\n", pairID - 2);
  do {
    fflush(stdin);
    __fpurge(stdin);
    printf("Digite o valor do ID do par: ");
    scanf("%d", &selection);
    // printf("Par %d selecionado\n", selection);
  } while (selection > pairID || selection < 0);

  // printf("Foi \n");

  // mvprintw(31, 0, "Todos os valores digitados precisam ter exatamente 2
  // digitos"); mvprintw(32, 0, "Digite o id do par de cor: ");

  // for(int i=0; i<2;i++){
  //     c[i] = getch();
  // }
  // selection = strtol(c, &ptr, 10);

  // mvprintw(32, 0, " "); mvprintw(31, 0, " ");

  initscr();
  showMap();
  setPairColor(selection);

  noecho();
}

// Abre um menu de cores

//*******************************************//###############//*******************************************//
//*******************************************// Menu de cores
////*******************************************//
//*******************************************//###############//*******************************************//

// TO DO
// void colorMenu() {

//   clearWindow();

//   colorMenuList();
// }

//*******************************************//############//*******************************************//
//*******************************************// Permissões
////*******************************************//
//*******************************************//############//*******************************************//

void disableSetPermission() { // Desabilita a troca de permissões
  makePermissionChange = 0;
  permission = 1;
}

void setPermission(int id) { // Habilita a troca de permissões e seta se a troca
                             // será pra 0 ou pra 1

  makePermissionChange = 1;

  if (id) {
    if (id == permission) {
      permission = 1;
    } else {
      permission = id;
    }
  } else {

    if (permission == 1) {
      permission = 0;
    } else if (permission == 0) {
      permission = 1;
    }
  }
}

void setInteractionZone() {

  echo();
  endwin();

  system("clear");
  do {
    fflush(stdin);
    __fpurge(stdin);
    printf("Digite o ID da interação: ");
  } while (!scanf("%d", &interactionId));

  setPermission(interactionId);

  initscr();
  showMap();
  noecho();
}

void enableInteractionSetter() {

  if (interactionId) {
    setPermission(interactionId);
  } else {
    setInteractionZone();
  }
}

//*******************************************//############//*******************************************//
//*******************************************// Caracteres
////*******************************************//
//*******************************************//############//*******************************************//

void charSwitch() {

  if (charMode) {
    charMode = 0;
  } else {
    charMode = 1;
  }

  showMap();
}

void keepChangingAlternate() {
  if (keepChangingChar) {
    keepChangingChar = 0;
  } else {
    keepChangingChar = 1;
  }
}

void charInputMap(char c) {
  mapCharacter[yPos][xPos] = c;
  charKeeped = c;
}

//*******************************************//##################//*******************************************//
//*******************************************// Files Management
////*******************************************//
//*******************************************//#################s#//*******************************************//

void loadFiles() {
  FILE *file; // rgb.game, rgb_pair.game, map_rgb.game screens.info
  int fodase;

  /********************************************************************/
  /*********************Loading permission.game*************************/

  if ((file = fopen("permission.game", "rb")) != NULL) {
    for (int y = 0; y <= SIZEY; y++) {
      for (int x = 0; x <= SIZEX; x++) {
        fread(&mapPermission[y][x], sizeof(int), 1, file); // Original
      }
    }
    fclose(file);
  } else {
    mvprintw(SIZEY + 2, 0, "| O arquivo 'permission.game' não existe| ");
  }

  /********************************************************************/
  /**********************Loading map_rgb.game**************************/

  if ((file = fopen("map_rgb.game", "rb")) != NULL) {
    for (int y = 0; y <= SIZEY; y++) {
      for (int x = 0; x <= SIZEX; x++) {
        fread(&mapColors[y][x], sizeof(int), 1, file); // Original
      }
    }
    fclose(file);
  } else {
    printw("| O arquivo 'map_rgb.game' não existe |");
  }

  /********************************************************************/
  /**********************Loading map_chars*************************/
  if ((file = fopen("map_chars.game", "rb")) != NULL) {
    for (int y = 0; y <= SIZEY; y++) {
      for (int x = 0; x <= SIZEX; x++) {
        fread(&mapCharacter[y][x], sizeof(char), 1, file); // Original
      }
    }
    fclose(file);
  } else {
    printw("| O arquivo 'map_chars.game' não existe |");
  }

  /********************************************************************/
  /************************Load rgb.game*****************************/

  int rgbTemp[3];

  if ((file = fopen("rgb.game", "rb")) != NULL) {
    while (1) {

      if (!fread(&rgbTemp[0], sizeof(int), 1, file)) {
        break;
      }
      if (!fread(&rgbTemp[1], sizeof(int), 1, file)) {
        break;
      }
      if (!fread(&rgbTemp[2], sizeof(int), 1, file)) {
        break;
      }

      setColorAtributes(-1, rgbTemp[0], rgbTemp[1], rgbTemp[2]);
    }
    fclose(file);
  } else {
    colorsList();
    mvprintw(SIZEY + 3, 0, "");
    printw("| O arquivo 'rgb.game' não existe |");
  }

  /********************************************************************/
  /************************Loading rgb_pair.game*****************************/

  int pairTemp[2];
  if ((file = fopen("rgb_pair.game", "rb")) != NULL) {

    while (1) {

      if (fread(&pairTemp[0], sizeof(int), 1, file) == 0) {
        break;
      }
      if (fread(&pairTemp[1], sizeof(int), 1, file) == 0) {
        break;
      }

      // printf("%d, %d\n", pairTemp[0], pairTemp[1]);

      setPair(pairTemp[0], pairTemp[1]);
    }
    fclose(file);

  } else {
    printw("| O arquivo 'map_rgb.game' não existe |");
  }
}

void saveFile() {
  FILE *file; // rgb.game, rgb_pair.game, map_rgb.game screens.info

  /********************************************************************/
  /*********************Saving permission.game*************************/

  file = fopen("permission.game", "wb");
  if (!file) {
    mvprintw(32, 0, "Sabagaça de permission.game não abre ¬¬");
  }

  for (int y = 0; y <= SIZEY; y++) {
    for (int x = 0; x <= SIZEX; x++) {
      // fprintf(file, "%d", mapPermission[y][x]);
      fwrite(&mapPermission[y][x], sizeof(int), 1, file); // Original
    }
  }
  fclose(file);

  /********************************************************************/
  /**********************Saving map_rgb.game***************************/

  file = fopen("map_rgb.game", "wb");

  if (!file) {
    mvprintw(33, 0, "Sabagaça de map_rgb.game não abre ¬¬");
  }

  for (int y = 0; y <= SIZEY; y++) {
    for (int x = 0; x <= SIZEX; x++) {
      // fprintf(file, "%d", mapPermission[y][x]);
      fwrite(&mapColors[y][x], sizeof(int), 1, file); // Original
    }
  }
  fclose(file);

  /********************************************************************/
  /************************Saving rgb.game*****************************/

  file = fopen("rgb.game", "wb");

  if (!file) {
    mvprintw(33, 0, "Sabagaça de rgb.game não abre ¬¬");
  }

  for (int y = 0; y < (sizeColorsAttr - 1); y++) {
    for (int x = 0; x < 3; x++) {
      // fprintf(file, "%d", mapPermission[y][x]);
      fwrite(&colorsAtributesSave[y][x], sizeof(int), 1, file); // Original
    }
  }

  fclose(file);

  /********************************************************************/
  /**********************Saving rgb_pair.game**************************/

  file = fopen("rgb_pair.game", "wb");

  if (!file) {
    mvprintw(33, 0, "Sabagaça de rgb.game não abre ¬¬");
  }

  for (int y = 0; y < (pairID - 1); y++) {
    for (int x = 0; x < 2; x++) {
      // fprintf(file, "%d", mapPermission[y][x]);
      fwrite(&pairColorSave[y][x], sizeof(int), 1, file); // Original
    }
  }

  fclose(file);

  file = fopen("map_chars.game", "wb");

  if (!file) {
    mvprintw(33, 0, "Sabagaça de map_chars.game não abre ¬¬");
  }

  for (int y = 0; y <= SIZEY; y++) {
    for (int x = 0; x <= SIZEX; x++) {
      // fprintf(file, "%d", mapPermission[y][x]);
      fwrite(&mapCharacter[y][x], sizeof(char), 1, file); // Original
    }
  }
  fclose(file);
}

void keyBoardListener() { // Pega os eventos de teclado

  char c; // recebe a tecla digitada

  while ((c = getch()) != 17) { // Tecla CTRL+Q fecha e salva o jogo

    if (charMode) {

      switch (c) {
      case 'A':
      case 'a':

        if ((xPos - 1) >= 0) {
          walk(yPos, (xPos - 1));
        }

        break;

      case 'D':
      case 'd':

        if ((xPos + 1) <= SIZEX) {
          walk(yPos, (xPos + 1));
        }

        break;

      case 'W':
      case 'w':

        if ((yPos - 1) >= 0) {
          walk((yPos - 1), xPos);
        }

        break;

      case 'S':
      case 's':

        if ((yPos + 1) <= SIZEY) {
          walk((yPos + 1), xPos);
        }

        break;

      case 24: // CTRL+X Habilita e desabilita o modo de edição de caracteres
        charSwitch();
        break;

      case 19: // CTRL+S: Salva o arquivo
        saveFile();
        break;

      case ',':
        keepChangingAlternate();
        break;

        /****************************/
        /******* Menu de Cores ******/
        // case '\n':
        //     colorMenu();
        // break;

      default:
        charInputMap(c);
        break;
      }

    } else {

      switch (c) {

        /**************************************/
        /******* Controles de Movimento *******/

      case 'A':
      case 'a':

        if ((xPos - 1) >= 0) {
          walk(yPos, (xPos - 1));
        }

        break;

      case 'D':
      case 'd':

        if ((xPos + 1) <= SIZEX) {
          walk(yPos, (xPos + 1));
        }

        break;

      case 'W':
      case 'w':

        if ((yPos - 1) >= 0) {
          walk((yPos - 1), xPos);
        }

        break;

      case 'S':
      case 's':

        if ((yPos + 1) <= SIZEY) {
          walk((yPos + 1), xPos);
        }

        break;

        /*******************************************************/
        /*************** Controles de Interações ***************/

      case 16: // CTRL+P - Permite selecionar um ID de interação
        setInteractionZone();
        break;

      case 'P': // Habilita a inserção de ID ou a remoção do ID (definido  por
                // setInteractionZone(), CTRL+P)
      case 'p':
        enableInteractionSetter();
        break;

      case ' ': // Tecla Espaço - Desabilita qualquer mudança na matriz de
                // permissões/interações
        disableSetPermission();
        break;

      case '\t': // Tecla Tab - Habilita a inserção de 0 ou 1 na matriz de
                 // permissões
        setPermission(0);
        break;

        /********************************************************************/
        /********************** Controles de Cores **************************/

      case '0': // Não há um if pra esse caso pois sempre haverá o par zero
        setPairColor(0);
        break;

      case '1':

        if (1 + colorSwitch < pairID - 1) {
          setPairColor(1 + colorSwitch);
        } else {
          makeColorChange = 0;
        }

        break;

      case '2':

        if (2 + colorSwitch < pairID - 1) {
          setPairColor(2 + colorSwitch);
        } else {
          makeColorChange = 0;
        }

        break;

      case '3':

        if (3 + colorSwitch < pairID - 1) {
          setPairColor(3 + colorSwitch);
        } else {
          makeColorChange = 0;
        }

        break;

      case '4':

        if (4 + colorSwitch < pairID - 1) {
          setPairColor(4 + colorSwitch);
        } else {
          makeColorChange = 0;
        }

        break;

      case '5':

        if (5 + colorSwitch < pairID - 1) {
          setPairColor(5 + colorSwitch);
        } else {
          makeColorChange = 0;
        }

        break;

      case '6':

        if (6 + colorSwitch < pairID - 1) {
          setPairColor(6 + colorSwitch);
        } else {
          makeColorChange = 0;
        }

        break;

      case '7':

        if (7 + colorSwitch < pairID - 1) {
          setPairColor(7 + colorSwitch);
        } else {
          makeColorChange = 0;
        }

        break;

      case '8':

        if (8 + colorSwitch < pairID - 1) {
          setPairColor(8 + colorSwitch);
        } else {
          makeColorChange = 0;
        }

        break;

      case '9':

        if (9 + colorSwitch < pairID - 1) {
          setPairColor(9 + colorSwitch);
        } else {
          makeColorChange = 0;
        }

        break;

      case '.': // tecla '>'

        if (colorSwitch > 0) {
          colorSwitch--;
        }

        break;

      case ',': // tecla '<'

        colorSwitch++;

        break;

      case 'N': // Permite criar cor
      case 'n':

        inputColor();

        break;

      case 'M': // Permite criar par de cores
      case 'm':

        inputPair();

        break;

      case '/': // Seleciona dinâmicamente entre as cores criadas

        dinamicPairSelection();

        break;

      case 19: // CTRL+S: Salva o arquivo
        saveFile();
        break;

      case 3: // CTRL+C - Permite ajustar em tempo real uma cor existente
        colorAjust();
        break;

      case 24: // CTRL+X Habilita e desabilita o modo de edição de caracteres
        charSwitch();
        break;

      /****************************/
      /******* Menu de Cores ******/
      case '\n':
        // colorMenu();
        break;

      default:
        break;
      }
    }
  }

  // Ao fechar com a tecla devida o jogo é salvo
  echo();
  endwin();

  system("clear");

  fflush(stdin);
  __fpurge(stdin);

  char saveWish;

  printf("Você deseja salvar o jogo?\n");
  printf("\t(S)-Sim\n");
  printf("\t(N)-Nop\n");
  printf("\n(na verdade, qualquer coisa diferente de S vai ser entendido como "
         "não, hehe)\n\n");

  scanf("%1c", &saveWish);

  system("clear");

  if (saveWish == 'S' || saveWish == 's') {
    printf("Mucho bueno, mapa salvo.\n");
    saveFile();
  } else {
    printf("Haha se fodeu otário, sem mapa pra você hoje.\n");
  }

  initscr();
}

void start() {
  xPos = 50;
  yPos = 29;
  mvprintw(yPos, xPos, "#");

  /**
   * Essa alocação está vazia, é apenas para iniciar a lista
   */
  colorsAtributesSave =
      (int **)malloc(1 * sizeof(int *)); // Aloca a primeira posição
  colorsAtributesSave[0] = (int *)malloc(3 * sizeof(int));

  sizeColorsAttr++;

  pairColorSave = (int **)malloc(1 * sizeof(int *)); // Aloca a primeira posição
  pairColorSave[0] = (int *)malloc(2 * sizeof(int));

  pairID++;
}

int main() {

  // system("gnome-terminal --hide-menubar --full-screen --zoom=0.5 --active");

  initscr();   // Inicia a ncurses
  curs_set(0); // Esconde o cursor
  noecho();    // esconte as entradas do teclado
  raw();       // Não permite que comandos cheguem ao terminal (como CTRL+S)

  start();

  start_color();
  // setPair(COLOR_WHITE, COLOR_BLACK);
  startMap();
  loadFiles();

  showMap();

  keyBoardListener();

  endwin(); // Finaliza a ncurses
  return 0;
}