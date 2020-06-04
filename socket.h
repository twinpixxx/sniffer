#include <stdlib.h> /* exit */
#include <sys/socket.h>
#include <sys/ioctl.h> // SIOCGIFINDEX
#include <netinet/if_ether.h> // ETH_P_IP, ethhdr
#include <net/if.h> // ifreq
#include <netdb.h> // gethostbyname
#include <linux/if_packet.h> // sockaddr_ll
#include <arpa/inet.h>
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

// Create a RAW Socket
SOCKET create_raw_socket()
{
	SOCKET raw_socket;

    raw_socket = socket(AF_PACKET, SOCK_RAW, IPPROTO_TCP);

    if (raw_socket < 0)
    {
        printf("Failed to create raw socket.\n");
        printf("Administrator privilege are required to continue.\n");
        exit(1);
    }

	return raw_socket;
}

// Assigning a name to a socket
void bind_socket(SOCKET raw_socket, struct hostent *local_host)
{
    struct sockaddr_ll sll;//sockaddr_ll sll;
    struct ifreq ifr;
    char eth[80];

    bzero(&sll, sizeof(sll));
    bzero(&ifr, sizeof(ifr));

    printf("Enter name of eth device: ");
    do
    {
        rewind(stdin);
        gets(eth);
    } while (eth == NULL);
    strncpy((char *)ifr.ifr_name, eth, IFNAMSIZ);
    if ((ioctl(raw_socket, SIOCGIFINDEX, &ifr)) == -1)
    {
        printf("Error getting the interface index\n");
        exit(1);
    }

    sll.sll_family = AF_PACKET; // Address family
    sll.sll_ifindex = ifr.ifr_ifindex; // Interface index
    sll.sll_protocol = htons(ETH_P_IP); // Link layer protocol

    if ((bind(raw_socket, (struct sockaddr *)&sll, sizeof(sll))) == -1)
    {
        printf("Bind failed.\n");
        exit(1);
    }
}

// Retrieve the local hostname
struct hostent * get_host()
{
    struct hostent *local_host;
    char hostname[100];
    // Retrieve the local hostname
    if (gethostname(hostname, sizeof(hostname)) == -1)
    {
        printf("Get host name: SOCKET_ERROR\n");
        exit(1);
    }
    // Retrieve all local ips
    local_host = gethostbyname(hostname);
    if (local_host == NULL)
    {
        printf("Get host by name: NULL\n");
    }
    return local_host;
}
