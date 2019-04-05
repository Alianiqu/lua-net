#define LUA_LIB
#define _GNU_SOURCE

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <lauxlib.h>
#include <libgen.h>
#include <lua.h>
#include <net/if.h>
#include <net/route.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <unistd.h>


#if LUA_VERSION_NUM < 502
#define luaL_newlib(L,l) (lua_newtable(L), luaL_register(L,NULL,l))
#define luaL_setfuncs(L,l,n) (assert(n==0), luaL_register(L,NULL,l))
#define luaL_checkunsigned(L,n) luaL_checknumber(L,n)
#endif

#if LUA_VERSION_NUM >= 503
#ifndef luaL_checkunsigned
#define luaL_checkunsigned(L,n) ((lua_Unsigned)luaL_checkinteger(L,n))
#endif
#endif

#ifdef NO_CHECK_UDATA
#define checkudata(L,i,tname)	lua_touserdata(L, i)
#else
#define checkudata(L,i,tname)	luaL_checkudata(L, i, tname)
#endif

#define lua_boxpointer(L,u) \
    (*(void **) (lua_newuserdata(L, sizeof(void *))) = (u))

#define lua_unboxpointer(L,i,tname) \
    (*(void **) (checkudata(L, i, tname)))

/* Max Lua arguments for function */
#define MAXVARS	200


const static int _setip(const char *ip, const char *mask, const char *name) {

  struct ifreq ifr;
  struct sockaddr_in* addr = (struct sockaddr_in*) &ifr.ifr_addr;

  /* const char * name = "enp3s0"; */
  const int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

  printf("ip %s", ip);
  printf("mask %s", mask);
  printf("name %s", name);

  strncpy(ifr.ifr_name, name, IFNAMSIZ);

  ifr.ifr_addr.sa_family = AF_INET;

  /* inet_pton(AF_INET, "10.12.0.1", &addr->sin_addr); */
  inet_pton(AF_INET, ip, &addr->sin_addr);
  ioctl(fd, SIOCSIFADDR, &ifr);

  /* inet_pton(AF_INET, "255.255.0.0", &addr->sin_addr); */
  inet_pton(AF_INET, mask, &addr->sin_addr);
  ioctl(fd, SIOCSIFNETMASK, &ifr);

  ioctl(fd, SIOCGIFFLAGS, &ifr);
  strncpy(ifr.ifr_name, name, IFNAMSIZ);
  ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);

  ioctl(fd, SIOCSIFFLAGS, &ifr);
  close(fd);
  return 0;
}

const static int _setroute(const char * dst, const char * mask,
			   const char * gw, const char * dev,
			   const unsigned int metric) {
  struct rtentry rt;

  const int sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (sockfd == -1) {
    fprintf(stderr, "socket creation failed\n");
    return 1;
  }

  printf("dst %s mask %s gw %s dev %s\n", dst, mask, gw, dev);

  struct sockaddr_in *sockinfo = (struct sockaddr_in *) &rt.rt_gateway;
  sockinfo->sin_family = AF_INET;
  sockinfo->sin_addr.s_addr = inet_addr(gw);

  sockinfo = (struct sockaddr_in *) &rt.rt_dst;
  sockinfo->sin_family = AF_INET;
  /* sockinfo->sin_addr.s_addr = INADDR_ANY; */
  sockinfo->sin_addr.s_addr = inet_addr(dst);

  sockinfo = (struct sockaddr_in *) &rt.rt_genmask;
  sockinfo->sin_family = AF_INET;
  /* sockinfo->sin_addr.s_addr = INADDR_ANY; */
  sockinfo->sin_addr.s_addr = inet_addr(mask);

  rt.rt_flags = RTF_UP | RTF_GATEWAY;

  /* rt.rt_dev = "eth0"; */

  rt.rt_metric = metric;
  rt.rt_metric += 1;
  if((rt.rt_dev = malloc(strlen(dev) * sizeof(char *))) == 0) {
    fprintf(stderr, "Out of memory!\n%s\n", strerror(errno));
    return 1;
  }

  strncpy(rt.rt_dev, dev, IFNAMSIZ);

  if(ioctl(sockfd, SIOCADDRT, &rt) < 0 ) {
    fprintf(stderr, "set_route ioctl failed\n");
    return 1;
  }
  free(rt.rt_dev);
  close(sockfd);
  return 0;
}

const static int _ifup(const char * ifname, int state) {
  int r;
  struct ifreq ifr;
  const int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (fd < 0) {
    fprintf(stderr, "Create socket failed for ifup\n");
    return 1;
  }
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, ifname, IFNAMSIZ);

  if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
    fprintf(stderr, "_ifup get flags SIOCGIFFLAGS failed\n");
    return 1;
  }
  if( state == 0 )
  {
    ifr.ifr_flags = ifr.ifr_flags & ~(IFF_UP | IFF_RUNNING);
  }
  else
  {
    ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);
  }

  if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) {
    fprintf(stderr, "_ifup set flags SIOCSIFFLAGS failed\n");
    return 1;
  }
  return 0;
}

// Method implementation
const static int net_setip(lua_State * L) {

  const int argc = lua_gettop(L);
  printf("\nargc %d\n", argc);
  int code;

  if(argc < 3) {
    fprintf(stderr, "you must pass 3 arguments!\n");
    fprintf(stderr, "ip mask dev!\n");
    goto exit;
  }
  if (lua_isstring(L, 1) != 1) {
    fprintf(stderr, "second argument must be a string: ip!\n");
    goto exit;
  }
  if (lua_isstring(L, 2) != 1) {
    fprintf(stderr, "third argument must be a string: mask!\n");
    goto exit;
  }
  if (lua_isstring(L, 3) != 1) {
    fprintf(stderr, "forth argument must be a string: dev!\n");
    goto exit;
  }

  char *ip = strdupa(luaL_checkstring(L, 1));
  char *mask = strdupa(luaL_checkstring(L, 2));
  char *dev = strdupa(luaL_checkstring(L, 3));

  printf("setip %s %s %s\n", ip, mask, dev);
  code = _setip(ip, mask, dev);
  printf("code %d\n", code);

 exit:
  return 0;
}

// Method implementation
const static int net_setroute(lua_State * L) {
  int code;
  const int argc = lua_gettop(L);
  printf("\nargc %d\n", argc);
  if(argc < 5) {
    fprintf(stderr, "you must pass 5 arguments!\n");
    fprintf(stderr, "dst, mask, ip, dev, metric\n");
    goto exit;
  }
  if (lua_isstring(L, 1) != 1) {
    fprintf(stderr, "dst: !\n");
    goto exit;
  }
  if (lua_isstring(L, 2) != 1) {
    fprintf(stderr, "mask: !\n");
    goto exit;
  }
  if (lua_isstring(L, 3) != 1) {
    fprintf(stderr, "gw: !\n");
    goto exit;
  }
  if (lua_isstring(L, 4) != 1) {
    fprintf(stderr, "dev: !\n");
    goto exit;
  }
  if (luaL_checkinteger(L, 5) < 0) {
    fprintf(stderr, "is positive int metric: !\n");
    goto exit;
  }
  const char *dst = strdupa(luaL_checkstring(L, 1));
  const char *mask = strdupa(luaL_checkstring(L, 2));
  const char *ip = strdupa(luaL_checkstring(L, 3));
  const char *dev = strdupa(luaL_checkstring(L, 4));
  const unsigned int metric = luaL_checkint(L, 5);
  printf("setroute %s %s %s %s %u\n", dst, mask, ip, dev, metric);
  code = _setroute(dst, mask, ip, dev, metric);
  printf("code %d\n", code);

 exit:
  return 0;
}

const static int net_ifup(lua_State * L) {
  int code;
  const int argc = lua_gettop(L);
  printf("argc %d", argc);

  if (argc < 1) {
    fprintf(stderr, "you must pass one argument: ifname\n");
    goto exit;
  }

  if (lua_isstring(L, 1) != 1) {
    fprintf(stderr, "ifname is string\n");
    goto exit;
  }

  const char * ifname = strdupa(luaL_checkstring(L, 1));
  printf("ifup %s\n", ifname);
  code = _ifup(ifname,1);

 exit:
  return code;
}

const static int net_ifdown(lua_State * L) {
  int code;
  const int argc = lua_gettop(L);
  printf("argc %d", argc);

  if (argc < 1) {
    fprintf(stderr, "you must pass one argument: ifname\n");
    goto exit;
  }

  if (lua_isstring(L, 1) != 1) {
    fprintf(stderr, "ifname is string\n");
    goto exit;
  }

  const char * ifname = strdupa(luaL_checkstring(L, 1));
  printf("ifdown %s\n", ifname);
  code = _ifup(ifname,0);

 exit:
  return code;
}

// Register library using this array
static const struct luaL_Reg NetLib[] = {
    {"setip", net_setip},
    {"setroute", net_setroute},
    {"ifup", net_ifup},
    {"ifdown", net_ifdown},
    {NULL, NULL}
};

LUALIB_API int luaopen_net_core(lua_State *L) {
  luaL_newlib(L, NetLib);
  return 1;
}
