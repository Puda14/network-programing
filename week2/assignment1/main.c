#include <stdio.h>

typedef struct student {
    char name[20];
    int eng;
    int math;
    int phys;
    double mean;
} STUDENT;

void calculate_mean(STUDENT* s) {
    s->mean = (s->eng + s->math + s->phys) / 3.0;
}

char get_grade(double mean) {
    if (mean >= 90 && mean <= 100) return 'S';
    else if (mean >= 80 && mean < 90) return 'A';
    else if (mean >= 70 && mean < 80) return 'B';
    else if (mean >= 60 && mean < 70) return 'C';
    else return 'D';
}

void print_students(STUDENT* p, int size) {
  printf("=========================\n");
  for (int i = 0; i < size; i++) {
    calculate_mean(&p[i]);

    printf("Student: %s\n", p[i].name);
    printf("English: %d, Math: %d, Physics: %d\n", p[i].eng, p[i].math, p[i].phys);
    printf("Mean Score: %.2f\n", p[i].mean);
    printf("Grade: %c\n\n", get_grade(p[i].mean));
  }
  printf("=========================\n");
}


int main() {
  STUDENT data[] = {
    {"Tuan", 82, 72, 58, 0.0},
    {"Nam", 77, 82, 79, 0.0},
    {"Khanh", 52, 62, 39, 0.0},
    {"Phuong", 61, 82, 88, 0.0}
  };

  STUDENT* p = data;

  int size = sizeof(data) / sizeof(data[0]);

  print_students(p, size);

  return 0;
}
