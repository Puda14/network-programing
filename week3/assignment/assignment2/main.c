#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

// Function to print a byte in binary format
void print_binary(unsigned char byte) {
    for (int i = 7; i >= 0; i--) {
        printf("%d", (byte >> i) & 1);  // Extract each bit and print
    }
    printf(" ");
}

void convert_ipv4() {
    char ip_str[INET_ADDRSTRLEN];  // Buffer to store the user input IP address as a string
    struct in_addr ipv4_addr;      // Structure to store the IPv4 address in binary format

    // Prompt the user to enter an IPv4 address
    printf("Enter an IPv4 address: ");
    scanf("%s", ip_str);

    // Convert the IPv4 address from string format to binary format
    if (inet_pton(AF_INET, ip_str, &ipv4_addr) == 1) {
        printf("inet_pton (IPv4): Successfully converted IP address: %s\n", ip_str);
    } else {
        // If conversion fails, print error and exit
        printf("inet_pton (IPv4): Failed to convert IP address: %s\n", ip_str);
        exit(EXIT_FAILURE);
    }

    // Display the binary form of the IP address as bytes
    unsigned char *bytes = (unsigned char *)&ipv4_addr;
    printf("IP address in binary form: ");
    for (int i = 0; i < sizeof(ipv4_addr); i++) {
        print_binary(bytes[i]);
    }
    printf("\n");

    char ip_str_converted[INET_ADDRSTRLEN];  // Buffer to hold the IP address converted back to string format

    // Convert the binary IPv4 address back to string format
    if (inet_ntop(AF_INET, &ipv4_addr, ip_str_converted, INET_ADDRSTRLEN)) {
        printf("inet_ntop (IPv4): IP address converted back to string: %s\n", ip_str_converted);
    } else {
        // If conversion back to string fails, print error and exit
        printf("inet_ntop (IPv4): Failed to convert IP address back to string\n");
        exit(EXIT_FAILURE);
    }
}

int main() {
    // Perform the IPv4 address conversion and display
    convert_ipv4();

    return 0;
}
