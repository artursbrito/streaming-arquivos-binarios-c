/*
 * Streaming de videos - manipulacao de arquivos binarios em C
 *
 * Arquivos:
 *   videos.bin   -> registros do tipo Video
 *   usuarios.bin -> registros do tipo Usuario (tem um vetor com IDs de videos favoritos)
 *
 * Relacionamento: Usuario.favoritos[] guarda IDs de Video.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARQ_VIDEOS   "videos.bin"
#define ARQ_USUARIOS "usuarios.bin"
#define ARQ_TEMP     "temp.bin"
#define MAX_FAV 20

/* IMPORTANTE: o campo "id" e sempre o primeiro de cada struct.
 * Isso permite funcoes genericas que leem o id com *(int *)registro. */
typedef struct {
    int  id;
    char titulo[60];
    char genero[30];
    int  duracao;   /* em minutos */
    int  ano;
} Video;

typedef struct {
    int  id;
    char nome[50];
    char email[50];
    int  qtd_fav;
    int  favoritos[MAX_FAV];   /* IDs dos videos favoritados */
} Usuario;

/* ---------- entrada de dados ---------- */

void lerLinha(char *dest, int tam) {
    if (fgets(dest, tam, stdin) == NULL) { dest[0] = '\0'; return; }
    dest[strcspn(dest, "\n")] = '\0';
}

int lerInt(void) {
    char buf[32];
    lerLinha(buf, sizeof(buf));
    return atoi(buf);
}

/* ---------- funcoes genericas de arquivo ---------- */

/* Procura um registro pelo ID. Retorna a posicao (0, 1, 2...) ou -1.
 * O registro encontrado fica em "out". */
long buscarRegistro(const char *arq, size_t tam, int id, void *out) {
    FILE *f = fopen(arq, "rb");
    if (!f) return -1;
    long pos = 0;
    while (fread(out, tam, 1, f) == 1) {
        if (*(int *)out == id) { fclose(f); return pos; }
        pos++;
    }
    fclose(f);
    return -1;
}

/* Sobrescreve o registro que esta na posicao "pos" (atualizacao). */
int atualizarRegistro(const char *arq, size_t tam, long pos, void *reg) {
    FILE *f = fopen(arq, "r+b");
    if (!f) return 0;
    fseek(f, pos * (long)tam, SEEK_SET);
    fwrite(reg, tam, 1, f);
    fclose(f);
    return 1;
}

/* Acrescenta um registro novo no final do arquivo. */
int gravarRegistro(const char *arq, size_t tam, void *reg) {
    FILE *f = fopen(arq, "ab");
    if (!f) return 0;
    fwrite(reg, tam, 1, f);
    fclose(f);
    return 1;
}

/* Gera o proximo ID (maior ID existente + 1). */
int proximoId(const char *arq, size_t tam) {
    FILE *f = fopen(arq, "rb");
    if (!f) return 1;
    void *buf = malloc(tam);
    int maior = 0;
    while (fread(buf, tam, 1, f) == 1)
        if (*(int *)buf > maior) maior = *(int *)buf;
    free(buf);
    fclose(f);
    return maior + 1;
}

/* Remocao: copia tudo, menos o registro, para um arquivo novo;
 * depois apaga o antigo e renomeia o novo. Retorna 1 se removeu. */
int removerRegistro(const char *arq, size_t tam, int id) {
    FILE *orig = fopen(arq, "rb");
    if (!orig) return 0;
    FILE *novo = fopen(ARQ_TEMP, "wb");
    if (!novo) { fclose(orig); return 0; }

    void *buf = malloc(tam);
    int achou = 0;
    while (fread(buf, tam, 1, orig) == 1) {
        if (*(int *)buf == id) achou = 1;          /* nao copia */
        else fwrite(buf, tam, 1, novo);
    }
    free(buf);
    fclose(orig);
    fclose(novo);

    if (achou) {
        remove(arq);
        rename(ARQ_TEMP, arq);
    } else {
        remove(ARQ_TEMP);
    }
    return achou;
}

/* ---------- impressao ---------- */

void imprimirVideo(const Video *v) {
    printf("[%d] %s | %s | %d min | %d\n",
           v->id, v->titulo, v->genero, v->duracao, v->ano);
}

void imprimirUsuario(const Usuario *u) {
    printf("[%d] %s <%s> | favoritos: %d\n", u->id, u->nome, u->email, u->qtd_fav);
}

/* ---------- videos ---------- */

void cadastrarVideo(void) {
    Video v;
    memset(&v, 0, sizeof(v));
    v.id = proximoId(ARQ_VIDEOS, sizeof(Video));
    printf("Titulo: ");  lerLinha(v.titulo, sizeof(v.titulo));
    printf("Genero: ");  lerLinha(v.genero, sizeof(v.genero));
    printf("Duracao (min): "); v.duracao = lerInt();
    printf("Ano: ");     v.ano = lerInt();
    if (gravarRegistro(ARQ_VIDEOS, sizeof(Video), &v))
        printf("Video cadastrado com ID %d.\n", v.id);
    else
        printf("Erro ao gravar.\n");
}

void buscarVideo(void) {
    Video v;
    printf("ID do video: ");
    int id = lerInt();
    if (buscarRegistro(ARQ_VIDEOS, sizeof(Video), id, &v) >= 0) imprimirVideo(&v);
    else printf("Video nao encontrado.\n");
}

void listarVideos(void) {
    FILE *f = fopen(ARQ_VIDEOS, "rb");
    if (!f) { printf("Nenhum video cadastrado.\n"); return; }
    Video v;
    int n = 0;
    printf("\n--- VIDEOS ---\n");
    while (fread(&v, sizeof(Video), 1, f) == 1) { imprimirVideo(&v); n++; }
    fclose(f);
    if (n == 0) printf("Nenhum video cadastrado.\n");
}

void atualizarVideo(void) {
    Video v;
    char buf[64];
    printf("ID do video: ");
    int id = lerInt();
    long pos = buscarRegistro(ARQ_VIDEOS, sizeof(Video), id, &v);
    if (pos < 0) { printf("Video nao encontrado.\n"); return; }

    printf("Atual: "); imprimirVideo(&v);
    printf("(Enter mantem o valor atual)\n");

    printf("Novo titulo: "); lerLinha(buf, sizeof(buf));
    if (buf[0]) strcpy(v.titulo, buf);
    printf("Novo genero: "); lerLinha(buf, sizeof(buf));
    if (buf[0]) strcpy(v.genero, buf);
    printf("Nova duracao: "); lerLinha(buf, sizeof(buf));
    if (buf[0]) v.duracao = atoi(buf);
    printf("Novo ano: "); lerLinha(buf, sizeof(buf));
    if (buf[0]) v.ano = atoi(buf);

    atualizarRegistro(ARQ_VIDEOS, sizeof(Video), pos, &v);
    printf("Video atualizado.\n");
}

void removerVideo(void) {
    printf("ID do video a remover: ");
    int id = lerInt();
    if (!removerRegistro(ARQ_VIDEOS, sizeof(Video), id)) {
        printf("Video nao encontrado.\n");
        return;
    }
    /* Mantem o relacionamento consistente: tira o ID dos favoritos dos usuarios */
    FILE *f = fopen(ARQ_USUARIOS, "r+b");
    if (f) {
        Usuario u;
        while (fread(&u, sizeof(Usuario), 1, f) == 1) {
            int mudou = 0;
            for (int i = 0; i < u.qtd_fav; i++) {
                if (u.favoritos[i] == id) {
                    u.favoritos[i] = u.favoritos[u.qtd_fav - 1];
                    u.qtd_fav--;
                    mudou = 1;
                    break;
                }
            }
            if (mudou) {
                fseek(f, -(long)sizeof(Usuario), SEEK_CUR);
                fwrite(&u, sizeof(Usuario), 1, f);
                fseek(f, 0, SEEK_CUR);   /* obrigatorio ao trocar escrita por leitura */
            }
        }
        fclose(f);
    }
    printf("Video removido.\n");
}

/* ---------- usuarios ---------- */

void cadastrarUsuario(void) {
    Usuario u;
    memset(&u, 0, sizeof(u));
    u.id = proximoId(ARQ_USUARIOS, sizeof(Usuario));
    printf("Nome: ");  lerLinha(u.nome, sizeof(u.nome));
    printf("Email: "); lerLinha(u.email, sizeof(u.email));
    u.qtd_fav = 0;
    if (gravarRegistro(ARQ_USUARIOS, sizeof(Usuario), &u))
        printf("Usuario cadastrado com ID %d.\n", u.id);
    else
        printf("Erro ao gravar.\n");
}

void buscarUsuario(void) {
    Usuario u;
    printf("ID do usuario: ");
    int id = lerInt();
    if (buscarRegistro(ARQ_USUARIOS, sizeof(Usuario), id, &u) >= 0) imprimirUsuario(&u);
    else printf("Usuario nao encontrado.\n");
}

void listarUsuarios(void) {
    FILE *f = fopen(ARQ_USUARIOS, "rb");
    if (!f) { printf("Nenhum usuario cadastrado.\n"); return; }
    Usuario u;
    int n = 0;
    printf("\n--- USUARIOS ---\n");
    while (fread(&u, sizeof(Usuario), 1, f) == 1) { imprimirUsuario(&u); n++; }
    fclose(f);
    if (n == 0) printf("Nenhum usuario cadastrado.\n");
}

void atualizarUsuario(void) {
    Usuario u;
    char buf[64];
    printf("ID do usuario: ");
    int id = lerInt();
    long pos = buscarRegistro(ARQ_USUARIOS, sizeof(Usuario), id, &u);
    if (pos < 0) { printf("Usuario nao encontrado.\n"); return; }

    printf("Atual: "); imprimirUsuario(&u);
    printf("(Enter mantem o valor atual)\n");

    printf("Novo nome: ");  lerLinha(buf, sizeof(buf));
    if (buf[0]) strcpy(u.nome, buf);
    printf("Novo email: "); lerLinha(buf, sizeof(buf));
    if (buf[0]) strcpy(u.email, buf);

    atualizarRegistro(ARQ_USUARIOS, sizeof(Usuario), pos, &u);
    printf("Usuario atualizado.\n");
}

void removerUsuario(void) {
    printf("ID do usuario a remover: ");
    int id = lerInt();
    if (removerRegistro(ARQ_USUARIOS, sizeof(Usuario), id))
        printf("Usuario removido.\n");
    else
        printf("Usuario nao encontrado.\n");
}

/* ---------- relacionamento ---------- */

void favoritarVideo(void) {
    Usuario u;
    Video v;
    printf("ID do usuario: ");
    int uid = lerInt();
    long pos = buscarRegistro(ARQ_USUARIOS, sizeof(Usuario), uid, &u);
    if (pos < 0) { printf("Usuario nao encontrado.\n"); return; }

    printf("ID do video: ");
    int vid = lerInt();
    if (buscarRegistro(ARQ_VIDEOS, sizeof(Video), vid, &v) < 0) {
        printf("Video nao encontrado.\n");
        return;
    }
    for (int i = 0; i < u.qtd_fav; i++) {
        if (u.favoritos[i] == vid) { printf("Video ja esta nos favoritos.\n"); return; }
    }
    if (u.qtd_fav >= MAX_FAV) { printf("Limite de %d favoritos atingido.\n", MAX_FAV); return; }

    u.favoritos[u.qtd_fav++] = vid;
    atualizarRegistro(ARQ_USUARIOS, sizeof(Usuario), pos, &u);
    printf("'%s' adicionado aos favoritos de %s.\n", v.titulo, u.nome);
}

/* ---------- relatorios ---------- */

/* Para cada usuario, mostra os TITULOS dos videos favoritados */
void relatorioUsuarios(void) {
    FILE *f = fopen(ARQ_USUARIOS, "rb");
    if (!f) { printf("Nenhum usuario cadastrado.\n"); return; }
    Usuario u;
    Video v;
    printf("\n===== RELATORIO: USUARIOS E SEUS FAVORITOS =====\n");
    while (fread(&u, sizeof(Usuario), 1, f) == 1) {
        printf("\nUsuario [%d] %s (%s)\n", u.id, u.nome, u.email);
        if (u.qtd_fav == 0) printf("   (nenhum favorito)\n");
        for (int i = 0; i < u.qtd_fav; i++) {
            if (buscarRegistro(ARQ_VIDEOS, sizeof(Video), u.favoritos[i], &v) >= 0)
                printf("   - %s (%s, %d)\n", v.titulo, v.genero, v.ano);
            else
                printf("   - (video %d nao encontrado)\n", u.favoritos[i]);
        }
    }
    fclose(f);
}

/* Para cada video, mostra os NOMES dos usuarios que o favoritaram */
void relatorioVideos(void) {
    FILE *fv = fopen(ARQ_VIDEOS, "rb");
    if (!fv) { printf("Nenhum video cadastrado.\n"); return; }
    Video v;
    Usuario u;
    printf("\n===== RELATORIO: VIDEOS E QUEM FAVORITOU =====\n");
    while (fread(&v, sizeof(Video), 1, fv) == 1) {
        printf("\nVideo [%d] %s (%s, %d)\n", v.id, v.titulo, v.genero, v.ano);
        int total = 0;
        FILE *fu = fopen(ARQ_USUARIOS, "rb");
        if (fu) {
            while (fread(&u, sizeof(Usuario), 1, fu) == 1) {
                for (int i = 0; i < u.qtd_fav; i++) {
                    if (u.favoritos[i] == v.id) {
                        printf("   - %s\n", u.nome);
                        total++;
                        break;
                    }
                }
            }
            fclose(fu);
        }
        if (total == 0) printf("   (ninguem favoritou)\n");
    }
    fclose(fv);
}

/* ---------- menu ---------- */

void menu(void) {
    printf("\n========== STREAMING ==========\n");
    printf(" 1 - Cadastrar video\n");
    printf(" 2 - Cadastrar usuario\n");
    printf(" 3 - Buscar video por ID\n");
    printf(" 4 - Buscar usuario por ID\n");
    printf(" 5 - Listar todos os videos\n");
    printf(" 6 - Listar todos os usuarios\n");
    printf(" 7 - Atualizar video\n");
    printf(" 8 - Atualizar usuario\n");
    printf(" 9 - Favoritar video para um usuario\n");
    printf("10 - Remover video\n");
    printf("11 - Remover usuario\n");
    printf("12 - Relatorio: usuarios e seus videos favoritos\n");
    printf("13 - Relatorio: videos e quem favoritou\n");
    printf(" 0 - Sair\n");
    printf("Opcao: ");
}

int main(void) {
    int op;
    do {
        menu();
        op = lerInt();
        switch (op) {
            case 1:  cadastrarVideo();    break;
            case 2:  cadastrarUsuario();  break;
            case 3:  buscarVideo();       break;
            case 4:  buscarUsuario();     break;
            case 5:  listarVideos();      break;
            case 6:  listarUsuarios();    break;
            case 7:  atualizarVideo();    break;
            case 8:  atualizarUsuario();  break;
            case 9:  favoritarVideo();    break;
            case 10: removerVideo();      break;
            case 11: removerUsuario();    break;
            case 12: relatorioUsuarios(); break;
            case 13: relatorioVideos();   break;
            case 0:  printf("Ate logo!\n"); break;
            default: printf("Opcao invalida.\n");
        }
    } while (op != 0);
    return 0;
}
