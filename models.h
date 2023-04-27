#include <ncurses.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int songId = 0, songOn = 1;
pthread_t songs;

// Reproduz as músicas
void *playSong() {
  if (songOn) {
    switch (songId) {
    case 1:
      system("cvlc -q --repeat songs/casa.mp3");
      break;
    case 2:
      system("cvlc -q --repeat songs/geral.mp3");
      break;
    case 3:
      system("cvlc -q --repeat songs/covil.mp3");
      break;
    case 4:
      system("cvlc -q --repeat songs/geral.mp3");
      break;
    case 5:
      system("cvlc -q --repeat songs/universidade.mp3");
      break;
    case 6:
      system("cvlc -q --repeat songs/morte.mp3");
      break;
    case 7:
      system("cvlc -q --repeat songs/morte_boss.mp3");
      break;
    case 8:
      system("cvlc -q --repeat songs/intro.mp3");
      break;
    case 9:
      system("cvlc -q --repeat songs/pos.mp3");
      break;
    }
  }
}

// Seletor de músicas
void selectSong(int numsong) {
  system("killall vlc");                         // Encerra música atual
  songId = numsong;                              // ID da próxima música
  pthread_create(&songs, NULL, &playSong, NULL); // Inicia nova música
  usleep(3000);
  system("clear");
}

void turnSong(int mapa) {
  if (songOn) {
    songOn = 0;
    system("killall vlc");
    usleep(2000);
    system("clear");
  } else {
    songOn = 1;
    selectSong(mapa);
  }
}

void pos() {
  selectSong(9);
  system("clear");
  mvprintw(22, 75, "PARABENS");
}

void morte(int tipo) // 1 - normal, 2 - boss
{
  if (tipo == 0)
    selectSong(6);
  else if (tipo == 1)
    selectSong(7);
  system("clear");
  mvprintw(22, 75, "GAME OVER");
}

char *inferno(int id, int tipo, int i) {
  if (!tipo) // tipo de ação
    switch (id) {
    case 7:
      return "02";
    case 8:
      return "03";
    }

  else if (tipo == 1) // numero de execuções
  {
    switch (id) {
    case 7:
      return "03";
    case 6:
      return "05";
    }
  }

  else if (tipo == 2) // retorno dos dados
    switch (id) {
    case 8:
      return "04"; // retorna item 1

    case 7:
      switch (i) // dialogo com Rumpt
      {
      case 0:
        return "Rumpt: Você quem fez isso tudo, todo o Caos do Mundo é culpa "
               "sua Jari, você o destruiu...";
      case 1:
        return "Rumpt: E agora vem até mim? A minha única culpa foi não ter "
               "matado antes...";
      case 2:
        return "Jari: A é? DA QUE EU TE DOU OUTRA, agora estou preparado para "
               "te derrotar.";
      }
      break;

    case 6:
      switch (i) // Quizz com Rumpt
      {
      case 0:
        return "O QUE HA DEPOIS DA CROSTA DE GELO DA TERRA PLANA?";
      case 1:
        return "1 - Plana?";
      case 2:
        return "2 - O domo";
      case 3:
        return "3 - Agua";
      case 4:
        return "01";

      case 5:
        return "ERA DITADURA OU INTERVENÇÃO MILITAR?";
      case 6:
        return "1 - Nenhum, foi uma inveção da midia";
      case 7:
        return "2 - Intervenção";
      case 8:
        return "3 - Ditadura";
      case 9:
        return "03";

      case 10:
        return "QUAL A REDE DE IMPRENSA QUE NUNCA TE DEIXA EM PAZ?";
      case 11:
        return "1 - Record";
      case 12:
        return "2 - Globo";
      case 13:
        return "3 - Rede TV";
      case 14:
        return "02";

      case 15:
        return "QUAL DESTES É UM ACLAMADO PEDAGOGO DO BRASIL?";
      case 16:
        return "1 - Paulo Freire";
      case 17:
        return "2 - Olavo de Carvalho";
      case 18:
        return "3 - Alexandre Frota";
      case 19:
        return "01";

      case 20:
        return "O MUNDO PRECISA DE DEMOCRACIA?";
      case 21:
        return "1 - Claro que não";
      case 22:
        return "2 - Depende..";
      case 23:
        return "3 - Sim";
      case 24:
        return "03";
      }
      break;
    }
}

char *muro(int id, int tipo, int i) {
  if (!tipo) // tipo de ação
    switch (id) {
    case 5:
      return "02";
    case 8:
      return "03";
    case 9:
      return "03";
    }

  else if (tipo == 1) // numero de execuções
  {
    switch (id) {
    case 5:
      return "01";
    case 6:
      return "03";
    }
  }

  else if (tipo == 2) // retorno dos dados
    switch (id) {
    case 8:
      return "03"; // retorna item 1

    case 9:
      return "05"; // troca para mapa 2

    case 5:
      switch (i) // dialogo com Minion
      {
      case 0:
        return "You shall not pass";
      }
      break;

    case 6:
      switch (i) // Quizz com Minion
      {
      case 0:
        return "ONDE FOI CONSTRUÍDO O MURO?";
      case 1:
        return "1 - China";
      case 2:
        return "2 - Mexico";
      case 3:
        return "3 - Aqui";
      case 4:
        return "03";

      case 5:
        return "QUEM GOVERNA O HELL?";
      case 6:
        return "1 - Donald Rumpt";
      case 7:
        return "2 - Nine fingers";
      case 8:
        return "3 - Dracula";
      case 9:
        return "01";

      case 10:
        return "QUAL O FORMATO DA TERRA?";
      case 11:
        return "1 - Plano";
      case 12:
        return "2 - Esfera";
      case 13:
        return "3 - Paraboloide hiperbolico";
      case 14:
        return "01";
      }
      break;
    }
}

char *covil(int id, int tipo, int i) {
  if (!tipo) // tipo de ação
    switch (id) {
    case 2:
      return "02";
    case 8:
      return "03";
    case 9:
      return "03";
    }

  else if (tipo == 1) // numero de execuções
  {
    switch (id) {
    case 2:
      return "07";
    case 3:
      return "04";
    }
  }

  else if (tipo == 2) // retorno dos dados
    switch (id) {
    case 8:
      return "02"; // retorna item 1

    case 9:
      return "04"; // troca para mapa 2

    case 2:
      switch (i) // dialogo com OAK
      {
      case 0:
        return "Oak: Já sabia que viria até mim.... Seja bem vindo Jari!!";
      case 1:
        return "Jari: Como sabia que eu estava aqui?";
      case 2:
        return "Oak: Jari quando você entende a terra plana, e está alinhado "
               "com seus astros, tudo pode estar ao seu alcance.";
      case 3:
        return "Jari: Quero me vingar de Rumpt.";
      case 4:
        return "Oak: O Mundo..... Muita coisa mudou desde o seu acordo com "
               "Rumpt... Protestos, fome, a ascensão dos maiores.... MORTE "
               "paga pelo hell.... o Mundo mudou Jari....";
      case 5:
        return "Oak:  Desde que abandonou sua humanidade e matou seus 3 "
               "filhos, a única coisa que te impede de derrotar Rumpt é você "
               "mesmo Jari.";
      case 6:
        return "JARI ENTRA EM ESTADO DE INTROSPECÇÃO";
      }
      break;

    case 3:
      switch (i) // Quizz com Jari
      {
      case 0:
        return "QUAL SUA FRUTA FAVORITA?";
      case 1:
        return "1 - Laranja";
      case 2:
        return "2 - Maça";
      case 3:
        return "3 - Abacaxi";
      case 4:
        return "01";

      case 5:
        return "NOSSA BANDEIRA JAMAIS SERA...?";
      case 6:
        return "1 - Preta";
      case 7:
        return "2 - Verde";
      case 8:
        return "3 - Vermelha";
      case 9:
        return "03";

      case 10:
        return "ONDE ESTA...?";
      case 11:
        return "1 - Minha carteira";
      case 12:
        return "2 - Queiroz";
      case 13:
        return "3 - Rumpt";
      case 14:
        return "02";

      case 15:
        return "O QUE É GOLDEN SHOWER?";
      case 16:
        return "1 - Cruza entre as raças de cão Golden Retriever e Chow-Chow";
      case 17:
        return "2 - Urgh.. ><'";
      case 18:
        return "3 - Programa da televisão americana apresentado por Oprah";
      case 19:
        return "02";
      }
      break;
    }
}

char *casa_to_covil(int id, int tipo, int i) {
  if (!tipo) // tipo de ação
    switch (id) {
    case 8:
      return "03";
    case 9:
      return "03";
    }

  else if (tipo == 1) {
  } // numero de execuções

  else if (tipo == 2) // retorno dos dados
    switch (id) {
    case 8:
      return "01"; // retorna item 1

    case 9:
      return "03"; // troca para mapa 2
    }
}

char *casa(int id, int tipo, int i) {
  if (!tipo) // tipo de ação
    switch (id) {
    case 3:
      return "05";
    case 9:
      return "03";
    }

  else if (tipo == 1) {
  } // numero de execuções

  else if (tipo == 2) // retorno dos dados
    switch (id) {
    case 3:
      return "01"; // retorna item 1

    case 9:
      return "02"; // troca para mapa 2
    }
}
