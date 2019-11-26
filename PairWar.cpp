#include <iostream>
#include <cstdio>
#include <ctime>
#include <pthread.h>
#include <cstdlib>
#include <unistd.h>
#include <deque>
#include <random>
#include <algorithm>
#include <fstream>

//threads for each player
#define NUM_THREADS 3
//number of rounds per game is 3
#define NUM_ROUNDS 3


using namespace std;

//struct for the deck of cards
struct Deck
{
    deque<int> cards;
};

//struct to hold player id and hand
struct Player
{
	int id;
	deque<int> hand;
};



Deck* deck = new Deck; //create deck object
bool winner = false; //set flag of winner of round to false
int status = 0; //set flag for status of game to 0

//initialize mutex lock for critical section
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


void create_deck(Deck*&);
void shuffle_deck(Deck*&);
void show_deck(deque<int>);
void deal_card(Player* player[], const int);
int top_card(Deck*&);
void print_game(Player* player[], const int);
void print_hand(Player* player);
void insert_card(Deck*&, int);
void discard_card(Player* player[]);
void player_draw(Player* player[]);
void end_round(Player*& p);
void* GamePlayer(void*);
void game_log(const string);



/***********************************
*end_round ends                    *
*the round for each player by      *
*erasing their cards               *
*-used after a winner is declared  *
***********************************/

void end_round(Player*& p)
{
    int cards = p->hand.size();
    for (int i = cards - 1; i >= 0; i--)
    {
        p->hand.erase(p->hand.begin() + 1);
    }
}

/**********************************
*create_deck                      *
*creates a deck of cards after    *
*destroying the deck from the     *
*previous round.                  *
**********************************/
void create_deck(Deck*& d)
{
    delete d;
    d = new Deck;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 1; j < 14; j++)
        {
            d->cards.push_front(j);
        }
    }
}

/**********************************
*shuffle_deck shuffles the deque  *
*->cards using the random_shuffle *
*function - seed is time-based to *
*consistently have different      *
*results.                         *
**********************************/
void shuffle_deck(Deck*& d)
{
    cout << "DEALER: Shuffling cards." << endl;
    game_log("DEALER: Shuffling cards.");
    std::random_shuffle(d->cards.begin(), d->cards.end());

}

/**********************************
*show_deck displays the contents  *
*of the deck - passed by value to *
*not disrupt the values within d  *
**********************************/
void show_deck(deque<int> d)
{
    string l = "DECK: ";
    cout << "DECK: ";
    while (!d.empty())
    {
        cout << d.front() << " ";
        l += to_string(d.front()) + " ";
        d.pop_front();
    }
    cout << endl;
    game_log(l);
}
/***********************************
*deal_card deals the card from the *
*top of the deck to each player in *
*the round.                        *
***********************************/
void deal_card(Player* player[], const int num_players)
{
    cout << "DEALER: Dealing cards." << endl;
    game_log("DEALER: Dealing cards.");
    for (int i = 0; i < num_players; i++)
    {
        player[i]->hand.push_front(top_card(deck));
        print_hand(player[i]);
        cout << endl;
    }

}
/***********************************
*top_card is a helper function that*
*assists in removing the top card  *
*from the deck - used for dealing  *
*and game play.                    *
***********************************/
int top_card(Deck*& d)
{
    int new_card = d->cards.front();
    d->cards.pop_front();
    return new_card;
}
/***********************************
*print_hand prints the current hand*
*of the player calling the function*
***********************************/
void print_hand(Player* player)
{
    cout << "PLAYER " << player->id << " hand: ";
    string l = "PLAYER " + to_string(player->id) + " hand: ";
    for (int i = player->hand.size() -1; i >= 0; i--)
    {
        l += to_string(player->hand.at(i)) + " ";
        cout << player->hand.at(i) << " ";
    }
    game_log(l);
}
/***********************************
*insert_card returns a card to the *
*bottom of the shuffled deck.      *
***********************************/
void insert_card(Deck*& d, int return_card)
{
    d->cards.push_back(return_card);
}
/***********************************
*discard_card is used to randomize *
*the card that the player discards *
*if they haven't obtained a match  *
***********************************/
void discard_card(Player* player)
{
    string l = "PLAYER " + to_string(player->id) + " discards: ";
    int n = player->hand.size();
    int card = rand() % n;
    l += to_string(player->hand.at(card));
    insert_card(deck, player->hand.at(card));
    player->hand.erase(player->hand.begin() + card);
    game_log(l);

}
/***********************************
*player_draw allows the player to  *
*draw the top card from the deck   *
***********************************/
void player_draw(Player* player)
{
    string l = "PLAYER " + to_string(player->id) + " draws: ";
    int card = top_card(deck);
    l += to_string(card);
    player->hand.push_back(card);
    game_log(l);
}
/***********************************
*main threaded function to process *
*game play. Takes the threads and  *
*uses helper functions to play the *
*game until one thread obtains a   *
*match and wins the game.          *
***********************************/
void* GamePlayer(void* player)
{
    Player* p = (Player*)player;

    //if round is won, have other threads discard
    if (status == 1)
    {
        pthread_mutex_lock(&mutex); //critical section
        if(p->hand.size() >= 2)
        {
            discard_card(p);
        }
    }
    else //play their turn
    {
        pthread_mutex_lock(&mutex); //critical section
        if(p->hand.size() >= 2)
        {
            discard_card(p);
            print_hand(p);
            cout << endl;
        }
        player_draw(p);
        print_hand(p);
        cout << endl;
        //check for match
        if (p->hand.at(0) == p->hand.at(1))
        {
            winner = true;
            status = 1;
            cout << "PLAYER " << p->id << " win: YES" << endl;
            game_log("PLAYER " + to_string(p->id) + " win: YES");

        }
        //discard and end turn
        else
        {
            cout << "PLAYER " << p->id << " win: NO" << endl;
            game_log("PLAYER " + to_string(p->id) + " win: NO");
            discard_card(p);

        }


    }

    //unlock critical section to allow next thread to go
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);

    usleep(1000);

}
//dealer thread utilized by main
int main()
{
    //initiate random seed using current time
    srand(time(NULL));

    //create players
    Player* p1 = new Player;
    Player* p2 = new Player;
    Player *p3 = new Player;
    Player* player[NUM_THREADS] = {p1, p2, p3};

    //set player IDs
    player[0]->id = 1;
    player[1]->id = 2;
    player[2]->id = 3;

    //create 3 threads for players
    pthread_t threads[NUM_THREADS];

    //play game for 3 rounds
    for (int i = 0; i < NUM_ROUNDS; i++)
    {
        //dealer actions
        game_log(""); //for file to open
        game_log("\nRound: " + to_string(i+1));
        cout << "\nRound: " << i+1 << endl;

        create_deck(deck);
        shuffle_deck(deck);
        show_deck(deck->cards);
        deal_card(player, NUM_THREADS);
        show_deck(deck->cards);

        //initialize threads to play game
        int rc;
        int t;
        while (winner == false)
        {
            for (t = 0; t < NUM_THREADS; t++)
            {
                rc = pthread_create(&threads[t], NULL, GamePlayer, (void *)player[t]);
                usleep(1000);

                if (rc)
                {
                    cout << "Error: unable to create thread, " << rc << endl;
                    exit(-1);
                }
            }
            //wait for other threads to join
            for (int i = 0; i < NUM_THREADS; i++)
            {
                if(pthread_join(threads[i], NULL))
                {
                    cout << "Error joining thread\n";
                    exit(2);
                }
            }
        }
        //display hands of threads after round is won
        for (int i = 0; i < NUM_THREADS; i++)
        {
            print_hand(player[i]);
            cout << endl;
            end_round(player[i]);
        }
        show_deck(deck->cards);
        //players exit the game
        for (int i = 0; i < NUM_THREADS; i++)
        {
            cout << "PLAYER " << i+1 << " exiting." << endl;
            game_log("PLAYER " + to_string(i+1) + " exiting.");
        }

        //reset flags for next round
        winner = false;
        status = 0;


    }
    //unlock mutex
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL); //threads exit game

    return 0;
}

/*****************************************
*game_log creates a log of the game play *
*and saves it to a file titled game.txt  *
*****************************************/
void game_log(string g)
{
    ofstream fout;
    ifstream fin;
    fin.open("game.txt");
    fout.open("game.txt", ios::app);
    if (fin.is_open())
    {
        fout << g << endl;
    }

    fin.close();
    fout.close();
}
