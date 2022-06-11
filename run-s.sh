g++ -o ./bin/server.bin ./src/glad.c ./src/server.cc -I ./include/ -lglfw -lGLEW -lGL -lm -lXrandr -lXi -lX11 -lXxf86vm -lpthread -ldl -lXinerama -lXcursor -lzmq
./bin/server.bin