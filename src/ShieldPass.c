#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void generate_password(char *password, int length);


const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;

int main(int argc, char* argv[]) {
    srand(time(NULL));
    int password_length = 12;
    char password[128];
    char fullText[256];

    Uint32 clickTime = 0;
    const int clickFlashDuration = 200; // en millisecondes

    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window* window = SDL_CreateWindow("ShieldPass",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT,
                                          SDL_WINDOW_SHOWN);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Définir l'icône
    SDL_Surface* icon = SDL_LoadBMP("eyes.bmp");
    if (icon != NULL) {
        SDL_SetWindowIcon(window, icon);
        SDL_FreeSurface(icon);
    }

    // Charger la police
    TTF_Font* font = TTF_OpenFont("fonts/OpenSans-Regular.ttf", 24);

    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Rect buttonRect = {200, 320, 240, 60};

    bool showText = false;

    bool showCopiedMessage = false;
    Uint32 copiedMessageTime = 0;
    const int copiedMessageDuration = 2000; // 2 secondes

    bool running = true;
    SDL_Event event;

    int sliderX = 50, sliderY = 100, sliderWidth = 500, sliderHeight = 20;
    int sliderPos = (password_length - 6) * (sliderWidth / 14); // Initialiser la position du curseur à 12 (password_length)
    bool dragging = false;

    int minLength = 6, maxLength = 20;   // Longueur du mot de passe entre 6 et 20 caractères

    while (running) {
        SDL_Surface* cometSurface = NULL;
        SDL_Texture* cometTexture = NULL;
        SDL_Rect cometRect;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int x = event.button.x;
                int y = event.button.y;

                // Vérifier si l'utilisateur a cliqué sur le curseur du slider
                if (x >= sliderX + sliderPos - 5 && x <= sliderX + sliderPos + 5 && y >= sliderY && y <= sliderY + sliderHeight) {
                    dragging = true;  // Lancer le drag
                }

                // Si le bouton "Generate" est cliqué, générer le mot de passe
                if (x >= buttonRect.x && x <= buttonRect.x + buttonRect.w &&
                    y >= buttonRect.y && y <= buttonRect.y + buttonRect.h) {
                    generate_password(password, password_length);
                    SDL_SetClipboardText(password);
                    showText = true;
                    showCopiedMessage = true;
                    copiedMessageTime = SDL_GetTicks();
                    clickTime = SDL_GetTicks(); 
                }
            } else if (event.type == SDL_MOUSEBUTTONUP) {
                dragging = false;  // Lâcher le curseur
            } else if (event.type == SDL_MOUSEMOTION && dragging) {
                int x = event.motion.x;
                // Calculer la nouvelle position du curseur en fonction du déplacement de la souris
                sliderPos = x - sliderX;
                if (sliderPos < 0) sliderPos = 0;
                if (sliderPos > sliderWidth) sliderPos = sliderWidth;
                password_length = minLength + (sliderPos * (maxLength - minLength)) / sliderWidth; // Met à jour la longueur
            }
        }

        // Effacer l'écran
        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255); // Fond gris
        SDL_RenderClear(renderer);

        // Choisir la couleur du bouton selon le clic
        Uint32 now = SDL_GetTicks();
        if (now - clickTime < clickFlashDuration) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 220, 255); // Bleu flash
        } else {
            SDL_SetRenderDrawColor(renderer, 0, 128, 255, 255); // Bleu normal
        }
        SDL_RenderFillRect(renderer, &buttonRect);

        // Texte du bouton "Generate"
        SDL_Surface* buttonTextSurface = TTF_RenderText_Solid(font, "Generate", textColor);
        SDL_Texture* buttonTextTexture = SDL_CreateTextureFromSurface(renderer, buttonTextSurface);

        SDL_Rect textRect;
        textRect.x = buttonRect.x + (buttonRect.w - buttonTextSurface->w) / 2;
        textRect.y = buttonRect.y + (buttonRect.h - buttonTextSurface->h) / 2;
        textRect.w = buttonTextSurface->w;
        textRect.h = buttonTextSurface->h;

        SDL_RenderCopy(renderer, buttonTextTexture, NULL, &textRect);

        SDL_FreeSurface(buttonTextSurface);
        SDL_DestroyTexture(buttonTextTexture);

        // Dessiner le slider (fond)
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &(SDL_Rect){sliderX, sliderY, sliderWidth, sliderHeight});  // Fond du slider

        // Dessiner le curseur du slider
        SDL_SetRenderDrawColor(renderer, 0, 128, 255, 255);  // Couleur du curseur (bleu)
        SDL_RenderFillRect(renderer, &(SDL_Rect){sliderX + sliderPos - 5, sliderY - 5, 10, sliderHeight + 10}); // Curseur

        // Afficher la longueur actuelle du mot de passe
        char lengthText[50];
        sprintf(lengthText, "Password Length: %d", password_length);
        SDL_Surface* lengthSurface = TTF_RenderText_Solid(font, lengthText, textColor);
        SDL_Texture* lengthTexture = SDL_CreateTextureFromSurface(renderer, lengthSurface);

        SDL_Rect lengthRect = {200, 50, lengthSurface->w, lengthSurface->h};
        SDL_RenderCopy(renderer, lengthTexture, NULL, &lengthRect);

        SDL_FreeSurface(lengthSurface);
        SDL_DestroyTexture(lengthTexture);

        // Si cliqué, afficher le mot de passe
        if (showText) {
            sprintf(fullText, "Password: %s", password);
            SDL_Surface* surfaceMessage = TTF_RenderText_Solid(font, fullText, textColor);
            SDL_Texture* message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
            SDL_Rect messageRect = {175, 210, surfaceMessage->w, surfaceMessage->h};
            SDL_RenderCopy(renderer, message, NULL, &messageRect);
            SDL_FreeSurface(surfaceMessage);
            SDL_DestroyTexture(message);
        }

        if (showCopiedMessage && (SDL_GetTicks() - copiedMessageTime < copiedMessageDuration)) {
            SDL_Surface* copySurface = TTF_RenderText_Solid(font, "Copied to clipboard!", textColor);
            SDL_Texture* copyTexture = SDL_CreateTextureFromSurface(renderer, copySurface);
            SDL_Rect copyRect = {220, 390, copySurface->w, copySurface->h}; // Position sous le bouton
            SDL_RenderCopy(renderer, copyTexture, NULL, &copyRect);
            SDL_FreeSurface(copySurface);
            SDL_DestroyTexture(copyTexture);
        }

        SDL_Color cometColor = {0, 128, 255, 255};
        cometSurface = TTF_RenderText_Solid(font, "By Comet", cometColor);
        cometTexture = SDL_CreateTextureFromSurface(renderer, cometSurface);

        cometRect.w = cometSurface->w;
        cometRect.h = cometSurface->h;
        cometRect.x = (WINDOW_WIDTH - cometRect.w) / 2;
        cometRect.y = WINDOW_HEIGHT - cometRect.h - 10;

        SDL_RenderCopy(renderer, cometTexture, NULL, &cometRect);

        SDL_FreeSurface(cometSurface);
        SDL_DestroyTexture(cometTexture);

        SDL_RenderPresent(renderer);
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}

// Fonction pour générer un mot de passe aléatoire
void generate_password(char *password, int length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*";
    int charset_size = sizeof(charset) - 1;

    for (int i = 0; i < length; i++) {
        int index = rand() % charset_size;
        password[i] = charset[index];
    }
    password[length] = '\0';
}
