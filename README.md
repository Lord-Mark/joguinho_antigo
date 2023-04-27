# joguinho_antigo
Joguinho feito na cadeira de Programação de Sistemas
----------------------------------------------------
Para compilar é necessário instalar a biblioteca ncurses.
Para instalar no Ubuntu:

sudo apt-get install libncurses5-dev libncursesw5-dev

Para compilar o criador de mapas:
gcc builder.c -lncurses -o builder_run

Para compilar o jogo:
gcc jogo.c -lncurses -lpthread -o run