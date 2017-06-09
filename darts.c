#include <conio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define MAX_PLAYERS 10

typedef enum {
	Inactive, //not playing
	Playing,
	Gold,
	Silver,
	Bronze,
	Finished,
	Loser, //last player
	Quiter, //when in_value > starting_points
} Status;

//keep #rounds for each prize category
typedef struct {
    int gold;
    int silver;
    int bronze;
    int finish;
} rank_t;

typedef struct
{
	Status status;
    char name[15];
	int points; //in game
    int rounds;
	int score; //in file
    int dart; //max dart value
} player_t;

player_t players[MAX_PLAYERS];
rank_t rank;

int darts = 5; //arg2
int starting_points = 501; //arg3
int players_count; //total players known in file
int active_players;

int get_response() {
    switch (getch())
    {
        case 'y':
        case 'Y':
        case '1':
        case '\r':
        case '\n':
            puts("Yes.");
            return 1;
        break;
        
        default:
            puts("No.");
            return 0;
        break;
    }
}

void offer_scores()
{
	for (int i = 0; i < players_count; ++i) {
		switch (players[i].status) {
			case Inactive:
				if (players[i].score) {
					players[i].score--;
				}
			break;
			
			case Quiter:
				if (players[i].score > 5) {
					players[i].score -= 5;
				}
				else {
					players[i].score = 0;
				}
			break;
			
			default:
			break;
			
			case Gold:
				players[i].score += 100;
			break;
			
			case Silver:
				players[i].score += 50;
			break;
			
			case Bronze:
				players[i].score += 10;
			break;
			
			case Finished:
				players[i].score += 2;
			break;
			
			case Loser:
				if (players[i].score > 100) {
					players[i].score -= 100;
				}
				else {
					players[i].score = 0;
				}
			break;
		}
	}
}

void order_players()
{
	int sorted = 0;
	player_t player;
	while (!sorted) {
		sorted = 1;
		for (int i = 0; i < players_count - 1; ++i) {
			if (players[i].score < players[i+1].score) {
				sorted = 0;
				player = players[i];
				players[i] = players[i+1];
				players[i+1] = player;
			}
		}
	}
}

int read_file(char * filename)
{
	FILE * fptr = fopen(filename, "r");
	fscanf(fptr, "%d", &players_count);
	if (players_count > MAX_PLAYERS) {
		fprintf(stderr, "You can have max %d players!\n", MAX_PLAYERS);
		return 0;
	}
    
	int line = 0;
	while (line < players_count) {
		fscanf(fptr, "%s", players[line].name);
		fscanf(fptr, "%d", &players[line].score);
		printf("Will %s play? ", players[line].name);
		if (get_response()) {
            ++active_players;
            players[line].status = Playing;
            players[line].points = starting_points;
        }
		else {
			players[line].status = Inactive;
		}
		++line;
    }
	fclose(fptr);
	return 1; //success
}

int write_file(char * filename)
{
	FILE * fptr = fopen(filename, "w");
	fprintf(fptr, "%d", players_count);
	for (int i = 0; i < players_count; ++i) {
		fprintf(fptr, "\n%s\t%d", players[i].name, players[i].score);
	}
	printf("\nScores have been recorded in %s.\n", filename);
	fclose(fptr);
}

void finish(int player)
{
    if (players[player].rounds <= rank.gold || !rank.gold) {
        rank.gold = players[player].rounds;
        players[player].status = Gold;
    }
    else if (players[player].rounds <= rank.silver || !rank.silver) {
        rank.silver = players[player].rounds;
        players[player].status = Silver;
    }
    else if (players[player].rounds <= rank.bronze || !rank.bronze) {
        rank.bronze = players[player].rounds;
        players[player].status = Bronze;
    }
    else if (players[player].rounds <= rank.finish || !rank.finish) {
        rank.finish = players[player].rounds;
        players[player].status = Finished;
    }
    else {
        players[player].status = Loser;
    }
    active_players--;
}

void process_player(int current_player)
{
	printf("\nIt's %s\'s turn.\n", players[current_player].name);
	
	int in_value;
	int points = players[current_player].points;
    ++players[current_player].rounds; //increment now
    //play #darts
	for (int d = 0; d < darts; ++d) { 		
        //GET DART VALUE
        printf("\nDart %d value: ", d+1);
        char input_line[100]; //TODO ??
		if (fgets(input_line, sizeof(input_line), stdin)) {
			if (1 != sscanf(input_line, "%d", &in_value)) {
				//TODO pressing just enter returns previous value, useful
                in_value = 0;
			}
		}
        //CHECK VALUE
		if (in_value < 0 || in_value > starting_points) { 
            //we have a quitter
            players[current_player].status = Quiter;
            puts("Quiter!");
            active_players--;
			return;
		}
		if (in_value > points) {
            //skip this round
			puts("Sorry, this is too much!");
			return;
		}
		else { 
            //OK
            points -= in_value;
            if (players[current_player].dart < points) {
                players[current_player].dart = points;
            }
			printf("Score %d to win.\n", points);
		}
	}
    
    players[current_player].points = points;
	if (!points) {
		finish(current_player);
	}
    else if (active_players == 1) {
        players[current_player].status = Loser;
        puts("Loser!");
        active_players--;
	    return;
    }
}

void print_table(int game_over)
{
	system("cls");
	printf("Name\t Points\t Round\tStatus");
	if (game_over) {
		puts("\tScore\tHighest");
	}
	else {
		puts("");
	}
	for (int i = 0; i < players_count; ++i) {
		if (players[i].status || game_over) {
			printf("%s\t %d\t %d\t", players[i].name, players[i].points, players[i].rounds);
			//transform status to string
            switch (players[i].status) {
				case Inactive:
					printf("Skip");
				break;
				
				case Playing:
					printf("Playing");
				break;
				
				case Gold:
					printf("Gold");
				break;
					
				case Silver:
					printf("Silver");
				break;
				
				case Bronze:
					printf("Bronze");
				break;
				
				case Finished:
					printf("Finish");
				break;
				
				case Loser:
					printf("Loser");
				break;
				
				case Quiter:
					printf("Quiter");
				break;
				
				default:
					printf("Unknown");
				break;
			}
			if (game_over) {
				printf("\t%d\t%d", players[i].score, players[i].dart);
			}
			puts(""); //newline
		}
	}
}

int main(int argc, char * argv[])
{
	system("cls");
	char filename[15] = "darts.in";
    switch (argc) {
		case 1:
			fprintf(stderr, "Using default input file: darts.in\n\n");
		break;
		
		case 2: //provide only input file
			strcpy(filename, argv[1]);
			fprintf(stderr, "Using %s as input file.", filename);
		break;
		
		case 4: //set #darts and starting_points
			strcpy(filename, argv[1]);
			char * p;
			darts = strtol(argv[2], &p, 10); //base10
            starting_points = strtol(argv[3], &p, 10);
            fprintf(stderr, "%s: %d darts, starting from %d points\n", filename, darts, starting_points);
		break;
        
        default:
            printf("For indications regarding expected arguments, ask lbucur.\n");
        break;
    }
	
	if (!read_file(filename)) {
		fprintf(stderr, "Something is wrong with the input file!\nQuitting now.\n");
		return -1;
	}
	
	//start game
	while (active_players) {
		for (int current_player = 0; current_player < players_count; ++current_player) {
			if (players[current_player].status != Playing) {
                continue; //skip inactive player
            }
            print_table(0); //don't show scores now
			process_player(current_player);
			//take commands
            switch (getch()) {
                case 'a':
                    //add new player
                break;
                
                default:
                    //move on
                break;
            }
		}
	}
	//end game
	offer_scores(); //update player score
	order_players(); //by descending scores
	print_table(1); //game over
    
    printf("\nRecord results to file? ");
    if (get_response()) {
        write_file(filename);
    }
	puts("Thanks for playing!");
    return 0;
}