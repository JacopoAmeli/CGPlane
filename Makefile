# Project: JacopoAmeliPlane

CPP  = g++
CC   = gcc
BIN  = JacopoAmeliPlane

OBJ  = main.o plane.o mesh.o Quaternion.o Environment.o HUD.o
LINKOBJ  = main.o plane.o mesh.o Quaternion.o Environment.o HUD.o

# Library linking
OS := $(shell uname)
ifeq ($(OS),Darwin)
## caso Mac OS
$(info Mac OS detected)
FRMPATH=-F /Library/Frameworks
LIBS =  -framework OpenGL -framework SDL2 -framework SDL2_image SDL2_ttf -lm
$(info SDL2 libraries must be in: $(FRMPATH))
else
ifeq ($(OS),MINGW32_NT-6.2)
## caso Windows MinGW
$(info Windows MinGW detected)
FRMPATH = -IC:\MinGW\include
LIBS = -LC:\MinGW\lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lopengl32 -lglu32 -lm -lstdc++
else
##caso Linux
$(info Linux detected)
#framework presenti solo nel caso MAC OS
FRMPATH =
LIBS = -lGL -lGLU -lSDL2 -lSDL2_image -lSDL2_ttf -lm
endif
endif

FLAG = -Wno-deprecated
RM = rm -f

all: $(BIN)

clean:
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CPP) $(LINKOBJ) -o $(BIN) $(FRMPATH) $(LIBS)

main.o: main.cpp
	$(CPP) -c $(FRMPATH) main.cpp -o main.o

plane.o: plane.cpp
	$(CPP) -c $(FRMPATH) plane.cpp -o plane.o

mesh.o: mesh.cpp
	$(CPP) -c $(FRMPATH) mesh.cpp -o mesh.o
	
Quaternion.o: Quaternion.cpp
	$(CPP) -c $(FRMPATH) Quaternion.cpp -o Quaternion.o
	
Environment.o: Environment.cpp
	$(CPP) -c $(FRMPATH) Environment.cpp -o Environment.o
	
HUD.o: HUD.cpp
	$(CPP) -c $(FRMPATH) HUD.cpp -o HUD.o
	