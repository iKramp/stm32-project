#ifndef SETTINGS_H
#define SETTINGS_H

#ifdef SERVER_BUILD
    #define SERVER true
#else
    #define SERVER false
#endif


#define IP {192, 168, 1, 100} //used by stm for checking if same ip for incoming packet
#define SERVER_IP "192.168.1.100" //used by linux client
#define SERVER_IP_BYTES {192, 168, 1, 50} //used by both clients
#define PORT 12345
#define SUBNET_MASK {255, 255, 255, 0}

#endif // SETTINGS_H
