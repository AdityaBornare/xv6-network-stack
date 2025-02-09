struct stat;
struct rtcdate;

// system calls
int fork(void);
int exit(void) __attribute__((noreturn));
int wait(void);
int pipe(int*);
int write(int, const void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(char*, char**);
int open(const char*, int);
int mknod(const char*, short, short);
int unlink(const char*);
int fstat(int fd, struct stat*);
int link(const char*, const char*);
int mkdir(const char*);
int chdir(const char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);
void ifset(uint, uint, uint);
void ifconfig();
int socket(int);
int bind(int, uint, ushort);
int listen(int);
int connect(int sockfd, uint dst_addr, ushort dst_port);
int accept(int sockfd);
int test(uint);
int get_icmp_echo_reply_status(void);
int icmp_send_echo_request(uint dst_ip, ushort seq_no);
int get_icmp_echo_reply_packet(void);


// ulib.c
int stat(const char*, struct stat*);
char* strcpy(char*, const char*);
void *memmove(void*, const void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void printf(int, const char*, ...);
char* gets(char*, int max);
uint strlen(const char*);
void* memset(void*, int, uint);
void* malloc(uint);
void free(void*);
int atoi(const char*);

// unet.c
uint inet_addr(char*);

// types and enums
enum socket_type { TCP };
