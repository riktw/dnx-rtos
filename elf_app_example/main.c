#include "stdio.h"
#include "string.h"
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(void) 
{
  printf("Hello world!\n");
  static const u8_t LED = 13;

  FILE *f = fopen("/dev/GPIOC", "r+");
    if (f)
    {
      for(int i = 0; i < 20; ++i)
      {
        ioctl(fileno(f), IOCTL_GPIO__TOGGLE_PIN, &LED);
        msleep(500);
      }
      fclose(f);
    } else {
      perror(NULL);
    }
  printf("Goodbye world!\n");
  return EXIT_SUCCESS;
}
