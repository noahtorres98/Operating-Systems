#include <iostream>
#include <pthread.h>
#include <cstdlib>
#include <semaphore.h>
#include <fstream>
//#include <bits/stdc++.h>
#include <string>
#include <sstream>

//using namespace std;

void *PlayerFunction(void *threadarg);
void *DealerFunction(void *threadarg);

pthread_mutex_t lock;
pthread_mutex_t lock1;
pthread_mutex_t lock2;
pthread_cond_t cv;
pthread_cond_t cv1;
pthread_cond_t cv2;


template <typename T>
std::string to_string(T val)
{
    std::stringstream stream;
    stream << val;
    return stream.str();
}


struct player {
    int id;
    int card1;
    int card2;
};

struct player player1;
struct player player2;
struct player player3;

int winner;
int roundNumber;

int deck[52];
int deckSize;

void PrintToFile(std::string stringToOutput){
    std::ofstream Logfile;
    Logfile.open("LogFile.txt" , std::ofstream::out | std::ofstream::app);
    Logfile << stringToOutput << std::endl;
    Logfile.close();

}



int getCard(int PlayerID) {

    int card = deck[0];//assigns the card to deal to the first card in the deck

    for (int i = 0; i < deckSize - 1; i++) {//shift everythging in the deck since we are taking out one card
        deck[i] = deck[i + 1];
    }
    deckSize--;//decrease deck size

    //output to file
    std::string printString = "PLAYER ";
    printString+= to_string(PlayerID);
    printString+= ": draws ";
    printString+= to_string(card);
    PrintToFile(printString);


    return card;
}

void returnCard(int card, int PlayerID) {
    //puts card at the end of the deck

    if(winner==-1){
        std::string printString = "PLAYER ";
        printString+= to_string(PlayerID);
        printString += ": discards ";
        printString+= to_string(card);
        PrintToFile(printString);
    }


    deck[deckSize++] = card;
}

std::string printDeckToFile() {
    std::string deckString = "DECK: ";

    for (int i = 0; i < deckSize; i++) {
        //std::cout << deck[i] << ", ";
        deckString += to_string(deck[i]);
        deckString += " ";

    }

    PrintToFile(deckString);
    //std::cout << std::endl;
   return 0;
}

std::string printDeck() {
    std::cout << "DECK: ";
    std::string deckString = "";
    for (int i = 0; i < deckSize; i++) {
        std::cout << deck[i] << " ";
        deckString += to_string(deck[i]);
        deckString += " ";

    }

    //PrintToFile(deckString);
    std::cout << std::endl;
    return 0;
}

std::string printHand(player *player_data){
    std::string tempString = "Player ";
    tempString += to_string(player_data->id);
    tempString += ": hand ";

    if(player_data->card1 != -1 && player_data->card2 == -1)
        tempString += to_string(player_data->card1);
    else if(player_data->card1 == -1 && player_data->card2 != -1)
        tempString += to_string(player_data->card2);
    else{
        tempString += to_string(player_data->card1);
        tempString += " ";
        tempString += to_string(player_data->card2);
    }

    return tempString;

}

void *PlayerFunction1(void *threadarg) {
    struct player *player_data;
    player_data = (struct player *) threadarg;
    pthread_mutex_lock(&lock);
    while (true){ //play until winner is updated
        pthread_cond_wait(&cv, &lock);
        if (winner != -1) {
            //someone has won
            //pthread_cond_signal(&cv1);
            //pthread_mutex_unlock(&lock);
            //std::cout << "someone has won" << std::endl;


            break;
        }

        // output hand to file before round start
        PrintToFile(printHand(player_data));

        // draw card
        if (player_data->card1 == -1) {
            player_data->card1 = getCard(player_data->id);
        }
        else{
            player_data->card2 = getCard(player_data->id);
        }

        //check if player won
        if (player_data->card1 == player_data->card2) {
            winner = player_data->id;
            //pthread_cond_signal(&cv1);
            //std::cout << "signaled" << std::endl;
            PrintToFile(printHand(player_data));
            PrintToFile("Player 1: wins");
            //pthread_mutex_unlock(&lock);
            break;
        }


        //if not discard one randomly
        if (rand() % 2 == 0){
            returnCard(player_data->card1, player_data->id);
            player_data->card1 = -1;
        }
        else{
            returnCard(player_data->card2, player_data->id);
            player_data->card2 = -1;
        }

        if (player_data->card1 != -1){
            std::cout << "PLAYER 1" << std::endl;
            std::cout << "HAND " << player_data->card1 << std::endl;
        }
        else if (player_data->card2 != -1){
            std::cout << "PLAYER 1" << std::endl;
            std::cout << "HAND " << player_data->card2 << std::endl;
        }

        PrintToFile(printHand(player_data));

        //If no winner print deck

        printDeckToFile();

        pthread_cond_signal(&cv1);

    }

    if (winner == player_data->id){
        std::cout << "PLAYER 1:" << std::endl;
        std::cout << "HAND " << player_data->card1 << ", " << player_data->card2 << std::endl;
        std::cout << "WIN yes" << std::endl;
    }
    else {
        if (player_data->card1 != -1){
            std::cout << "PLAYER 1:" << std::endl;
            std::cout << "HAND " << player_data->card1 << std::endl;
            std::cout << "WIN no" << std::endl;
        }
        if (player_data->card2 != -1){
            std::cout << "PLAYER 1:" << std::endl;
            std::cout << "HAND " << player_data->card2 << std::endl;
            std::cout << "WIN no" << std::endl;
        }
    }

    if (player_data->card1 != -1){
        returnCard(player_data->card1, player_data->id);
        player_data->card1 = -1;
    }
    if (player_data->card2 != -1){
        returnCard(player_data->card2, player_data->id);
        player_data->card2 = -1;
    }
    //output round exit

    PrintToFile("Player 1: exits round");

    pthread_cond_signal(&cv1);
    pthread_mutex_unlock(&lock);


    return ((void*)NULL);
}

void *PlayerFunction2(void *threadarg) {
    struct player *player_data;
    player_data = (struct player *) threadarg;
    pthread_mutex_lock(&lock1);
    while (true){ //play until winner is updated
        pthread_cond_wait(&cv1, &lock1);
        if (winner != -1) {
            //someone has won
            //std::cout << "someone has won" << std::endl;


            //pthread_cond_signal(&cv2);
            //pthread_mutex_unlock(&lock1);
            break;
        }


        PrintToFile(printHand(player_data));


        if (player_data->card1 == -1) {
            player_data->card1 = getCard(player_data->id);
        }
        else{
            player_data->card2 = getCard(player_data->id);
        }

        //check if player won
        if (player_data->card1 == player_data->card2) {
            winner = player_data->id;
            //pthread_cond_signal(&cv2);
            //std::cout << "signaled" << std::endl;
            PrintToFile(printHand(player_data));
            PrintToFile("Player 2: wins");
            //pthread_mutex_unlock(&lock1);
            break;
        }

        //if not discard one randomly
        if (rand() % 2 == 0){
            returnCard(player_data->card1, player_data->id);
            player_data->card1 = -1;
        }
        else{
            returnCard(player_data->card2, player_data->id);
            player_data->card2 = -1;
        }

        if (player_data->card1 != -1){
            std::cout << "PLAYER 2" << std::endl;
            std::cout << "HAND " << player_data->card1 << std::endl;
        }
        else if (player_data->card2 != -1){
            std::cout << "PLAYER 2" << std::endl;
            std::cout << "HAND " << player_data->card2 << std::endl;
        }

        PrintToFile(printHand(player_data));

        //If no winner print deck
        printDeckToFile();

        pthread_cond_signal(&cv2);

    }

    if (winner == player_data->id){
        std::cout << "PLAYER 2" << std::endl;
        std::cout << "HAND " << player_data->card1 << ", " << player_data->card2 << std::endl;
        std::cout << "WIN yes" << std::endl;
    }
    else {
        if (player_data->card1 != -1){
            std::cout << "PLAYER 2" << std::endl;
            std::cout << "HAND " << player_data->card1 << std::endl;
            std::cout << "WIN no" << std::endl;
        }
        if (player_data->card2 != -1){
            std::cout << "PLAYER 2" << std::endl;
            std::cout << "HAND " << player_data->card2 << std::endl;
            std::cout << "WIN no" << std::endl;
        }
    }

    if (player_data->card1 != -1){
        returnCard(player_data->card1, player_data->id);
        player_data->card1 = -1;
    }
    if (player_data->card2 != -1){
        returnCard(player_data->card2, player_data->id);
        player_data->card2 = -1;
    }

    PrintToFile("Player 2: exits round");

    pthread_cond_signal(&cv2);
    pthread_mutex_unlock(&lock1);

    return ((void*)NULL);
}

void *PlayerFunction3(void *threadarg) {
    struct player *player_data;
    player_data = (struct player *) threadarg;
    pthread_mutex_lock(&lock2);
    while (true){ //play until winner is updated
        pthread_cond_wait(&cv2, &lock2);
        if (winner != -1) {
            //someone has won
            break;
        }

        // output hand to file before round start
        PrintToFile(printHand(player_data));

        if (player_data->card1 == -1) {
            player_data->card1 = getCard(player_data->id);
        }
        else{
            player_data->card2 = getCard(player_data->id);
        }

        //check if player won
        if (player_data->card1 == player_data->card2) {
            winner = player_data->id;
            //pthread_cond_signal(&cv);
            std::cout << "signaled" << std::endl;
            PrintToFile(printHand(player_data));
            PrintToFile("Player 3: wins");
            //pthread_mutex_unlock(&lock2);
            break;
        }

        //if not discard one randomly
        if (rand() % 2 == 0){
            returnCard(player_data->card1, player_data->id);
            player_data->card1 = -1;
        }
        else{
            returnCard(player_data->card2, player_data->id);
            player_data->card2 = -1;
        }
        if (player_data->card1 != -1){
            std::cout << "PLAYER 3" << std::endl;
            std::cout << "HAND " << player_data->card1 << std::endl;
        }
        else if (player_data->card2 != -1){
            std::cout << "PLAYER 3" << std::endl;
            std::cout << "HAND " << player_data->card2 << std::endl;
        }

        //print hand after playing
        PrintToFile(printHand(player_data));

        //If no winner print deck

        printDeckToFile();

        pthread_cond_signal(&cv);

    }

    if (winner == player_data->id){
        std::cout << "PLAYER 3" << std::endl;
        std::cout << "HAND " << player_data->card1 << ", " << player_data->card2 << std::endl;
        std::cout << "WIN yes" << std::endl;
    }
    else {
        if (player_data->card1 != -1){
            std::cout << "PLAYER 3" << std::endl;
            std::cout << "HAND " << player_data->card1 << std::endl;
            std::cout << "WIN no" << std::endl;
        }
        if (player_data->card2 != -1){
            std::cout << "PLAYER 3" << std::endl;
            std::cout << "HAND " << player_data->card2 << std::endl;
            std::cout << "WIN no" << std::endl;
        }
    }

    if (player_data->card1 != -1){
        returnCard(player_data->card1, player_data->id);
        player_data->card1 = -1;
    }
    if (player_data->card2 != -1){
        returnCard(player_data->card2, player_data->id);
        player_data->card2 = -1;
    }

    PrintToFile("Player 3: exits round");

    pthread_cond_signal(&cv);
    pthread_mutex_unlock(&lock2);

    return ((void*)NULL);
}

void *DealerFunction(void *threadarg) {

    winner = -1;

    //shuffle
    for (int i = 0; i < 52; i++) {
        int r = i + (rand() % (52 - i));
        std::swap(deck[i], deck[r]);
    }

    PrintToFile("DEALER: shuffle");

    //printDeck();

    player1.card1 = getCard(1);
    player2.card1 = getCard(2);
    player3.card1 = getCard(3);

    if (roundNumber == 1){
        pthread_cond_signal(&cv);
        roundNumber++;
    }
    else if (roundNumber == 2){
        pthread_cond_signal(&cv1);
        roundNumber++;
    }
    else{
        pthread_cond_signal(&cv2);
        roundNumber++;
    }
    return ((void*)NULL);
}

int main() {

    //clean log file

    remove("LogFile.txt");

    pthread_t dealer;
    pthread_t thread1;
    pthread_t thread2;
    pthread_t thread3;

    player1.id = 1;
    player1.card1 = -1;
    player1.card2 = -1;

    player2.id = 2;
    player2.card1 = -1;
    player2.card2 = -1;

    player3.id = 3;
    player3.card1 = -1;
    player3.card2 = -1;

    pthread_attr_t attr;

    roundNumber = 1;

    pthread_mutex_init(&lock, NULL);
    pthread_mutex_init(&lock1, NULL);
    pthread_mutex_init(&lock2, NULL);
    pthread_cond_init(&cv, NULL);
    pthread_cond_init(&cv1, NULL);
    pthread_cond_init(&cv2, NULL);


    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    deckSize = 0;

    srand((unsigned)time(NULL));

    for (int i = 0; i < 4; i++) {
        for(int j = 1; j <= 13; j++) {
            deck[deckSize++] = j;
        }
    }

    std::cout << "round: " << roundNumber << std::endl;

    if (pthread_create (&thread1, &attr, PlayerFunction1, (void *)&player1) != 0){
        std::cout << "failed to create thread" << std::endl;

    }
    if (pthread_create (&thread2, &attr, PlayerFunction2, (void *)&player2) != 0){
        std::cout << "failed to create thread" << std::endl;

    }
    if (pthread_create (&thread3, &attr, PlayerFunction3, (void *)&player3) != 0){
        std::cout << "failed to create thread" << std::endl;
    }
    if (pthread_create (&dealer, &attr, DealerFunction, NULL) != 0){
        std::cout << "failed to create thread" << std::endl;
    }


    if (pthread_join(thread1, NULL) != 0){
        std::cout << "failed to join thread" << std::endl;
    }
    if (pthread_join(thread2, NULL) != 0){
        std::cout << "failed to join thread" << std::endl;
    }
    if (pthread_join(thread3, NULL) != 0){
        std::cout << "failed to join thread" << std::endl;
    }
    printDeck();
    if (pthread_join(dealer, NULL) != 0){
        std::cout << "failed to join thread" << std::endl;
    }


    std::cout << std::endl;
    std::cout << "round: " << roundNumber << std::endl;

    if (pthread_create (&thread1, &attr, PlayerFunction1, (void *)&player1) != 0){
        std::cout << "failed to create thread" << std::endl;

    }
    if (pthread_create (&thread2, &attr, PlayerFunction2, (void *)&player2) != 0){
        std::cout << "failed to create thread" << std::endl;

    }
    if (pthread_create (&thread3, &attr, PlayerFunction3, (void *)&player3) != 0){
        std::cout << "failed to create thread" << std::endl;
    }
    if (pthread_create (&dealer, &attr, DealerFunction, NULL) != 0){
        std::cout << "failed to create thread" << std::endl;
    }


    if (pthread_join(thread1, NULL) != 0){
        std::cout << "failed to join thread" << std::endl;
    }
    if (pthread_join(thread2, NULL) != 0){
        std::cout << "failed to join thread" << std::endl;
    }
    if (pthread_join(thread3, NULL) != 0){
        std::cout << "failed to join thread" << std::endl;
    }
    printDeck();
    if (pthread_join(dealer, NULL) != 0){
        std::cout << "failed to join thread" << std::endl;
    }

    std::cout << std::endl;
    std::cout << "round: " << roundNumber << std::endl;

    if (pthread_create (&thread1, &attr, PlayerFunction1, (void *)&player1) != 0){
        std::cout << "failed to create thread" << std::endl;

    }
    if (pthread_create (&thread2, &attr, PlayerFunction2, (void *)&player2) != 0){
        std::cout << "failed to create thread" << std::endl;

    }
    if (pthread_create (&thread3, &attr, PlayerFunction3, (void *)&player3) != 0){
        std::cout << "failed to create thread" << std::endl;
    }

    if (pthread_create (&dealer, &attr, DealerFunction, NULL) != 0){
        std::cout << "failed to create thread" << std::endl;
    }


    if (pthread_join(thread1, NULL) != 0){
        std::cout << "failed to join thread" << std::endl;
    }
    if (pthread_join(thread2, NULL) != 0){
        std::cout << "failed to join thread" << std::endl;
    }
    if (pthread_join(thread3, NULL) != 0){
        std::cout << "failed to join thread" << std::endl;
    }
    printDeck();
    if (pthread_join(dealer, NULL) != 0){
        std::cout << "failed to join thread" << std::endl;
    }


    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&lock);
    pthread_mutex_destroy(&lock1);
    pthread_mutex_destroy(&lock2);

    pthread_exit(NULL);
}
