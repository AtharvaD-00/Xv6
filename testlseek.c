// Tests the 'lseek()' system call


#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int
main(int argc, char *argv[])
{
  int off, fd;
  char data[5];

  printf(1, "testlseek starting\n\n");

  fd = open("README", O_RDONLY);

  lseek(fd, 138, SEEK_SET);
  read(fd, data, sizeof(data));
  if(strcmp("riscv", data) != 0){
    printf(1, "SEEK_SET failed\n");
    exit();
  }
  printf(1, "SEEK_SET ok\n");

  lseek(fd, 631, SEEK_CUR);
  read(fd, data, sizeof(data));
  if(strcmp("lapic", data) != 0){
    printf(1, "SEEK_CUR failed\n");
    exit();
  }
  printf(1, "SEEK_CUR ok\n");

  lseek(fd, -315, SEEK_END);
  read(fd, data, sizeof(data));
  if(strcmp("Linux", data) != 0){
    printf(1, "SEEK_END failed\n");
    exit();
  }
  printf(1, "SEEK_END ok\n");

  lseek(fd, 0, SEEK_SET);
  off = lseek(fd, -5, SEEK_SET);
  if(off >= 0){
    printf(1, "negative offset failed\n");
    exit();
  }
  read(fd, data, sizeof(data));
  if(strcmp("NOTE:", data) != 0){
    printf(1, "negative offset failed\n");
    exit();
  }
  printf(1, "negative offset ok\n");

  lseek(fd, 0, SEEK_SET);
  off = lseek(fd, 5, SEEK_END);
  if(off >= 0){
    printf(1, "offset beyond file size failed\n");
    exit();
  }
  read(fd, data, sizeof(data));
  if(strcmp("NOTE:", data) != 0){
    printf(1, "offset beyond file size failed\n");
    exit();
  }
  printf(1, "offset beyond file size ok\n");

  off = lseek(fd, 0, 3);
  if(off >= 0){
    printf(1, "invalid whence failed\n");
    exit();
  }
  printf(1, "invalid whence ok\n");

  printf(1, "\ntestlseek ok\n");

  close(fd);
  exit();
}
