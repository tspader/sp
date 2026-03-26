#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/kd.h>

int main(void) {
  struct winsize ws;
  if (ioctl(0, TIOCGWINSZ, &ws) == 0)
    puts("true");
  else
    puts("false");
  return 0;
}
