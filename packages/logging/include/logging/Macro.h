#pragma once


#define CONCATE_(X,Y) X##Y
#define CONCATE(X,Y) CONCATE_(X,Y)
#define CONCATE3(X,Y,Z) CONCATE_(X,CONCATE(Y, Z))
#define CONCATE4(X,Y,Z,W) CONCATE_(X,CONCATE3(Y, Z, W))

#define STRINGIFY(X) #X

#define UNIQUE(NAME) CONCATE(NAME, __LINE__)

#define UNUSED(symbol) (void)(symbol)
