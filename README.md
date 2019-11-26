# Pair-War
I use POSIX threads to create a card game in which someone wins as soon as they have a pair of matching card

The main function is used as the dealer thread. The dealer is responsible for beginning each round. The dealer begins by shuffling the deck and dealing one card to each player.

There are p-threads created for each player. The players then draw from the deck, check for a match, then discard if they do not have a match. If there is a match, the game ends, and each thread exits the round.

This program plays 3 rounds until each has a winner.
