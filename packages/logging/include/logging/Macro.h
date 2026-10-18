#pragma once


#define CONCATE_(X,Y) X##Y
#define CONCATE_3(X,Y,Z) X##Y##Z
#define CONCATE_4(X,Y,Z,W) X##Y##Z##W
#define CONCATE(X,Y) CONCATE_(X,Y)
#define CONCATE3(X,Y,Z) CONCATE_3(X,Y,Z)
#define CONCATE4(X,Y,Z,W) CONCATE_4(X,Y,Z,W)

#define STRINGIFY(X) #X

#define UNIQUE(NAME) CONCATE3(NAME, __LINE__, __COUNTER__)

#define UNUSED(symbol) (void)(symbol)
