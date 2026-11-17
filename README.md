Streaming de Vídeos - Arquivos Binários em C

Entrega: em dupla

Nome	RA
Artur Brito	863921
Isaac Fortes	864128
Sobre

Programa em C que manipula dois arquivos binários:

videos.bin: registros de vídeo (id, título, gênero, duração, ano)
usuarios.bin: registros de usuário (id, nome, email e vetor com os IDs dos vídeos favoritados)

Operações: cadastro, leitura de um registro ou de todos, atualização, remoção (copia para arquivo novo, apaga o antigo e renomeia) e relatórios que mostram os títulos dos vídeos favoritados por cada usuário.

Como executar
gcc -Wall -o streaming streaming.c
./streaming
