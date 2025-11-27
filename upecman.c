/***************************************************************************
 *   upecman.c                                Version 20160529.013231      *
 *                                                                         *
 *   Pacman Ncurses                                                        *
 *   Copyright (C) 2016         by Ruben Carlo Benante                     *
 ***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 *   To contact the author, please write to:                               *
 *   Ruben Carlo Benante                                                   *
 *   Email: rcb@beco.cc                                                    *
 *   Webpage: http://www.beco.cc                                           *
 *   Phone: +55 (81) 3184-7555                                             *
 ***************************************************************************/

/* ---------------------------------------------------------------------- */
/**
 * @file upecman.c
 * @ingroup GroupUnique
 * @brief Pacman Ncurses
 * @details This program really do a nice job as a template, and template only!
 * @version 20160529.013231
 * @date 2016-05-29
 * @author Ruben Carlo Benante <<rcb@beco.cc>>
 * @par Webpage
 * <<a href="http://www.beco.cc">www.beco.cc</a>>
 * @copyright (c) 2016 GNU GPL v3
 * @note This program is free software: you can redistribute it
 * and/or modify it under the terms of the
 * GNU General Public License as published by
 * the Free Software Foundation version 3 of the License.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.
 * If not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA. 02111-1307, USA.
 * Or read it online at <<http://www.gnu.org/licenses/>>.
 *
 *
 * @todo Now that you have the template, hands on! Programme!
 * @warning Be carefull not to lose your mind in small things.
 * @bug This file right now does nothing usefull
 *
 */

/*
 * Instrucoes para compilar:
 *      $make
 * ou
 *      $gcc upecman.c -o upecman.x -Wall -lncurses -g -Og
 *          -Wextra -ansi -pedantic-errors -DDEBUG=1
 */

/* ---------------------------------------------------------------------- */
/* includes */

#include <stdio.h> /* Standard I/O functions */
#include <stdlib.h> /* Miscellaneous functions (rand, malloc, srand)*/
#include <ncurses.h> /* Screen handling and optimisation functions */
#include <getopt.h> /* get options from system argc/argv */
#include <string.h> /* Strings functions definitions */
#include <assert.h> /* Verify assumptions with assert */
#include <unistd.h> /* UNIX standard function */
#include <time.h> /* Time functions for random seed */
#include "upecman.h" /* Inclui o header com o mapa e definicoes */

/* ---------------------------------------------------------------------- */
/* definitions */

#define PONTOS_PELOTA 10
#define PONTOS_PILULA 50
#define PONTOS_CEREJA 500
#define FANTASMA_MULTIPLIER 200
#define BONUS_QUATRO_FANTASMAS 12000
#define POWERUP_DURATION 10000 /* 10 segundos em milissegundos */
#define SCATTER_DURATION 7000  /* 7 segundos em modo dispersão */
#define CHASE_DURATION 20000   /* 20 segundos em modo perseguição */

/* ---------------------------------------------------------------------- */
/* globals */

static int local_verb = 0; /**< verbose level, global within the file */
static int start_y = 0; /**< starting y position for centering */
static int start_x = 0; /**< starting x position for centering */
static t_direction last_direction = left; /* Última direção do Pacman para movimento contínuo */
static clock_t powerup_start_time = 0; /* Tempo de início do power-up */
static int powerup_active = 0; /* Flag indicando se o power-up está ativo */
static clock_t mode_start_time = 0; /* Tempo de início do modo atual */
static t_ghostmode current_ghost_mode = scatter; /* Modo atual dos fantasmas */

/* ---------------------------------------------------------------------- */
/* prototypes */

void help(void); /* print some help */
void copyr(void); /* print version and copyright information */
t_game upecman_init(void); /* initialize game variables */
void printlab(t_game g); /* print the labyrinth */
int mostra_menu(void);
void mostra_tutorial_game(void);
int mostra_menu_pausa(void);
int mostra_game_over(t_game g);
int mostra_vitoria(t_game g);
int is_wall(char lab[LABL][LABC], int y, int x);
void try_move_ghost(t_game *g, int gi, t_direction d);
t_direction random_new_direction(void);
t_direction best_direction_to_target(t_game *g, int gi, int ty, int tx);
t_pos ghost_chase_target(t_game *g, int gi);
void move_ghost_frightened(t_game *g, int gi);
void move_ghosts(t_game *g);
void move_pacman(t_game *g, int direction);
void move_pacman_continuous(t_game *g); /* Nova função para movimento contínuo */
int check_collision(t_game *g);
void update_score(t_game *g);
int check_victory(t_game *g);
void move_ghost_from_cage(t_game *g, int gi);
void calculate_center(void);
void reset_positions_after_collision(t_game *g);
void update_ghost_modes(void); /* Nova função para atualizar modos */
int is_powerup_active(void); /* Verifica se o power-up está ativo */
void activate_powerup(void); /* Ativa o power-up */

/* ---------------------------------------------------------------------- */
/**
 * @brief Calculate center position for the game
 */
void calculate_center(void)
{
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    start_y = (max_y - LABL) / 2;
    start_x = (max_x - LABC) / 2;

    /* Ensure positive values */
    if(start_y < 0) start_y = 0;
    if(start_x < 0) start_x = 0;
}

/* ---------------------------------------------------------------------- */
/**
 * @brief Verifica se o power-up está ativo
 */
int is_powerup_active(void)
{
    if (!powerup_active) return 0;

    clock_t current_time = clock();
    double elapsed_ms = ((double)(current_time - powerup_start_time) / CLOCKS_PER_SEC) * 1000;

    if (elapsed_ms >= POWERUP_DURATION)
    {
        powerup_active = 0;
        return 0;
    }

    return 1;
}

/* ---------------------------------------------------------------------- */
/**
 * @brief Ativa o power-up
 */
void activate_powerup(void)
{
    powerup_active = 1;
    powerup_start_time = clock();
}

/* ---------------------------------------------------------------------- */
/**
 * @brief Atualiza os modos dos fantasmas (scatter/chase)
 */
void update_ghost_modes(void)
{
    clock_t current_time = clock();
    double elapsed_ms = ((double)(current_time - mode_start_time) / CLOCKS_PER_SEC) * 1000;

    if (current_ghost_mode == scatter && elapsed_ms >= SCATTER_DURATION)
    {
        current_ghost_mode = chase;
        mode_start_time = clock();
    }
    else if (current_ghost_mode == chase && elapsed_ms >= CHASE_DURATION)
    {
        current_ghost_mode = scatter;
        mode_start_time = clock();
    }
}

/* ---------------------------------------------------------------------- */
/**
 * @brief Reset positions after collision without resetting the labirinth
 */
void reset_positions_after_collision(t_game *g)
{
    /* Save current lab state */
    char saved_lab[LABL][LABC];
    int y, x;

    for(y = 0; y < LABL; y++)
    {
        for(x = 0; x < LABC; x++)
        {
            saved_lab[y][x] = g->lab[y][x];
        }
    }

    /* Reset pacman position */
    g->pacman.pos.y = 17;
    g->pacman.pos.x = 10;
    g->pacman.dir = left;
    last_direction = left;

    /* Reset ghosts positions and modes */
    int f;
    for(f = blinky; f <= clyde; f++)
    {
        switch(f)
        {
            case blinky:
                g->ghost[f].pos.y = 7;
                g->ghost[f].pos.x = 9;
                break;
            case pinky:
                g->ghost[f].pos.y = 9;
                g->ghost[f].pos.x = 11;
                break;
            case inky:
                g->ghost[f].pos.y = 10;
                g->ghost[f].pos.x = 11;
                break;
            case clyde:
                g->ghost[f].pos.y = 11;
                g->ghost[f].pos.x = 11;
                break;
        }
        g->ghost[f].dir = left;
        g->ghost[f].mode = current_ghost_mode;
    }

    /* Restore lab state */
    for(y = 0; y < LABL; y++)
    {
        for(x = 0; x < LABC; x++)
        {
            g->lab[y][x] = saved_lab[y][x];
        }
    }
}

/* ---------------------------------------------------------------------- */
/**
 * @brief Move Pacman continuously in the last direction
 */
void move_pacman_continuous(t_game *g)
{
    int ny, nx;

    ny = g->pacman.pos.y;
    nx = g->pacman.pos.x;

    /* Move based on last direction */
    switch(last_direction)
    {
        case up:
            ny--;
            break;
        case down:
            ny++;
            break;
        case left:
            nx--;
            break;
        case right:
            nx++;
            break;
    }

    /* Verifica se a nova posição é válida (não é parede e está dentro dos limites) */
    if(!is_wall(g->lab, ny, nx) && ny >= 0 && ny < LABL && nx >= 0 && nx < LABC)
    {
        g->pacman.pos.y = ny;
        g->pacman.pos.x = nx;
        g->pacman.dir = last_direction;
    }
}

/* ---------------------------------------------------------------------- */
/**
 * @ingroup GroupUnique
 * @brief This is the main event of the evening
 * @details Ladies and Gentleman... It's tiiiime!
 * Fightiiiiing at the blue corner,
 * he, who has compiled more C code than any other adversary in the history,
 * he, who has developed UNIX and Linux, and is an inspiration to maaany languages
 * and compilers, the GNU C Compiler, GCC!
 * Fightiiiiing at the red corner, the challenger, in his first fight, lacking of any
 * valid experience but angrily, blindly, and no doubtfully, will try to
 * compile this program without errors. He, the student, the apprentice,
 * the developer, beco!!
 *
 * @param[in] argc Argument counter
 * @param[in] argv Argument strings (argument values)
 *
 * @retval 0 If succeed (EXIT_SUCCESS).
 * @retval 1 Or another error code if failed.
 *
 * @par Example
 * @code
 *    $./upecman -h
 * @endcode
 *
 * @warning   Be carefull with...
 * @bug There is a bug with...
 * @todo Need to do...
 * @note You can read more about it at <<a href="http://www.beco.cc">www.beco.cc</a>>
 * @author Ruben Carlo Benante
 * @version 20160529.013231
 * @date 2016-05-29
 *
 */
int main(int argc, char *argv[])
{
    /* Declare all variables at the beginning */
    int opt;
    t_game g;
    const char *sready;
    int kin;
    int menu_choice;
    int pause_choice;
    int game_over_choice;
    int victory_choice;
    int game_running;
    int frame_count;
    int ghost_release_timer;

    IFDEBUG("main()\n");

    /* Initialize variables */
    opt = 0;
    sready = "Are you ready? (y/n)";
    kin = 0;
    menu_choice = 0;
    pause_choice = 0;
    game_over_choice = 0;
    victory_choice = 0;
    game_running = 1;
    frame_count = 0;
    ghost_release_timer = 0;

    IFDEBUG("Starting optarg loop...\n");

    /* getopt() configured options:
     *        -h  help
     *        -c  copyright & version
     *        -v  verbose
     */
    opterr = 0;
    while((opt = getopt(argc, argv, "hvc")) != EOF)
    {
        switch(opt)
        {
            case 'h':
                help();
                return EXIT_SUCCESS;
            case 'v':
                verb++;
                break;
            case 'c':
                copyr();
                return EXIT_SUCCESS;
            case '?':
            default:
                printf("Type\n\t$man %s\nor\n\t$%s -h\nfor help.\n\n", argv[0], argv[0]);
                return EXIT_FAILURE;
        }
    }

    if(verb)
    {
        printf("Verbose level set at: %d\n", verb);
    }

    /* Initialize random seed */
    srand(time(NULL));

    /* ...and we are done */
    IFDEBUG("Starting the game now...\n");

    /* Initialize ncurses early */
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    timeout(100);
    start_color();

    /* Inicializa cores com as novas cores para os fantasmas */
    init_pair(1, COLOR_RED, COLOR_BLACK);        /* Blinky - Vermelho */
    init_pair(2, COLOR_MAGENTA, COLOR_BLACK);    /* Pinky - Rosa */
    init_pair(3, COLOR_CYAN, COLOR_BLACK);       /* Inky - Azul claro */
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);     /* Clyde - Amarelo (laranja não disponível, usando amarelo) */
    init_pair(5, COLOR_BLUE, COLOR_BLACK);       /* Fantasmas vulneráveis - Azul */
    init_pair(6, COLOR_YELLOW, COLOR_BLACK);     /* Pacman - Amarelo */

    /* Calculate center position */
    calculate_center();

    /* Show initial Menu */
    while(1)
    {
        menu_choice = mostra_menu();
        if(menu_choice == 1) /* Jogar */
        {
            break;
        }
        else if(menu_choice == 2) /* Sair */
        {
            endwin();
            return EXIT_SUCCESS;
        }
        else if(menu_choice == 3) /* Tutorial do jogo */
        {
            mostra_tutorial_game();
        }
    }

    /* Initialize game after menu selection */
    g = upecman_init();
    last_direction = left; /* Inicializa com direção para esquerda */
    mode_start_time = clock(); /* Inicializa o timer dos modos */

    /* Clear screen and print centered lab */
    clear();
    printlab(g);

    /* Center the "Are you ready?" message */
    mvprintw(start_y + 10, start_x + (LABC - strlen(sready)) / 2, "%s", sready);
    refresh();

    while(1)
    {
        kin = getch();
        if(kin == 'y' || kin == KEY_LEFT || kin == 'Y')
        {
            break;
        }
        if(kin == 'n' || kin == 'N')
        {
            endwin();
            return EXIT_SUCCESS;
        }
    }

    /* Clear the "Are you ready?" message and start game */
    clear();
    printlab(g);

    /* Display initial score and lives (centered below the maze) */
    mvprintw(start_y + LABL + 1, start_x, "Score: %d", g.pacman.score);
    mvprintw(start_y + LABL + 2, start_x, "Lives: %d", g.pacman.life);
    mvprintw(start_y + LABL + 3, start_x, "Use arrow keys to move, 'p' to pause");
    refresh();

    /* Game Loop */
    while(game_running)
    {
        kin = getch();

        if(kin == 'p' || kin == 'P' || kin == ' ') /* Botao de Pause */
        {
            pause_choice = mostra_menu_pausa();
            if(pause_choice == 1) /* Retornar ao Jogo */
            {
                /* Continue the game */
                timeout(100);
            }
            else if(pause_choice == 2) /* Reiniciar o jogo */
            {
                g = upecman_init();
                last_direction = left;
                mode_start_time = clock();
                clear();
                printlab(g);
                mvprintw(start_y + LABL + 1, start_x, "Score: %d", g.pacman.score);
                mvprintw(start_y + LABL + 2, start_x, "Lives: %d", g.pacman.life);
                mvprintw(start_y + LABL + 3, start_x, "Use arrow keys to move, 'p' to pause");
                refresh();
                frame_count = 0;
                ghost_release_timer = 0;
            }
            else if(pause_choice == 3) /* Sair do Jogo */
            {
                game_running = 0;
                break;
            }
        }
        else
        {
            /* Process input for Pacman direction */
            if(kin == KEY_UP || kin == KEY_DOWN || kin == KEY_LEFT || kin == KEY_RIGHT)
            {
                move_pacman(&g, kin);
            }

            /* Move Pacman continuously every frame */
            move_pacman_continuous(&g);

            /* Check for victory */
            if(check_victory(&g))
            {
                victory_choice = mostra_vitoria(g);
                if(victory_choice == 'r')
                {
                    g = upecman_init();
                    last_direction = left;
                    mode_start_time = clock();
                    clear();
                    printlab(g);
                    mvprintw(start_y + LABL + 1, start_x, "Score: %d", g.pacman.score);
                    mvprintw(start_y + LABL + 2, start_x, "Lives: %d", g.pacman.life);
                    mvprintw(start_y + LABL + 3, start_x, "Use arrow keys to move, 'p' to pause");
                    refresh();
                    frame_count = 0;
                    ghost_release_timer = 0;
                }
                else if(victory_choice == 'q')
                {
                    game_running = 0;
                    break;
                }
            }

            /* Move ghosts every 10 frames (slower movement for ghosts) */
            frame_count++;
            ghost_release_timer++;

            if(frame_count >= 10)
            {
                /* Atualiza os modos dos fantasmas (scatter/chase) */
                update_ghost_modes();

                move_ghosts(&g);
                frame_count = 0;
            }

            /* Check collisions */
            if(check_collision(&g))
            {
                /* COLISÃO FATAL - Pacman perde uma vida */
                g.pacman.life--;

                /* Atualiza display imediatamente */
                clear();
                printlab(g);
                mvprintw(start_y + LABL + 1, start_x, "Score: %d", g.pacman.score);
                mvprintw(start_y + LABL + 2, start_x, "Lives: %d", g.pacman.life);
                refresh();

                /* Pequena pausa para o jogador ver a mensagem */
                napms(1000);

                if(g.pacman.life == 0)
                {
                    /* Game Over */
                    game_over_choice = mostra_game_over(g);
                    if(game_over_choice == 'r')
                    {
                        g = upecman_init();
                        last_direction = left;
                        mode_start_time = clock();
                        clear();
                        printlab(g);
                        mvprintw(start_y + LABL + 1, start_x, "Score: %d", g.pacman.score);
                        mvprintw(start_y + LABL + 2, start_x, "Lives: %d", g.pacman.life);
                        mvprintw(start_y + LABL + 3, start_x, "Use arrow keys to move, 'p' to pause");
                        refresh();
                        frame_count = 0;
                        ghost_release_timer = 0;
                    }
                    else if(game_over_choice == 'q')
                    {
                        game_running = 0;
                        break;
                    }
                }
                else
                {
                    /* Ainda tem vidas - reset das posições SEM resetar o labirinto */
                    reset_positions_after_collision(&g);

                    clear();
                    printlab(g);
                    mvprintw(start_y + LABL + 1, start_x, "Score: %d", g.pacman.score);
                    mvprintw(start_y + LABL + 2, start_x, "Lives: %d", g.pacman.life);
                    mvprintw(start_y + LABL + 3, start_x, "Use arrow keys to move, 'p' to pause");
                    refresh();
                    frame_count = 0;
                    ghost_release_timer = 0;
                }
            }

            /* Update score */
            update_score(&g);

            /* Redraw game */
            clear();
            printlab(g);

            /* Display score and lives (centered below the maze) */
            mvprintw(start_y + LABL + 1, start_x, "Score: %d", g.pacman.score);
            mvprintw(start_y + LABL + 2, start_x, "Lives: %d", g.pacman.life);

            /* Mostra timer do power-up se estiver ativo */
            if(is_powerup_active())
            {
                clock_t current_time = clock();
                double elapsed_ms = ((double)(current_time - powerup_start_time) / CLOCKS_PER_SEC) * 1000;
                int remaining = (POWERUP_DURATION - (int)elapsed_ms) / 1000;
                mvprintw(start_y + LABL + 4, start_x, "Power-up: %d seconds", remaining + 1);
            }

            mvprintw(start_y + LABL + 3, start_x, "Use arrow keys to move, 'p' to pause");
            refresh();
        }

        /* Small delay to control game speed */
        napms(100); /* Aumentado para melhor controle do movimento contínuo */
    }

    endwin();
    return EXIT_SUCCESS;
}

/* ---------------------------------------------------------------------- */
/**
 * @ingroup GroupUnique
 * @brief Prints help information and exit
 * @details Prints help information (usually called by opt -h)
 * @return Void
 * @author Ruben Carlo Benante
 * @version 20160529.013231
 * @date 2016-05-29
 *
 */
void help(void)
{
    IFDEBUG("help()");
    printf("%s - %s\n", "upecman", "Pacman Ncurses");
    printf("\nUsage: ./%s [-h|-v|-c]\n", "upecman");
    printf("\nOptions:\n");
    printf("\t-h,  --help\n\t\tShow this help.\n");
    printf("\t-c,  --copyright, --version\n\t\tShow version and copyright information.\n");
    printf("\t-v,  --verbose\n\t\tSet verbose level (cumulative).\n");
    /* add more options here */
    printf("\nExit status:\n\t0 if ok.\n\t1 some error occurred.\n");
    printf("\nTodo:\n\tLong options not implemented yet.\n");
    printf("\nAuthor:\n\tWritten by %s <%s>\n\n", "Ruben Carlo Benante", "rcb@beco.cc");
}

/*-----------------------------------------------------------------------*/
/**
 * @ingroup GroupUnique
 * @brief Shows the initial menu
 * @details Displays the main menu with options to play, exit, or TUTORIAL
 * @return The chosen option (1 for play, 2 for Exit, 3 for Tutorial)
 * @author Gabriel Henrique Goncalves Da Silva
 * @date 2025-11-21
 * @version 20160529.013231
 */
int mostra_menu(void)
{
    int escolha;
    int entrada;
    int max_y, max_x;
    int menu_start_y, menu_start_x;

    escolha = 1;
    timeout(-1);

    getmaxyx(stdscr, max_y, max_x);

    while(1)
    {
        clear();

        menu_start_y = max_y / 2 - 5;
        menu_start_x = max_x / 2 - 10;

        if(menu_start_y < 0) menu_start_y = 0;
        if(menu_start_x < 0) menu_start_x = 0;

        /* U vermelho, PE branco (sem cor), CMAN amarelo */
        attron(COLOR_PAIR(1) | A_BOLD); /* U vermelho */
        mvprintw(menu_start_y, menu_start_x + 8, "U");
        attroff(COLOR_PAIR(1));

        /* PE em branco padrão (sem cor especial) */
        attron(A_BOLD);
        mvprintw(menu_start_y, menu_start_x + 9, "PE");
        attroff(A_BOLD);

        attron(COLOR_PAIR(6) | A_BOLD); /* CMAN amarelo */
        mvprintw(menu_start_y, menu_start_x + 11, "CMAN");
        attroff(COLOR_PAIR(6));

        mvprintw(menu_start_y + 2, menu_start_x + 6, "MENU INICIAL");
        mvprintw(menu_start_y + 4, menu_start_x, "%s 1. JOGAR", (escolha == 1) ? ">" : " ");
        mvprintw(menu_start_y + 5, menu_start_x, "%s 2. SAIR", (escolha == 2) ? ">" : " ");
        mvprintw(menu_start_y + 6, menu_start_x, "%s 3. TUTORIAL", (escolha == 3) ? ">" : " ");
        mvprintw(menu_start_y + 8, menu_start_x, "Use as Setas para navegar e Enter para Selecionar.");
        refresh();

        entrada = getch();

        switch(entrada)
        {
            case KEY_UP:
                if(escolha > 1)
                {
                    escolha--;
                }
                break;
            case KEY_DOWN:
                if(escolha < 3)
                {
                    escolha++;
                }
                break;
            case '\n':
                timeout(100);
                return escolha;
        }
    }
}

/* ---------------------------------------------------------------------- */
/**
 * @ingroup GroupUnique
 * @brief Shows the tutorial screen
 * @details Displays instructions on how to play the game
 * @return Void
 * @author Gabriel Henrique Goncalves Da Silva
 * @version 20160529.013231
 * @date 2025-11-21
 */
void mostra_tutorial_game(void)
{
    int entrada;
    int max_y, max_x;
    int tutorial_start_y;

    timeout(-1);
    getmaxyx(stdscr, max_y, max_x);
    tutorial_start_y = max_y / 2 - 10;

    while(1)
    {
        clear();

        mvprintw(tutorial_start_y, 5, "Tutorial do Upecman");
        mvprintw(tutorial_start_y + 2, 5, "Objetivo: Coma todos os pontos no labirinto sem ser pego pelos fantasmas");
        mvprintw(tutorial_start_y + 4, 5, "Controles:");
        mvprintw(tutorial_start_y + 5, 5, " - Setas: Mover o Pacman (movimento contínuo)");
        mvprintw(tutorial_start_y + 6, 5, " - P ou Espaco: Pausar o Jogo");
        mvprintw(tutorial_start_y + 8, 5, "Pontos:");
        mvprintw(tutorial_start_y + 9, 5, " - Pontos normais: 10 pontos cada");
        mvprintw(tutorial_start_y + 10, 5, " - Bolinhas grandes: 50 pontos + 10 segundos de power-up");
        mvprintw(tutorial_start_y + 12, 5, "Fantasmas:");
        mvprintw(tutorial_start_y + 13, 5, " - Blinky (B): Vermelho - Persegue diretamente");
        mvprintw(tutorial_start_y + 14, 5, " - Pinky (P): Rosa - Tenta interceptar à frente");
        mvprintw(tutorial_start_y + 15, 5, " - Inky (I): Azul claro - Comportamento imprevisível");
        mvprintw(tutorial_start_y + 16, 5, " - Clyde (C): Laranja - Foge quando perto do Pacman");
        mvprintw(tutorial_start_y + 18, 5, "Power-up: Fantasmas ficam azuis e vulneráveis por 10 segundos");
        mvprintw(tutorial_start_y + 19, 5, "Modos: Dispersão (7s) - Perseguição (20s)");
        mvprintw(tutorial_start_y + 21, 5, "Vidas: Voce tem 3 vidas. Perde uma ao ser pego por um fantasma");
        mvprintw(tutorial_start_y + 23, 5, "Pressione qualquer tecla para voltar ao menu inicial.");

        entrada = getch();
        if(entrada != ERR)
        {
            timeout(100);
            break;
        }
    }
}

/* ---------------------------------------------------------------------- */
/**
 *@ingroup GroupUnique
 *@brief Shows the pause menu
 * @details Displays The pause Menu with options to resume, restart, or exit
 * @return The chosen option ( 1 for Resume, 2 for Restart, 3 for Exit)
 * @author Gabriel Henrique Goncalves da Silva
 * @version 20160529.013231
 * @date 2025-11-22
 */
int mostra_menu_pausa(void)
{
    int escolha;
    int entrada;
    int max_y;
    int max_x;
    int pause_start_y, pause_start_x;

    escolha = 1;
    getmaxyx(stdscr, max_y, max_x);

    pause_start_y = max_y / 2 - 3;
    pause_start_x = max_x / 2 - 20;

    timeout(-1);

    while(1)
    {
        clear();

        mvprintw(pause_start_y, pause_start_x + 20, "Jogo pausado");
        mvprintw(pause_start_y + 2, pause_start_x, "%s 1. Retornar ao Jogo", (escolha == 1) ? ">" : " ");
        mvprintw(pause_start_y + 3, pause_start_x, "%s 2. Reiniciar o jogo", (escolha == 2) ? ">" : " ");
        mvprintw(pause_start_y + 4, pause_start_x, "%s 3. Sair do Jogo", (escolha == 3) ? ">" : " ");
        mvprintw(pause_start_y + 6, pause_start_x, "Use as setas para navegar e Enter para selecionar.");
        refresh();

        entrada = getch();
        switch(entrada)
        {
            case KEY_UP:
                if(escolha > 1)
                {
                    escolha--;
                }
                break;
            case KEY_DOWN:
                if(escolha < 3)
                {
                    escolha++;
                }
                break;
            case '\n':
                timeout(100);
                return escolha;
        }
    }
}

/* ---------------------------------------------------------------------- */
/**
 * @ingroup GroupUnique
 * @brief Shows the game over screen
 * @details Displays the game over screen with score and options to restart or quit
 * param[in] g The game Strcture containing score
 * @return The chosen option ('r' for restart, 'q' for quit)
 * @author Gabriel Henrique Goncalves Da Silva
 * @version 20160529.013231
 * @date 2025-11-22
 */
int mostra_game_over(t_game g)
{
    int entrada;
    int max_y;
    int max_x;
    char pontos_str[50];
    int game_over_start_y, game_over_start_x;

    getmaxyx(stdscr, max_y, max_x);
    sprintf(pontos_str, "PONTUACAO: %d", g.pacman.score);

    game_over_start_y = max_y / 2 - 3;
    game_over_start_x = max_x / 2 - 20;

    timeout(-1);

    while(1)
    {
        clear();

        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(game_over_start_y, game_over_start_x + 20, "GAME OVER");
        attroff(COLOR_PAIR(1) | A_BOLD);

        mvprintw(game_over_start_y + 1, game_over_start_x + 15, "Obrigado por jogar");
        mvprintw(game_over_start_y + 2, game_over_start_x + 15, "%s", pontos_str);

        /* Instruções centralizadas */
        mvprintw(game_over_start_y + 4, game_over_start_x + 10, "Pressione 'R' para reiniciar o jogo");
        mvprintw(game_over_start_y + 5, game_over_start_x + 10, "Pressione 'Q' para sair");
        refresh();

        entrada = getch();
        if(entrada == 'r' || entrada == 'R')
        {
            timeout(100);
            return 'r';
        }
        else if(entrada == 'q' || entrada == 'Q')
        {
            timeout(100);
            return 'q';
        }
    }
}

/* ---------------------------------------------------------------------- */
/**
 * @ingroup GroupUnique
 * @brief Shows the victory screen
 * @details Displays the victory screen with score and options to restart or quit
 * param[in] g The game Strcture containing score
 * @return The chosen option ('r' for restart, 'q' for quit)
 * @author Assistant
 * @version 20160529.013231
 * @date 2025-11-22
 */
int mostra_vitoria(t_game g)
{
    int entrada;
    int max_y;
    int max_x;
    char pontos_str[50];
    int victory_start_y, victory_start_x;

    getmaxyx(stdscr, max_y, max_x);
    sprintf(pontos_str, "PONTUACAO FINAL: %d", g.pacman.score);

    victory_start_y = max_y / 2 - 3;
    victory_start_x = max_x / 2 - 20;

    timeout(-1);

    while(1)
    {
        clear();

        mvprintw(victory_start_y, victory_start_x + 20, "VITORIA!");
        mvprintw(victory_start_y + 1, victory_start_x + 10, "Parabens! Voce comeu todos os pontos!");
        mvprintw(victory_start_y + 2, victory_start_x + 15, "%s", pontos_str);
        mvprintw(victory_start_y + 4, victory_start_x, "Pressione R para jogar novamente");
        mvprintw(victory_start_y + 5, victory_start_x, "Pressione Q para sair do jogo");
        refresh();

        entrada = getch();
        if(entrada == 'r' || entrada == 'R')
        {
            timeout(100);
            return 'r';
        }
        else if(entrada == 'q' || entrada == 'Q')
        {
            timeout(100);
            return 'q';
        }
    }
}

/* ---------------------------------------------------------------------- */
/**
 * @ingroup GroupUnique
 * @brief Prints version and copyright information and exit
 * @details Prints version and copyright information (usually called by opt -V)
 * @return Void
 * @author Ruben Carlo Benante
 * @version 20160529.013231
 * @date 2016-05-29
 */
void copyr(void)
{
    IFDEBUG("copyr()");
    printf("%s - Version %s\n", "upecman", VERSION);
    printf("\nCopyright (C) %d %s <%s>, GNU GPL version 3 <http://gnu.org/licenses/gpl.html>. This  is  free  software:  you are free to change and redistribute it. There is NO WARRANTY, to the extent permitted by law. USE IT AS IT IS. The author takes no responsability to any damage this software may inflige in your data.\n\n", 2016, "Ruben Carlo Benante", "rcb@beco.cc");
    if(verb > 3)
    {
        printf("copyr(): Verbose: %d\n", verb);
    }
}

/* ---------------------------------------------------------------------- */
/**
 * @ingroup GroupUnique
 * @brief This function initializes some operations before start
 * @details Details to be written
 * @return Void
 * @todo Need to implement it. Its empty now.
 * @author Ruben Carlo Benante
 * @version 20160530.224016
 * @date 2016-05-30
 */
t_game upecman_init(void)
{
    IFDEBUG("init()");
    t_game g;
    int f;
    int y;

    for(y = 0; y < LABL; y++)
    {
        strcpy(g.lab[y], labmodel[y]);
    }

    /* Posição inicial do Pacman (@ no mapa) */
    g.pacman.pos.y = 17;
    g.pacman.pos.x = 10;
    g.pacman.dir = left;
    g.pacman.life = 3;
    g.pacman.score = 0;

    /* Posições iniciais dos fantasmas (B, P, I, C no mapa) */
    for(f = blinky; f <= clyde; f++)
    {
        switch(f)
        {
            case blinky:
                g.ghost[f].pos.y = 7;
                g.ghost[f].pos.x = 9;
                g.ghost[f].starget.y = 1;
                g.ghost[f].starget.x = 1;
                break;
            case pinky:
                g.ghost[f].pos.y = 9;
                g.ghost[f].pos.x = 11;
                g.ghost[f].starget.y = 1;
                g.ghost[f].starget.x = LABC - 2;
                break;
            case inky:
                g.ghost[f].pos.y = 10;
                g.ghost[f].pos.x = 11;
                g.ghost[f].starget.y = LABL - 2;
                g.ghost[f].starget.x = LABC - 2;
                break;
            case clyde:
                g.ghost[f].pos.y = 11;
                g.ghost[f].pos.x = 11;
                g.ghost[f].starget.y = LABL - 2;
                g.ghost[f].starget.x = 1;
                break;
        }
        g.ghost[f].dir = left;
        g.ghost[f].mode = current_ghost_mode;
    }

    powerup_active = 0;
    return g;
}

/* ---------------------------------------------------------------------- */
/**
 * @ingroup GroupUnique
 * @brief This function initializes some operations before start
 * @details Details to be written
 * @return Void
 * @todo Need to implement it. Its empty now.
 * @author Ruben Carlo Benante
 * @version 20160530.224016
 * @date 2016-05-30
 */
void printlab(t_game g)
{
    IFDEBUG("printlab()");
    int y;
    int f;
    int x;

    clear();

    /* Primeiro desenha o labirinto centralizado */
    for(y = 0; y < LABL; y++)
    {
        for(x = 0; x < LABC; x++)
        {
            char c = g.lab[y][x];
            /* Substitui caracteres especiais por espaços ou paredes */
            if(c == '#' || c == '.' || c == 'o' || c == ' ')
            {
                mvaddch(start_y + y, start_x + x, c);
            }
            else if(c == '-' || c == '|')
            {
                mvaddch(start_y + y, start_x + x, ' '); /* Remove as barras da gaiola */
            }
            else
            {
                mvaddch(start_y + y, start_x + x, ' ');
            }
        }
    }

    /* Desenha os fantasmas com as novas cores */
    for(f = blinky; f <= clyde; f++)
    {
        if(is_powerup_active() && g.ghost[f].mode != dead)
        {
            /* Fantasma azul quando vulnerável durante power-up */
            attron(COLOR_PAIR(5) | A_BOLD);
            mvaddch(start_y + g.ghost[f].pos.y, start_x + g.ghost[f].pos.x, 'I');
            attroff(COLOR_PAIR(5) | A_BOLD);
        }
        else if(g.ghost[f].mode == dead)
        {
            /* Fantasma branco quando morto */
            attron(A_BOLD);
            mvaddch(start_y + g.ghost[f].pos.y, start_x + g.ghost[f].pos.x, 'X');
            attroff(A_BOLD);
        }
        else
        {
            /* Fantasmas com cores específicas */
            attron(COLOR_PAIR(f + 1) | A_BOLD);
            mvaddch(start_y + g.ghost[f].pos.y, start_x + g.ghost[f].pos.x, 'I');
            attroff(COLOR_PAIR(f + 1) | A_BOLD);
        }
    }

    /* Finalmente desenha o Pacman centralizado */
    attron(COLOR_PAIR(6) | A_BOLD);
    mvaddch(start_y + g.pacman.pos.y, start_x + g.pacman.pos.x, 'C');
    attroff(COLOR_PAIR(6) | A_BOLD);

    refresh();
}

/* ---------------------------------------------------------------------- */
/* Game Logic Functions */

/**
 * @brief Verifica se uma posicao e parede
 */
int is_wall(char lab[LABL][LABC], int y, int x)
{
    if (y < 0 || y >= LABL || x < 0 || x >= LABC)
    {
        return 1;
    }
    char c = lab[y][x];
    /* Considera como parede apenas o '#' */
    return (c == '#');
}

/**
 * @brief Move fantasma para fora da gaiola
 */
void move_ghost_from_cage(t_game *g, int gi)
{
    /* No novo mapa, os fantasmas já começam fora da gaiola */
    return;
}

/**
 * @brief Tenta mover ghost para direcao
 */
void try_move_ghost(t_game *g, int gi, t_direction d)
{
    int ny;
    int nx;

    ny = g->ghost[gi].pos.y;
    nx = g->ghost[gi].pos.x;

    switch(d)
    {
        case up:
            ny--;
            break;
        case down:
            ny++;
            break;
        case left:
            nx--;
            break;
        case right:
            nx++;
            break;
    }

    if(!is_wall(g->lab, ny, nx))
    {
        g->ghost[gi].pos.y = ny;
        g->ghost[gi].pos.x = nx;
        g->ghost[gi].dir = d;
    }
}

/**
 * @brief Escolhe direcao aleatoria
 */
t_direction random_new_direction(void)
{
    return (t_direction)(rand() % 4);
}

/**
 * @brief Escolhe melhor direcao para perseguir alvo
 */
t_direction best_direction_to_target(t_game *g, int gi, int ty, int tx)
{
    int y;
    int x;
    int best_dist;
    t_direction best;
    t_direction dirs[4];
    int i;
    int ny;
    int nx;
    int dy;
    int dx;
    int dist;

    y = g->ghost[gi].pos.y;
    x = g->ghost[gi].pos.x;
    best_dist = 999999;
    best = g->ghost[gi].dir;
    dirs[0] = up;
    dirs[1] = down;
    dirs[2] = left;
    dirs[3] = right;

    for(i = 0; i < 4; i++)
    {
        ny = y;
        nx = x;

        switch(dirs[i])
        {
            case up:
                ny--;
                break;
            case down:
                ny++;
                break;
            case left:
                nx--;
                break;
            case right:
                nx++;
                break;
        }

        if(is_wall(g->lab, ny, nx))
        {
            continue;
        }

        dy = ty - ny;
        dx = tx - nx;
        dist = dy*dy + dx*dx;

        if(dist < best_dist)
        {
            best_dist = dist;
            best = dirs[i];
        }
    }

    return best;
}

/**
 * @brief Alvo de perseguicao dos fantasmas
 */
t_pos ghost_chase_target(t_game *g, int gi)
{
    t_pos t;
    int dy;
    int dx;
    int dist2;

    switch(gi)
    {
        case blinky:
            /* Blinky: persegue diretamente o Pacman */
            t = g->pacman.pos;
            break;

        case pinky:
            /* Pinky: tenta interceptar 4 posições à frente do Pacman */
            t = g->pacman.pos;
            switch(g->pacman.dir)
            {
                case up:
                    t.y -= 4;
                    t.x -= 4; /* Pequeno ajuste para melhor interceptação */
                    break;
                case down:
                    t.y += 4;
                    break;
                case left:
                    t.x -= 4;
                    break;
                case right:
                    t.x += 4;
                    break;
            }
            break;

        case inky:
            /* Inky: comportamento imprevisível baseado na posição do Blinky */
            {
                t_pos blinky_pos = g->ghost[blinky].pos;
                t = g->pacman.pos;
                switch(g->pacman.dir)
                {
                    case up:
                        t.y -= 2;
                        break;
                    case down:
                        t.y += 2;
                        break;
                    case left:
                        t.x -= 2;
                        break;
                    case right:
                        t.x += 2;
                        break;
                }
                /* Fórmula do Inky: dobra o vetor do Pacman e subtrai a posição do Blinky */
                t.y = 2 * t.y - blinky_pos.y;
                t.x = 2 * t.x - blinky_pos.x;
            }
            break;

         case clyde:
            /* Clyde: foge para o canto quando muito perto do Pacman */
            dy = g->pacman.pos.y - g->ghost[clyde].pos.y;
            dx = g->pacman.pos.x - g->ghost[clyde].pos.x;
            dist2 = dy*dy + dx*dx;

            if(dist2 < 64) /* 8*8 = 64 */
            {
                t.y = LABL - 2;
                t.x = 1;
            }
            else
            {
                t = g->pacman.pos;
            }
            break;
    }

    /* Garante que o alvo está dentro dos limites do labirinto */
    if(t.y < 0)
    {
        t.y = 0;
    }
    if(t.y >= LABL)
    {
        t.y = LABL - 1;
    }
    if(t.x < 0)
    {
        t.x = 0;
    }
    if(t.x >= LABC)
    {
        t.x = LABC - 1;
    }

    return t;
}

/**
 * @brief Movimento frightened (azul) durante power-up
 */
void move_ghost_frightened(t_game *g, int gi)
{
    t_direction nd;
    nd = random_new_direction();
    try_move_ghost(g, gi, nd);
}

/**
 * @brief Movimentacao geral dos fantasmas
 */
void move_ghosts(t_game *g)
{
    int i;
    t_ghost *gh;
    t_pos t;
    t_direction nd;

    for(i = 0; i < 4; i++)
    {
        gh = &g->ghost[i];

        /* Durante power-up, todos os fantasmas ficam com medo */
        if(is_powerup_active() && gh->mode != dead)
        {
            move_ghost_frightened(g, i);
            continue;
        }

        /* Modo scatter: fantasmas voltam para seus cantos */
        if(gh->mode == scatter)
        {
            t = gh->starget;
            nd = best_direction_to_target(g, i, t.y, t.x);
            try_move_ghost(g, i, nd);
            continue;
        }

        /* Modo chase: perseguição com comportamentos específicos */
        if(gh->mode == chase)
        {
            t = ghost_chase_target(g, i);
            nd = best_direction_to_target(g, i, t.y, t.x);
            try_move_ghost(g, i, nd);
            continue;
        }

        /* Fantasmas mortos voltam para casa */
        if(gh->mode == dead)
        {
            switch(i)
            {
                case blinky:
                    nd = best_direction_to_target(g, i, 7, 9);
                    break;
                case pinky:
                    nd = best_direction_to_target(g, i, 9, 11);
                    break;
                case inky:
                    nd = best_direction_to_target(g, i, 10, 11);
                    break;
                case clyde:
                    nd = best_direction_to_target(g, i, 11, 11);
                    break;
            }
            try_move_ghost(g, i, nd);

            /* Se o fantasma morto chegou em casa, revive */
            if((i == blinky && gh->pos.y == 7 && gh->pos.x == 9) ||
               (i == pinky && gh->pos.y == 9 && gh->pos.x == 11) ||
               (i == inky && gh->pos.y == 10 && gh->pos.x == 11) ||
               (i == clyde && gh->pos.y == 11 && gh->pos.x == 11))
            {
                gh->mode = current_ghost_mode;
            }
            continue;
        }
    }
}

/**
 * @brief Move o pacman baseado na direcao (apenas muda a direção)
 */
void move_pacman(t_game *g, int direction)
{
    /* Apenas atualiza a última direção para movimento contínuo */
    switch(direction)
    {
        case KEY_UP:
            last_direction = up;
            break;
        case KEY_DOWN:
            last_direction = down;
            break;
        case KEY_LEFT:
            last_direction = left;
            break;
        case KEY_RIGHT:
            last_direction = right;
            break;
        default:
            return;
    }
    g->pacman.dir = last_direction;
}

/**
 * @brief Verifica colisoes entre pacman e fantasmas
 */
int check_collision(t_game *g)
{
    int i;

    for(i = 0; i < 4; i++)
    {
        if(g->pacman.pos.y == g->ghost[i].pos.y &&
           g->pacman.pos.x == g->ghost[i].pos.x)
        {
            /* Durante power-up, Pacman pode comer fantasmas */
            if(is_powerup_active() && g->ghost[i].mode != dead)
            {
                g->ghost[i].mode = dead;
                g->pacman.score += 200;
                return 0; /* Não perde vida */
            }
            else if(g->ghost[i].mode != dead)
            {
                /* Fantasma pega pacman - PERDE VIDA */
                return 1;
            }
        }
    }
    return 0;
}

/**
 * @brief Atualiza pontuacao baseada no que pacman comeu
 */
void update_score(t_game *g)
{
    char current_tile;
    int i;

    current_tile = g->lab[g->pacman.pos.y][g->pacman.pos.x];

    switch(current_tile)
    {
        case '.':
            g->pacman.score += PONTOS_PELOTA;
            g->lab[g->pacman.pos.y][g->pacman.pos.x] = ' ';
            break;
        case 'o':
            g->pacman.score += PONTOS_PILULA;
            g->lab[g->pacman.pos.y][g->pacman.pos.x] = ' ';
            /* Ativa o power-up por 10 segundos */
            activate_powerup();
            break;
    }
}

/**
 * @brief Verifica se o jogador venceu (comeu todos os pontos)
 */
int check_victory(t_game *g)
{
    int y;
    int x;

    for(y = 0; y < LABL; y++)
    {
        for(x = 0; x < LABC; x++)
        {
            if(g->lab[y][x] == '.' || g->lab[y][x] == 'o')
            {
                return 0; /* Ainda tem pontos */
            }
        }
    }
    return 1; /* Todos os pontos foram comidos */
}

/* ---------------------------------------------------------------------- */
/* vi: set ai et ts=4 sw=4 tw=0 wm=0 fo=croql : C config for Vim modeline */
/* Template by Dr. Beco <rcb at beco dot cc> Version 20160612.142044      */
