//Praphasiri Wannawong 6513116 
//Puttipong Yomabut 6513134
//Patiharn Kamenkit 6513170
//Phattaradanai Sornsawang 6513172

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define FILE_LENGTH 2*BUFSIZ


static void sig_end() { exit(EXIT_SUCCESS);}

struct map_st{
  int  print_1,previous_1,current_1;
  int  print_2,previous_2,current_2;  
  char data_1[BUFSIZ];
  char data_2[BUFSIZ];
};

int main(int argc, char *argv[]) {
  int fd;
  void* file_memory = NULL;
  struct map_st *map_area;
  char buffer[BUFSIZ] = "";
  int pid;

  if (argc < 2 || (strcmp(argv[1], "1") != 0 && strcmp(argv[1], "2") != 0) ||
      argc > 2) {
    fprintf(stderr, "Usage: %s [1 or 2]\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  
  fd = open ("chat_log", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if(fd < 0) {
    perror("open file failed\n");
    exit(EXIT_FAILURE);
  }
  
  lseek (fd, FILE_LENGTH+1, SEEK_SET);
  write (fd, "", 1);
  lseek (fd, 0, SEEK_SET);

  file_memory = mmap (0, FILE_LENGTH,PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
  
  if(file_memory == MAP_FAILED)
  {
    fprintf(stderr, "mmap failed\n");
    exit(EXIT_FAILURE);
  }
  map_area = (struct map_st*)file_memory;
  
  memset(map_area, 0, FILE_LENGTH);

  map_area -> print_1=0;
  map_area -> print_2=0;
  map_area -> current_1 = 0;
  map_area -> current_2 = 0;
  map_area -> previous_1 = 0;
  map_area -> previous_2 = 0;

  close (fd);

  signal(SIGUSR1, sig_end);
  argv++;

  if (strcmp(*argv, "1") == 0) {
    pid = fork();
    switch (pid) {
    case -1:
      perror("Forking failed");
      exit(EXIT_FAILURE);
      case 0 : while(strncmp(buffer,"end chat",8)){                  
        if(map_area -> print_2){
          memset(buffer, 0, BUFSIZ);
          write(2,"User 2:", 7);               
          int j=0;
          for(int i = map_area -> previous_2 ; i < map_area -> current_2 ; i++,j++){
              buffer[j] = map_area -> data_2[i];
            }  
          write(1,buffer,j);                 
            map_area -> print_2 = 0;    
        }                  
      }
      break;
      default : while(strncmp(buffer, "end chat", 8)){                
        memset(buffer, 0, BUFSIZ);  
        int word = read(0, buffer, BUFSIZ);
        if(word > 1){ 
          map_area -> previous_1  = map_area -> current_1;
          map_area -> current_1 += word;
          strcat(map_area->data_1, buffer);             
          map_area -> print_1 = 1;                 
        }
      }
    }
  } else if (strcmp(*argv, "2") == 0) {
    pid = fork();
    switch (pid) {
    case -1:
      perror("Forking failed");
      exit(EXIT_FAILURE);
    case 0 : while(strncmp(buffer,"end chat",8)){                  
      if(map_area -> print_1){
        memset(buffer, 0, BUFSIZ);
        write(2,"User 1:", 7);               
        int j=0;
        for(int i = map_area -> previous_1 ; i < map_area -> current_1 ; i++,j++){
            buffer[j] = map_area -> data_1[i];
          }  
        write(1,buffer,j);                 
          map_area -> print_1 = 0;    
      }                  
    }
    break;
    default : while(strncmp(buffer, "end chat", 8)){                
      memset(buffer, 0, BUFSIZ);  
      int word = read(0, buffer, BUFSIZ);
      if(word > 1){ 
        map_area -> previous_2  = map_area -> current_2;
        map_area -> current_2 += word;
        strcat(map_area->data_2, buffer);             
        map_area -> print_2 = 1;                 
      }
    }
    }
  }

  if (pid > 0)
    kill(pid, SIGUSR1);

  else if (pid == 0)
    kill(getppid(), SIGUSR1);

  munmap (file_memory, FILE_LENGTH);
  exit(EXIT_SUCCESS);
  
  return 0;
}
