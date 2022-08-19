PLATFORM = $(shell uname)
ifeq ($(PLATFORM), Darwin)
CC = clang++
FLAGS = -std=c++14 -stdlib=libc++
#CC = g++-5.1.0
#FLAGS = -Wall -std=c++14 -pedantic
endif
ifeq ($(PLATFORM), Linux)
CC = g++
FLAGS = -std=c++14 `pkg-config --cflags --libs opencv4`
endif

LIBS = -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_objdetect -lopencv_features2d -lopencv_imgcodecs -lopencv_videoio
SRC = src/mathable_main.cpp
EXEC = mathable

CFLAGS= -I${includedir}/opencv -I${includedir}/opencv2 -I${includedir}/opencv4


# Create the executable
all:
	$(CC) $(FLAGS) $(SRC) -o $(EXEC) $(LIBS)

clean:
	rm -f $(EXEC)
