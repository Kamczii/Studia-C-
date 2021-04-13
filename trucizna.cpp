#pragma warning (disable : 4996)
#include <iostream>
#include <stdio.h>



using namespace std;

enum colors_t { GREEN, BLUE, RED, VIOLET, YELLOW, WHITE, BLACK };
const char* const COLORS[] = { "green", "blue", "red", "violet", "yellow", "white", "black" };


#define COLORS_NR 7
#define BUFFER_SIZE 1024
#define NEGATIVE -1
#define SAVE_FILE "game_state.txt"
#define DECK_FILE "deck.txt"


struct card {
    int value;
    int color;
};

struct deck_el {
    card* card;
    deck_el* next;
};

struct player {
    int player_nr;
    deck_el* player_hand_head;
    deck_el* player_deck_head;
};



//Pobiera wartości początkowe od użytkownika
void readStartingValues(int* players_nr, int* cauldrons, int* greens, int* greens_v, int* others, int* explosion_threshold, int*& others_v) {

    cin >> *players_nr;
    cin >> *cauldrons;
    cin >> *greens;
    cin >> *greens_v;
    cin >> *others;
    cin >> *explosion_threshold;

    others_v = new int[*others];

    for (int i = 0;i < *others;i++) {
        cin >> others_v[i];
    }
}


//Tworzy nowego gracza o numerze player_nr i zwraca wskaźnik do niego
player* createPlayer(int player_nr) {

    player* newPlayer = (player*)malloc(sizeof(player));

    if (newPlayer != NULL) {
        newPlayer->player_nr = player_nr;
        newPlayer->player_hand_head = NULL;
        newPlayer->player_deck_head = NULL;
    }

    return newPlayer;
}


//Funkcja przyjmuje kolor postaci tekstu i zwraca jego index w tablicy COLORS, jeśli go nie znajdzie zwraca NEGATIVE
int getColorId(char* value) {

    int i = 0;

    while (i<COLORS_NR) {
        if (strcmp(COLORS[i], value) == 0) {
            return i;
        }
        i++;
    }

    return NEGATIVE;
}


//Każdy nowy element staje się głową listy. Element dodany jako pierwszy jest na samym dole i wskazuje na wartośc NULL
void addCardToDeck(deck_el** head_ptr, card* card_ptr) {

    deck_el* el = (deck_el*)malloc(sizeof(deck_el));

    el->card = card_ptr;
    el->next = (*head_ptr); 

    (*head_ptr) = el; 
}



//Sumuje wartości kart w talii
int sumDeckValues(deck_el* deck) {

    int sum = 0;

    while (deck != NULL) {
        sum += deck->card->value;
        deck = deck->next;
    }

    return sum;
}

//Wyświetla talię kart od góry do dołu: wartość kolor
void printDeck(deck_el* head) {

    while (head != NULL) {
        cout << head->card->value << " " << COLORS[head->card->color] << "\t";
        head = head->next;
    }

}


//Odwraca talię
void reverseDeck(deck_el** head_ptr) {

    deck_el* previous = NULL;
    deck_el* current = *head_ptr;
    deck_el* next = NULL;

    while (current != NULL) {
        next = current->next;

        current->next = previous;

        previous = current;
        current = next;
    }

    *head_ptr = previous;
}


//Funkcja zwraca kartę z góry talii i usuwą ją z niej
card* getCardFromCardDeck(deck_el** head_ptr) {

    if ((*head_ptr) != NULL) {
        deck_el* temp = (*head_ptr)->next;
        card* card_ptr = (*head_ptr)->card;
        free((*head_ptr));
        (*head_ptr) = temp;
        return card_ptr;
    }

    return NULL;
}


//Zwraca index karty o najmniejszej wartości w talii, jeśli takiej nie ma to zwraca NEGATIVE
int getIndexOfWeakestCardInDeck(deck_el* head_ptr) {

    int weakest_index = NEGATIVE;
    int weakest_value;

    if (head_ptr != NULL) {

        weakest_index = 0;
        weakest_value = head_ptr->card->value;

        head_ptr = head_ptr->next;
        int index = 1;

        while (head_ptr != NULL) {

            if (head_ptr->card->value < weakest_value) {
                weakest_index = index;
                weakest_value = head_ptr->card->value;
            }

            head_ptr = head_ptr->next;
            index++;
        }

    }

    return weakest_index;
}

//Zwraca index karty o najmniejszej wartości i wskazanym kolorze w talii, jeśli takiej nie ma to zwraca NEGATIVE
int getIndexOfWeakestCardInDeckWithColor(deck_el* head_ptr, int color) {

    int weakest_index = NEGATIVE;
    int weakest_value;

    if (head_ptr != NULL) {

        weakest_index = 0;
        weakest_value = head_ptr->card->value;

        head_ptr = head_ptr->next;
        int index = 1;

        while (head_ptr != NULL) {

            if (head_ptr->card->value < weakest_value && head_ptr->card->color == color) {
                weakest_index = index;
                weakest_value = head_ptr->card->value;
            }

            head_ptr = head_ptr->next;
            index++;
        }

    }

    return weakest_index;
}


//Zwraca index kociołka z najmniejszą sumą wartości
int getIndexOfCauldronWithLowestSum(deck_el** cauldrons, int cauldrons_count) {

    int lowest_index = 0;
    int lowest_sum = INT_MAX;

    for (int i = 0;i < cauldrons_count;i++) {
        int sum = sumDeckValues(cauldrons[i]);
        if (sum < lowest_sum) {
            lowest_sum = sum;
            lowest_index = i;
        }
    }

    return lowest_index;
}


//Tworzy kartę i zwraca wskaźnik
card* createCard(int color, int value) {

    card* newCard = (card*)malloc(sizeof(card));

    if (newCard != NULL) {
        newCard->color = color;
        newCard->value = value;
    }

    return newCard;
}


//Zwalnia pamięć talii, jeśli with_cards to zwalnia też pamięć kart w niej zawartych
void freeDeck(deck_el*& head, bool with_cards) {

    deck_el* tmp;

    while (head != NULL) {
        tmp = head;

        if(with_cards)
            free(tmp->card);

        head = head->next;
        free(tmp);
    }

    tmp = NULL;
    head = NULL;
}


//Wyświetla aktywnego gracza, ilość graczy,  próg wybuchu oraz karty w ręce i zakryte przed sobą wszystkich graczy
void printPlayers(player** players_ptr, int players_count, int active_player, int explosion_threshold) {

    cout << "active player = "<< active_player <<"\n";
    cout << "players number = " << players_count << "\n";
    cout << "explosion threshold = " << explosion_threshold << "\n";

    for (int i = 0;i < players_count;i++) {


        cout << i + 1 << " player hand cards:\t";
        printDeck(players_ptr[i]->player_hand_head);
        cout << "\n";



        cout << i + 1 << " player deck cards:\t";
        printDeck(players_ptr[i]->player_deck_head);
        cout << "\n";
    }
}


//Wyświetla ile każdy gracz ma kart w ręce i przed sobą
void printPlayersCardsCount(player** players_ptr, int players_count) {

    for (int i = 0;i < players_count;i++) {

        int counter = 0;

        deck_el* head = players_ptr[i]->player_hand_head;

        while (head != NULL) {
            head = head->next;
            counter++;
        }

        cout << players_ptr[i]->player_nr << " player has " << counter << " cards on hand\n";


        counter = 0;

        head = players_ptr[i]->player_deck_head;

        while (head != NULL) {
            head = head ->next;
            counter++;
        }

        cout << players_ptr[i]->player_nr << " player has "<< counter <<" cards in front of him\n";
    }
}


//Jeśli znajdzie kartę o kolorze color w source_head to doda wskaźnik do destination_head
void findCardsWithColor(deck_el* source_head, deck_el*& destination_head, int color) {

    while (source_head != NULL) {

        if (source_head->card->color == color) {
            card* card_ptr = source_head->card;
            addCardToDeck(&destination_head, card_ptr);
        }

        source_head = source_head->next;

    }
}

//Zwraca ilość wystąpień karty o kolorze color w talii source_head
int countCardWithColor(deck_el* source_head, int color) {

    int counter = 0;

    while (source_head != NULL) {

        if (source_head->card->color == color) {
            counter++;
        }

        source_head = source_head->next;

    }

    return counter;
}

//Wyświetla ile jest kart zielonych, ich brak lub informację o różnicy wartości
void printGreenCardsCount(player** players_ptr, deck_el** cauldrons_array, int players_count, int cauldrons_count) {

    deck_el* green_cards_head = NULL;

    //sprawdzanie kart graczy
    for (int i = 0;i < players_count;i++) {

        findCardsWithColor(players_ptr[i]->player_hand_head, green_cards_head, GREEN);

        findCardsWithColor(players_ptr[i]->player_deck_head, green_cards_head, GREEN);

    }

    //sprawdzanie kart w każdym kociołku
    for (int i = 0;i < cauldrons_count;i++) {

        findCardsWithColor(cauldrons_array[i], green_cards_head, GREEN);

    }

    //tworzy kopię, aby iterować po liście
    deck_el* green_cards_head_tmp = green_cards_head;

    if (green_cards_head_tmp != NULL) {

        int green_v = green_cards_head_tmp->card->value;
        int counter = 0;

        while (green_cards_head_tmp != NULL) {

            if (green_cards_head_tmp->card->value != green_v) {
                cout << "Different green cards values occurred\n";
                freeDeck(green_cards_head,false);
                return;
            }

            counter++;
            green_cards_head_tmp = green_cards_head_tmp->next;
        }

        cout << "Found "<< counter <<" green cards, all with "<< green_v  << " value\n";

        freeDeck(green_cards_head, false);
        return;
    }
    else {
        cout << "Green cards does not exist\n";
    }
    freeDeck(green_cards_head, false);
}


//Wyświetla ile jest kart innego koloru niż zielony
void printOtherCardsCount(player** players_ptr, deck_el** cauldrons_array, int players_count, int cauldrons_count) {

    deck_el** other_cards_head = (deck_el**)malloc((COLORS_NR-1)*sizeof(deck_el*));

    if (other_cards_head != NULL)
    {
        for (int n = 0;n < COLORS_NR - 1;n++) {
            other_cards_head[n] = NULL;
        }

        for (int n = 0;n < COLORS_NR - 1;n++) {

            //sprawdzanie kart graczy
            for (int i = 0;i < players_count;i++) {

                findCardsWithColor(players_ptr[i]->player_hand_head, other_cards_head[n], n + 1); //n+1 bo nie liczymy kart zielonych

                findCardsWithColor(players_ptr[i]->player_deck_head, other_cards_head[n], n + 1);

            }

            //sprawdzanie kart w każdym kociołku
            for (int i = 0;i < cauldrons_count;i++) {

                findCardsWithColor(cauldrons_array[i], other_cards_head[n], n + 1);

            }
        }


        int count = countCardWithColor(other_cards_head[0], 1);

        for (int n = 1;n < COLORS_NR - 1;n++) {
            if (other_cards_head[n] != NULL && count != NEGATIVE) {
                if (countCardWithColor(other_cards_head[n], n + 1) != count)
                    count = NEGATIVE;
            }
        }

        if (count == NEGATIVE) {

            cout << "At least two colors with a different number of cards were found: \n";

            for (int n = 0;n < COLORS_NR - 1;n++) {
                if (other_cards_head[n] != NULL) {

                    cout << COLORS[n] << " cards are " << countCardWithColor(other_cards_head[n], n + 1) << "\n";

                }
            }
        }
        else {
            cout << "The number cards of all colors is equal: " << count << "\n";
        }

        for (int i = 0;i < COLORS_NR - 1;i++) {
            freeDeck(other_cards_head[i], false);
        }
        free(other_cards_head);
    }
    else {
        free(other_cards_head);
    }

    
}


//Sortuje talię przez pole wartość
void sortDeckByValue(deck_el* head) {

    deck_el* head_tmp = head;


    if (head != NULL)
    {
        bool change = true;

        while (change) {

            change = false;

            while (head->next != NULL) {
                card* nextCard = head->next->card;
                card* currentCard = head->card;
                if (nextCard->value < currentCard->value) {
                    head->next->card = currentCard;
                    head->card = nextCard;
                    change = true;
                }
                head = head->next;

            }

            head = head_tmp;

        }
    }
}


//Sortuje talię przez pole kolor
void sortDeckByColor(deck_el* head) {
    deck_el* head_tmp = head;


    if (head != NULL)
    {
        bool change = true;

        while (change) {

            change = false;

            while (head->next != NULL) {

                card* nextCard = head->next->card;
                card* currentCard = head->card;

                if (nextCard->color < currentCard->color) {

                    head->next->card = currentCard;
                    head->card = nextCard;
                    change = true;

                }

                head = head->next;

            }

            head = head_tmp;

        }
    }
}

//Zwraca wskaźnik do pozycji w talii o indexie index i usuwa tą kartę z talii
card* getCardByIndex(deck_el*& head, int index) {

    deck_el* tmp = head;


    deck_el* previous = NULL;
    deck_el* current = head;


    if (index == 0)
        return getCardFromCardDeck(&head);

    for (int i = 0;i < index;i++) {

        previous = current;
        current = current->next;

    }

    previous->next = current->next;

    card* index_card = current->card;
    free(current);

    return index_card;
}


//Wyświetla wartości kart w talli w postaci ciągłej linii
void printCardValues(deck_el* head) {

    while (head != NULL) {
        cout << head->card->value << "\t";
        head = head->next;
    }

}


//Zwraca true jeśli karty inne niż zielone mają różne wartości
bool areOtherCardsValuesInvalid(player** players_ptr, deck_el** cauldrons_array, int players_count, int cauldrons_count) {

    int colors_c = COLORS_NR-1;

    deck_el** other_cards_head = (deck_el**)malloc(colors_c * sizeof(deck_el*));

    if (other_cards_head) {

        for (int n = 0;n < colors_c;n++) {
            other_cards_head[n] = NULL;
        }

        for (int n = 0;n < colors_c;n++) {

            //sprawdzanie kart graczy
            for (int i = 0;i < players_count;i++) {

                findCardsWithColor(players_ptr[i]->player_hand_head, other_cards_head[n], n + 1); //n+1 bo nie liczymy kart zielonych

                findCardsWithColor(players_ptr[i]->player_deck_head, other_cards_head[n], n + 1);

            }

            //sprawdzanie kart w każdym kociołku
            for (int i = 0;i < cauldrons_count;i++) {

                findCardsWithColor(cauldrons_array[i], other_cards_head[n], n + 1);

            }
        }


        for (int n = 0;n < colors_c;n++) {
            sortDeckByValue(other_cards_head[n]);
        }


        int n = 1;
        bool diff = false;

        while (!diff && n < colors_c && other_cards_head[n] != NULL) {

            deck_el* prev = other_cards_head[n - 1];
            deck_el* current = other_cards_head[n];

            while (!diff && current != NULL) {

                if (prev->card->value != current->card->value)
                    diff = true;

                prev = prev->next;
                current = current->next;
            }

            n++;
        }


        for (int i = 0;i < colors_c;i++) {
            freeDeck(other_cards_head[i], false);
        }
        free(other_cards_head);

        return diff;
    }
}


//Tworzy karty zgodnie z parametrami i dodaje każdą nową na szczyt talii.
void createDeck(int cauldrons, int greens_c, int greens_v, int others_c, int*& others_v, deck_el** head_ptr) {

    for (int i = 0;i < greens_c;i++) {
        card* new_card = createCard(GREEN, greens_v);
        addCardToDeck(head_ptr, new_card);
    }

    for (int i = 1;i < cauldrons + 1;i++) {

        for (int j = 0; j < others_c;j++) {

            card* new_card = (card*)malloc(sizeof(card));

            if (new_card != NULL) {
                new_card->color = i;
                new_card->value = *(others_v + j);
            }

            addCardToDeck(head_ptr, new_card);
        }

    }
}


//Wypisz zawartość kociołków
void printCauldrons(deck_el** cauldrons, int cauldrons_size) {

    for (int i = 0;i < cauldrons_size;i++) {
        cout << i + 1 << " pile cards: ";
        printDeck(cauldrons[i]);
        cout << "\n";
    }

}

//Czyta karty w jednej linii i dodaje je do talii destination_deck. piece powinien wskazywać na wartość pierwszej karty
void readLineOfCards(char*& piece, deck_el*& destination_deck) {
    while (strcmp("\n",piece)!=0) {

        int value = strtol(piece, &piece, 10); //char -> int

        piece = strtok(NULL, "\t"); // piece to kolor np. green

        int color = getColorId(piece); //zwraca nr koloru

        card* newCard = createCard(color, value); //tworzy nową kartę
        addCardToDeck(&destination_deck, newCard); //dodaje kartę do ręki gracza i+1

        piece = strtok(NULL, " "); //piece = wartość karty

    }
}


//Tworzy graczy i ładuje ich karty
void loadPlayersData(int* players_count, player** players, char* line, FILE* in) {

    for (int i = 0;i < *players_count;i++) {

        players[i] = createPlayer(i + 1);


        //wczytuje karty trzymane w ręce
        fgets(line, BUFFER_SIZE, in);

        if (strtok(line, " ") == NULL) //Omija nr. gracza
            return;

         char* piece = strtok(NULL, "player hand cards:"); //piece = pierwsza wartość karty

         if (strcmp("\t\n", piece) != 0)
            readLineOfCards(piece, players[i]->player_hand_head);



        //wczytuje karty trzymane przed sobą
        fgets(line, BUFFER_SIZE, in);

        if (strtok(line, " ") == NULL) //Omija nr. gracza
            return;

        piece = strtok(NULL, "player deck cards:"); //piece = pierwsza wartość karty
        if(strcmp("\t\n",piece)!=0)
            readLineOfCards(piece, players[i]->player_deck_head);
    }
}


//Ładuje karty w kociołkach
void loadCauldrons(deck_el**& cauldrons, int* cauldrons_count, char* line, FILE* in) {

    fgets(line, BUFFER_SIZE, in);

    char* piece;

    piece = strtok(line, " ");

    int pile_nr = NEGATIVE;

    while (piece!=NULL) {

        pile_nr = atoi(piece);

        if (pile_nr == 1) {

            cauldrons = (deck_el**)malloc(pile_nr * sizeof(deck_el*));

        }
        else {

            deck_el** tmp = (deck_el**)realloc(cauldrons, pile_nr * sizeof(deck_el*));

            if (tmp != NULL) {
                cauldrons = tmp;
            }
        }

        if (cauldrons != NULL) {
            cauldrons[pile_nr - 1] = NULL;
            piece = strtok(NULL, "pile cards:");
            if (strcmp("\t\n", piece) != 0)
                readLineOfCards(piece, cauldrons[pile_nr - 1]);
        }
        else {
            return;
        }

        if (fgets(line, BUFFER_SIZE, in) != NULL)
            piece = strtok(line, " ");
        else
            piece = NULL;
    }

    *cauldrons_count = pile_nr;
}


//wyświetla ilość kart w każdym kociołku
void printCauldronsCardCount(deck_el** cauldrons_array, int cauldrons_count) {


    for (int i = 0;i < cauldrons_count;i++) {

        int counter = 0;

        deck_el* head = cauldrons_array[i];

        while (head != NULL) {

            head = head->next;
            counter++;

        }

        cout << "there are "<< counter << " cards on " << i + 1 << " pile\n";
    }
}




//zwraca długość talii
int getDeckLength(deck_el* head_ptr) {

    int counter = 0;

    while (head_ptr != NULL) {
        counter++;
        head_ptr = head_ptr->next;
    }

    return counter;
}


//Czy karty w rękach graczy mają odpowiednie wartości, tzn. żaden z graczy nie wyrzucił karty dwa razy
bool arePlayersCardsOkay(int active_player, int players_count, player** players) {

    int* hand_c = new int[players_count];

    for (int i = 0;i < players_count; i++) {
        hand_c[i] = getDeckLength(players[i]->player_hand_head);
    }

    //sprawdza czy różnice wartości nie są większe niż 1, tzn. czy gracz nie wystawił 2 kart na raz
    bool err = true;
    for (int i = 1;i < players_count;i++) {
        if (abs(hand_c[i] - hand_c[i - 1]) > 1) {
            err = false;
        }
    }
    if (abs(hand_c[players_count - 1] - hand_c[0])>1) {
        err = false;
    }

    delete[] hand_c;
    return err;
}


//czy kociołek zawiera karty tego samego koloru
bool areCauldronCardsSameColor(deck_el* cauldron_head) {

    int color = NULL;

    while (cauldron_head != NULL) {
        if (color == NULL) {
            color = cauldron_head->card->color;
        }
        else {
            if (cauldron_head->card->color != color || cauldron_head->card->color == GREEN)
                return false;
        }
        cauldron_head = cauldron_head->next;
    }

    return true;
}


//Zwraca index kociołka, który obsługuje dany kolor
int getCauldronWithColor(deck_el** cauldrons, int cauldrons_count, int color) {

    bool toPut = false;

    if (color == GREEN)
        return 0;

    for (int i = 0;i < cauldrons_count;i++) {
        if (!toPut) {
            deck_el* current_cauldron = cauldrons[i];

            while (current_cauldron != NULL && !toPut) {
                toPut = true;
                if (current_cauldron->card->color != color)
                    toPut = false;
                current_cauldron = current_cauldron->next;
            }

            if (toPut)
                return i;
        }
    }
    return NULL;
}

//Wyświetl jeśli w kociołku są dwa różne kolory
void printCauldronsColorsStateIfInvalid(int cauldrons_count, deck_el** cauldrons) {

    for (int i = 0;i < cauldrons_count; i++) {
        if (!areCauldronCardsSameColor(cauldrons[i])) {
            cout << "Two different colors were found on the "<< i+1 <<" pile\n";
        }
    }

}



//True jeśli kociołek przekroczył próg wybuchu
bool isCauldronReachedExplosionThreshold(deck_el* cauldron, int explosion_threshold) {
    return sumDeckValues(cauldron) > explosion_threshold;
}


//Wyświetla informacje, który kociołek powinien już wybuchnąć
void printIfCauldronReachedExplosionThreshold(deck_el** cauldrons, int cauldrons_count, int explosion_threshold) {

    bool err = false;

    for (int i = 0;i < cauldrons_count;i++) {
        if (isCauldronReachedExplosionThreshold(cauldrons[i], explosion_threshold)) {
            cout << "Pile number " << i+1 << " should explode earlier\n";
            err = true;
        }
    }

    if (!err)
        cout << "Current state of the game is ok\n";
}


//Waliduje dane wejściowe, wyświetla odpowiednie komunikaty
void printGameValidation(int active_player, int cauldrons_count, int explosion_threshold, int players_count, player** players, deck_el** cauldrons) {

    if (!arePlayersCardsOkay(active_player, players_count, players)) {
        cout << "The number of players cards on hand is wrong\n";
    }

    printCauldronsColorsStateIfInvalid(cauldrons_count, cauldrons);

    printIfCauldronReachedExplosionThreshold(cauldrons, cauldrons_count, explosion_threshold);
   
}


//Przenosi talię source_head do talii destination_head
void moveDeckToDeck(deck_el*& source_head, deck_el** destination_head) {
    while (source_head != NULL) {
        card* current_card = getCardFromCardDeck(&source_head);
        addCardToDeck(destination_head, current_card);
    }
}


//Funkcja zwraca kartę o najmniejszej wartości z talii i usuwą ją z niej
card* getWeakestCardFromCardDeck(deck_el** head_ptr, int color) {

    int weakest_index;

    if (color == NEGATIVE)
        weakest_index = getIndexOfWeakestCardInDeck(*head_ptr);
    else
        weakest_index = getIndexOfWeakestCardInDeckWithColor(*head_ptr, color);


    card* weakest_card = getCardByIndex(*head_ptr, weakest_index);

    return weakest_card;
}


//Zwraca kolor, który obsługuje dany kociołek, jeśli nie jest przypisany kolor to zwraca NEGATIVE
int getCauldronColor(deck_el* head_ptr) {

    while (head_ptr != NULL) {
        if (head_ptr->card->color != GREEN)
            return head_ptr->card->color;
        head_ptr = head_ptr->next;
    }
     
    return NEGATIVE;
}


//Sprawdza czy istnieje kociołek obsługujący dany kolor, jeśli tak zwraca true, jeśli nie zwraca false
bool existOtherCauldronWithColor(deck_el** cauldrons, int cauldrons_count, int current_cauldron_index, int color) {

    if (color != GREEN) {

        for (int i = 0;i < cauldrons_count;i++) {
            if (i != current_cauldron_index) {
                int current_color = getCauldronColor(cauldrons[i]);

                if (current_color == color)
                    return true; //istnieje już kociołek z tym kolorem, więc zwracamy jego index
            }

        }

    }

    return false;

}


//Prosty ruch polegający na wybraniu kociołka o najmniejszej sumie i dodaniu tam najmniejszej możliwej karty
void simpleMove(int* active_player, int cauldrons_count, int players_count, int explosion_threshold, player** players, deck_el**& cauldrons) {


    player* current_player = players[*active_player - 1];

    if (current_player->player_hand_head != NULL) {


        //card* selected_card = getCardFromCardDeck(&current_player->player_hand_head);
        card* selected_card = getWeakestCardFromCardDeck(&current_player->player_hand_head,NEGATIVE);


        int color = selected_card->color;
        int cauldron_index;
        if (color == GREEN)
            cauldron_index = color;
        else
            cauldron_index = color-1;
            
        addCardToDeck(&cauldrons[cauldron_index], selected_card);


        if (isCauldronReachedExplosionThreshold(cauldrons[cauldron_index], explosion_threshold)) {
            moveDeckToDeck(cauldrons[cauldron_index], &current_player->player_deck_head);
        }

    }


    *active_player = *active_player + 1;

    if (*active_player > players_count) {
        *active_player = 1;
    }
}


//Wyświetl stan rundy w takiej samej postaci jak dane wejściowe
void printRoundState(int active_player, int players_count, int explosion_threshold, int cauldrons_count, player** players, deck_el** cauldrons) {
    printPlayers(players, players_count, active_player, explosion_threshold);
    printCauldrons(cauldrons, cauldrons_count);
}


//Zapisuje talię do pliku w linii ciągłej
void saveDeckToFile(FILE* out, deck_el* deck) {
    while (deck != NULL) {

        fprintf(out, "%d %s\t", deck->card->value, COLORS[deck->card->color]);
        deck = deck->next;
    }
}


//Zapisuje stan rundy do pliku
void saveRoundStateToFile(int active_player, int players_count, int explosion_threshold, int cauldrons_count, player** players, deck_el** cauldrons) {

    FILE* out = fopen(SAVE_FILE,"w");

    fprintf(out, "active player = %d\n", active_player);
    fprintf(out, "players number = %d\n", players_count);
    fprintf(out, "explosion threshold = %d\n", explosion_threshold);

    for (int i = 0;i < players_count;i++) {
        fprintf(out, "%d player hand cards:\t",i+1);
        saveDeckToFile(out, players[i]->player_hand_head);
        fprintf(out, "\n");

        fprintf(out, "%d player deck cards:\t", i + 1);
        saveDeckToFile(out, players[i]->player_deck_head);
        fprintf(out, "\n");
    }
    
    for (int i = 0;i < cauldrons_count;i++) {
        fprintf(out,"%d pile cards:\t",i+1);

        saveDeckToFile(out, cauldrons[i]);
        fprintf(out, "\n");
    }

    fclose(out);

}


//Zwraca najczęściej występujący kolor, jeśli takiego nie ma lub są dwa to zwraca NEGATIVE
int mostCommonColor(deck_el* head) {

    sortDeckByColor(head);

    printDeck(head);
    int most_common_color = NEGATIVE;
    int most_common_count = 1;
    int tmp = 1;

    while (head != NULL && head->next != NULL) {

        if (head->card->color == head->next->card->color)
            tmp++;
        else {
            if (tmp > most_common_count) {
                most_common_count = tmp;
                most_common_color = head->card->color;
            }
            tmp = 1;
        }

        head = head->next;
    }

    return most_common_color;
}

//Zwraca nr. gracza zaczynając od 1, który jest odporny na kolor, jeśli żaden nie jest to zwraca NEGATIVE
int findResistantPlayerByColor(player** players, int players_count, int color) {

        int largest_amount = 0;
        int largest_amount_player_nr = NEGATIVE;

        for (int j = 0;j < players_count;j++) {
            int current_player_amount = countCardWithColor(players[j]->player_deck_head, color);
            if (current_player_amount > largest_amount) {
                largest_amount = current_player_amount;
                largest_amount_player_nr = j+1;
            }
            else if (current_player_amount == largest_amount) {
                largest_amount_player_nr = NEGATIVE;
            }

        }

        return largest_amount_player_nr;
}

//Sprawdza czy graczom zostały karty w ręce
bool checkIfCardsLeftInHand(player** players, int players_count) {

    for(int i = 0;i < players_count;i++) {

        player* current = players[i];

        if (current->player_hand_head != NULL)
            return true;
    }

    return false;
}


//Sprawdza czy w tablicy resistance_colors na pozycji odpowiadającej kolorowi znajduje się dany kolor
bool isColorResistant(int resistance_colors[COLORS_NR - 1], int color) {
    if (resistance_colors[color-1] == color)
            return true;
    return false;
}


//Sumuje ilość kart z uwzględnieniem odporności i podwójnej wartości koloru zielonego
int sumDeckCountWithResistance(deck_el* deck, int resistance_colors[COLORS_NR-1]) {
    int sum = 0;
    while (deck != NULL) {
        int color = deck->card->color;
        if (!isColorResistant(resistance_colors, color)) {
            if (color == GREEN)
                sum += 2;
            else
                sum += 1;
        }
        deck = deck->next;
    }
    return sum;
}


//ładuje dane 
void loadRoundStateToFile(int* active_player, int* players_count, int* explosion_threshold, player**& players, deck_el**& cauldrons, int* cauldrons_count) {

    FILE* in = fopen(SAVE_FILE, "r");

    char line[BUFFER_SIZE];

    char* piece;

    fgets(line, BUFFER_SIZE, in);

    piece = strtok(line, "active player = ");
    *active_player = strtol(piece, &piece, 10);


    fgets(line, BUFFER_SIZE, in);
    piece = strtok(line, "players number = ");
    *players_count = strtol(piece, &piece, 10);


    fgets(line, BUFFER_SIZE, in);
    piece = strtok(line, "explosion threshold = ");
    *explosion_threshold = strtol(piece, &piece, 10);



    players = (player**)malloc(*players_count * sizeof(player));


    loadPlayersData(players_count, players, line, in);
    loadCauldrons(cauldrons, cauldrons_count, line, in);

    fclose(in);

    printPlayersCardsCount(players, *players_count);
    printCauldronsCardCount(cauldrons, *cauldrons_count);
    printGreenCardsCount(players, cauldrons, *players_count, *cauldrons_count);
    printOtherCardsCount(players, cauldrons, *players_count, *cauldrons_count);
    printGameValidation(*active_player, *cauldrons_count, *explosion_threshold, *players_count, players, cauldrons);
}

//Rozegranie rundy, która kończy się, gdy karty na rękach graczy skończą się
void round(int* active_player, int* cauldrons_count, int *players_count, int* explosion_threshold, player** players, deck_el**& cauldrons) {
    
    bool cardsLeft = true;

    //Dopóki gracze mają karty w ręce rozgrywka trwa
    while (cardsLeft) {

        cout << "\n\n\n";
        cout << "\n";
        printRoundState(*active_player, *players_count, *explosion_threshold, *cauldrons_count, players, cauldrons);
        simpleMove(active_player, *cauldrons_count, *players_count, *explosion_threshold, players, cauldrons);
        cardsLeft = checkIfCardsLeftInHand(players, *players_count);
    }

    //Zliczanie punktów
    //tablica odporności: gracze ponumerowani od 0, kolory również blue = 0;
    int** players_resistance = new int*[*players_count];

    for (int i = 0;i < *players_count;i++) {
        players_resistance[i] = new int[COLORS_NR-1];
        for (int j = 0;j < COLORS_NR - 1;j++) {
            players_resistance[i][j] = NEGATIVE;
        }
    }

    for (int i = 1;i < COLORS_NR;i++) {
        int player_nr = findResistantPlayerByColor(players, *players_count, i);
        if (player_nr != NEGATIVE) {
            cout << "Na kolor "<<COLORS[i]<<" odporny jest gracz "<< player_nr <<"\n";
            players_resistance[player_nr-1][i-1] = i;
        }
    }

    cout << "\n";
    for (int i = 0;i < *players_count;i++) {
        player* current = players[i];

        int deck_count = getDeckLength(current->player_deck_head);
        int sum = sumDeckCountWithResistance(current->player_deck_head, players_resistance[i]);
        cout << "Wynik gracza " << i + 1 << " = " << sum<<"\n";
    }

    for (int i = 0;i < *players_count;i++) {
        delete[] players_resistance[i];
    }

    delete[] players_resistance;
}

//zwalnia pamięć na koniec gry
void freeMemory(player**& players_ptr, deck_el**& cauldrons_array, deck_el*& main_deck, int players_count, int cauldrons_count) {


    for (int i = 0;i < players_count;i++) {

        freeDeck(players_ptr[i]->player_hand_head, true);

        freeDeck(players_ptr[i]->player_deck_head, true);

        free(players_ptr[i]);

    }

    for (int i = 0;i < cauldrons_count;i++) {

        freeDeck(cauldrons_array[i], true);
        free(cauldrons_array[i]);
    }

    free(cauldrons_array);
    free(players_ptr);
    freeDeck(main_deck, true);
}


//Tasuje talię
void shuffleDeck(deck_el*& deck, int deck_size) {
    srand(time(NULL));
    for (int i = 0;i < deck_size;i++) {
        int index = rand()% deck_size;
        card* current_card = getCardByIndex(deck, index);
        addCardToDeck(&deck, current_card);
    }
}


//Zapisuje utworzoną talię do pliku
void saveMainDeckToFile(deck_el* deck) {
    FILE* file = fopen(DECK_FILE, "w");
    saveDeckToFile(file, deck);
    fclose(file);
}



int main()
{

    int players_count, cauldrons_count, explosion_threshold, greens_c, greens_v, others_c;    //liczba graczy, kociołki = liczba kolorów inne niż zielone, próg wybuchu, ilośc zielonych kart, wartość zielonej karty, ilość kart koloru niezielonego
    int* others_v = NULL;   //wartości kart innych niż zielone
    player** players = NULL; 
    deck_el** cauldrons = NULL;

    int active_player = 1;   //Nr. aktualnego gracza

    deck_el* main_deck = NULL; //Główna talia

    readStartingValues(&players_count, &cauldrons_count, &greens_c, &greens_v, &others_c, &explosion_threshold, others_v);


    
    players = (player**)malloc(players_count * sizeof(player*)); //tablica wskaźników na graczy
    cauldrons = (deck_el**)malloc(cauldrons_count * sizeof(deck_el*)); //tablica wskaźników na kociołki

    //Inicjalizjemy tablicę wskaźników wskaźnikami na nowych graczy
    for (int i = 0;i < players_count;i++) {
        players[i] = createPlayer(i+1);
    }
    
    //Inicjalizujemy kociołki
    for (int i = 0;i < cauldrons_count;i++) {
        cauldrons[i] = NULL;
    }

    createDeck(cauldrons_count, greens_c, greens_v, others_c, others_v, &main_deck);//tworzę talię i odwracam ją, aby najpierw rozdawać karty zielone
    saveMainDeckToFile(main_deck);

    delete[] others_v; //tablica wartości nie będzie nam już potrzebna dlatego zwalniamy miejsce



    int card_c = greens_c+others_c*cauldrons_count;

    shuffleDeck(main_deck, card_c);

    //Rozdajemy karty graczom
    for (int i = 0;main_deck != NULL;i++) {
        addCardToDeck(&(players[i % players_count]->player_hand_head), getCardFromCardDeck(&main_deck));
    }


    //Zapsuje stan gry do pliku i zwalniam miejsce
    saveRoundStateToFile(active_player, players_count, explosion_threshold, cauldrons_count, players, cauldrons);
    freeMemory(players, cauldrons, main_deck, players_count, cauldrons_count);


    //Ładuję stan gry z pliku
    loadRoundStateToFile(&active_player, &players_count, &explosion_threshold, players, cauldrons, &cauldrons_count);

    //przeprowadzam symulację gry, gdzie gracze wyciągają swoje najsłabsze karty
    round(&active_player, &cauldrons_count, &players_count, &explosion_threshold, players, cauldrons);
    

    //Zapsuje stan gry do pliku i zwalniam miejsce
    saveRoundStateToFile(active_player, players_count, explosion_threshold, cauldrons_count, players, cauldrons);
    freeMemory(players, cauldrons, main_deck, players_count, cauldrons_count);

    

    return 0;
}
