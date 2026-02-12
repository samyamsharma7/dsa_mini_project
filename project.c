#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define VOTER_FILE "voters.txt"
#define CANDIDATE_FILE "candidates.txt"
#define MAX_CANDIDATES 100
#define CURRENT_YEAR 2026


typedef struct {
    int id;
    char name[100];
    int age;
    char dob[15];
    char sex;
    int hasVoted;
} Voter;

typedef struct {
    int id;
    char name[50];
    int votes;
} Candidate;

struct QueueNode {
    int voterId;
    struct QueueNode* next;
};

struct QueueNode *front = NULL, *rear = NULL;

void enqueue(int id) {
    struct QueueNode* newNode = (struct QueueNode*)malloc(sizeof(struct QueueNode));
    if (!newNode) return;
    newNode->voterId = id;
    newNode->next = NULL;
    if (rear == NULL) {
        front = rear = newNode;
    } else {
        rear->next = newNode;
        rear = newNode;
    }
    printf("\nVoter %d added to queue.\n", id);
}

int dequeue() {
    if (front == NULL) return -1;
    struct QueueNode* temp = front;
    int id = temp->voterId;
    front = front->next;
    if (front == NULL) rear = NULL;
    free(temp);
    return id;
}

int getVoterStatus(int id, Voter* foundVoter) {
    FILE* fp = fopen(VOTER_FILE, "r");
    if (!fp) return 0;

    Voter v;
    while (fscanf(fp, "%d | %[^|] | %d | %s | %c | %d\n", &v.id, v.name, &v.age, v.dob, &v.sex, &v.hasVoted) != EOF) {
        if (v.id == id) {
            if (foundVoter) *foundVoter = v;
            fclose(fp);
            return (v.hasVoted) ? 1 : 2;
        }
    }
    fclose(fp);
    return 0;
}

void markVotedInFile(int id) {
    FILE* fp = fopen(VOTER_FILE, "r");
    FILE* temp = fopen("temp_voters.txt", "w");
    Voter v;

    if (!fp || !temp) return;

    while (fscanf(fp, "%d | %[^|] | %d | %s | %c | %d\n", &v.id, v.name, &v.age, v.dob, &v.sex, &v.hasVoted) != EOF) {
        if (v.id == id) v.hasVoted = 1;
        fprintf(temp, "%d | %s | %d | %s | %c | %d\n", v.id, v.name, v.age, v.dob, v.sex, v.hasVoted);
    }
    fclose(fp);
    fclose(temp);
    remove(VOTER_FILE);
    rename("temp_voters.txt", VOTER_FILE);
}

void registerVoter() {
    Voter v;
    int d, m, y;
    srand(time(NULL));
    
    v.id = (rand() % 900000) + 100000;
    while (getVoterStatus(v.id, NULL) != 0) {
        v.id = (rand() % 900000) + 100000;
    }

    printf("\nVoter Registration\n");
    
    printf("Enter Full Name: ");
    getchar(); 
    scanf("%[^\n]", v.name);
    
    printf("Enter DOB (DD/MM/YYYY): "); 
    scanf("%s", v.dob);
    
    // calculate age automatically
    if (sscanf(v.dob, "%d/%d/%d", &d, &m, &y) == 3) {
        v.age = CURRENT_YEAR - y;
    } else {
        printf("\nInvalid DOB format! Use DD/MM/YYYY.\n");
        return;
    }

    printf("Enter Sex (M/F/O): "); 
    scanf(" %c", &v.sex);
    v.hasVoted = 0;

    if (v.age < 18) {
        printf("\nError: Voter age is %d. Must be 18+ to register.\n", v.age);
        return;
    }

    FILE* fp = fopen(VOTER_FILE, "a");
    if (fp) {
        fprintf(fp, "%d | %s | %d | %s | %c | %d\n", v.id, v.name, v.age, v.dob, v.sex, v.hasVoted);
        fclose(fp);
        printf("\nRegistered! ID: %d | Calculated Age: %d\n", v.id, v.age);
        enqueue(v.id);
    }
}

void showCandidates() {
    FILE* fp = fopen(CANDIDATE_FILE, "r");
    if (!fp) {
        printf("\nError: candidates.txt missing!\n");
        return;
    }
    Candidate c;
    printf("\nID\tCandidate Name\t\tVotes\n--------------------------------------\n");
    while (fscanf(fp, "%d %s %d", &c.id, c.name, &c.votes) != EOF) {
        printf("%d\t%-20s\t%d\n", c.id, c.name, c.votes);
    }
    fclose(fp);
}

void updateCandidateVote(int cid) {
    FILE* fp = fopen(CANDIDATE_FILE, "r");
    FILE* temp = fopen("temp_cand.txt", "w");
    Candidate c;
    int found = 0;

    if (!fp || !temp) return;
    while (fscanf(fp, "%d %s %d", &c.id, c.name, &c.votes) != EOF) {
        if (c.id == cid) {
            c.votes++;
            found = 1;
        }
        fprintf(temp, "%d %s %d\n", c.id, c.name, c.votes);
    }
    fclose(fp); fclose(temp);
    remove(CANDIDATE_FILE);
    rename("temp_cand.txt", CANDIDATE_FILE);
    
    if (!found) printf("\nInvalid Candidate ID!\n");
    else printf("\nVote Recorded.\n");
}

void castVote() {
    int vid = dequeue();
    if (vid == -1) {
        printf("\nVoting Queue is empty!\n");
        return;
    }

    Voter v;
    int status = getVoterStatus(vid, &v);

    if (status == 1) {
        printf("\nVoter %d has already voted!\n", vid);
    } else if (status == 0) {
        printf("\nVoter ID %d not found!\n", vid);
    } else {
        showCandidates();
        int cid;
        printf("\n[Voter: %s] Enter Candidate ID: ", v.name);
        scanf("%d", &cid);
        updateCandidateVote(cid);
        markVotedInFile(vid);
    }
}

void showWinner() {
    FILE* fp = fopen(CANDIDATE_FILE, "r");
    if (!fp) return;
    Candidate c, winner;
    winner.votes = -1;
    while (fscanf(fp, "%d %s %d", &c.id, c.name, &c.votes) != EOF) {
        if (c.votes > winner.votes) winner = c;
    }
    fclose(fp);
    if (winner.votes != -1)
        printf("\nLeader: %s (%d votes)\n", winner.name, winner.votes);
}

int main() {
    int choice, id;
    while (1) {
        printf("\n======= ELECTION SYSTEM (2026) =======\n");
        printf("1. Register Voter\n");
        printf("2. Add ID to Queue\n");
        printf("3. Cast Vote\n");
        printf("4. Show Candidates\n");
        printf("5. Show Winner\n");
        printf("6. Exit\n");
        printf("Choice: ");
        if (scanf("%d", &choice) != 1) {
            while(getchar() != '\n'); 
            continue;
        }

        switch (choice) {
            case 1: registerVoter(); break;
            case 2: printf("Enter Voter ID: "); scanf("%d", &id); enqueue(id); break;
            case 3: castVote(); break;
            case 4: showCandidates(); break;
            case 5: showWinner(); break;
            case 6: exit(0);
        }
    }
    return 0;
}