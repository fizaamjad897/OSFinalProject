#include <stdio.h>

int main() {
    int memoryRequired;
    printf("Executing Memory allocating task");
    printf("Enter memory size to allocate (in GB): ");
    scanf("%d", &memoryRequired);
    printf("%d", memoryRequired);
    fflush(stdout); // Print memory requirement to be captured by the main program
    return 0;
}
