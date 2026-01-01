/*
 * Virtual Memory Paging Simulator
 * OS Lab Semester Project (Group of 3)
 * 
 * Group Members:
 * 1. [Student 1 Name] - Roll No: [xxx] (FIFO, LRU Implementation)
 * 2. [Student 2 Name] - Roll No: [xxx] (Optimal, Second Chance Implementation)  
 * 3. [Student 3 Name] - Roll No: [xxx] (File I/O, Comparison Mode, Reports)
 *
 * Features:
 * - 4 Page Replacement Algorithms
 * - File Input/Output Support
 * - Algorithm Comparison Mode
 * - Step-by-Step Analysis: history tracking
 * - Performance Report Generation
 * - Multiple test cases: 4 different input methods supported
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#define MAX_FRAMES 10
#define MAX_PAGES 50

typedef struct {
    int pageFaults;
    int pageHits;
    float hitRatio;
    float faultRatio;
    char algorithmName[30];
} AlgorithmStats;

typedef struct {
    int frameState[MAX_PAGES][MAX_FRAMES];
    char status[MAX_PAGES][10];
    int steps;
} SimulationHistory;

int frames[MAX_FRAMES];
int pageRefs[MAX_PAGES];
int numFrames, numPages;
int pageFaults = 0, pageHits = 0;
SimulationHistory history;

void displayWelcome();
void displayMainMenu();
int getChoice();
void inputFromKeyboard();
void inputFromFile();
void generateRandomInput();
void saveInputToFile();
void fifoAlgorithm();
void lruAlgorithm();
void optimalAlgorithm();
void secondChanceAlgorithm();
void compareAllAlgorithms();
void generateDetailedReport();
void showStats();
void printFrames();
void saveFrameState(int step);
void displayHistory();
int searchPage(int page);
void resetCounters();
void resetFrames();
void clearScreen();

int main() {
    int choice, algoChoice;
    
    displayWelcome();
    
    while(1) {
        displayMainMenu();
        choice = getChoice();
        
        switch(choice) {
            case 1:
                inputFromKeyboard();
                break;
            case 2:
                inputFromFile();
                break;
            case 3:
                generateRandomInput();
                break;
            case 4:
                saveInputToFile();
                break;
            case 5:
                if(numPages == 0) {
                    printf("\nError: No input data! Please enter data first.\n");
                    break;
                }
                
                printf("\n--- Select Algorithm ---\n");
                printf("1. FIFO\n2. LRU\n3. Optimal\n4. Second Chance\n");
                printf("Enter choice: ");
                scanf("%d", &algoChoice);
                
                resetCounters();
                resetFrames();
                history.steps = 0;
                
                switch(algoChoice) {
                    case 1: fifoAlgorithm(); break;
                    case 2: lruAlgorithm(); break;
                    case 3: optimalAlgorithm(); break;
                    case 4: secondChanceAlgorithm(); break;
                    default: printf("Invalid choice!\n");
                }
                break;
                
            case 6:
                if(numPages == 0) {
                    printf("\nError: No input data! Please enter data first.\n");
                    break;
                }
                compareAllAlgorithms();
                break;
                
            case 7:
                displayHistory();
                break;
                
            case 8:
                generateDetailedReport();
                break;
                
            case 9:
                printf("\n========================================\n");
                printf("Thank you for using the simulator!\n");
                printf("Project by: [Group Member Names]\n");
                printf("========================================\n");
                return 0;
                
            default:
                printf("\nInvalid choice! Please try again.\n");
        }
        
        printf("\nPress Enter to continue...");
        getchar();
        getchar();
    }
    
    return 0;
}

void displayWelcome() {
    printf("\n");
    printf("*********************************************************\n");
    printf("*                                                       *\n");
    printf("*      VIRTUAL MEMORY PAGING SIMULATOR v2.0            *\n");
    printf("*      Operating Systems Lab - Group Project           *\n");
    printf("*                                                       *\n");
    printf("*********************************************************\n");
    printf("\n");
}

void displayMainMenu() {
    printf("\n");
    printf("========================================\n");
    printf("           MAIN MENU\n");
    printf("========================================\n");
    printf("1. Enter Input from Keyboard\n");
    printf("2. Load Input from File\n");
    printf("3. Generate Random Input\n");
    printf("4. Save Current Input to File\n");
    printf("5. Run Single Algorithm\n");
    printf("6. Compare All Algorithms\n");
    printf("7. View Simulation History\n");
    printf("8. Generate Performance Report\n");
    printf("9. Exit\n");
    printf("========================================\n");
}

int getChoice() {
    int ch;
    printf("Enter your choice: ");
    scanf("%d", &ch);
    return ch;
}

void inputFromKeyboard() {
    int i;
    
    printf("\n--- Manual Input ---\n");
    printf("Enter number of frames (1-10): ");
    scanf("%d", &numFrames);
    
    if(numFrames < 1 || numFrames > MAX_FRAMES) {
        printf("Invalid! Using default 3 frames.\n");
        numFrames = 3;
    }
    
    printf("Enter number of pages (1-50): ");
    scanf("%d", &numPages);
    
    if(numPages < 1 || numPages > MAX_PAGES) {
        printf("Invalid! Using default 10 pages.\n");
        numPages = 10;
    }
    
    printf("Enter page reference string:\n");
    for(i = 0; i < numPages; i++) {
        printf("Page %d: ", i+1);
        scanf("%d", &pageRefs[i]);
        if(pageRefs[i] < 0) {
            printf("Warning: Negative page number entered. Using absolute value.\n");
            pageRefs[i] = abs(pageRefs[i]);
        }
    }
    
    printf("\nInput accepted successfully!\n");
    printf("Reference String: ");
    for(i = 0; i < numPages; i++) {
        printf("%d ", pageRefs[i]);
    }
    printf("\n");
}

void inputFromFile() {
    FILE *fp;
    char filename[50];
    int i;
    
    printf("\n--- Load from File ---\n");
    printf("Enter filename (e.g., input.txt): ");
    scanf("%s", filename);
    
    fp = fopen(filename, "r");
    if(fp == NULL) {
        printf("Error: Cannot open file %s (File not found or permission denied)\n", filename);
        return;
    }
    
    if(fscanf(fp, "%d", &numFrames) != 1) {
        printf("Error: Invalid file format. Expected number of frames.\n");
        fclose(fp);
        return;
    }
    
    if(fscanf(fp, "%d", &numPages) != 1) {
        printf("Error: Invalid file format. Expected number of pages.\n");
        fclose(fp);
        return;
    }
    
    if(numFrames < 1 || numFrames > MAX_FRAMES) {
        printf("Error: Invalid number of frames in file (%d). Must be between 1 and %d.\n", numFrames, MAX_FRAMES);
        fclose(fp);
        return;
    }
    
    if(numPages < 1 || numPages > MAX_PAGES) {
        printf("Error: Invalid number of pages in file (%d). Must be between 1 and %d.\n", numPages, MAX_PAGES);
        fclose(fp);
        return;
    }
    
    for(i = 0; i < numPages; i++) {
        if(fscanf(fp, "%d", &pageRefs[i]) != 1) {
            printf("Error: Invalid file format. Not enough page references (expected %d, got %d).\n", numPages, i);
            fclose(fp);
            return;
        }
        if(pageRefs[i] < 0) {
            printf("Warning: Negative page number found at position %d. Using absolute value.\n", i+1);
            pageRefs[i] = abs(pageRefs[i]);
        }
    }
    
    fclose(fp);
    
    printf("\nFile loaded successfully!\n");
    printf("Frames: %d\n", numFrames);
    printf("Pages: %d\n", numPages);
    printf("Reference String: ");
    for(i = 0; i < numPages; i++) {
        printf("%d ", pageRefs[i]);
    }
    printf("\n");
}

void generateRandomInput() {
    int i, maxPage;
    
    printf("\n--- Generate Random Input ---\n");
    printf("Enter number of frames: ");
    scanf("%d", &numFrames);
    
    if(numFrames < 1 || numFrames > MAX_FRAMES) {
        printf("Error: Invalid number of frames. Must be between 1 and %d. Using default 3.\n", MAX_FRAMES);
        numFrames = 3;
    }
    
    printf("Enter number of page references: ");
    scanf("%d", &numPages);
    
    if(numPages < 1 || numPages > MAX_PAGES) {
        printf("Error: Invalid number of pages. Must be between 1 and %d. Using default 10.\n", MAX_PAGES);
        numPages = 10;
    }
    
    printf("Enter maximum page number: ");
    scanf("%d", &maxPage);
    
    if(maxPage <= 0) {
        printf("Error: Maximum page number must be > 0. Using default 9.\n");
        maxPage = 9;
    }
    
    srand(time(NULL));
    
    for(i = 0; i < numPages; i++) {
        pageRefs[i] = rand() % (maxPage + 1);
    }
    
    printf("\nRandom input generated!\n");
    printf("Reference String: ");
    for(i = 0; i < numPages; i++) {
        printf("%d ", pageRefs[i]);
    }
    printf("\n");
}

void saveInputToFile() {
    FILE *fp;
    char filename[50];
    int i;
    
    if(numPages == 0) {
        printf("\nNo data to save!\n");
        return;
    }
    
    printf("\n--- Save to File ---\n");
    printf("Enter filename to save (e.g., output.txt): ");
    scanf("%s", filename);
    
    fp = fopen(filename, "w");
    if(fp == NULL) {
        printf("Error: Cannot create file\n");
        return;
    }
    
    fprintf(fp, "%d\n", numFrames);
    fprintf(fp, "%d\n", numPages);
    
    for(i = 0; i < numPages; i++) {
        fprintf(fp, "%d ", pageRefs[i]);
    }
    
    fclose(fp);
    printf("Data saved to %s successfully!\n", filename);
}

void resetCounters() {
    pageFaults = 0;
    pageHits = 0;
}

void resetFrames() {
    int i;
    for(i = 0; i < MAX_FRAMES; i++) {
        frames[i] = -1;
    }
}

void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

int searchPage(int page) {
    int i;
    for(i = 0; i < numFrames; i++) {
        if(frames[i] == page) {
            return i;
        }
    }
    return -1;
}

void printFrames() {
    int i;
    printf("[");
    for(i = 0; i < numFrames; i++) {
        if(frames[i] == -1)
            printf(" - ");
        else
            printf(" %d ", frames[i]);
    }
    printf("]");
}

void saveFrameState(int step) {
    int i;
    if(step >= MAX_PAGES) {
        printf("Warning: Maximum history steps reached. Some history may be lost.\n");
        return;
    }
    for(i = 0; i < numFrames && i < MAX_FRAMES; i++) {
        history.frameState[step][i] = frames[i];
    }
    history.steps = step + 1;
}

void displayHistory() {
    int i, j;
    
    if(history.steps == 0) {
        printf("\nNo simulation history available!\n");
        return;
    }
    
    printf("\n========================================\n");
    printf("     SIMULATION HISTORY\n");
    printf("========================================\n");
    
    printf("\nStep | Page | Status | Frame Contents\n");
    printf("-----|------|--------|----------------\n");
    
    for(i = 0; i < history.steps; i++) {
        printf("%4d | %4d | %6s | ", i+1, pageRefs[i], history.status[i]);
        printf("[");
        for(j = 0; j < numFrames; j++) {
            if(history.frameState[i][j] == -1)
                printf(" - ");
            else
                printf(" %d ", history.frameState[i][j]);
        }
        printf("]\n");
    }
}

void fifoAlgorithm() {
    int i;
    int position = 0;
    int filled = 0;
    
    printf("\n========================================\n");
    printf("   FIFO Algorithm Simulation\n");
    printf("========================================\n");
    printf("\nStep\tPage\tStatus\t\tFrames\n");
    printf("----\t----\t------\t\t------\n");
    
    for(i = 0; i < numPages; i++) {
        int currentPage = pageRefs[i];
        
        printf("%d\t%d\t", i+1, currentPage);
        
        if(searchPage(currentPage) != -1) {
            printf("HIT\t\t");
            pageHits++;
            strcpy(history.status[i], "HIT");
        }
        else {
            printf("FAULT\t\t");
            pageFaults++;
            strcpy(history.status[i], "FAULT");
            
            if(filled < numFrames) {
                frames[filled] = currentPage;
                filled++;
            }
            else {
                frames[position] = currentPage;
                position = (position + 1) % numFrames;
            }
        }
        
        printFrames();
        printf("\n");
        saveFrameState(i);
    }
    
    showStats();
}

void lruAlgorithm() {
    int i, j;
    int filled = 0;
    int recent[MAX_FRAMES];
    
    for(i = 0; i < MAX_FRAMES; i++) {
        recent[i] = -1;
    }
    
    printf("\n========================================\n");
    printf("   LRU Algorithm Simulation\n");
    printf("========================================\n");
    printf("\nStep\tPage\tStatus\t\tFrames\n");
    printf("----\t----\t------\t\t------\n");
    
    for(i = 0; i < numPages; i++) {
        int currentPage = pageRefs[i];
        int pos = searchPage(currentPage);
        
        printf("%d\t%d\t", i+1, currentPage);
        
        if(pos != -1) {
            printf("HIT\t\t");
            pageHits++;
            recent[pos] = i;
            strcpy(history.status[i], "HIT");
        }
        else {
            printf("FAULT\t\t");
            pageFaults++;
            strcpy(history.status[i], "FAULT");
            
            if(filled < numFrames) {
                frames[filled] = currentPage;
                recent[filled] = i;
                filled++;
            }
            else {
                int lruPos = 0;
                int minTime = recent[0];
                
                for(j = 1; j < numFrames; j++) {
                    if(recent[j] < minTime) {
                        minTime = recent[j];
                        lruPos = j;
                    }
                }
                
                frames[lruPos] = currentPage;
                recent[lruPos] = i;
            }
        }
        
        printFrames();
        printf("\n");
        saveFrameState(i);
    }
    
    showStats();
}

void optimalAlgorithm() {
    int i, j, k;
    int filled = 0;
    
    printf("\n========================================\n");
    printf("   Optimal Algorithm Simulation\n");
    printf("========================================\n");
    printf("\nStep\tPage\tStatus\t\tFrames\n");
    printf("----\t----\t------\t\t------\n");
    
    for(i = 0; i < numPages; i++) {
        int currentPage = pageRefs[i];
        
        printf("%d\t%d\t", i+1, currentPage);
        
        if(searchPage(currentPage) != -1) {
            printf("HIT\t\t");
            pageHits++;
            strcpy(history.status[i], "HIT");
        }
        else {
            printf("FAULT\t\t");
            pageFaults++;
            strcpy(history.status[i], "FAULT");
            
            if(filled < numFrames) {
                frames[filled] = currentPage;
                filled++;
            }
            else {
                int replacePos = 0;
                int farthest = i + 1;
                
                for(j = 0; j < numFrames; j++) {
                    int nextUse = -1;
                    
                    for(k = i + 1; k < numPages; k++) {
                        if(frames[j] == pageRefs[k]) {
                            nextUse = k;
                            break;
                        }
                    }
                    
                    if(nextUse == -1) {
                        replacePos = j;
                        break;
                    }
                    
                    if(nextUse > farthest) {
                        farthest = nextUse;
                        replacePos = j;
                    }
                }
                
                frames[replacePos] = currentPage;
            }
        }
        
        printFrames();
        printf("\n");
        saveFrameState(i);
    }
    
    showStats();
}

void secondChanceAlgorithm() {
    int i;
    int filled = 0;
    int pointer = 0;
    int referenceBit[MAX_FRAMES];
    
    for(i = 0; i < MAX_FRAMES; i++) {
        referenceBit[i] = 0;
    }
    
    printf("\n========================================\n");
    printf("   Second Chance Algorithm Simulation\n");
    printf("========================================\n");
    printf("\nStep\tPage\tStatus\t\tFrames\n");
    printf("----\t----\t------\t\t------\n");
    
    for(i = 0; i < numPages; i++) {
        int currentPage = pageRefs[i];
        int pos = searchPage(currentPage);
        
        printf("%d\t%d\t", i+1, currentPage);
        
        if(pos != -1) {
            printf("HIT\t\t");
            pageHits++;
            referenceBit[pos] = 1;
            strcpy(history.status[i], "HIT");
        }
        else {
            printf("FAULT\t\t");
            pageFaults++;
            strcpy(history.status[i], "FAULT");
            
            if(filled < numFrames) {
                frames[filled] = currentPage;
                referenceBit[filled] = 1;
                filled++;
            }
            else {
                while(1) {
                    if(referenceBit[pointer] == 0) {
                        frames[pointer] = currentPage;
                        referenceBit[pointer] = 1;
                        pointer = (pointer + 1) % numFrames;
                        break;
                    }
                    else {
                        referenceBit[pointer] = 0;
                        pointer = (pointer + 1) % numFrames;
                    }
                }
            }
        }
        
        printFrames();
        printf("\n");
        saveFrameState(i);
    }
    
    showStats();
}

void compareAllAlgorithms() {
    AlgorithmStats stats[4];
    int i;
    
    printf("\n========================================\n");
    printf("   COMPARING ALL ALGORITHMS\n");
    printf("========================================\n");
    printf("\nPlease wait...\n\n");
    
    resetCounters();
    resetFrames();
    fifoAlgorithm();
    stats[0].pageFaults = pageFaults;
    stats[0].pageHits = pageHits;
    stats[0].hitRatio = (float)pageHits / numPages * 100;
    stats[0].faultRatio = (float)pageFaults / numPages * 100;
    strcpy(stats[0].algorithmName, "FIFO");
    
    printf("\n--- Press Enter for next algorithm ---");
    fflush(stdin);
    getchar();
    
    resetCounters();
    resetFrames();
    lruAlgorithm();
    stats[1].pageFaults = pageFaults;
    stats[1].pageHits = pageHits;
    stats[1].hitRatio = (float)pageHits / numPages * 100;
    stats[1].faultRatio = (float)pageFaults / numPages * 100;
    strcpy(stats[1].algorithmName, "LRU");
    
    printf("\n--- Press Enter for next algorithm ---");
    fflush(stdin);
    getchar();
    
    resetCounters();
    resetFrames();
    optimalAlgorithm();
    stats[2].pageFaults = pageFaults;
    stats[2].pageHits = pageHits;
    stats[2].hitRatio = (float)pageHits / numPages * 100;
    stats[2].faultRatio = (float)pageFaults / numPages * 100;
    strcpy(stats[2].algorithmName, "Optimal");
    
    printf("\n--- Press Enter for next algorithm ---");
    fflush(stdin);
    getchar();
    
    resetCounters();
    resetFrames();
    secondChanceAlgorithm();
    stats[3].pageFaults = pageFaults;
    stats[3].pageHits = pageHits;
    stats[3].hitRatio = (float)pageHits / numPages * 100;
    stats[3].faultRatio = (float)pageFaults / numPages * 100;
    strcpy(stats[3].algorithmName, "Second Chance");
    
    printf("\n\n========================================\n");
    printf("     COMPARISON SUMMARY\n");
    printf("========================================\n");
    printf("\nAlgorithm\t\tFaults\tHits\tFault%%\n");
    printf("------------------------------------------------\n");
    
    for(i = 0; i < 4; i++) {
        printf("%-20s\t%d\t%d\t%.2f%%\n", 
               stats[i].algorithmName, 
               stats[i].pageFaults, 
               stats[i].pageHits,
               stats[i].faultRatio);
    }
    
    int minFaults = stats[0].pageFaults;
    int bestAlgo = 0;
    
    for(i = 1; i < 4; i++) {
        if(stats[i].pageFaults < minFaults) {
            minFaults = stats[i].pageFaults;
            bestAlgo = i;
        }
    }
    
    printf("\n>>> Best Algorithm: %s (Minimum Faults: %d)\n", 
           stats[bestAlgo].algorithmName, minFaults);
    printf("========================================\n");
}

void generateDetailedReport() {
    FILE *fp;
    char filename[50];
    time_t t;
    struct tm *timeinfo;
    char timeStr[100];
    int i;
    AlgorithmStats stats[4];
    
    if(numPages == 0) {
        printf("\nNo data available for report!\n");
        return;
    }
    
    printf("\n--- Generate Report ---\n");
    printf("Enter report filename (e.g., report.txt): ");
    scanf("%s", filename);
    
    fp = fopen(filename, "w");
    if(fp == NULL) {
        printf("Error: Cannot create report file\n");
        return;
    }
    
    time(&t);
    timeinfo = localtime(&t);
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeinfo);
    
    printf("\nGenerating report... Running all algorithms...\n");
    
    resetCounters();
    resetFrames();
    fifoAlgorithm();
    stats[0].pageFaults = pageFaults;
    stats[0].pageHits = pageHits;
    stats[0].hitRatio = (float)pageHits / numPages * 100;
    stats[0].faultRatio = (float)pageFaults / numPages * 100;
    strcpy(stats[0].algorithmName, "FIFO");
    
    resetCounters();
    resetFrames();
    lruAlgorithm();
    stats[1].pageFaults = pageFaults;
    stats[1].pageHits = pageHits;
    stats[1].hitRatio = (float)pageHits / numPages * 100;
    stats[1].faultRatio = (float)pageFaults / numPages * 100;
    strcpy(stats[1].algorithmName, "LRU");
    
    resetCounters();
    resetFrames();
    optimalAlgorithm();
    stats[2].pageFaults = pageFaults;
    stats[2].pageHits = pageHits;
    stats[2].hitRatio = (float)pageHits / numPages * 100;
    stats[2].faultRatio = (float)pageFaults / numPages * 100;
    strcpy(stats[2].algorithmName, "Optimal");
    
    resetCounters();
    resetFrames();
    secondChanceAlgorithm();
    stats[3].pageFaults = pageFaults;
    stats[3].pageHits = pageHits;
    stats[3].hitRatio = (float)pageHits / numPages * 100;
    stats[3].faultRatio = (float)pageFaults / numPages * 100;
    strcpy(stats[3].algorithmName, "Second Chance");
    
    int minFaults = stats[0].pageFaults;
    int bestAlgo = 0;
    for(i = 1; i < 4; i++) {
        if(stats[i].pageFaults < minFaults) {
            minFaults = stats[i].pageFaults;
            bestAlgo = i;
        }
    }
    
    fprintf(fp, "========================================\n");
    fprintf(fp, "  VIRTUAL MEMORY PAGING SIMULATOR\n");
    fprintf(fp, "     PERFORMANCE REPORT\n");
    fprintf(fp, "========================================\n\n");
    fprintf(fp, "Generated on: %s\n", timeStr);
    fprintf(fp, "Group Members: [Names]\n\n");
    
    fprintf(fp, "INPUT CONFIGURATION:\n");
    fprintf(fp, "-------------------\n");
    fprintf(fp, "Number of Frames: %d\n", numFrames);
    fprintf(fp, "Number of Pages: %d\n", numPages);
    fprintf(fp, "Reference String: ");
    for(i = 0; i < numPages; i++) {
        fprintf(fp, "%d ", pageRefs[i]);
    }
    fprintf(fp, "\n\n");
    
    fprintf(fp, "ALGORITHM COMPARISON:\n");
    fprintf(fp, "--------------------\n");
    fprintf(fp, "%-20s | %8s | %8s | %10s | %10s\n", "Algorithm", "Faults", "Hits", "Fault %%", "Hit %%");
    fprintf(fp, "---------------------|----------|----------|------------|------------\n");
    
    for(i = 0; i < 4; i++) {
        fprintf(fp, "%-20s | %8d | %8d | %9.2f%% | %9.2f%%\n",
               stats[i].algorithmName,
               stats[i].pageFaults,
               stats[i].pageHits,
               stats[i].faultRatio,
               stats[i].hitRatio);
    }
    
    fprintf(fp, "\n");
    fprintf(fp, ">>> Best Algorithm: %s (Minimum Faults: %d, Hit Ratio: %.2f%%)\n",
           stats[bestAlgo].algorithmName, minFaults, stats[bestAlgo].hitRatio);
    
    fprintf(fp, "\nDETAILED STATISTICS:\n");
    fprintf(fp, "-------------------\n");
    for(i = 0; i < 4; i++) {
        fprintf(fp, "\n%s Algorithm:\n", stats[i].algorithmName);
        fprintf(fp, "  Total Page References: %d\n", numPages);
        fprintf(fp, "  Total Page Faults:     %d\n", stats[i].pageFaults);
        fprintf(fp, "  Total Page Hits:       %d\n", stats[i].pageHits);
        fprintf(fp, "  Page Fault Ratio:      %.2f%%\n", stats[i].faultRatio);
        fprintf(fp, "  Page Hit Ratio:        %.2f%%\n", stats[i].hitRatio);
    }
    fprintf(fp, "\nPERFORMANCE ANALYSIS:\n");
    fprintf(fp, "--------------------\n");
    fprintf(fp, "Optimal Algorithm Performance: %d faults (theoretical best)\n", stats[2].pageFaults);
    for(i = 0; i < 4; i++) {
        if(i != 2) {  // Skip Optimal in comparison
            float efficiency = ((float)(stats[2].pageFaults - stats[i].pageFaults) / stats[2].pageFaults) * 100;
            if(efficiency < 0) efficiency = 0;
            fprintf(fp, "%s vs Optimal: %.2f%% efficiency (", stats[i].algorithmName, efficiency);
            if(stats[i].pageFaults == stats[2].pageFaults) {
                fprintf(fp, "Equal to optimal)\n");
            } else {
                fprintf(fp, "%d more faults)\n", stats[i].pageFaults - stats[2].pageFaults);
            }
        }
    }
    
    fprintf(fp, "\n========================================\n");
    fprintf(fp, "End of Report\n");
    fprintf(fp, "========================================\n");
    
    fclose(fp);
    
    printf("\nReport generated successfully: %s\n", filename);
    printf("Report includes comparison of all 4 algorithms with detailed statistics.\n");
}

void showStats() {
    float hitRatio, faultRatio;
    
    hitRatio = (float)pageHits / numPages * 100;
    faultRatio = (float)pageFaults / numPages * 100;
    
    printf("\n========================================\n");
    printf("        SIMULATION STATISTICS\n");
    printf("========================================\n");
    printf("Total Page References : %d\n", numPages);
    printf("Total Page Faults     : %d\n", pageFaults);
    printf("Total Page Hits       : %d\n", pageHits);
    printf("Page Fault Ratio      : %.2f%%\n", faultRatio);
    printf("Page Hit Ratio        : %.2f%%\n", hitRatio);
    printf("========================================\n");
}