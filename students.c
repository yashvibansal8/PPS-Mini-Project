#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX       100
#define SUBJECTS  3
#define FILE_NAME "students.txt"

struct Student {
    int   roll;
    char  name[50];
    float marks[SUBJECTS];
    float total;
    float percentage;
    char  grade;
};

struct Student list[MAX];   /* array of structures */
int count = 0;              /* number of students stored */

/* ---------- Input helpers ---------- */

void getText(const char *prompt, char *buf, int size) {
    printf("%s", prompt);
    if (fgets(buf, size, stdin) == NULL) exit(0);   /* end of input */
    buf[strcspn(buf, "\n")] = '\0';                 /* remove newline */
}

int getInt(const char *prompt, int min, int max) {
    char line[50];
    int n;
    while (1) {
        getText(prompt, line, sizeof line);
        if (sscanf(line, "%d", &n) == 1 && n >= min && n <= max)
            return n;
        printf("  Enter a number from %d to %d.\n", min, max);
    }
}

float getFloat(const char *prompt, float min, float max) {
    char line[50];
    float n;
    while (1) {
        getText(prompt, line, sizeof line);
        if (sscanf(line, "%f", &n) == 1 && n >= min && n <= max)
            return n;
        printf("  Enter a number from %.0f to %.0f.\n", min, max);
    }
}

/* ---------- Core functions ---------- */

void calculate(struct Student *s) {
    s->total = 0;
    for (int i = 0; i < SUBJECTS; i++)
        s->total += s->marks[i];
    s->percentage = s->total / SUBJECTS;

    if      (s->percentage >= 90) s->grade = 'A';
    else if (s->percentage >= 75) s->grade = 'B';
    else if (s->percentage >= 60) s->grade = 'C';
    else if (s->percentage >= 40) s->grade = 'D';
    else                          s->grade = 'F';
}

int findByRoll(int roll) {
    for (int i = 0; i < count; i++)
        if (list[i].roll == roll) return i;
    return -1;   /* not found */
}

void saveFile(void) {
    FILE *fp = fopen(FILE_NAME, "w");
    if (fp == NULL) { printf("Could not save file!\n"); return; }
    for (int i = 0; i < count; i++)
        fprintf(fp, "%d %.1f %.1f %.1f %s\n", list[i].roll,
                list[i].marks[0], list[i].marks[1], list[i].marks[2],
                list[i].name);
    fclose(fp);
}

void loadFile(void) {
    FILE *fp = fopen(FILE_NAME, "r");
    if (fp == NULL) return;   /* first run: no file yet */
    struct Student s;
    while (count < MAX &&
           fscanf(fp, "%d %f %f %f %49[^\n]", &s.roll, &s.marks[0],
                  &s.marks[1], &s.marks[2], s.name) == 5) {
        calculate(&s);
        list[count++] = s;
    }
    fclose(fp);
}

void printHeader(void) {
    printf("\n%-6s %-20s %8s %8s %6s\n", "Roll", "Name", "Total", "Perc%", "Grade");
    printf("-----------------------------------------------------\n");
}

void printStudent(const struct Student *s) {
    printf("%-6d %-20s %8.1f %8.2f %6c\n",
           s->roll, s->name, s->total, s->percentage, s->grade);
}

/* ---------- Menu operations ---------- */

void addStudent(void) {
    if (count == MAX) { printf("List is full!\n"); return; }

    struct Student s;
    s.roll = getInt("Roll number: ", 1, 99999);
    if (findByRoll(s.roll) != -1) {
        printf("Roll number already exists.\n");
        return;
    }
    getText("Name: ", s.name, sizeof s.name);
    if (s.name[0] == '\0') { printf("Name cannot be empty.\n"); return; }

    for (int i = 0; i < SUBJECTS; i++) {
        char prompt[40];
        sprintf(prompt, "Marks in subject %d (0-100): ", i + 1);
        s.marks[i] = getFloat(prompt, 0, 100);
    }
    calculate(&s);

    list[count++] = s;
    saveFile();
    printf("Student added.\n");
}

void displayAll(void) {
    if (count == 0) { printf("\nNo records found.\n"); return; }
    printHeader();
    for (int i = 0; i < count; i++)
        printStudent(&list[i]);
    printf("\nTotal students: %d\n", count);
}

void searchStudent(void) {
    printf("Search by: 1) Roll number  2) Name\n");
    int choice = getInt("Choice: ", 1, 2);
    int found = 0;

    if (choice == 1) {
        int i = findByRoll(getInt("Roll number: ", 1, 99999));
        if (i != -1) {
            printHeader();
            printStudent(&list[i]);
            found = 1;
        }
    } else {
        char key[50];
        getText("Name (or part of it): ", key, sizeof key);
        for (int i = 0; i < count; i++) {
            if (strstr(list[i].name, key) != NULL) {   /* substring match */
                if (!found) printHeader();
                printStudent(&list[i]);
                found = 1;
            }
        }
    }
    if (!found) printf("No matching record.\n");
}

void editStudent(void) {
    int i = findByRoll(getInt("Roll number to edit: ", 1, 99999));
    if (i == -1) { printf("Student not found.\n"); return; }

    char name[50];
    printf("Current name: %s\n", list[i].name);
    getText("New name (blank = keep): ", name, sizeof name);
    if (name[0] != '\0') strcpy(list[i].name, name);

    for (int j = 0; j < SUBJECTS; j++) {
        char prompt[50];
        sprintf(prompt, "Subject %d (was %.1f): ", j + 1, list[i].marks[j]);
        list[i].marks[j] = getFloat(prompt, 0, 100);
    }
    calculate(&list[i]);
    saveFile();
    printf("Record updated.\n");
}

void deleteStudent(void) {
    int i = findByRoll(getInt("Roll number to delete: ", 1, 99999));
    if (i == -1) { printf("Student not found.\n"); return; }

    for (int j = i; j < count - 1; j++)   /* shift records left */
        list[j] = list[j + 1];
    count--;
    saveFile();
    printf("Record deleted.\n");
}

void sortStudents(void) {
    /* bubble sort: highest total first */
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - 1 - i; j++) {
            if (list[j].total < list[j + 1].total) {
                struct Student temp = list[j];
                list[j] = list[j + 1];
                list[j + 1] = temp;
            }
        }
    }
    saveFile();
    printf("Sorted by marks (high to low).\n");
    displayAll();
}

void report(void) {
    if (count == 0) { printf("No records.\n"); return; }

    int top = 0, low = 0;
    float sum = 0;
    for (int i = 0; i < count; i++) {
        if (list[i].total > list[top].total) top = i;
        if (list[i].total < list[low].total) low = i;
        sum += list[i].percentage;
    }
    printf("\n===== REPORT =====\n");
    printf("Class average : %.2f%%\n", sum / count);
    printf("Topper        : %s (%.2f%%)\n", list[top].name, list[top].percentage);
    printf("Lowest        : %s (%.2f%%)\n", list[low].name, list[low].percentage);
}

/* ---------- Main ---------- */

int main(void) {
    int choice;
    loadFile();

    do {
        printf("\n===== STUDENT RECORD SYSTEM =====\n");
        printf("1. Add student\n2. Display all\n3. Search\n4. Edit\n");
        printf("5. Delete\n6. Sort by marks\n7. Report\n0. Exit\n");
        choice = getInt("Choice: ", 0, 7);

        switch (choice) {
            case 1: addStudent();    break;
            case 2: displayAll();    break;
            case 3: searchStudent(); break;
            case 4: editStudent();   break;
            case 5: deleteStudent(); break;
            case 6: sortStudents();  break;
            case 7: report();        break;
            case 0: printf("Goodbye!\n"); break;
        }
    } while (choice != 0);

    return 0;
}