//YANISA SUPHATSATHIENKUL 6213196 EGCO
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define FILE_LENGTH 2*(BUFSIZ+12)

struct mm_st
{
  int  written_1,pre_1,cur_1;
  int  written_2,pre_2,cur_2;  
  char data_1[BUFSIZ];
  char data_2[BUFSIZ];
};

static void sig_end()
{  
  wait(NULL);
  exit(EXIT_SUCCESS);
} 

int main(int argc, char* argv[]) 
{
  int  child,fd;
  void *file_memory = NULL;  
  char buffer[BUFSIZ] = "";

  if(argc < 2)
  {
    fprintf(stderr,"--------------------------------\n");
    fprintf(stderr,"  [    INVALID ARGUMENT    ]\n");
    fprintf(stderr,"  [  USAGE : %s <[1 , 2]>  ]\n",argv[0]);
    fprintf(stderr,"--------------------------------\n");
    exit(EXIT_FAILURE);
  }

  else if(argc > 2)  
  {
    fprintf(stderr,"--------------------------------\n");
    fprintf(stderr,"  [    OVER ARGUMENT   ]\n");
    fprintf(stderr,"  [  USAGE : %s <[1 , 2]>  ]\n",argv[0]);
    fprintf(stderr,"--------------------------------\n");
    exit(EXIT_FAILURE);
  }

  if(strcmp(argv[1],"1")!=0 && strcmp(argv[1],"2")!=0)
  {
    fprintf(stderr,"--------------------------------\n");
    fprintf(stderr,"  [     INPUT MISMATCH     ]\n");
    fprintf(stderr,"  [  USAGE : %s <[1 , 2]>  ]\n",argv[0]);
    fprintf(stderr,"--------------------------------\n");
    exit(EXIT_FAILURE);
  }

  fd = open ("Chat log", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

  if(fd < 0)
  {
    perror("open file failed\n");
    exit(EXIT_FAILURE);
  }

  lseek (fd, FILE_LENGTH+1, SEEK_SET);
  write (fd, "", 1);
  lseek (fd, 0, SEEK_SET);   

  file_memory = mmap(0, FILE_LENGTH, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

  if(file_memory == MAP_FAILED)
  {
    fprintf(stderr, "mmap failed\n");
    exit(EXIT_FAILURE);
  }

  struct mm_st* mm_area;
  mm_area = (struct mm_st*)file_memory;

  memset(mm_area, 0, FILE_LENGTH);

  mm_area -> written_1=0;
  mm_area -> written_2=0;
  mm_area -> cur_1 = 0;
  mm_area -> cur_2 = 0;
  mm_area -> pre_1 = 0;
  mm_area -> pre_2 = 0;

  close(fd);

  argv++;
  signal(SIGUSR1,sig_end);

  if(strcmp(*argv, "1") == 0)
  {
    child = fork();
    switch(child)
    {
      case -1: perror("Forking failed\n");exit(EXIT_FAILURE);
      case 0 : while(strncmp(buffer,"end chat",8)){                  
                if(mm_area -> written_1){
                  memset(buffer, 0, BUFSIZ);
                  write(2,"User 2:", 7);               
                  int j=0;
                  for(int i = mm_area -> pre_1 ; i < mm_area -> cur_1 ; i++,j++){
                        buffer[j] = mm_area -> data_2[i];
                    }  
                  write(1,buffer,j);                 
                  mm_area -> written_1 = 0;    
                }                  
              }
              break;
      default : while(strncmp(buffer, "end chat", 8)){                
                memset(buffer, 0, BUFSIZ);  
                int nbytes = read(0, buffer, BUFSIZ);
                if(nbytes > 1){ 
                  mm_area -> pre_1  = mm_area -> cur_1;
                  mm_area -> cur_1 += nbytes;
                  strcat(mm_area->data_1, buffer);             
                  mm_area -> written_1 = 1;                 
                }
              }
    }
  }

  else if (strcmp(*argv, "2") == 0)
  {
    child = fork();
    switch(child)
    {
      case -1: perror("Forking failed\n");exit(EXIT_FAILURE);
      case 0 : while(strncmp(buffer,"end chat",8)){    
                if(mm_area -> written_1){
                  memset(buffer, 0, BUFSIZ);
                  write(2,"User 1:", 7);
                  int j=0;
                  for(int i = mm_area -> pre_1 ; i < mm_area -> cur_1 ; i++,j++){
                        buffer[j]=mm_area->data_1[i];   
                    }
                  write(1,buffer,j);                          
                  mm_area -> written_1 = 0; 
                }                      
              }break;
      default : while(strncmp(buffer, "end chat", 8)){
                memset(buffer, 0, BUFSIZ);  
                int nbytes = read(0, buffer, BUFSIZ);   
                if(nbytes > 1){    
                  mm_area->pre_1  = mm_area->cur_1;
                  mm_area->cur_1 += nbytes;              
                  strcat(mm_area -> data_1, buffer);                  
                  mm_area -> written_1 = 1; 
                }
              }
    }
  }

  if(child > 0){
    kill(child,SIGKILL);
    wait(NULL);
  }
  else if(child == 0){
    kill(getppid(),SIGUSR1);
  }

  munmap (file_memory, FILE_LENGTH);
  exit(EXIT_SUCCESS);

}