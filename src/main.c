/*          Chemin de code pour atteindre ce programme. Attention, il faut se placer à l'intérieur du dossier contenant bin, lib, include et src

gcc src/main.c -o bin/prog -I include -L lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer
bin/prog

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL.h>
#include <SDL_mixer.h>

#define WINDOW_RULES_WIDTH 900            //Définition de la largeur de la fenêtre et de sa hauteur pour les règles et l'accueil
#define WINDOW_RULES_HEIGHT 1013

#define JvJ 0           //Définition des équivalents des modes et des menus pour l'accueil
#define JvIA 1
#define J1 1
#define J2 2

#define NOIR 1          //Définition des équivalents des pierre pour le tableau
#define BLANC -1
#define NOIR_V 2
#define BLANC_V -2


/*------------------------------------------------------------------------------------------------------------------------*/

int countLetters(char word[])
{
    int i = 0;                //On met i à 0
    while (word[i] != '\0')   //Tant que le mot/phrase n'est pas fini
        i++;                  //on incrémente i de 1
    return i;                 //On retourne i
}

/*------------------------------------------------------------------------------------------------------------------------*/

int calculTest(int *t, int turn, int lastCase, int dimension)
{
    int compt = 0;                                              //On initialise un compteur à 0

    if(t[lastCase-1] == turn || lastCase % dimension == 0)      //Si la case à gauche de la lastCase est du tour "turn", ou si la case est sur le bord gauche du plateau
        compt++;                                                //on incrémente le comtpeur de 1
    else if(t[lastCase-1] == -turn){                            //Sinon si la case à gauche de lastCase est de l'autre tour
        t[lastCase] = -2 * turn;                                //alors on change lastCase en "BLANC_V" ou "NOIR_V" qui signifient que la case est "vérifiée"
        if(calculTest(t, turn, lastCase-1, dimension) == 1)     //On rappelle alors la fonction calculTest sur la case à gauche de la case testée
            compt++;                                            //Si la case à gauche est entourée, alors on incrémente le compteur de 1
    }
    else if(t[lastCase-1] == -2 * turn)                         //Sinon, si la case à gauche de lastCase est déjà vérifiée, c'est à dire si elle vaut "BLANC_V" ou NOIR_V
        compt++;                                                //on incrémente le compteur de 1
    /*----------------------------*/
    if(t[lastCase-dimension] == turn || lastCase < dimension)   //Si la case au dessus de la lastCase est du tour "turn", ou si la case est sur le bord haut du plateau
        compt++;                                                //Même chose qu'avant
    else if(t[lastCase-dimension] == -turn){
        t[lastCase] = -2 * turn;
        if(calculTest(t, turn, lastCase-dimension, dimension) == 1)
            compt++;
    }
    else if(t[lastCase-dimension] == -2 * turn)
        compt++;
    /*----------------------------*/
    if(t[lastCase+1] == turn || lastCase % dimension == dimension -1)   //Si la case à droite de la lastCase est du tour "turn", ou si la case est sur le bord droit du plateau
        compt++;                                                        //Même chose qu'avant
    else if(t[lastCase+1] == -turn){
        t[lastCase] = -2 * turn;
        if(calculTest(t, turn, lastCase+1, dimension) == 1)
            compt++;
    }
    else if(t[lastCase+1] == -2 * turn)
        compt++;
    /*----------------------------*/
    if(t[lastCase+dimension] == turn || lastCase > dimension*(dimension -1))    //Si la case au dessous de la lastCase est du tour "turn", ou si la case est sur le bord bas du plateau
        compt++;                                                                //Même chose qu'avant
    else if(t[lastCase+dimension] == -turn){
        t[lastCase] = -2 * turn;
        if(calculTest(t, turn, lastCase+dimension, dimension) == 1)
            compt++;
    }
    else if(t[lastCase+dimension] == -2 * turn)
        compt++;
    /*----------------------------*/
    if (compt==4){                  //Si le compteur arrive à 4, toutes les conditions sont validées
        t[lastCase] = -2 * turn;    //on peut donc mettre lastCase à "BLANC_V" ou "NOIR_V" car la case est bien entourée
        return 1;                   //et on retourne 1
    }
    else                            //Sinon les conditions ne sont pas validées
        return 0;                   //on retourne donc 0
}

/*------------------------------------------------------------------------------------------------------------------------*/

int transformTab(int *t, int dimension, int change, int score, int isAmbient, Mix_Music *music)
{
    int temp = score;
    if (change == 0){
        for (int i = 0; i < dimension*dimension; i++)
        {
            if(t[i] == -2)
                t[i] = -1;
            else if(t[i] == 2)
                t[i] = 1;
        }
    }
    else if(change == 1){
        for (int i = 0; i < dimension*dimension; i++)
        {
            if(t[i] == BLANC_V){
                t[i] = 0;
                score++;
            }
        }
        if (score != temp && isAmbient == 1)
        {
            Mix_PlayMusic(music, 1);
            for (int i = 0; i < (score-temp-1); i++){
                SDL_Delay(20);
                Mix_RewindMusic();
            }
        }
    }
    else if(change == 2){
        for (int i = 0; i < dimension*dimension; i++)
        {
            if(t[i] == NOIR_V){
                t[i] = 0;
                score++;
            }
        }
        if (score != temp && isAmbient == 1)
        {
            Mix_PlayMusic(music, 1);
            for (int i = 0; i < (score-temp-1); i++){
                SDL_Delay(20);
                Mix_RewindMusic();
            }
        }
    }
    return score;
}
            
/*------------------------------------------------------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    SDL_bool rulesLaunched = SDL_TRUE;      //Variable de lancement du sous-programme qui affiche les règles
    SDL_bool welcomeLaunched = SDL_TRUE;    //Variable de lancement du sous-programme qui affiche l'accueil
    SDL_bool programLaunched = SDL_TRUE;    //Variable pseudo-infini du programme principal
    SDL_bool endGameLaunched = SDL_TRUE;    //Variable de lancement du sous-programme qui affiche le résultat

    SDL_Rect rectangle;                     //Variable rectangle qu'on utilise dans tout le programme pour afficher les textures à des points précis
    
    SDL_Surface *texte = NULL;              //Variable texte qu'on utilise dans tout le programme 

    SDL_Event event;                        //Variable evenement permettant de connaître en temps réel l'evenement qui a lieu (ou pas)

    SDL_Window *window = NULL;              //Création de la fenêtre
    SDL_Renderer *renderer = NULL;          //Création du rendu

    int i;                                  //Variable de comptage
 
    int mode = 0, dimension = 9, menuName = 0, isMusic = 1, isAmbient = 1;   //Variables intervenant dans la page "welcome" (mode de jeu, dimension du plateau, menu du nom de joueur, et si il y a de la musique)
    char nameJ1[20], nameJ2[20], *tempC, dimensionC[3];                        //Nom du joueur 1, Nom du joueur 2, tableau de caractères de sauvegarde / manipulation, dimension du plateau sous forme de caractères
    sprintf(nameJ1, " ");                                                      //On met un espace dans nameJ1 pour toujours avoir quelquechose à afficher
    sprintf(nameJ2, " ");                                                      //On met un espace dans nameJ2 pour toujours avoir quelquechose à afficher

    int turn = 1, *t, *temp, clicMouse, passB, passN;     //Variables intervenant dans le programme principal : le tour, le tableau du jeu, un tableau de sauvegarde, et une variable sauvegardant le clic de la souris

    int scoreNoir = 0, scoreBlanc = 0;      //Variable contenant les scores en format int
    char scoreNoirC[50], scoreBlancC[50];   //Variables contenant les score en format char (ex : "Joueur 1 : 12")
    char gameOver[50];                      //Variable contenant le messages du Game Over : le gagnant de la partie
    
    int lastNoir, lastBlanc;                //Variables contenant le dernier coup joué par chacun des joueurs (pour eviter la répétition des coups)

/*------------------------------------------------------------*/

    if(SDL_Init(SDL_INIT_VIDEO) != 0)       //Lancement SDL
        goto endProg;

                                            //Lancement de SDL_mixer
    if(Mix_OpenAudio(96000, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024)<0) // création de la configuration de la carte son
        goto endProg;
    Mix_VolumeMusic(50);

    if(TTF_Init() == -1)                    //Lancement SDL_TTF :
        goto endProg;

    TTF_Font *police = NULL;                    //Variable police du jeu
    police = TTF_OpenFont("src/police.ttf", 40);//On sauvegarde l'emplacement de la police dans la variable
    SDL_Color noir = {0,0,0};                   //Définition de la couleur du texte, ici noir

    if (SDL_CreateWindowAndRenderer(WINDOW_RULES_WIDTH, WINDOW_RULES_HEIGHT, 0, &window, &renderer) != 0 )  //Création fenêtre et rendu avec la largeur WINDOW_RULES_WIDTH et en hauteur WINDOW_RULES_HEIGHT
        goto endProg;
        
    if(SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE) != 0)                                    //On détermine la couleur des figures dessinée plus tard 
        goto endProg;

    SDL_SetWindowTitle(window, "Jeu de Go");                //On détermine le nom de la fenêtre

    SDL_Surface *iconSurface = IMG_Load("src/logo.png");    //On charge une surface qui correspond a l'icone de la fenêtre
    SDL_SetWindowIcon(window, iconSurface);                 //On sauvegarde le'icone dans la fenêtre

/*-------------------------------*/             //Chargement de la texture "rulesText" et de son affichage
    SDL_Surface *rules = NULL;                                                      //On initialise une nouvelle surface appelé "rules"
    rules = IMG_Load("src/Regles.png");                                             //On charge l'image "Regles" dans la nouvelle surface
    if(rules == NULL)                                                               //On vérifie si l'image a bien été chargée
        goto endProg;      //sinon on quitte le programme 
    SDL_Texture *rulesText = NULL;                                                  //On initialise une nouvelle texture appelé "rulesText"
    rulesText = SDL_CreateTextureFromSurface(renderer, rules);                      //On transforme la surface "rules" en une texture sauvegardée dans "rulesText"
    if(rulesText == NULL)                                                           //On vérifie si la texture a chargée
        goto endProg;   //sinon on quitte le programme
    SDL_FreeSurface(rules);                                                         //On libère la mémoire de la surface "rules" car on n'en a plus besoin
                            //On défini un rectangle où l'on placera la texture
    rectangle.w = WINDOW_RULES_WIDTH;                                               //On défini la largeur de ce que l'on affichera
    rectangle.h = WINDOW_RULES_HEIGHT;                                              //On défini la hauteur de ce que l'on affichera
    rectangle.x = 0;                                                                //On défini la coordonnée x du coin en haut à gauche de ce que l'on affichera
    rectangle.y = 0;                                                                //On défini la coordonnée y du coin en haut à gauche de ce que l'on affichera

    if(SDL_RenderCopy(renderer, rulesText, NULL, &rectangle))                               //On copie la texture dans le rendu dans les coordonnées donnée par "rectangle" et on vérifie si ça a marché
        goto endProg;   //sinon on quitte


    SDL_RenderPresent(renderer);                    //On actualise le rendu 
    while (rulesLaunched)                           //Boucle des règles, affiché tant que l'utilisateur ne clique pas
    {
        while(SDL_PollEvent(&event))                //Boucle d'evenement, qui détermine si il y a un evenement
        {
            switch(event.type)                      //Boucle de type d'evenement, qui détermine quel type d'evenement il y a eu 
            {
                case SDL_MOUSEBUTTONDOWN :          //Dans le cas d'un clic de souris :
                    rulesLaunched = SDL_FALSE;      //on quitte la page des règles
                    break;

                case SDL_QUIT :                     //Dans le cas d'une fermeture de la fenêtre
                    rulesLaunched = SDL_FALSE;      //on ferme les règles
                    welcomeLaunched = SDL_FALSE;    //on ferme l'accueil avant qu'il se lance 
                    programLaunched = SDL_FALSE;    //on ferme le programme principal avant qu'il se lance
                    endGameLaunched = SDL_FALSE;    //on ferme l'annonce du gagnant avant qu'il ne se lance
                    break;

                default :
                    break;
            }
        }
    }
    SDL_DestroyTexture(rulesText);                  //On libère la mémoire de la texture "rulesText" car on n'en a plus besoin

/*------------------------------------------------------------*/
/*-------------------------------*/             //Chargement de la texture "welcomeText"
    SDL_Surface *welcome = NULL;               
    welcome = IMG_Load("src/Accueil.png");
    if(welcome == NULL)
        goto endProg;
    SDL_Texture *welcomeText = NULL;
    welcomeText = SDL_CreateTextureFromSurface(renderer, welcome);
    if(welcomeText == NULL)
        goto endProg;

    SDL_FreeSurface(welcome);

    while (welcomeLaunched)                                             //Boucle d'accueil, qui affiche la page d'accueil jusqu'a ce qu'on clique sur "GO !"
    {
                            //On défini un rectangle où l'on placera la texture "welcomeText"
        rectangle.w = WINDOW_RULES_WIDTH;                              
        rectangle.h = WINDOW_RULES_HEIGHT;
        rectangle.x = 0;
        rectangle.y = 0;

                            //On affiche la texture "welcomeText"
        if(SDL_RenderCopy(renderer, welcomeText, NULL, &rectangle))
            goto endProg; 

        sprintf(dimensionC, "%i", dimension);                           //On transforme l'entier "dimension" en caractères dans "dimensionC"
        
        texte = TTF_RenderText_Blended(police, dimensionC, noir);       //On transforme le texte "dimensionC" en surface avec la police "police" et la couleur "noir"

/*-------------------------------*/             //Chargement de la texture "textDim" et affichage à deux endroits
        SDL_Texture *textDim = NULL;                                    //On initialise une nouvelle texture "textDim"
        textDim = SDL_CreateTextureFromSurface(renderer, texte);        //On transforme la surface "texte" en texture sauvegardé dans "textDim"
        SDL_FreeSurface(texte);                                         //On libère la mémoire de la surface "texte" car on n'en a plus besoin
        if(textDim == NULL)                                             //On vérifie que ça marche
            goto endProg;
                            //On défini un rectangle où l'on placera la texture
        rectangle.w = 24*countLetters(dimensionC);                      //On calcul le nombre de lettres de "dimensionC" et en déduit la largeur du rectangle 
        rectangle.h = 36;
        rectangle.x = 311;
        rectangle.y = 323;

        if(SDL_RenderCopy(renderer, textDim, NULL, &rectangle))         //On place la texture
            goto endProg; 
                            //On défini un rectangle où l'on placera la texture
        rectangle.x = 383;
        rectangle.y = 323;
        
        if(SDL_RenderCopy(renderer, textDim, NULL, &rectangle))
            goto endProg; 

/*-------------------------------*/             //Chargement de la texture "textJ1" et affichage
        texte = TTF_RenderText_Blended(police, nameJ1, noir);
        
        SDL_Texture *textJ1 = NULL;
        textJ1 = SDL_CreateTextureFromSurface(renderer, texte);
        SDL_FreeSurface(texte);
        if(textJ1 == NULL)
            goto endProg;
                            //On défini un rectangle où l'on placera la texture
        rectangle.w = 24*(countLetters(nameJ1)-1);
        rectangle.h = 43;
        rectangle.x = 159;
        rectangle.y = 560;

        if(SDL_RenderCopy(renderer, textJ1, NULL, &rectangle))
            goto endProg; 
        SDL_DestroyTexture(textJ1);

/*-------------------------------*/             //Chargement de la texture "textJ2" et affichage
        texte = TTF_RenderText_Blended(police, nameJ2, noir);
        
        SDL_Texture *textJ2 = NULL;
        textJ2 = SDL_CreateTextureFromSurface(renderer, texte);
        if(textJ2 == NULL)
            goto endProg;

        rectangle.w = 24*(countLetters(nameJ2)-1);
        rectangle.h = 43;
        rectangle.x = 159;
        rectangle.y = 737;

                    //On affiche la texture dans le rendu 
        if(SDL_RenderCopy(renderer, textJ2, NULL, &rectangle))
            goto endProg; 
        SDL_DestroyTexture(textJ2);

        /*if(isMusic==-1)
        {
            if(SDL_RenderDrawLine(renderer, 54,44,154,144) != 0)   //On affiche une ligne sous l'emplacement du pseudo 2
                goto endProg;
        }*/
        if(isAmbient==-1)
        {
            if(SDL_RenderDrawLine(renderer, 746,44,846,144) != 0)   //On affiche une ligne sous l'emplacement du pseudo 2
                goto endProg;
        }


        if(mode == JvJ)                 //Si le mode est "Joueur versus Joueur"
        {
            rectangle.w = 36;         
            rectangle.h = 24;    
            rectangle.x = 194;
            rectangle.y = 402;
            if(SDL_RenderFillRect(renderer, &rectangle) != 0)       //On affiche un rectangle noir sur la case "Joueur versus Joueur"
                goto endProg;
        }
        /*else if(mode == JvIA)           //Si le mode est "Joueur versus IA"
        {
            rectangle.w = 36;
            rectangle.h = 24;    
            rectangle.x = 194;
            rectangle.y = 438;
            if(SDL_RenderFillRect(renderer, &rectangle) != 0)       //On affiche un rectangle noir sur la case "Joueur versus IA"
                goto endProg;
        }*/

        if(menuName == J1)              //Si la case du pseudo joueur 1 est selectionnée
        {
            if(SDL_RenderDrawLine(renderer, 160, 603, 639, 603) != 0)   //On affiche une ligne sous l'emplacement du pseudo 1
                goto endProg;
        }
        else if(menuName == J2)         //Si la case du pseudo joueur 2 est selectionnée
        {
            if(SDL_RenderDrawLine(renderer, 160, 780, 639, 780) != 0)   //On affiche une ligne sous l'emplacement du pseudo 2
                goto endProg;
        }

        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_MOUSEBUTTONDOWN :                                                                                  //Si on clique
                    menuName = 0;                                                                                           //De base la case du pseudo des joueur n'est pas selectionnée
                    if (event.motion.x >= 3 && event.motion.x <= 897 && event.motion.y >= 904 && event.motion.y <= 999)     //Si on clique sur le bouton "GO !"
                        welcomeLaunched = SDL_FALSE;                                                                        //On sort de la boucle d'accueil

                    /*if (event.motion.x >=54 && event.motion.x <= 154 && event.motion.y >= 44 && event.motion.y <= 144)
                        isMusic*=-1;*/
                        
                    if (event.motion.x >=746 && event.motion.x <= 846 && event.motion.y >= 44 && event.motion.y <= 144)
                        isAmbient*=-1;

                    if (event.motion.x >= 442 && event.motion.x <= 478 && event.motion.y >= 323 && event.motion.y <= 359)   //Si on clique sur le "+"
                    {
                        if(dimension<25)                                                                                   
                            dimension++;                                                                                    //On incrémente dimension de 1
                    }

                    if (event.motion.x >= 272 && event.motion.x <= 308 && event.motion.y >= 323 && event.motion.y <= 359)   //Si on clique sur le "-"
                    {
                        if(dimension>5)
                            dimension--;                                                                                    //On décremente dimesion de 1
                    }

                    if (event.motion.x >= 192 && event.motion.x <= 232 && event.motion.y >= 400 && event.motion.y <= 428)   //Si on clique sur "Joueur versus Joueur"
                        mode = 0;                                                                                           //On met mode à 0

                    /*if (event.motion.x >= 192 && event.motion.x <= 232 && event.motion.y >= 436 && event.motion.y <= 464)   //Si on clique sur "Joueur versus IA"
                        mode = 1;                                                                                           //On met mode à 1*/

                    if (event.motion.x >= 159 && event.motion.x <= 641 && event.motion.y >= 560 && event.motion.y <= 603)   //Si on clique sur la case du pseudo 1
                        menuName = 1;                                                                                       //On met menuName à 1

                    if (event.motion.x >= 159 && event.motion.x <= 641 && event.motion.y >= 737 && event.motion.y <= 780)   //Si on clique sur la case du pseudo 2
                        menuName = 2;                                                                                       //On met menuName à 2
                    break;

                case SDL_KEYDOWN :                                      //Si on appuye sur une touche du clavier 
                    if(menuName == 1)                                   //Si la case du pseudo du joueur 1 est selectionnée
                    {
                        if(countLetters(nameJ1) < 17)                              //Si il y a moins de 17 caractères dans nameJ1
                        {
                            switch(event.key.keysym.sym)                //Selon la touche appuyée, on sauvegarde la touche dans nameJ1
                            {
                                case SDLK_a : 
                                    sprintf(nameJ1, "%sA", nameJ1);
                                    continue;
                                case SDLK_b : 
                                    sprintf(nameJ1, "%sB", nameJ1);
                                    continue;
                                case SDLK_c : 
                                    sprintf(nameJ1, "%sC", nameJ1);
                                    continue;
                                case SDLK_d : 
                                    sprintf(nameJ1, "%sD", nameJ1);
                                    continue;
                                case SDLK_e : 
                                    sprintf(nameJ1, "%sE", nameJ1);
                                    continue;
                                case SDLK_f : 
                                    sprintf(nameJ1, "%sF", nameJ1);
                                    continue;
                                case SDLK_g : 
                                    sprintf(nameJ1, "%sG", nameJ1);
                                    continue;
                                case SDLK_h : 
                                    sprintf(nameJ1, "%sH", nameJ1);
                                    continue;
                                case SDLK_i : 
                                    sprintf(nameJ1, "%sI", nameJ1);
                                    continue;
                                case SDLK_j : 
                                    sprintf(nameJ1, "%sJ", nameJ1);
                                    continue;
                                case SDLK_k : 
                                    sprintf(nameJ1, "%sK", nameJ1);
                                    continue;
                                case SDLK_l : 
                                    sprintf(nameJ1, "%sL", nameJ1);
                                    continue;
                                case SDLK_m : 
                                    sprintf(nameJ1, "%sM", nameJ1);
                                    continue;
                                case SDLK_n : 
                                    sprintf(nameJ1, "%sN", nameJ1);
                                    continue;
                                case SDLK_o : 
                                    sprintf(nameJ1, "%sO", nameJ1);
                                    continue;
                                case SDLK_p : 
                                    sprintf(nameJ1, "%sP", nameJ1);
                                    continue;
                                case SDLK_q : 
                                    sprintf(nameJ1, "%sQ", nameJ1);
                                    continue;
                                case SDLK_r : 
                                    sprintf(nameJ1, "%sR", nameJ1);
                                    continue;
                                case SDLK_s : 
                                    sprintf(nameJ1, "%sS", nameJ1);
                                    continue;
                                case SDLK_t : 
                                    sprintf(nameJ1, "%sT", nameJ1);
                                    continue;
                                case SDLK_u : 
                                    sprintf(nameJ1, "%sU", nameJ1);
                                    continue;
                                case SDLK_v : 
                                    sprintf(nameJ1, "%sV", nameJ1);
                                    continue;
                                case SDLK_w : 
                                    sprintf(nameJ1, "%sW", nameJ1);
                                    continue;
                                case SDLK_x : 
                                    sprintf(nameJ1, "%sZ", nameJ1);
                                    continue;
                                case SDLK_y : 
                                    sprintf(nameJ1, "%sY", nameJ1);
                                    continue;
                                case SDLK_z : 
                                    sprintf(nameJ1, "%sZ", nameJ1);
                                    continue;
                                case SDLK_BACKSPACE :
                                    i = countLetters(nameJ1);
                                    if(i>1)
                                    {
                                        tempC = calloc(i, sizeof(char));
                                        for (int j = 0; j < i-1; j++)
                                        {
                                            tempC[j] = nameJ1[j];
                                        }
                                        tempC[i] = '\0';
                                        sprintf(nameJ1, "%s", tempC);
                                        free(tempC);
                                    }
                                    continue;
                                    
                                default : 
                                    continue;
                            }
                        }
                        else                                            
                        {
                            if (event.key.keysym.sym == SDLK_BACKSPACE) //Si on appuye sur la touche retour 
                            {
                                i = countLetters(nameJ1);               //On compte le nombre de lettre de nameJ1
                                if(i>1)                                 //Si il y a plus d'une lettre 
                                {
                                    tempC = calloc(i, sizeof(char));        //On se sert de tempC comme d'une sauvegarde
                                    for (int j = 0; j < i-1; j++)       //On met chaque caractère de nameJ1 dans tempC SAUF le dernier
                                    {
                                        tempC[j] = nameJ1[j];
                                    }
                                    tempC[i] = '\0';                    //On met la marque de fin de chaîne de caractères sur le dernier caractère
                                    sprintf(nameJ1, "%s", tempC);       //On met tempC dans nameJ1
                                    free(tempC);                        //On libère tempC
                                }
                            }
                            continue;
                        }
                    }
                    else if(menuName == 2)                                   //Si la case du pseudo du joueur 2 est selectionnée
                    {
                        if(countLetters(nameJ2) < 17)
                        {
                            switch(event.key.keysym.sym)
                            {
                                case SDLK_a : 
                                    sprintf(nameJ2, "%sA", nameJ2);
                                    continue;
                                case SDLK_b : 
                                    sprintf(nameJ2, "%sB", nameJ2);
                                    continue;
                                case SDLK_c : 
                                    sprintf(nameJ2, "%sC", nameJ2);
                                    continue;
                                case SDLK_d : 
                                    sprintf(nameJ2, "%sD", nameJ2);
                                    continue;
                                case SDLK_e : 
                                    sprintf(nameJ2, "%sE", nameJ2);
                                    continue;
                                case SDLK_f : 
                                    sprintf(nameJ2, "%sF", nameJ2);
                                    continue;
                                case SDLK_g : 
                                    sprintf(nameJ2, "%sG", nameJ2);
                                    continue;
                                case SDLK_h : 
                                    sprintf(nameJ2, "%sH", nameJ2);
                                    continue;
                                case SDLK_i : 
                                    sprintf(nameJ2, "%sI", nameJ2);
                                    continue;
                                case SDLK_j : 
                                    sprintf(nameJ2, "%sJ", nameJ2);
                                    continue;
                                case SDLK_k : 
                                    sprintf(nameJ2, "%sK", nameJ2);
                                    continue;
                                case SDLK_l : 
                                    sprintf(nameJ2, "%sL", nameJ2);
                                    continue;
                                case SDLK_m : 
                                    sprintf(nameJ2, "%sM", nameJ2);
                                    continue;
                                case SDLK_n : 
                                    sprintf(nameJ2, "%sN", nameJ2);
                                    continue;
                                case SDLK_o : 
                                    sprintf(nameJ2, "%sO", nameJ2);
                                    continue;
                                case SDLK_p : 
                                    sprintf(nameJ2, "%sP", nameJ2);
                                    continue;
                                case SDLK_q : 
                                    sprintf(nameJ2, "%sQ", nameJ2);
                                    continue;
                                case SDLK_r : 
                                    sprintf(nameJ2, "%sR", nameJ2);
                                    continue;
                                case SDLK_s : 
                                    sprintf(nameJ2, "%sS", nameJ2);
                                    continue;
                                case SDLK_t : 
                                    sprintf(nameJ2, "%sT", nameJ2);
                                    continue;
                                case SDLK_u : 
                                    sprintf(nameJ2, "%sU", nameJ2);
                                    continue;
                                case SDLK_v : 
                                    sprintf(nameJ2, "%sV", nameJ2);
                                    continue;
                                case SDLK_w : 
                                    sprintf(nameJ2, "%sW", nameJ2);
                                    continue;
                                case SDLK_x : 
                                    sprintf(nameJ2, "%sZ", nameJ2);
                                    continue;
                                case SDLK_y : 
                                    sprintf(nameJ2, "%sY", nameJ2);
                                    continue;
                                case SDLK_z : 
                                    sprintf(nameJ2, "%sZ", nameJ2);
                                    continue;
                                case SDLK_BACKSPACE :
                                    i = 0;
                                    while(nameJ2[i] != '\0')
                                        i++;
                                    if(i>1)
                                    {
                                        tempC = calloc(i, sizeof(char));
                                        for (int j = 0; j < i-1; j++)
                                        {
                                            tempC[j] = nameJ2[j];
                                        }
                                        tempC[i] = '\0';
                                        sprintf(nameJ2, "%s", tempC);
                                        free(tempC);
                                    }
                                    continue;

                                default : 
                                    continue;
                            }
                        }
                        else 
                        {
                            if (event.key.keysym.sym == SDLK_BACKSPACE)
                            {
                                i = 0;
                                while(nameJ2[i] != '\0')
                                    i++;
                                if(i>1)
                                {
                                tempC = calloc(i, sizeof(char));
                                    for (int j = 0; j < i-1; j++)
                                    {
                                        tempC[j] = nameJ2[j];
                                    }
                                    tempC[i] = '\0';
                                    sprintf(nameJ2, "%s", tempC);
                                    free(tempC);
                                    continue;
                                }
                                continue;
                            }
                            continue;
                        }  
                    }

                case SDL_QUIT :                     //Si on quitte la fenêtre
                    rulesLaunched = SDL_FALSE;
                    welcomeLaunched = SDL_FALSE;
                    programLaunched = SDL_FALSE;
                    endGameLaunched = SDL_FALSE;
                    break;

                default :
                    break;
            }
        }
        SDL_RenderPresent(renderer);
    }
    SDL_DestroyTexture(welcomeText);

    if(nameJ1[1]=='\0')                 //Sécurité pour les noms des joueurs : s'ils sont vide, on met "Noir" ou "Blanc"
        sprintf(nameJ1, "Noir");
    if(nameJ2[1]=='\0')
        sprintf(nameJ2, "Blanc");

/*------------------------------------------------------------*/

    SDL_DestroyRenderer(renderer);      //On détruis le rendu et la fenêtre 
    SDL_DestroyWindow(window);

    if (SDL_CreateWindowAndRenderer(dimension*36, (dimension+2)*36, 0, &window, &renderer) != 0 )  //On recréé la fenêtre avec les dimensions choisies par les joueurs
        goto endProg;

    SDL_SetWindowTitle(window, "Jeu de Go");

    SDL_SetWindowIcon(window, iconSurface);

    Mix_Music *mc_hurt = NULL;
    mc_hurt = Mix_LoadMUS("src/mc_hurt.mp3");

    Mix_Music *posePierre1 = NULL;
    posePierre1 = Mix_LoadMUS("src/posePierre1.mp3");

    Mix_Music *posePierre2 = NULL;
    posePierre2 = Mix_LoadMUS("src/posePierre2.mp3");


/*-------------------------------*/             //Chargement de la texture "textTab" = une intersection du plateau
    SDL_Surface *tab = NULL;
    tab = IMG_Load("src/Go.png");
    if(tab == NULL)
        goto endProg;
    SDL_Texture *textTab = NULL;
    textTab = SDL_CreateTextureFromSurface(renderer, tab);
    if(textTab == NULL)
        goto endProg;
    SDL_FreeSurface(tab);
/*-------------------------------*/             //Chargement de la texture "textTabFond" = une case vide de la couleur des autres cases 
    SDL_Surface *tabFond = NULL;
    tabFond = IMG_Load("src/GoFond.png");
    if(tabFond == NULL)
        goto endProg;
    SDL_Texture *textTabFond = NULL;
    textTabFond = SDL_CreateTextureFromSurface(renderer, tabFond);
    if(textTabFond == NULL)
        goto endProg;  
    SDL_FreeSurface(tabFond); 
/*-------------------------------*/             //Chargement de la texture "textGoDown" = une case fin de plateau en bas
    SDL_Surface *tabGoDown = NULL;
    tabGoDown = IMG_Load("src/GoDown.png");
    if(tabGoDown == NULL)
        goto endProg;
    SDL_Texture *textGoDown = NULL;
    textGoDown = SDL_CreateTextureFromSurface(renderer, tabGoDown);
    if(textGoDown == NULL)
        goto endProg;   
    SDL_FreeSurface(tabGoDown);  
/*-------------------------------*/             //Chargement de la texture "textGoDownL" = une case coin de plateau en bas à gauche
    SDL_Surface *tabGoDownL = NULL;
    tabGoDownL = IMG_Load("src/GoDownL.png");
    if(tabGoDownL == NULL)
        goto endProg;
    SDL_Texture *textGoDownL = NULL;
    textGoDownL = SDL_CreateTextureFromSurface(renderer, tabGoDownL);
    if(textGoDownL == NULL)
        goto endProg;
    SDL_FreeSurface(tabGoDownL);
/*-------------------------------*/             //Chargement de la texture "textGoDownR" = une case coin de plateau en bas à droite
    SDL_Surface *tabGoDownR = NULL;
    tabGoDownR = IMG_Load("src/GoDownR.png");
    if(tabGoDownR == NULL)
        goto endProg;
    SDL_Texture *textGoDownR = NULL;
    textGoDownR = SDL_CreateTextureFromSurface(renderer, tabGoDownR);
    if(textGoDown == NULL)
        goto endProg;
    SDL_FreeSurface(tabGoDownR);
/*-------------------------------*/             //Chargement de la texture "textGoL" = une case fin de plateau à gauche
    SDL_Surface *tabGoL = NULL;
    tabGoL = IMG_Load("src/GoL.png");
    if(tabGoL == NULL)
        goto endProg;
    SDL_Texture *textGoL = NULL;
    textGoL = SDL_CreateTextureFromSurface(renderer, tabGoL);
    if(textGoL == NULL)
        goto endProg;
    SDL_FreeSurface(tabGoL);
/*-------------------------------*/             //Chargement de la texture "textGoR" = une case fin de plateau à droite
    SDL_Surface *tabGoR = NULL;
    tabGoR = IMG_Load("src/GoR.png");
    if(tabGoR == NULL)
        goto endProg;
    SDL_Texture *textGoR = NULL;
    textGoR = SDL_CreateTextureFromSurface(renderer, tabGoR);
    if(textGoR == NULL)
        goto endProg;
    SDL_FreeSurface(tabGoR);
/*-------------------------------*/             //Chargement de la texture "textGoUp" = une case fin de plateau en haut
    SDL_Surface *tabGoUp = NULL;
    tabGoUp = IMG_Load("src/GoUp.png");
    if(tabGoUp == NULL)
        goto endProg;
    SDL_Texture *textGoUp = NULL;
    textGoUp = SDL_CreateTextureFromSurface(renderer, tabGoUp);
    if(textGoUp == NULL)
        goto endProg;
    SDL_FreeSurface(tabGoUp);
/*-------------------------------*/             //Chargement de la texture "textGoUpL" = une case coin de plateau en haut à gauche
    SDL_Surface *tabGoUpL = NULL;
    tabGoUpL = IMG_Load("src/GoUpL.png");
    if(tabGoUpL == NULL)
        goto endProg;
    SDL_Texture *textGoUpL = NULL;
    textGoUpL = SDL_CreateTextureFromSurface(renderer, tabGoUpL);
    if(textGoUpL == NULL)
        goto endProg;
    SDL_FreeSurface(tabGoUpL);
/*-------------------------------*/             //Chargement de la texture "textGoUpR" = une case coin de plateau en haut à droite
    SDL_Surface *tabGoUpR = NULL;
    tabGoUpR = IMG_Load("src/GoUpR.png");
    if(tabGoUpR == NULL)
        goto endProg;
    SDL_Texture *textGoUpR = NULL;
    textGoUpR = SDL_CreateTextureFromSurface(renderer, tabGoUpR);
    if(textGoUpR == NULL)
        goto endProg;
    SDL_FreeSurface(tabGoUpR);
/*-------------------------------*/             //Chargement de la texture "textBlanc" = une pierre blanche
    SDL_Surface *pierreBlanc = NULL;
    pierreBlanc = IMG_Load("src/PierreGoBlanc.png");
    if(pierreBlanc == NULL)
        goto endProg;
    SDL_Texture *textBlanc = NULL;
    textBlanc = SDL_CreateTextureFromSurface(renderer, pierreBlanc);
    if(textBlanc == NULL)
        goto endProg;
    SDL_FreeSurface(pierreBlanc);
/*-------------------------------*/             //Chargement de la texture "textNoir" = une pierre noire
    SDL_Surface *pierreNoir = NULL;
    pierreNoir = IMG_Load("src/PierreGoNoir.png");
    if(pierreNoir == NULL)
        goto endProg;
    SDL_Texture *textNoir = NULL;
    textNoir = SDL_CreateTextureFromSurface(renderer, pierreNoir);
    if(textNoir == NULL)
        goto endProg;
    SDL_FreeSurface(pierreNoir);
/*-------------------------------*/

    SDL_Texture *texteNoir = NULL;      //Texture des textes
    SDL_Texture *texteBlanc = NULL;
    SDL_Texture *texteGO = NULL;

/*------------------------------------------------------------------------------------------------------------------------*/  
    
    t = calloc(dimension*dimension, sizeof(int));           //On alloue la mémoire du tableau de jeu selon la dimension choisie

    while (programLaunched)                                 //Boucle du programme principal : le jeu 
    {   
        SDL_RenderClear(renderer);                          //On reset le rendu 

/*-------------------------------*/             //Affichage de la texture "textGoUpL" = le coin en haut à gauche
        if(SDL_QueryTexture(textGoUpL, NULL, NULL, &rectangle.w, &rectangle.h))
            goto endProg;
        rectangle.x = 0;
        rectangle.y = 0;
        if(SDL_RenderCopy(renderer, textGoUpL, NULL, &rectangle))
            goto endProg;

/*-------------------------------*/             //Affichage de la texture "textGoUp" = la fin du plateau en haut
        for (int i = 1; i < dimension-1; i++)
        {
            if(SDL_QueryTexture(textGoUp, NULL, NULL, &rectangle.w, &rectangle.h))
                goto endProg;
            rectangle.x = 36*i;
            rectangle.y = 0;
            if(SDL_RenderCopy(renderer, textGoUp, NULL, &rectangle))
                goto endProg;
        }   

/*-------------------------------*/             //Affichage de la texture "textGoUpR" = le coin en haut à droite
        if(SDL_QueryTexture(textGoUpR, NULL, NULL, &rectangle.w, &rectangle.h))
            goto endProg;
        rectangle.x = 36*(dimension-1);
        rectangle.y = 0;
        if(SDL_RenderCopy(renderer, textGoUpR, NULL, &rectangle))
            goto endProg;


/*-------------------------------*/             //Affichage de chaque ligne du plateau dans l'ordre : fin de plateau à gauche (textGoL), intersection (dimension-2 fois) (textTab) et fin de plateau à droite (textGoR)
        for (int i = 1; i < dimension-1; i++)
        {
            if(SDL_QueryTexture(textGoL, NULL, NULL, &rectangle.w, &rectangle.h))
                goto endProg;
            rectangle.x = 0;
            rectangle.y = 36*i;
            if(SDL_RenderCopy(renderer, textGoL, NULL, &rectangle))
                goto endProg;

            for (int j = 1; j < dimension-1; j++)
            {
                if(SDL_QueryTexture(textTab, NULL, NULL, &rectangle.w, &rectangle.h))
                    goto endProg;
                rectangle.x = 36*j;
                rectangle.y = 36*i;
                if(SDL_RenderCopy(renderer, textTab, NULL, &rectangle))
                    goto endProg;
            }   

            if(SDL_QueryTexture(textGoR, NULL, NULL, &rectangle.w, &rectangle.h))
                goto endProg;
            rectangle.x = 36*(dimension-1);
            rectangle.y = 36*i;
            if(SDL_RenderCopy(renderer, textGoR, NULL, &rectangle))
                goto endProg;
        }

/*-------------------------------*/             //Affichage de la texture "textGoDownL" = le coin en bas à gauche
        if(SDL_QueryTexture(textGoDownL, NULL, NULL, &rectangle.w, &rectangle.h))
            goto endProg;
        rectangle.x = 0;
        rectangle.y = 36*(dimension-1);
        if(SDL_RenderCopy(renderer, textGoDownL, NULL, &rectangle))
            goto endProg;

/*-------------------------------*/             //Affichage de la texture "textGoDown" = la fin du plateau en bas
        for (int i = 1; i < dimension-1; i++)
        {
            if(SDL_QueryTexture(textGoDown, NULL, NULL, &rectangle.w, &rectangle.h))
                goto endProg;
            rectangle.x = 36*i;
            rectangle.y = 36*(dimension-1);
            if(SDL_RenderCopy(renderer, textGoDown, NULL, &rectangle))
                goto endProg;
        }   

/*-------------------------------*/             //Affichage de la texture "textGoDownR" = le coin en bas à droite
        if(SDL_QueryTexture(textGoDownR, NULL, NULL, &rectangle.w, &rectangle.h))
            goto endProg;
        rectangle.x = 36*(dimension-1);
        rectangle.y = 36*(dimension-1);
        if(SDL_RenderCopy(renderer, textGoDownR, NULL, &rectangle))
            goto endProg;

/*-------------------------------*/             //Affichage de la texture "textTabFond" = des cases vides pour mettre le pseudo et le score de joueur 1
        for (int i = 0; i < dimension; i++)
        {
            if(SDL_QueryTexture(textTabFond, NULL, NULL, &rectangle.w, &rectangle.h))
                goto endProg;
            rectangle.x = 36*i;
            rectangle.y = 36*dimension;
            if(SDL_RenderCopy(renderer, textTabFond, NULL, &rectangle))
                goto endProg;
        } 
/*-------------------------------*/             //Affichage de la texture "textTabFond" = des cases vides pour mettre le pseudo et le score de joueur 2
        for (int i = 0; i < dimension; i++)
        {
            if(SDL_QueryTexture(textTabFond, NULL, NULL, &rectangle.w, &rectangle.h))
                goto endProg;
            rectangle.x = 36*i;
            rectangle.y = 36*(dimension+1);
            if(SDL_RenderCopy(renderer, textTabFond, NULL, &rectangle))
                goto endProg;
        } 
        
        while(SDL_PollEvent(&event)) 
        {
            switch(event.type)
            {
                case SDL_MOUSEBUTTONDOWN :                                                                  //Si on clique
                    clicMouse = (int)(floor(event.motion.x/36) + dimension * floor(event.motion.y/36));    //On sauvegarde la case sur laquelle on clique (chaque case faisant 36 pixel, on divise la coordonnée x par 36, on arrondi et on obtient le numéro de ligne, puis on fait la même chose avec la coordonnée y : on peut alors faire x + dimension * y = la case cliqué)
                    if (turn == BLANC && t[clicMouse] == 0 && clicMouse != lastBlanc && !(t[clicMouse-1]==NOIR && t[clicMouse+1]==NOIR && t[clicMouse+dimension]==NOIR && t[clicMouse-dimension]==NOIR))                                         //Si c'est le tour des blancs, que la case cliqué est vide et que ce n'est pas le même coup que le tour d'avant
                    {
                        if(isAmbient == 1)
                            Mix_PlayMusic(posePierre1, 1);          //On met le bruitage de la pierre blanche
                        passB = 0;                                                                      //On mets la variable passB à 0, en effet le joueur Blanc vient de jouer, il ne veut donc pas passer son tour
                        t[clicMouse] = BLANC;                                                           //On rempli la case avec une pierre blanche
                        lastBlanc = clicMouse;                                                          //On sauvegarde la case cliqué
                        if (clicMouse % dimension != dimension - 1 && t[clicMouse + 1] == NOIR)         //Si la case cliqué n'est pas au bord à droite et que la case à droite contient une pierre noir
                        {
                            if(calculTest(t,  BLANC, clicMouse + 1, dimension))            //On calcul si la pierre noire à droite est entourée, si oui la fonction retourne 1, sinon 0 
                                scoreBlanc = transformTab(t, dimension, 2, scoreBlanc, isAmbient, mc_hurt);    //On transforme le tableau pour supprimer la(es) pierre(s) entourée(s) et pour mettre à jour le score
                            else
                                transformTab(t, dimension, 0, 0, isAmbient, mc_hurt);                          //Si la fonction retourne 0, il faut nettoyer les cases qu'on a vérifié
                        }
                        if (clicMouse <= dimension * (dimension-1) && t[clicMouse + dimension] == NOIR) //Si la case cliqué n'est pas au bord en bas et que la case en bas contient une pierre noir
                        {
                            if(calculTest(t, BLANC, clicMouse + dimension, dimension))
                                scoreBlanc = transformTab(t, dimension, 2, scoreBlanc, isAmbient, mc_hurt);
                            else
                                transformTab(t, dimension, 0, 0, isAmbient, mc_hurt);
                        }
                        if(clicMouse % dimension != 0 && t[clicMouse - 1] == NOIR)                      //Si la case cliqué n'est pas au bord à gauche et que la case à gauche contient une pierre noir
                        {
                            if(calculTest(t, BLANC, clicMouse - 1, dimension))
                                scoreBlanc = transformTab(t, dimension, 2, scoreBlanc, isAmbient, mc_hurt);
                            else
                                transformTab(t, dimension, 0, 0, isAmbient, mc_hurt);
                        }
                        if(clicMouse > dimension - 1 && t[clicMouse - dimension] == NOIR)               //Si la case cliqué n'est pas au bord en haut et que la case en haut contient une pierre noir
                        {
                            if(calculTest(t, BLANC, clicMouse - dimension, dimension))
                                scoreBlanc = transformTab(t, dimension, 2, scoreBlanc, isAmbient, mc_hurt);
                            else
                                transformTab(t, dimension, 0, 0, isAmbient, mc_hurt);
                        }
                        turn *= -1;     //On change de tour
                        break;
                    }
                    if (turn == NOIR && t[clicMouse] == 0 && clicMouse != lastNoir && !(t[clicMouse-1]==BLANC && t[clicMouse+1]==BLANC && t[clicMouse+dimension]==BLANC && t[clicMouse-dimension]==BLANC)){                  //Si c'est le tour des noirs, que la case cliqué est vide et que ce n'est pas le même coup que le tour d'avant
                        if(isAmbient == 1)
                            Mix_PlayMusic(posePierre2, 1);          //On met le bruitage de la pierre noire
                        passN = 0;                                                                      //On mets la variable passB à 0, en effet le joueur Blanc vient de jouer, il ne veut donc pas passer son tour
                        t[clicMouse] = NOIR;                                                            //On rempli la case avec une pierre blanche
                        lastNoir = clicMouse;                                                           //On sauvegarde la case cliqué
                        if ((clicMouse % dimension != dimension - 1) && t[clicMouse + 1] == BLANC)      //Si la case cliqué n'est pas au bord à droite et que la case à droite contient une pierre blanche
                        {
                            if(calculTest(t,  NOIR, clicMouse + 1, dimension))
                                scoreNoir = transformTab(t, dimension, 1, scoreNoir, isAmbient, mc_hurt);
                            else
                                transformTab(t, dimension, 0, 0, isAmbient, mc_hurt);
                        }
                        if (clicMouse <= dimension * (dimension-1) && t[clicMouse + dimension] == BLANC)//Si la case cliqué n'est pas au bord en bas et que la case en bas contient une pierre blanche
                        {
                            if(calculTest(t, NOIR, clicMouse + dimension, dimension))
                                scoreNoir = transformTab(t, dimension, 1, scoreNoir, isAmbient, mc_hurt);
                            else
                                transformTab(t, dimension, 0, 0, isAmbient, mc_hurt);
                        }
                        if(clicMouse % dimension != 0 && t[clicMouse - 1] == BLANC)                     //Si la case cliqué n'est pas au bord à gauche et que la case à gauche contient une pierre blanche
                        {
                            if(calculTest(t, NOIR, clicMouse - 1, dimension))
                                scoreNoir = transformTab(t, dimension, 1, scoreNoir, isAmbient, mc_hurt);
                            else
                                transformTab(t, dimension, 0, 0, isAmbient, mc_hurt);
                        }
                        if(clicMouse > dimension - 1 && t[clicMouse - dimension] == BLANC)              //Si la case cliqué n'est pas au bord en haut et que la case en haut contient une pierre blanche
                        {
                            if(calculTest(t, NOIR, clicMouse - dimension, dimension))
                                scoreNoir = transformTab(t, dimension, 1, scoreNoir, isAmbient, mc_hurt);
                            else
                                transformTab(t, dimension, 0, 0, isAmbient, mc_hurt);
                        }
                        turn *= -1;         //On change de tour
                        break;
                    }
                    else
                        break;

                case SDL_KEYDOWN :
                    if(event.key.keysym.sym == SDLK_p && turn == BLANC){
                        passB = 1;
                        turn*=-1;
                        break;
                    }
                    if(event.key.keysym.sym == SDLK_p && turn == NOIR){
                        passN = 1;
                        turn*=-1;
                        break;
                    }
                    break;


                case SDL_QUIT :                                 //Si on quitte la fenêtre, on ferme le programme
                    programLaunched = SDL_FALSE;
                    endGameLaunched = SDL_FALSE;
                    break;

                default :
                    break;
            }
        }
        for (int i = 0; i < dimension; i++)                 //On parcourt le tableau  
        {
            for (int j = 0; j < dimension; j++)
            {
                if(t[dimension*i + j] == BLANC){           //Si il y a une pierre blanche dans le tableau, on l'affiche avec ses coordonnées
                    if(SDL_QueryTexture(textBlanc, NULL, NULL, &rectangle.w, &rectangle.h))
                        goto endProg;
                    rectangle.x = j * 36 + 1;
                    rectangle.y = i * 36 + 1;
                    if(SDL_RenderCopy(renderer, textBlanc, NULL, &rectangle))
                        goto endProg;
                }
                else if(t[dimension*i + j] == NOIR){       //Si il y a une pierre noir dans le tableau, on l'affiche avec ses coordonnées
                    if(SDL_QueryTexture(textNoir, NULL, NULL, &rectangle.w, &rectangle.h))
                        goto endProg;
                    rectangle.x = j * 36 + 1;
                    rectangle.y = i * 36 + 1;
                    if(SDL_RenderCopy(renderer, textNoir, NULL, &rectangle))
                        goto endProg;
                }
            }
        }

        sprintf(scoreNoirC,"%s : %i", nameJ1, scoreNoir);  //On stock dans la variable "scoreNoirC" le nom du joueur et son score sous la forme "Nom : score"
        if (passN == 1)                                    //Si le joueur a passé
            sprintf(scoreNoirC, "%s (P)", scoreNoirC);     //on affiche (P) derrière son pseudo

/*-------------------------------*/             //Chargement et affichage de la texture "texteNoir" = le nom et le score du joueur 1
        texte = TTF_RenderText_Blended(police, scoreNoirC, noir);       
        if(texte == NULL)
            printf("%s", TTF_GetError());
        texteNoir = SDL_CreateTextureFromSurface(renderer, texte);
        SDL_FreeSurface(texte);
        if(texteNoir == NULL)
            goto endProg;
        rectangle.w = 36*dimension - 10;
        rectangle.h = 35-countLetters(scoreNoirC);
        rectangle.x = 5;
        rectangle.y = 36*dimension;
        if(SDL_RenderCopy(renderer, texteNoir, NULL, &rectangle)!=0)
            goto endProg;
        SDL_DestroyTexture(texteNoir);

        sprintf(scoreBlancC,"%s : %i", nameJ2, scoreBlanc);
        if (passB == 1)                                    //Si le joueur a passé
            sprintf(scoreBlancC, "%s (P)", scoreBlancC);     //on affiche (P) derrière son pseudo
        
/*-------------------------------*/             //Chargement et affichage de la texture "texteNoir" = le nom et le score du joueur 2
        texte = TTF_RenderText_Blended(police, scoreBlancC, noir);
        if(texte == NULL)
            printf("%s", TTF_GetError());
        texteBlanc = SDL_CreateTextureFromSurface(renderer, texte);
        SDL_FreeSurface(texte);
        if(texteBlanc == NULL)
            goto endProg;
        rectangle.h = 35-countLetters(scoreBlancC);
        rectangle.x = 5;
        rectangle.y = 36*(dimension+1);
        if(SDL_RenderCopy(renderer, texteBlanc, NULL, &rectangle)!=0)
            goto endProg;
        SDL_DestroyTexture(texteBlanc);


        sprintf(scoreBlancC, " ");  //On reset l'affichage des scores
        sprintf(scoreNoirC, " ");
        
        if(passB==1 && passN==1)
            programLaunched = SDL_FALSE;

        SDL_RenderPresent(renderer);    //On actualise le rendu
    }

/*------------------------------------------------------------------------------------------------------------------------*/ 

    while(endGameLaunched)
    {
        SDL_RenderClear(renderer);
        for (int i = 0; i < dimension+2; i++)
        {
            for (int j = 0; j < dimension; j++)
            {
                if(SDL_QueryTexture(textTabFond, NULL, NULL, &rectangle.w, &rectangle.h))
                    goto endProg;
                rectangle.x = 36*j;
                rectangle.y = 36*i;
                if(SDL_RenderCopy(renderer, textTabFond, NULL, &rectangle))
                    goto endProg;
            }
        }
        if(scoreBlanc < scoreNoir)
            sprintf(gameOver, "%s est vainqueur !", nameJ1);
        else if(scoreBlanc > scoreNoir)
            sprintf(gameOver, "%s est vainqueur !", nameJ2);
        else if(scoreBlanc == scoreNoir)
            sprintf(gameOver, "Match nul...");

        texte = TTF_RenderText_Blended(police, gameOver, noir);
        if(texte == NULL)
            printf("%s", TTF_GetError());
        texteGO = SDL_CreateTextureFromSurface(renderer, texte);
        SDL_FreeSurface(texte);
        if(texteGO == NULL)
            goto endProg;
        rectangle.w = 36*dimension - 10;
        rectangle.h = 35-countLetters(gameOver);
        rectangle.x = 5;
        rectangle.y = 36*(dimension/2);
        if(SDL_RenderCopy(renderer, texteGO, NULL, &rectangle)!=0)
            goto endProg;
        SDL_DestroyTexture(texteGO);

        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT :                     //Dans le cas d'une fermeture de la fenêtre
                    endGameLaunched = SDL_FALSE;    //on ferme l'annonce du gagnant
                    break;
                
                case SDL_MOUSEBUTTONDOWN :
                    endGameLaunched = SDL_FALSE;
                    break;

                default :
                    break;
            }
        }
        SDL_RenderPresent(renderer);
    }


/*------------------------------------------------------------*/        //On ferme toutes les variables la SDL
endProg:        //On met la balise de fin de programme en cas de bug 
    SDL_FreeSurface(iconSurface);
    SDL_DestroyTexture(textTab);
    SDL_DestroyTexture(textTabFond);
    SDL_DestroyTexture(textGoUp);
    SDL_DestroyTexture(textGoR);
    SDL_DestroyTexture(textGoDown);
    SDL_DestroyTexture(textGoL);
    SDL_DestroyTexture(textGoDownL);
    SDL_DestroyTexture(textGoUpL);
    SDL_DestroyTexture(textGoDownR);
    SDL_DestroyTexture(textGoUpR);
    SDL_DestroyTexture(textBlanc);
    SDL_DestroyTexture(textNoir);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_FreeMusic(mc_hurt);
    Mix_FreeMusic(posePierre1);
    Mix_FreeMusic(posePierre2);
    TTF_Quit();
    Mix_CloseAudio();
    SDL_Quit();
    
    free(t);
    free(temp);

    return EXIT_SUCCESS;
}