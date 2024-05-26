#include <stdio.h>

int main() {
    int fileSize;
    printf("Enter file size to access (in GB): ");
    scanf("%d", &fileSize);
    printf("%d", fileSize); // Print file size requirement to be captured by the main program
    return 0;
}
